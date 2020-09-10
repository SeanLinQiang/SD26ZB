/******************** (C) COPYRIGHT 2020 INNOTECH **************************
* COMPANY:			INNOTECH
* DATE:					2020/05
* AUTHOR:				Sean
* IC:						SN32F700B
* DESCRIPTION:	User related include.
*____________________________________________________________________________
* REVISION	Date				User		Description
* 1.0				2020/05/7	  Sean			First release
*
*____________________________________________________________________________

*****************************************************************************/

#ifndef __USERAPP_H
#define __USERAPP_H

/* Includes ------------------------------------------------------------------*/
#include <SN32F700B.h>

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
enum{
	STEP0,
	STEP1,
	STEP2,
	STEP3,
	STEP4,
	STEP5,
};

enum{
	KEY_SHORT_DOWN,
	KEY_SHORT_UP,
	KEY_LONG_DOWN,
	KEY_LONG_UP,
	KEY_ADD_SHORT_DOWN,
	KEY_ADD_SHORT_UP,
	KEY_ADD_LONG_DOWN,
	KEY_ADD_LONG_5S_DOWN,
	KEY_ADD_LONG_UP,
	KEY_DEC_SHORT_DOWN,
	KEY_DEC_SHORT_UP,
	KEY_DEC_LONG_DOWN,
	KEY_DEC_LONG_5S_DOWN,
	KEY_DEC_LONG_UP,
	KEY_NULL
};

typedef struct {
	uint8_t flag;                                                                 //存储标志(是否调过亮度)
	uint8_t led_type;                                                             //led类型
  uint8_t led_switch;                                                           //开关
	uint8_t brightness_en;                                                        //调亮度使能 
	uint32_t led_angle;                                                           //导通角度
	uint32_t led_angle_min;                                                       //最低导通角                                               
  uint32_t bright_value;                                                        //亮度值
	uint32_t brightness_min;                                                      //最低亮度 
}DEVICE_PARAM;

/* Exported macro ------------------------------------------------------------*/
#define ZERO           0

#define ON             1
#define OFF            0


//串口数据
#define FRAME_HEAD_1            0xAA       

#define UART_MODE               0x00
#define UART_SWITCH             0x9A
#define UART_BRIGHT_VALUE       0x9B
#define UART_BRIGHT_VALUE_MIN   0x9C
#define UART_BRIGHT_KEY_PRESS   0x9D

//指示灯数据
#define LED1_ON          GPIO_Clr(GPIO_PORT1, GPIO_PIN4)
#define LED2_ON          GPIO_Clr(GPIO_PORT1, GPIO_PIN3)
#define LED3_ON          GPIO_Clr(GPIO_PORT1, GPIO_PIN2) 
#define LED4_ON          GPIO_Clr(GPIO_PORT1, GPIO_PIN1)
#define LED5_ON          GPIO_Clr(GPIO_PORT0, GPIO_PIN8)

#define LED1_OFF         GPIO_Set(GPIO_PORT1, GPIO_PIN4)
#define LED2_OFF         GPIO_Set(GPIO_PORT1, GPIO_PIN3)
#define LED3_OFF         GPIO_Set(GPIO_PORT1, GPIO_PIN2) 
#define LED4_OFF         GPIO_Set(GPIO_PORT1, GPIO_PIN1)
#define LED5_OFF         GPIO_Set(GPIO_PORT0, GPIO_PIN8)

//按键数据
#define KEY_POWER        0x01  	
#define KEY_DIM_UP       0x02  
#define KEY_DIM_DOWN     0x04  

#define KEY_TIMEZERO     50
#define KEY_TIMESHORT    1000
#define KEY_TIMELONG_2S  2000
#define KEY_TIMELONG_5S  5000

#define MIN_BR           40
#define MAX_BR				   140

#define MODE_NORMAL      0     //正常工作模式
#define MODE_BRIGHT_SET  1     //最低亮度设置模式

//Assign ISP Address
#define	ISP_TARGET_ADDR	 0x3C00//0x1FE0				//Word-alignment

#define	USER_KEY_ADD		 0x3FFC
#define BACK_KEY_ADD     0x7FFC

// key parameter
extern uint16_t KeyPowerTime;
extern uint16_t KeyDimUpTime;
extern uint16_t KeyDimDownTime;

extern DEVICE_PARAM Device_Param;
extern uint8_t WorkMode;   //0:正常工作模式 1:最低亮度设置模式
extern uint32_t light_angle;
extern uint32_t current_light_angle;
extern uint8_t  AngleBackup;     //保存设置最低亮度时的当前亮度值
extern uint32_t AngleMinBackup;  //保存设置最低亮度时的当前最低亮度值
extern uint32_t BrightnessBackup;
extern uint16_t SetTimeout;      //设置最低亮度模式超时

extern uint8_t SetFinishFlag;
extern uint8_t Fre_Cnt_Flag;

extern uint8_t NormalSwitch;
extern uint8_t backup_value;

extern uint16_t TimeoutMs;
extern uint8_t IsSwitch; 

/* Exported functions ------------------------------------------------------- */
void ZCD_Process(void);
void Key_Process(void);
void Fade_Process(void);
void SetTimeout_Process(void);
void Led_Process(void);
void Dimmer_Save_Parameter(void);
static void System_Factory(void);
void Factory_Test_Mode(void);

extern void UartB_ReceiveData(uint8_t RcvData);
void Connect_With_Switch(void);
extern void UartB_Process(void);
extern void UartB_SendData(uint8_t cmd, uint8_t sync_data);

#endif


