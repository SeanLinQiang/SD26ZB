/******************** (C) COPYRIGHT 2020 INNOTECH **************************
* COMPANY:			INNOTECH
* DATE:					2020/05
* AUTHOR:				Sean
* IC:						SN32F700B
* DESCRIPTION:	User related functions.
*____________________________________________________________________________
* REVISION	Date				User		Description
* 1.0				2020/05/7	  Sean		First release
*
*____________________________________________________________________________

*****************************************************************************/
#include <SN32F700B.h>
#include "GPIO.h"
#include "CT16B0.h"
#include "CT16B1.h"
#include "CT16B2.h"
#include "SysTick.h"
#include "UART.h"
#include "Flash.h"
#include "PMU_drive.h"
#include "Utility.h"
#include "WDT.h"
#include "CT16.h"
#include "userApp.h"
#include "mcu_api.h"
#include "zigbee.h"

DEVICE_PARAM Device_Param;

uint8_t StatusChangeFlag = FALSE;

//按键处理数据
uint8_t  KeyValue = 0;
uint16_t KeyPowerTime = 0;      //按键power时间计数
uint16_t KeyDimUpTime = 0;
uint16_t KeyDimDownTime = 0;

//串口处理数据
uint8_t UartRxStep = STEP0;        //串口接收步骤
uint8_t UartRxLen = 0;             //串口接收长度
uint8_t UartRxBuf[10];             //串口接收缓存
uint8_t ReceiveFinishFlag = FALSE; //串口接收完成标志
uint8_t UartRecvData1 = 0;
uint8_t UartRecvData2 = 0;
uint16_t TimeoutMs;
uint8_t IsSwitch;
uint8_t NormalSwitch = TRUE;

//最低亮度设置
uint8_t KeyUpClick = 0;           //dim+长按单击标志
uint8_t KeyDownClick = 0;         //dim-长按单击标志
uint8_t WorkMode = MODE_NORMAL;   //0:正常工作模式 1:最低亮度设置模式
uint8_t SetFinishFlag = FALSE;    //设置完成标志 
uint8_t AngleBackup = MIN_BR;     //保存设置最低亮度时的当前亮度值
uint32_t AngleMinBackup = MIN_BR; //保存设置最低亮度时的当前最低亮度值
uint32_t BrightnessBackup = 10;   //保存设置最低亮度时的当前最低亮度百分比 %1
uint16_t SetTimeout;              //设置最低亮度模式超时

//零点调光数据
static uint16_t PowerFrequency = 24490;
static uint16_t PwmDelay=MIN_BR;

uint32_t light_angle = MIN_BR;
uint32_t current_light_angle = MIN_BR;

//长按渐变处理数据
static uint8_t DimFadeAddFlag = FALSE;    // 无级调光加 渐变标志
static uint8_t DimFadeDecFlag = FALSE;    // 无级调光减 渐变标志
static uint8_t DimFadeAddCount = 0;
static uint8_t DimFadeDecCount = 0;
static uint8_t DimReportCount = 0;
static uint8_t DimReportLimitCount = 0;    //防止%1和%100多次上报
 
//计算零点周期的数据 //预留
uint8_t Fre_Cnt_Flag = FALSE;
uint8_t Int_Step = 1;
static uint8_t frq_tab[6];
static uint8_t frq_Cnt = 0;
static uint8_t Fre_Cnt = 0;

extern volatile uint8_t FadeProcessFlag;
extern volatile uint8_t SetTimeoutFlag;

extern volatile uint8_t FactoryTestFlag;
extern volatile uint8_t FactoryTestStep;
extern volatile uint8_t FactoryStepFlag;
extern volatile uint16_t FactoryTickCount;
uint8_t FactoryFinishFlag = FALSE;


uint8_t zigbee_reset_flag = 0;

/**********************************  类实现 **********************************/
/*****************************************************************************
函数名称 : FilterData_Process
功能描述 : 过滤数据处理
输入参数 : 无
返回参数 : 无
*****************************************************************************/
uint16_t FilterData_Process(void)
{
	uint8_t i,j,tmp;
	uint16_t sum=0;
	
	for(i=0; i<6-1; i++){
		for(j=0; j<6-1-i; j++){
			if(frq_tab[j]>frq_tab[j+1]){
				tmp = frq_tab[j+1];
				frq_tab[j+1] = frq_tab[j];
				frq_tab[j] = tmp;
			}
		}
	}
	
	for(i=1; i<5; i++){
		sum += frq_tab[i];
	}
	sum >>=2;
	
	return sum;
}

/*****************************************************************************
函数名称 : ZCD_Process
功能描述 : 零点中断处理
输入参数 : 无
返回参数 : 无
*****************************************************************************/
void ZCD_Process(void)
{	
		if(Fre_Cnt_Flag == TRUE)
		{
			if(Fre_Cnt++ < 7)
			{
				if(Int_Step == 1)
				{
					Timer1_Value = 0;
					Int_Step = 2;
				}else if(Int_Step == 2)
				{
					Int_Step = 2;
					frq_tab[frq_Cnt++] = Timer1_Value;
					Timer1_Value = 0;
				}
			}else if(Fre_Cnt > 7)
			{
				Fre_Cnt = 0;
				frq_Cnt = 0;
				Fre_Cnt_Flag = FALSE;
				PowerFrequency = FilterData_Process();
				PowerFrequency = PowerFrequency*50*3;
			}
		}
	  //WDT_ReloadValue(250);
		//调光数据
		if(Device_Param.brightness_en == FALSE)
		{
			if(light_angle <= MIN_BR)
			{
				SN_CT16B2->PWMCTRL = mskCT16_PWM2EN_EN|mskCT16_PWM2MODE_FORCE_0|mskCT16_PWM2N_GPIO;
				SN_GPIO0->DATA_b.DATA3 = 0;
				if(StatusChangeFlag == TRUE)
				{
					StatusChangeFlag = FALSE;
					Dimmer_Save_Parameter();
				}
				return;
			}
		}
	  
		PwmDelay = (180-light_angle)*PowerFrequency/180;

		SN_CT16B2->MCTRL = (mskCT16_MR9IE_EN|mskCT16_MR9STOP_EN);
		SN_CT16B2->PWMCTRL = mskCT16_PWM2EN_EN|mskCT16_PWM2MODE_2|mskCT16_PWM2N_MODE1;	//Enable PWM3N function, IO and select as PWM mode 2	
		SN_CT16B2->MR2 = PwmDelay;
		SN_CT16B2->MR9 = PwmDelay+2500;		
		SN_CT16B2->TC = 0;
		SN_CT16B2->TMRCTRL_b.CEN = 1;
		if(StatusChangeFlag == TRUE)
		{
			StatusChangeFlag = FALSE;
			Dimmer_Save_Parameter();
		}
}


/*****************************************************************************
函数名称 : UartB_ReceiveData
功能描述 : 串口B接收数据
输入参数 : data : 接收的数据
返回参数 : 无
*****************************************************************************/
void UartB_ReceiveData(uint8_t RcvData)
{
	switch(UartRxStep)
	{
		case STEP0:
		{
			if(RcvData == FRAME_HEAD_1)
			{
				UartRxBuf[UartRxLen++] = RcvData;
				UartRxStep = STEP1;
				IsSwitch = FALSE;
				TimeoutMs = 0;
				NormalSwitch = FALSE;
			}
//			else
//			{
//					UartRxLen = 0;
//					UartRxStep = STEP0;
//			}
			break;
		}
		case STEP1:
		{
			UartRxBuf[UartRxLen++] = RcvData;
			if(UartRxLen > 2)
			{
				ReceiveFinishFlag = TRUE;
				UartRxStep = STEP0;
				UartRxLen = 0;
			}
		}
		break;
		default :
			break;
	}
}

/*****************************************************************************
函数名称 : Connect_With_Switch
功能描述 : 串口B开关互连处理
输入参数 : 无
返回参数 : 无
*****************************************************************************/
void Connect_With_Switch(void)
{
	uint8_t gpio_value = 0;
	
	//if(IsSwitch == TRUE && NormalSwitch == TRUE)
	{
		gpio_value = SN_GPIO0->DATA_b.DATA4;
		if(backup_value != gpio_value) 
		{
			  backup_value = gpio_value;
				IsSwitch = FALSE;
				TimeoutMs = 0;
				if(Device_Param.led_switch == OFF)
				{
					Device_Param.led_switch = ON;
					Device_Param.brightness_en = TRUE;
					current_light_angle = Device_Param.led_angle;
					mcu_dp_bool_update(DPID_SWITCH_LED_1, ON);
					//mcu_dp_value_update(DPID_BRIGHT_VALUE_1, Device_Param.bright_value);
					StatusChangeFlag = TRUE;
				}
				else if(Device_Param.led_switch == ON)
				{
					Device_Param.led_switch = OFF;
					Device_Param.brightness_en = FALSE;
					//Device_Param.led_angle = light_angle;
					current_light_angle = Device_Param.led_angle;
					mcu_dp_bool_update(DPID_SWITCH_LED_1, OFF);
					StatusChangeFlag = TRUE;
				}
		}
	}
}

/*****************************************************************************
函数名称 : UartB_Write_Data
功能描述 : 串口1发送数据
输入参数 : data:发送的字符
返回参数 : 无
*****************************************************************************/
void UartB_Write_Data(uint8_t *uartData, uint8_t len)
{
	if((NULL == uartData) || (0 == len))
	{
		return ;
	}
	while(len--)
	{
		//发送
		UART1_SendByte(*uartData); 
		uartData++;
	}
	SN_UART1->CTRL =(UART_EN										//Enable UART0
									|	UART_RX_EN									//Enable RX
									| UART_TX_EN);								//Enable TX
	//UT_DelayNms(10);
  GPIO_Clr(GPIO_PORT0, GPIO_PIN5);
}
uint8_t TxBuf[3];
/*****************************************************************************
函数名称 : UartB_SendData
功能描述 : 串口B发送数据
输入参数 : cmd : 命令 data : 发送的同步数据
返回参数 : 无
*****************************************************************************/
void UartB_SendData(uint8_t cmd, uint8_t sync_data)
{
	
	
	//发送时关闭串口接收中断
	SN_UART1->CTRL &= 0xBF;
	GPIO_Set (GPIO_PORT0, GPIO_PIN5);
	UT_DelayNms(2);

	TxBuf[0] = FRAME_HEAD_1;
	TxBuf[1] = cmd;
	TxBuf[2] = sync_data;
	
	UartB_Write_Data((uint8_t *)TxBuf, 3);
}

/**************************************************************************** 
 * 函数名称 : UartB_Process
 * 功能描述 : 处理串口数据
 * 参    数 : 无
 * 参    数 : 无
 * 返 回 值 : 无
 *****************************************************************************/
void UartB_Process(void)
{
	uint32_t bright_value = 0;
	
	if(ReceiveFinishFlag == FALSE)
		return;
	
	ReceiveFinishFlag = FALSE;
	
	UartRecvData1 = UartRxBuf[1];
  UartRecvData2 = UartRxBuf[2];
	
	switch(UartRecvData1)
	{
		//开关同步
		case UART_SWITCH:
		{
			Device_Param.brightness_en = FALSE;
		  Device_Param.led_switch = OFF;
			Device_Param.led_angle = current_light_angle;
			current_light_angle = MIN_BR;
			
		  mcu_dp_bool_update(DPID_SWITCH_LED_1, OFF);
      StatusChangeFlag = TRUE;
			break;
		}
		//亮度同步
		case UART_BRIGHT_VALUE:
		{
			Device_Param.led_switch = ON;
			Device_Param.brightness_en = TRUE;
			Device_Param.led_angle = UartRecvData2;
		  //mcu_dp_bool_update(DPID_LED_SWITCH_1, ON);
      current_light_angle = Device_Param.led_angle;
			
			if(Device_Param.led_angle == Device_Param.led_angle_min)
			{
				Device_Param.bright_value = 10;
			}
			else
			{
				Device_Param.bright_value = (current_light_angle - MIN_BR)*10;
			}
      mcu_dp_bool_update(DPID_SWITCH_LED_1, ON);
		  mcu_dp_value_update(DPID_BRIGHT_VALUE_1, Device_Param.bright_value);
			break;
		}
		//最低亮度同步
		case UART_BRIGHT_VALUE_MIN:
		{
			Device_Param.led_angle_min = UartRxBuf[2];
			mcu_dp_value_update(DPID_BRIGHTNESS_MIN_1, Device_Param.led_angle_min);
			break;
		}
		//按键亮度同步
		case UART_BRIGHT_KEY_PRESS:
		{	
			Device_Param.led_switch = ON;
			Device_Param.brightness_en = TRUE;

			Device_Param.led_angle = (MAX_BR - Device_Param.led_angle_min)*UartRecvData2/100 + Device_Param.led_angle_min;
			current_light_angle = Device_Param.led_angle; 
			bright_value = UartRecvData2*10;
			
			Device_Param.bright_value = bright_value;
		  mcu_dp_value_update(DPID_BRIGHT_VALUE_1, bright_value);
			break;
		}
		default:
			break;
	}
}

/*****************************************************************************
函数名称 : Key_Scan
功能描述 : 按键扫描
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static uint8_t Key_Scan(void)
{
	uint8_t Temp=0;
	
	if((SN_GPIO1->DATA_b.DATA0 ) == 0) Temp|=0x01;  // P1.0 Power
	if((SN_GPIO2->DATA_b.DATA6 ) == 0) Temp|=0x02;  // P2.6 DIM+
	if((SN_GPIO2->DATA_b.DATA7 ) == 0) Temp|=0x04;  // P2.7 DIM-
	
	return Temp;
}

/*****************************************************************************
函数名称 : Key_Check
功能描述 : 按键检测
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static uint8_t Key_Check(void)
{
	static uint8_t KeyPower = FALSE;
	static uint8_t KeyDimUp = FALSE;
	static uint8_t KeyDimDown = FALSE;
	
	static uint8_t KeyPowerVal = KEY_NULL;
	static uint8_t KeyDimUpVal = KEY_NULL;
	static uint8_t KeyDimDownVal = KEY_NULL;

	uint8_t KeyTempVal = KEY_NULL;
	
	KeyValue = Key_Scan();
	
	//key_power  check process*******************************************// 
	if(KeyValue&KEY_POWER){
		if(!KeyPower){
			KeyPower = TRUE;
			KeyPowerTime = ZERO;
		}
	}else{
		KeyPower = FALSE;
	}
	
	if(KeyPower){
		if((KeyPowerTime > KEY_TIMEZERO) && (KeyPowerTime <= KEY_TIMESHORT) && (KeyPowerVal == KEY_NULL)){
			KeyPowerVal = KEY_SHORT_DOWN;
			KeyTempVal = KeyPowerVal;
		}else if(KeyPowerTime > KEY_TIMELONG_5S && KeyPowerVal == KEY_SHORT_DOWN){
			KeyPowerVal = KEY_LONG_DOWN;
			KeyTempVal = KeyPowerVal;
		}
	}else{
		switch(KeyPowerVal){
			case KEY_SHORT_DOWN:
				KeyPowerVal = KEY_SHORT_UP;
				KeyTempVal = KeyPowerVal;
				KeyPowerVal = KEY_NULL;
				break;
				
			case KEY_LONG_DOWN:
				KeyPowerVal = KEY_LONG_UP;
				KeyTempVal = KeyPowerVal;
				KeyPowerVal = KEY_NULL;
				break;
		}
	}
	
	if(Device_Param.brightness_en  == TRUE)
	{
		//key_up  check process **********************************************// 
		if(KeyValue&KEY_DIM_UP){
			if(!KeyDimUp){
				KeyDimUp = TRUE;
				KeyDimUpTime = ZERO;
			}
		}else{
			KeyDimUp = FALSE;
		}
		
		if(KeyDimUp){
			if((KeyDimUpTime > KEY_TIMEZERO) && (KeyDimUpTime < KEY_TIMESHORT) && (KeyDimUpVal == KEY_NULL)){
				KeyDimUpVal = KEY_ADD_SHORT_DOWN;
				KeyTempVal = KeyDimUpVal;
				//KeyUpClick = 0;
			}else if(KeyDimUpTime > 1000 && KeyDimUpTime < KEY_TIMELONG_2S){
				if((KeyValue&KEY_DIM_DOWN) != KEY_DIM_DOWN)
				{
					KeyUpClick = 1;
				}else if((KeyValue&KEY_DIM_DOWN) == KEY_DIM_DOWN){
					//按键组合
					KeyUpClick = 2;
				}
			}else if((KeyDimUpTime > KEY_TIMELONG_2S) && (KeyUpClick == 1)){
				KeyDimUpVal = KEY_ADD_LONG_DOWN;
				KeyTempVal = KeyDimUpVal;
			}else if((KeyDimUpTime > KEY_TIMELONG_5S) && (KeyUpClick == 2)){
				KeyDimUpVal = KEY_ADD_LONG_5S_DOWN;
				KeyTempVal = KeyDimUpVal;
				KeyUpClick = 3;
			}
			
		}else{
			switch(KeyDimUpVal){
				case KEY_ADD_SHORT_DOWN:
					KeyDimUpVal = KEY_ADD_SHORT_UP;
					KeyTempVal = KeyDimUpVal;
					KeyDimUpVal = KEY_NULL;
					break;
				case KEY_ADD_LONG_DOWN:
					KeyDimUpVal = KEY_ADD_LONG_UP;
					KeyTempVal = KeyDimUpVal;
					KeyDimUpVal = KEY_NULL;
					break;
				case KEY_ADD_LONG_5S_DOWN:
					//KeyDimUpVal = KEY_ADD_LONG_UP;
					KeyTempVal = KeyDimUpVal;
					KeyDimUpVal = KEY_NULL;
					break;
			}
		}
	}

    if(Device_Param.brightness_en  == TRUE)
	{
		//key_dim_down  check process **********************************************// 
		if(KeyValue&KEY_DIM_DOWN){
			if(!KeyDimDown){
				KeyDimDown = TRUE;
				KeyDimDownTime = ZERO;
			}
		}else{
			KeyDimDown = FALSE;
		}
		
		if(KeyDimDown){
			if((KeyDimDownTime > KEY_TIMEZERO) && (KeyDimDownTime < KEY_TIMESHORT) && (KeyDimDownVal == KEY_NULL)){ 
				KeyDimDownVal = KEY_DEC_SHORT_DOWN;
				KeyTempVal = KeyDimDownVal;
				//KeyDownClick = 0;
			}else if(KeyDimDownTime > 1000 && KeyDimDownTime < KEY_TIMELONG_2S){
				if((KeyValue&KEY_DIM_UP) != KEY_DIM_UP)
				{
					KeyDownClick = 1;
				}else if((KeyValue&KEY_DIM_UP) == KEY_DIM_UP){
					//按键组合
					KeyDownClick = 2;
				}
			}else if((KeyDimDownTime > KEY_TIMELONG_2S) && (KeyDownClick == 1)){
				KeyDimDownVal = KEY_DEC_LONG_DOWN;
				KeyTempVal = KeyDimDownVal;
			}else if((KeyDimDownTime > KEY_TIMELONG_5S) && (KeyDownClick == 2)){
				KeyDimDownVal = KEY_DEC_LONG_5S_DOWN;
				KeyTempVal = KeyDimUpVal;
				KeyDownClick = 3;
			}
			
		}else{
			switch(KeyDimDownVal){
				case KEY_DEC_SHORT_DOWN:
					KeyDimDownVal = KEY_DEC_SHORT_UP;
					KeyTempVal = KeyDimDownVal;
					KeyDimDownVal = KEY_NULL;
					break;
				
				case KEY_DEC_LONG_DOWN:
					KeyDimDownVal = KEY_DEC_LONG_UP;
					KeyTempVal = KeyDimDownVal;
					KeyDimDownVal = KEY_NULL;
					break;
				case KEY_DEC_LONG_5S_DOWN:
					KeyTempVal = KeyDimDownVal;
					KeyDimDownVal = KEY_NULL;
					break;		
			}
		}
	}
	return KeyTempVal;
}

/*****************************************************************************
函数名称 : Key_Power_ShortPress_Handle
功能描述 : power按键短按处理
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void Key_Power_ShortPress_Handle(void)
{
	if(Device_Param.led_switch == OFF) 
	{
		Device_Param.led_switch = ON;
		Device_Param.brightness_en = TRUE;
		if(WorkMode == MODE_BRIGHT_SET)
		{
			Device_Param.led_angle = AngleBackup;
			Device_Param.led_angle_min = AngleMinBackup;
			Device_Param.bright_value = BrightnessBackup;
			current_light_angle = Device_Param.led_angle;
    }
		else
		{
			current_light_angle = Device_Param.led_angle;
		}
		UartB_SendData(UART_BRIGHT_VALUE, current_light_angle); //发送串口数据
		mcu_dp_bool_update(DPID_SWITCH_LED_1, Device_Param.led_switch);		
	}
	else
	{
		Device_Param.led_switch = OFF;
		Device_Param.brightness_en = FALSE;
		UartB_SendData(UART_SWITCH, 0x00); //发送串口数据 
		mcu_dp_bool_update(DPID_SWITCH_LED_1, Device_Param.led_switch);
		
		if(WorkMode == MODE_BRIGHT_SET)
		{
			Device_Param.led_angle = AngleBackup;
			Device_Param.led_angle_min = AngleMinBackup;
			Device_Param.bright_value = BrightnessBackup;
			current_light_angle = Device_Param.led_angle;
    }
		else
		{
			Device_Param.led_angle = current_light_angle;
			current_light_angle = MIN_BR;
		}
	}
}

/*****************************************************************************
函数名称 : Key_Power_LongPress_Handle
功能描述 : power按键长按处理
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void Key_Power_LongPress_Handle(void)
{
	//复位zigbee
	mcu_network_start();
	zigbee_reset_flag = 1;
	zigbee_work_state = 0;
	
	// 恢复出厂设置
	System_Factory();
}

/*****************************************************************************
函数名称 : Key_Add_ShortPress_Handle
功能描述 : dimmer+按键短按处理
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void Key_Add_ShortPress_Handle(void)
{
	int32_t bright_value = 0;
	
	if(Device_Param.brightness_en == TRUE)
	{
		(WorkMode == MODE_NORMAL) ? (SetFinishFlag = FALSE):(SetFinishFlag = TRUE);
		//正常工作模式
		if(WorkMode == MODE_NORMAL)
		{
			//当前导通角度值
			bright_value = Device_Param.bright_value;
			if((bright_value >= 0) && (bright_value < 50))
			{
				bright_value = 50;
			}else if((bright_value >= 50) && (bright_value < 200))
			{
				bright_value = 200;
			}else if((bright_value >= 200) && (bright_value < 350))
			{
				bright_value = 350;
			}else if((bright_value >= 350) && (bright_value < 500))
			{
				bright_value = 500;
			}else if((bright_value >= 500) && (bright_value < 650))
			{
				bright_value = 650;
			}else if((bright_value >= 650) && (bright_value < 800))
			{
				bright_value = 800;
			}else if((bright_value >= 800) && (bright_value <= 1000))
			{
				bright_value = 1000;
			}
			
			current_light_angle = bright_value/10+MIN_BR;
			
			Device_Param.led_switch = ON;	
			Device_Param.led_angle =  current_light_angle;
			Device_Param.bright_value = bright_value;
			UartB_SendData(UART_BRIGHT_VALUE, current_light_angle);
      //UartB_SendData(UART_BRIGHT_KEY_PRESS, bright_value/10); //发送串口数据 
			mcu_dp_value_update(DPID_BRIGHT_VALUE_1, Device_Param.bright_value);
	  }
		else if(WorkMode == MODE_BRIGHT_SET)
		{
			if(current_light_angle < MAX_BR)
			{
				current_light_angle += 1;
			}
			else
			{
				// 最大亮度
				current_light_angle = MAX_BR;
			}

			Device_Param.led_angle =  current_light_angle;
			Device_Param.bright_value = (current_light_angle - MIN_BR)*10;
			mcu_dp_value_update(DPID_BRIGHT_VALUE_1, Device_Param.bright_value);	
      UartB_SendData(UART_BRIGHT_VALUE, Device_Param.led_angle); //发送串口数据			
		}
	}
}

/*****************************************************************************
函数名称 : Key_Add_LongPress_Down_Handle
功能描述 : dimmer+按键长按处理
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void Key_Add_LongPress_Down_Handle(void)
{
	(WorkMode == MODE_NORMAL) ? (SetFinishFlag = FALSE):(SetFinishFlag = TRUE);
	DimFadeAddFlag = TRUE;
	Device_Param.led_switch = ON;
	if(DimReportCount % 4 == 0)
	{
		DimReportCount = 0;
		Device_Param.led_angle =  current_light_angle;
		if(Device_Param.led_angle == Device_Param.led_angle_min)
		{
			Device_Param.bright_value = 10;
		}
		else
		{
			Device_Param.bright_value = (current_light_angle-MIN_BR)*10;
		}
		if(Device_Param.bright_value >= 1000)
		{
			Device_Param.bright_value = 1000;
		}

		if(Device_Param.led_angle == MAX_BR)
		{
			DimReportLimitCount++;
			if(DimReportLimitCount >= 2)
			{
				return;
			}
		}
		mcu_dp_value_update(DPID_BRIGHT_VALUE_1, Device_Param.bright_value);	 
    UartB_SendData(UART_BRIGHT_VALUE, Device_Param.led_angle); //发送串口数据		
	}
}

/*****************************************************************************
函数名称 : Key_Add_LongPress_Up_Handle
功能描述 : dimmer+按键长按处理
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void Key_Add_LongPress_Up_Handle(void)
{
	DimFadeAddFlag = FALSE;
	DimReportLimitCount = 0;
	KeyUpClick = 0;
}

/*****************************************************************************
函数名称 : Key_Dec_ShortPress_Handle
功能描述 : dimmer-按键短按处理
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void Key_Dec_ShortPress_Handle(void)
{
	int32_t bright_value = 0;
	
	if(Device_Param.brightness_en == TRUE)
	{
		(WorkMode == MODE_NORMAL) ? (SetFinishFlag = FALSE):(SetFinishFlag = TRUE);
		if(WorkMode == MODE_NORMAL)//正常工作模式
		{
			  Device_Param.led_angle = current_light_angle;
			
				//当前导通角度值
				bright_value = (current_light_angle - MIN_BR)*10;
			
				if((bright_value >= 0) && (bright_value <= 50))
				{
					bright_value = 10;
				}else if((bright_value > 50) && (bright_value <= 200))
				{
					bright_value = 50;
				}else if((bright_value > 200) && (bright_value <= 350))
				{
					bright_value = 200;
				}else if((bright_value > 350) && (bright_value <= 500))
				{
					bright_value = 350;
				}else if((bright_value > 500) && (bright_value <= 650))
				{
					bright_value = 500;
				}else if((bright_value > 650) && (bright_value <= 800))
				{
					bright_value = 650;
				}else if((bright_value > 800) && (bright_value <= 1000))
				{
					bright_value = 800;
				}
				current_light_angle = bright_value/10+MIN_BR;
				
				Device_Param.led_switch = ON;
				Device_Param.led_angle =  current_light_angle;			
				Device_Param.bright_value = bright_value;
				if(Device_Param.led_angle <= Device_Param.led_angle_min)
				{
					Device_Param.led_angle =  Device_Param.led_angle_min;	
					current_light_angle = Device_Param.led_angle;					
					Device_Param.bright_value = (Device_Param.led_angle - MIN_BR)*10;
				}
				UartB_SendData(UART_BRIGHT_VALUE, current_light_angle);
				//UartB_SendData(UART_BRIGHT_KEY_PRESS, bright_value/10); //发送串口数据
				mcu_dp_value_update(DPID_BRIGHT_VALUE_1, Device_Param.bright_value);			 
		}
		else if(WorkMode == MODE_BRIGHT_SET)
		{		
			if(current_light_angle > MIN_BR)
			{
					current_light_angle -= 1;
			}
			else
			{
				// 最低亮度
				current_light_angle = MIN_BR;
			}

			Device_Param.led_angle = current_light_angle;
		  
			if(Device_Param.led_angle == Device_Param.led_angle_min)
			{
				Device_Param.bright_value = 10;
			} 
			else
			{
				Device_Param.bright_value = (current_light_angle-MIN_BR)*10;
			}
			
			mcu_dp_value_update(DPID_BRIGHT_VALUE_1, Device_Param.bright_value);
      UartB_SendData(UART_BRIGHT_VALUE, Device_Param.led_angle); //发送串口数据			
		}
	}
}

/*****************************************************************************
函数名称 : Key_Dec_LongPress_Down_Handle
功能描述 : dimmer-按键长按处理
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void Key_Dec_LongPress_Down_Handle(void)
{
	(WorkMode == MODE_NORMAL) ? (SetFinishFlag = FALSE):(SetFinishFlag = TRUE);
	DimFadeDecFlag = TRUE;
	
	Device_Param.led_switch = ON;
	if(DimReportCount % 4 == 0)
	{
		DimReportCount = 0;
		Device_Param.led_angle =  current_light_angle;
		Device_Param.bright_value = (current_light_angle-MIN_BR)*10;
		
		if(Device_Param.led_angle == Device_Param.led_angle_min)
		{
			DimReportLimitCount++;
			if(DimReportLimitCount >= 2)
				return;
		}
		mcu_dp_value_update(DPID_BRIGHT_VALUE_1, Device_Param.bright_value);	
    UartB_SendData(UART_BRIGHT_VALUE, Device_Param.led_angle); //发送串口数据 		
	}
}

/*****************************************************************************
函数名称 : Key_Dec_LongPress_Up_Handle
功能描述 : dimmer-按键长按处理
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void Key_Dec_LongPress_Up_Handle(void)
{
	DimFadeDecFlag = FALSE;
	DimReportLimitCount = 0;
	KeyDownClick = 0;
}

/*****************************************************************************
函数名称 : Key_Process
功能描述 : 按键处理
输入参数 : 无
返回参数 : 无
*****************************************************************************/
void Key_Process(void)
{
	uint8_t Key_Value = KEY_NULL;

	Key_Value = Key_Check();

	switch(Key_Value)
	{
		// 电源键短按抬起
		case KEY_SHORT_UP:
		{
				Key_Power_ShortPress_Handle();	
				StatusChangeFlag = TRUE;
				WorkMode = MODE_NORMAL;
				SetFinishFlag = FALSE;
				break;
		}
		// 电源键长按未抬起
		case KEY_LONG_DOWN:
		{
				Key_Power_LongPress_Handle();
				WorkMode = MODE_NORMAL;
				SetFinishFlag = FALSE;
				break;
		}
		// dim+ 短按抬起
		case KEY_ADD_SHORT_UP:
		{
				Key_Add_ShortPress_Handle();
				StatusChangeFlag = TRUE;
				break;
		}
		// dim+ 长按未抬起
		case KEY_ADD_LONG_DOWN:
		{
				Key_Add_LongPress_Down_Handle();
				break;
		}
		case KEY_ADD_LONG_UP:
		{
				Key_Add_LongPress_Up_Handle();
				StatusChangeFlag = TRUE;
				break;
		}
		// dim- 短按抬起
		case KEY_DEC_SHORT_UP:
		{
				Key_Dec_ShortPress_Handle();
				StatusChangeFlag = TRUE;
				break;
		}
		// dim- 长按未抬起
		case KEY_DEC_LONG_DOWN:
		{
				Key_Dec_LongPress_Down_Handle();
				break;
		}
		// dim- 长按抬起
		case KEY_DEC_LONG_UP:
		{
				Key_Dec_LongPress_Up_Handle();
				StatusChangeFlag = TRUE;
				break;
		}
		default:
				break;
	}

	if((KeyUpClick == 3) && (KeyDownClick == 3))
	{
		//进入最低亮度设置模式
		KeyUpClick = 0;
		KeyDownClick = 0;
		KeyDimUpTime = 0;
		KeyPowerTime = 0;
		KeyDimDownTime = 0;

		if(SetFinishFlag == FALSE)
		{
			WorkMode = MODE_BRIGHT_SET;
			SetTimeout = 0;
			SetTimeoutFlag = FALSE;
			//保存当前值
			AngleBackup = current_light_angle;
			AngleMinBackup = Device_Param.led_angle_min;
			BrightnessBackup = Device_Param.bright_value;
			
			Device_Param.bright_value = (Device_Param.led_angle_min - MIN_BR)*10;
			current_light_angle = Device_Param.led_angle_min;
			if(Device_Param.bright_value == 0)
				Device_Param.bright_value = 10;
			
			mcu_dp_value_update(DPID_BRIGHT_VALUE_1, Device_Param.bright_value);
			UartB_SendData(UART_BRIGHT_VALUE, current_light_angle); //发送串口数据 
		}
		else
		{
			//设置完成
			WorkMode = MODE_NORMAL;
			Device_Param.led_angle = current_light_angle;
			Device_Param.led_angle_min = Device_Param.led_angle;
			Device_Param.bright_value = (Device_Param.led_angle_min - MIN_BR)*10;
			if(Device_Param.bright_value == 0)
				Device_Param.bright_value = 10;
			
			mcu_dp_value_update(DPID_BRIGHT_VALUE_1, Device_Param.bright_value);
			mcu_dp_value_update(DPID_BRIGHTNESS_MIN_1, Device_Param.bright_value);
			UartB_SendData(UART_BRIGHT_VALUE_MIN, Device_Param.led_angle_min); //设置完，同步最低亮度
		}
	}
}

/**************************************************************************** 
 * 函数名称 : SetTimeout_Process
 * 功能描述 : 处理设置最低亮度超时事件
 * 参    数 : 无
 * 参    数 : 无
 * 返 回 值 : 无
 *****************************************************************************/
void SetTimeout_Process(void)
{
	if(WorkMode == MODE_BRIGHT_SET)
	{
		if(SetTimeoutFlag == TRUE)
		{
			SetTimeoutFlag = FALSE;
			WorkMode = MODE_NORMAL;
			SetFinishFlag = FALSE;
			Device_Param.led_angle = AngleBackup;
			Device_Param.led_angle_min = AngleMinBackup;
			Device_Param.bright_value = BrightnessBackup;
			current_light_angle = Device_Param.led_angle;

			mcu_dp_value_update(DPID_BRIGHT_VALUE_1, Device_Param.bright_value);	
      UartB_SendData(UART_BRIGHT_VALUE, Device_Param.led_angle);  
			UartB_SendData(UART_BRIGHT_VALUE_MIN, Device_Param.led_angle_min);  		  
		}
	}
}

/**************************************************************************** 
 * 函数名称 : Fade_Process
 * 功能描述 : 处理渐变数据
 * 参    数 : 无
 * 参    数 : 无
 * 返 回 值 : 无
 *****************************************************************************/
void Fade_Process(void)
{
	//无级调光渐变
	if(FadeProcessFlag == TRUE)
	{
		FadeProcessFlag = FALSE;
		if(DimFadeAddFlag == TRUE)
		{
			DimFadeAddCount++;
			DimReportCount++;
			//if(DimFadeAddCount % 4 == 0)
			{
				DimFadeAddCount = 0;
				if(current_light_angle < MAX_BR)
				{
					current_light_angle++;
					Device_Param.led_angle = current_light_angle;
					current_light_angle = Device_Param.led_angle;
				}
			}
		}
		
		if(DimFadeDecFlag == TRUE)
		{
			DimFadeDecCount++;
			DimReportCount++;
			//if(DimFadeDecCount % 4 == 0)
			{
				DimFadeDecCount = 0;
				if(current_light_angle > Device_Param.led_angle_min)
				{
					current_light_angle--;
					Device_Param.led_angle = current_light_angle;
					current_light_angle = Device_Param.led_angle;
				}
			}
		}
		// 等级调光渐变
		if(light_angle < current_light_angle)
		{
			light_angle++;
		}
		else if(light_angle > current_light_angle)
		{
			light_angle--;
		}
	}
}

/**************************************************************************** 
 * 函数名称 : Led_Process
 * 功能描述 : 指示灯处理
 * 参    数 : 无
 * 参    数 : 无
 * 返 回 值 : 无
 *****************************************************************************/
void Led_Process(void)
{
	int32_t bright_value = 0;	
	
	if(zigbee_reset_flag == 0)
	{
		SN_GPIO0->DATA_b.DATA9 ^= 0x0;
	}
	if(Device_Param.led_switch == OFF)
	{
		current_light_angle = MIN_BR;
	}
	bright_value = (light_angle - MIN_BR)*10 ;
	if(bright_value == 0)
	{
		bright_value = 10;
	}
	if(bright_value == 10 && Device_Param.led_switch == OFF)
	{
		LED1_OFF;
		LED2_OFF;
		LED3_OFF;
		LED4_OFF;
		LED5_OFF;
	}
	else if(bright_value >= 0 && bright_value < 200)
	{
		if(Device_Param.led_switch == ON)
		{
			LED1_ON;
			LED2_OFF;
			LED3_OFF;
			LED4_OFF;
			LED5_OFF;
		}
		else if(Device_Param.led_switch == OFF)
		{
			current_light_angle = MIN_BR;
			LED1_ON;
			LED2_OFF;
			LED3_OFF;
			LED4_OFF;
			LED5_OFF;
		}
	}
	else if(bright_value >= 200 && bright_value < 350)
	{
		LED1_ON;
		LED2_OFF;
		LED3_OFF;
		LED4_OFF;
		LED5_OFF;
	}
	else if(bright_value >= 350 && bright_value < 500)
	{
		LED1_ON;
		LED2_ON;
		LED3_OFF;
		LED4_OFF;
		LED5_OFF;
	}else if(bright_value >= 500 && bright_value < 650)
	{
		LED1_ON;
		LED2_ON;
		LED3_ON;
		LED4_OFF;
		LED5_OFF;
	}
	else if(bright_value >= 650 && bright_value <= 800)
	{
		LED1_ON;
		LED2_ON;
		LED3_ON;
		LED4_ON;
		LED5_OFF;
	}
	else if(bright_value > 800 && bright_value <= 1000)
	{
		LED1_ON;
		LED2_ON;
		LED3_ON;
		LED4_ON;
		LED5_ON;
	}
}

/**************************************************************************** 
 * 函数名称 : Dimmer_Save_Parameter
 * 功能描述 : 保存数据
 * 参    数 : 无
 * 参    数 : 无
 * 返 回 值 : 无
 *****************************************************************************/
void Dimmer_Save_Parameter(void)
{
	Device_Param.flag = 0xAA;
	
	if (FLASH_ProgramPage(ISP_TARGET_ADDR, sizeof(Device_Param), (uint8_t *)(&Device_Param)) == FAIL){
				while(1);			//Program Fail
	}
}

/*****************************************************************************
函数名称 : System_Factory
功能描述 : 恢复出厂设置
输入参数 : 无
返回参数 : 无
*****************************************************************************/
static void System_Factory(void)
{
	Device_Param.flag = 0xAA;
	Device_Param.led_type = 0x01;
	Device_Param.led_angle = MAX_BR;
	Device_Param.led_angle_min = MIN_BR;
	Device_Param.bright_value = 1000;
	Device_Param.brightness_min = 10;
	current_light_angle = Device_Param.led_angle;
	
	if (FLASH_ProgramPage(ISP_TARGET_ADDR, sizeof(Device_Param), (uint8_t *)(&Device_Param)) == FAIL){
				while(1);			//Program Fail
	}
}

/*****************************************************************************
函数名称 : Factory_Test_Mode
功能描述 : 厂测模式
输入参数 : 无
返回参数 : 无
*****************************************************************************/
void Factory_Test_Mode(void)
{
	if(((SN_GPIO2->DATA_b.DATA6 )) == 0 )
	{
		FactoryTestFlag = TRUE;
		FactoryStepFlag = TRUE;
		FactoryTickCount = 0;
	}	
	
	while(1)
	{
		if(FactoryTestFlag == TRUE)
		{
			if(FactoryStepFlag == TRUE && FactoryFinishFlag == FALSE)
			{
				FactoryStepFlag = FALSE;
				FactoryTestStep++;
				switch(FactoryTestStep)
				{
					case 0x01:
						SN_GPIO0->DATA_b.DATA9 = 0x1;
						current_light_angle = MIN_BR;
						Device_Param.brightness_en = TRUE;
						LED1_ON;
						LED2_OFF;
						LED3_OFF;
						LED4_OFF;
						LED5_OFF;
						break;
					case 0x02:
						mcu_network_start();
						current_light_angle = MAX_BR;
						LED1_ON;
						LED2_ON;
						LED3_ON;
						LED4_ON;
						LED5_ON;
					
						break;
					case 0x03:
						//mcu_start_wifitest();
						current_light_angle = MIN_BR;
						Device_Param.brightness_en = FALSE;
						LED1_OFF;
						LED2_OFF;
						LED3_OFF;
						LED4_OFF;
						LED5_OFF;
					  UT_DelayNms(1000);
						break;
					case 0x04:
						
						FactoryFinishFlag = TRUE;
						FactoryTestStep = 0;
						break;
					default:
						break;
				}
				zigbee_uart_service();
			}
			Fade_Process();
		}
		else 
		{
			return;
		}
	}
}
