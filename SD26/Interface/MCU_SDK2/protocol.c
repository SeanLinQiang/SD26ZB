/**********************************Copyright (c)**********************************
**                     All rights reserved (C), 2015-2020, Tuya
**
**                             http://www.tuya.com
**
*********************************************************************************/
/**
 * @file    protocol.c
 * @author  Tuya Team
 * @version v2.5.4
 * @date    2020.4.15
 * @brief                *******Very important, be sure to watch!!!********
 *          1. The user implements the data delivery/reporting function in this file.
 *          2. DP ID / TYPE and data processing functions require the user to implement according to the actual definition
 *          3. There are #err hints inside the function that needs the user to implement the code after starting some macro definitions. 
 *             Please delete the #err after completing the function.
 */

/******************************************************************************
                                Transplant instructions:
1:The MCU must directly call the wifi_uart_service() function in mcu_api.c in the while.
2:After the normal initialization of the program is completed, 
  it is recommended not to turn off the serial port interrupt. 
  If the interrupt must be turned off, the off interrupt time must be short, 
  and the interrupt will cause the serial port packet to be lost.
3:Do not call the escalation function in the interrupt/timer interrupt
******************************************************************************/

#include "wifi.h"
#include <SN32F700B.h>
#include "userApp.h"
#include "flash.h"
#include "Utility.h"
#include "GPIO.h"

extern uint8_t IsUpdate;
extern uint8_t StatusChangeFlag;
uint8_t Write_Flash_FIFO[256] __attribute__((aligned (4)));

#ifdef WEATHER_ENABLE
/**
 * @var    weather_choose
 * @brief  Weather data parameter selection array
 * @note   Users can customize the required parameters, comments or uncomment,
 *         pay attention to the changes
 */
const char *weather_choose[WEATHER_CHOOSE_CNT] = {
    "temp",
    "humidity",
    "condition",
    "pm25",
    /*"pressure",
    "realFeel",
    "uvi",
    "tips",
    "windDir",
    "windLevel",
    "windSpeed",
    "sunRise",
    "sunSet",
    "aqi",
    "so2 ",
    "rank",
    "pm10",
    "o3",
    "no2",
    "co",*/
 };
#endif


/******************************************************************************
                              The first step: initialization
1:Include "wifi.h" in files that need to use wifi related files
2:Call the wifi_protocol_init() function in the mcu_api.c file in the MCU initialization
3:Fill the MCU serial single-byte send function into the uart_transmit_output 
   function in the protocol.c file, and delete #error
4:Call the uart_receive_input function in the mcu_api.c file in the MCU serial receive 
   function and pass the received byte as a parameter.
5:The wifi_uart_service() function in the mcu_api.c file is called after the MCU enters the while loop.
******************************************************************************/

/******************************************************************************
                        1:dp data point sequence type comparison table
          **This is the automatic generation of code, such as the relevant changes in
              the development platform, please re-download MCU_SDK**
******************************************************************************/
const DOWNLOAD_CMD_S download_cmd[] =
{
  {DPID_SWITCH_LED_1, DP_TYPE_BOOL},
  {DPID_BRIGHT_VALUE_1, DP_TYPE_VALUE},
  {DPID_BRIGHTNESS_MIN_1, DP_TYPE_VALUE},
  {DPID_LED_TYPE_1, DP_TYPE_ENUM},
};

/******************************************************************************
                        2:Serial single-byte send function
Please fill in the MCU serial port send function into the function,
and pass the received data as a parameter to the serial port send function.
******************************************************************************/


/**
 * @brief  Send data processing
 * @param[in] {value} Serial port receives byte data
 * @return Null
 */
void uart_transmit_output(unsigned char value)
{
  //#error "Please fill in the MCU serial port send function and delete the line"

/*
  //Example:
  extern void Uart_PutChar(unsigned char value);
  Uart_PutChar(value);	                                //Serial port send function
*/
	extern void UART0_SendByte(unsigned char value);
	UART0_SendByte(value);                                  //串口发送函数
}
/******************************************************************************
                           Step 2: Implement a specific user function
1:APP send data processing
2:Data upload processing
******************************************************************************/

/******************************************************************************
                            1:All data upload processing
The current function handles all data upload (including deliverable/reportable and report only)

  Users need to implement according to the actual situation:
  1:Need to implement the reportable/reportable data point report
  2:Need to report only reported data points
This function must be called internally by the MCU.
Users can also call this function to achieve all data upload.
******************************************************************************/

//Automated generation of data reporting functions

/**
 * @brief  All dp point information of the system is uploaded to realize APP and muc data synchronization
 * @param  Null
 * @return Null
 * @note   This function SDK needs to be called internally;
 *         The MCU must implement the data upload function in the function;
 *         including only reporting and reportable hair style data.
 */
void all_data_update(void)
{
  //#error "Please process the reportable data and report only the data. After the processing is completed, delete the line"
  /* 
  //This code is automatically generated by the platform. 
  //Please modify each reportable and reportable function according to the actual data.
  mcu_dp_bool_update(DPID_SWITCH_LED_1,当前开关1); //BOOL型数据上报;
  mcu_dp_value_update(DPID_BRIGHT_VALUE_1,当前亮度值1); //VALUE型数据上报;
  mcu_dp_value_update(DPID_BRIGHTNESS_MIN_1,当前最小亮度1); //VALUE型数据上报;
  mcu_dp_enum_update(DPID_LED_TYPE_1,当前光源类型1); //枚举型数据上报;

 */
	mcu_dp_bool_update(DPID_SWITCH_LED_1,Device_Param.led_switch); //BOOL型数据上报;
  mcu_dp_value_update(DPID_BRIGHT_VALUE_1,Device_Param.bright_value); //VALUE型数据上报;
  mcu_dp_value_update(DPID_BRIGHTNESS_MIN_1,Device_Param.brightness_min); //VALUE型数据上报;
  mcu_dp_enum_update(DPID_LED_TYPE_1,Device_Param.led_type); //枚举型数据上报;
}


/******************************************************************************
                                WARNING!!!    
                            2:All data upload processing
Automate code template functions, please implement data processing by yourself
******************************************************************************/
/*****************************************************************************
函数名称 : dp_download_switch_led_1_handle
功能描述 : 针对DPID_SWITCH_LED_1的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static unsigned char dp_download_switch_led_1_handle(const unsigned char value[], unsigned short length)
{
  //示例:当前DP类型为BOOL
  unsigned char ret;
  //0:关/1:开
  unsigned char switch_led_1;
  
  switch_led_1 = mcu_get_dp_download_bool(value,length);
	
  if(switch_led_1 == 0)
  {
    //开关关
		Device_Param.brightness_en = FALSE;
		Device_Param.led_switch = OFF;
		
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
  else
  {
    //开关开
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
			Device_Param.led_angle = Device_Param.bright_value/10 + MIN_BR;
			current_light_angle = Device_Param.led_angle;
		}
  }
  StatusChangeFlag = TRUE;
	WorkMode = MODE_NORMAL;
	
  Device_Param.led_switch = switch_led_1;
  //处理完DP数据后应有反馈
  ret = mcu_dp_bool_update(DPID_SWITCH_LED_1,switch_led_1);
  if(ret == SUCCESS)
    return SUCCESS;
  else
    return ERROR;
}
/*****************************************************************************
函数名称 : dp_download_bright_value_1_handle
功能描述 : 针对DPID_BRIGHT_VALUE_1的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static unsigned char dp_download_bright_value_1_handle(const unsigned char value[], unsigned short length)
{
  //示例:当前DP类型为VALUE
  unsigned char ret;
  unsigned long bright_value_1;
  
  bright_value_1 = mcu_get_dp_download_value(value,length);
  /*
  //VALUE类型数据处理
  
  */
	if((bright_value_1 % 10) >= 5)
		Device_Param.bright_value = bright_value_1/10 +1;
	else
		Device_Param.bright_value = bright_value_1/10;

	Device_Param.led_angle = Device_Param.bright_value +MIN_BR;
	//Device_Param.led_angle = (unsigned char)(Device_Param.bright_value/10+MIN_BR);  // 40°导通角为%0  140°导通角为100%
	Device_Param.bright_value = Device_Param.bright_value *10;
	current_light_angle = Device_Param.led_angle; 
	StatusChangeFlag = TRUE;
  Device_Param.led_switch = ON;
	Device_Param.brightness_en = TRUE;
	
	//上报开的状态
	mcu_dp_bool_update(DPID_SWITCH_LED_1,1);
	
  //处理完DP数据后应有反馈
  ret = mcu_dp_value_update(DPID_BRIGHT_VALUE_1,bright_value_1);
  if(ret == SUCCESS)
    return SUCCESS;
  else
    return ERROR;
}
/*****************************************************************************
函数名称 : dp_download_brightness_min_1_handle
功能描述 : 针对DPID_BRIGHTNESS_MIN_1的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static unsigned char dp_download_brightness_min_1_handle(const unsigned char value[], unsigned short length)
{
  //示例:当前DP类型为VALUE
  unsigned char ret;
  unsigned long brightness_min_1;
  
  brightness_min_1 = mcu_get_dp_download_value(value,length);
  /*
  //VALUE类型数据处理
  
  */
	//(workMode == MODE_NORMAL) ? (SetFinishFlag = FALSE):(SetFinishFlag = TRUE);
	
	Device_Param.brightness_min = brightness_min_1;
	if(brightness_min_1 == 10)
		Device_Param.led_angle_min = MIN_BR;
	else
		Device_Param.led_angle_min = Device_Param.brightness_min/10+MIN_BR;  //最低亮度导通角为%1  140°导通角为100%
	Device_Param.led_angle = Device_Param.led_angle_min;
	
	if(Device_Param.led_switch == ON)
	{
		current_light_angle = Device_Param.led_angle;
	}
	else if(Device_Param.led_switch == OFF)
	{
		Device_Param.led_angle = current_light_angle;
		current_light_angle = MIN_BR;
	}
	
  //处理完DP数据后应有反馈
  ret = mcu_dp_value_update(DPID_BRIGHTNESS_MIN_1,brightness_min_1);
  if(ret == SUCCESS)
    return SUCCESS;
  else
    return ERROR;
}
/*****************************************************************************
函数名称 : dp_download_led_type_1_handle
功能描述 : 针对DPID_LED_TYPE_1的处理函数
输入参数 : value:数据源数据
        : length:数据长度
返回参数 : 成功返回:SUCCESS/失败返回:ERROR
使用说明 : 可下发可上报类型,需要在处理完数据后上报处理结果至app
*****************************************************************************/
static unsigned char dp_download_led_type_1_handle(const unsigned char value[], unsigned short length)
{
  //示例:当前DP类型为ENUM
  unsigned char ret;
  unsigned char led_type_1;
  
  led_type_1 = mcu_get_dp_download_enum(value,length);
  switch(led_type_1)
  {
    case 0:
      
      break;
      
    case 1:
      
      break;
      
    case 2:
      
      break;
      
    default:
      
      break;
  }
  Device_Param.led_type = led_type_1;
	
	Dimmer_Save_Parameter();
  //处理完DP数据后应有反馈
  ret = mcu_dp_enum_update(DPID_LED_TYPE_1,led_type_1);
  if(ret == SUCCESS)
    return SUCCESS;
  else
    return ERROR;
}


/******************************************************************************
                                WARNING!!!                     
This code is called internally by the SDK. 
Please implement the internal data of the function according to the actual dp data.
******************************************************************************/
#ifdef SUPPORT_MCU_FIRM_UPDATE
/**
 * @brief  Upgrade package size selection
 * @param[in] {package_sz} Upgrade package size
 * @ref           0x00: 256byte (default)
 * @ref           0x01: 512byte
 * @ref           0x02: 1024byte
 * @return Null
 * @note   MCU needs to implement this function by itself
 */
void upgrade_package_choose(unsigned char package_sz)
{
  //#error "Please implement the upgrade package size selection processing code by yourself. Please delete this line after completion"
  unsigned short length = 0;
  length = set_wifi_uart_byte(length,package_sz);
  wifi_uart_write_frame(UPDATE_START_CMD,MCU_SEND_VER,length);

	IsUpdate = TRUE;
	if(Device_Param.led_switch == ON)
	{
		GPIO_Set(GPIO_PORT0, GPIO_PIN3);
		LED1_ON;
		LED2_ON;
		LED3_ON;
		LED4_ON;
		LED5_ON;
		LED6_ON;
	}
	else
	{
		GPIO_Clr(GPIO_PORT0, GPIO_PIN3);
		LED1_OFF;
		LED2_OFF;
		LED3_OFF;
		LED4_OFF;
		LED5_OFF;
		LED6_OFF;
	}
}

/*****************************************************************************
* Function		: Goto_Bootloader
* Description	: 					
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
__asm void Goto_Bootloader(void)
{
        LDR    r0, =0x1FFF0301            //** bootloader address = 0x1FFF0300, under Thumb mode usage LSB=1
        BX     r0
}

/**
 * @brief  MCU enters firmware upgrade mode
 * @param[in] {value} Firmware buffer
 * @param[in] {position} The current data packet is in the firmware location
 * @param[in] {length} Current firmware package length (when the firmware package length is 0, it indicates that the firmware package is sent)
 * @return Null
 * @note   MCU needs to implement this function by itself
 */
unsigned char mcu_firm_update_handle(const unsigned char value[],unsigned long position,unsigned short length)
{
	
	for(int len = 0;len < 256;len++)
	{
		Write_Flash_FIFO[len] = value[len];
	}
  //#error "Please complete the MCU firmware upgrade processing code yourself. Please delete the line after completion"
  if(length == 0)
  {
		uint32_t flash_user_flag = 0;

		//写完成标志
		FLASH_ProgramPage(USER_KEY_ADD,4,(uint8_t *)&flash_user_flag);
		UT_DelayNms(200);
    //Firmware data transmission completed
//		NVIC->ICER[0] = 0xFFFFFFFF ;										// Disable All interrupt.
//		SN_WDT->CFG = 0x5AFA0000; 											// Disable WDT
//		SysTick->CTRL = 0;                              // Disable Systick

		*(volatile unsigned long *)(0x40060024)=0xA5A50000;				//Set Map to boot
		FLASH_ProgramPage(USER_KEY_ADD,4,(uint8_t *)&flash_user_flag);

		//Goto_Bootloader();
		*(volatile unsigned long*)(0x40060024) =0xa5a50000;	//map boot			//into boot update user
		NVIC_SystemReset();
  }
  else
  {
    //Firmware data processing
		//FLASH_ProgramPage(position+0x4000,length,(uint8_t *)&value[0]);  
		uint8_t pack = length >> 6;																			// /64
		position += 0x4000;
		for(unsigned int i=0; i<pack; i++)
		{
			if(position<(0x3FC1+0x4000))																	//¸üÐÂµØÖ·Ð¡ÓÚ16K
			{
				FLASH_ProgramPage(position,64,(uint8_t *)&Write_Flash_FIFO[i<<6]); 
				position += 64;
			}
		}
		pack = length % 64;
		if(pack)
		{
			if(position<(0x3FC1+0x4000))																	//¸üÐÂµØÖ·Ð¡ÓÚ16K
			{
				FLASH_ProgramPage(position,pack,(uint8_t *)&Write_Flash_FIFO[length-pack]);						
				position += pack;
			}
		}
  }
  
  return SUCCESS;
}
#endif

#ifdef SUPPORT_GREEN_TIME
/**
 * @brief  Gets the green time
 * @param[in] {time} Get the time data
 * @return Null
 * @note   MCU needs to implement this function by itself
 */
void mcu_get_greentime(unsigned char time[])
{
  #error "Complete the code yourself and delete the line"
  /*
  time[0] is the flag of success for obtaining the time,0: failure / 1: success
  time[1] is year, 0x00 :2000 year
  time[2] is month, It starts at 1 and ends at 12
  time[3] is date, It starts at 1 and ends at 31
  time[4] is hour, It starts at 0 and ends at 23
  time[5] is minute, It starts at 0 and ends at 59
  time[6] is second, It starts at 0 and ends at 59
*/
  if(time[0] == 1)
  {
    //Receive the green data returned by wifi module correctly
  }
  else
  {
  	//There is an error in obtaining the green time. It may be that the current wifi module is not connected to the Internet
  }
}
#endif

#ifdef SUPPORT_MCU_RTC_CHECK
/**
 * @brief  MCU proofreads local RTC clock
 * @param[in] {time} Get the time data
 * @return Null
 * @note   MCU needs to implement this function by itself
 */
void mcu_write_rtctime(unsigned char time[])
{
  #error "Please complete the RTC clock write code yourself and delete the line"
  /*
  Time[0] is the time success flag, 0 is a failure, and 1 is a success.
  Time[1] is the year and 0x00 is the year 2000.
  Time[2] is the month, starting from 1 to ending at 12
  Time[3] is the date, starting from 1 to 31
  Time[4] is the clock, starting from 0 to ending at 23
  Time[5] is minutes, starting from 0 to ending at 59
  Time[6] is seconds, starting from 0 to ending at 59
  Time[7] is the week, starting from 1 to 7 and 1 is Monday.
 */
  if(time[0] == 1)
  {
    //Correctly receive the local clock data returned by the wifi module
	 
  }
  else
  {
  	//Error getting local clock data, it may be that the current wifi module is not connected
  }
}
#endif

#ifndef WIFI_TEST_ENABLE
/**
 * @brief  Wifi function test feedback
 * @param[in] {result} Wifi function test
 * @ref       0: failure
 * @ref       1: success
 * @param[in] {rssi} Test success indicates wifi signal strength / test failure indicates error type
 * @return Null
 * @note   MCU needs to implement this function by itself
 */
void wifi_test_result(unsigned char result,unsigned char rssi)
{
  //#error "Please implement the wifi function test success/failure code by yourself. Please delete the line after completion"
	SN_GPIO0->DATA_b.DATA9 = 0x0;
  if(result == 0)
  {
    //Test failed
    if(rssi == 0x00)
    {
      //Can't scan to the router named tuya_mdev_test, please check
    }
    else if(rssi == 0x01)
    {
      //Module not authorized
    }
		LED1_OFF;
		LED2_OFF;
		LED3_OFF;
		LED4_OFF;
		LED5_OFF;
		LED6_OFF;
  }
  else
  {
    //Test success
    //rssiis the signal strength (0-100, 0 signal is the worst, 100 signal is the strongest)
		if(rssi < 60)
		{
			LED1_OFF;
			LED2_OFF;
			LED3_OFF;
			LED4_OFF;
			LED5_OFF;
			LED6_OFF;
		}
		else if(rssi >= 60)
		{
			LED1_ON;
			LED2_ON;
			LED3_ON;
			LED4_ON;
			LED5_ON;
			LED6_ON;
		}
  }
  
}
#endif

#ifdef WEATHER_ENABLE
/**
 * @brief  Turn on the weather function
 * @param[in] {res} Turn on the weather function to return the result
 * @ref       0: failure
 * @ref       1: success
 * @param[in] {err} Error code
 * @return Null
 * @note   MCU needs to implement this function by itself
 */
void weather_open_return_handle(unsigned char res, unsigned char err)
{
  #error "Please complete the M open weather function to return the data processing code. Please delete the line after completion"
  unsigned char err_num = 0;
  
  if(res == 1)
  {
    //Open the weather and return successfully
  }
  else if(res == 0)
  {
    //Open weather return failed
    //Get the error code
    err_num = err; 
  }
}

/**
 * @brief  Weather data user self-processing function
 * @param[in] {name} parameter name
 * @param[in] {type} parameter type
 * @ref       0: int
 * @ref       1: string
 * @param[in] {data} the address of the parameter value
 * @param[in] {day} What day's weather. range of value: 0~6
 * @ref       0: today
 * @ref       1: tomorrow
 * @return Null
 * @note   MCU needs to implement this function by itself
 */
void weather_data_user_handle(char *name, unsigned char type, const unsigned char *data, char day)
{
  #error "Here is just an example. Please correct the weather data processing code yourself. Please delete the line after you finish."
  int value_int;
  char value_string[50];//Since there are many parameters, the default is 50. You can reduce this value appropriately based on the defined parameters.
  
  my_memset(value_string, '\0', 50);
  
  //First get the data type
  if(type == 0) //The parameter is INT type
  {
    value_int = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
  }
  else if(type == 1)
  {
    my_strcpy(value_string, data);
  }
  
  //Note that the parameter value is obtained according to the selected parameter type! ! !
  if(my_strcmp(name, "temp") == 0)
  {
    printf("day:%d temp value is:%d\r\n", day, value_int);            //int type
  }
  else if(my_strcmp(name, "humidity") == 0)
  {
    printf("day:%d humidity value is:%d\r\n", day, value_int);        //int type
  }
  else if(my_strcmp(name, "pm25") == 0)
  {
    printf("day:%d pm25 value is:%d\r\n", day, value_int);            //int type
  }
  else if(my_strcmp(name, "condition") == 0)
  {
    printf("day:%d condition value is:%s\r\n", day, value_string);    //string type
  }
}
#endif

#ifdef MCU_DP_UPLOAD_SYN
/**
 * @brief  Status reports results synchronously
 * @param[in] {result} Result
 * @ref       0: failure
 * @ref       1: success
 * @return Null
 * @note   MCU needs to implement this function by itself
 */
void get_upload_syn_result(unsigned char result)
{
  #error "Please complete the status report result processing code by yourself and delete the line"
    
  if(result == 0)
  {
  	//Synchronous report error
  }
  else
  {
    //Report synchronously successfully
  }
}
#endif

#ifdef GET_WIFI_STATUS_ENABLE
/**
 * @brief  Get the WIFI status result
 * @param[in] {result} Wifi working state
 * @ref       0x00: wifi status 1  smartconfig configuration status
 * @ref       0x01: wifi status 2  AP configuration status
 * @ref       0x02: wifi status 3  WIFI configured but not connected to the router
 * @ref       0x03: wifi status 4  WIFI Configured and connected to the router
 * @ref       0x04: wifi status 5  The router is connected and connected to the cloud
 * @ref       0x05: wifi status 6  The wifi device is in low power mode
 * @ref       0x06: wifi status 7  The wifi device is in smartconfig&AP mode
 * @return Null
 * @note   MCU needs to implement this function by itself
 */
void get_wifi_status(unsigned char result)
{
  #error "Complete the get the WIFI status result processing code yourself and delete the line"
 
  switch(result) {
    case 0:
      //wifi working state 1
      break;
  
    case 1:
      //wifi working state 2
      break;
      
    case 2:
      //wifi working state 3
      break;
      
    case 3:
      //wifi working state 4
      break;
      
    case 4:
      //wifi working state 5
      break;
      
    case 5:
      //wifi working state 6
      break;
    
    case 6:
      //wifi working state 7
      break;
      
    default:
    break;
  }
}
#endif

#ifdef WIFI_STREAM_ENABLE
/**
 * @brief  Stream service delivery
 * @param[in] {id} ID number
 * @param[in] {buffer} The address at which the packet is sent
 * @param[in] {buf_len} Length of packet sent
 * @return Stream_trans result
 * -           0(ERROR): failure
 * -           1(SUCCESS): success
 * @note   MCU needs to implement this function by itself
 */
unsigned char stream_trans_send(unsigned int id, unsigned char *buffer, unsigned long buf_len)
{
  #error "Here is just an example. Please correct the stream service delivery processing code yourself. Please delete the line after you finish."
  unsigned long map_offset = 0;
  unsigned int pack_num = 0;
  unsigned int rest_length = 0;

  if(stop_update_flag == ENABLE)
    return SUCCESS;

  pack_num = buf_len / STREM_PACK_LEN;
  rest_length = buf_len - pack_num * STREM_PACK_LEN;
  if (rest_length > 0)
  {
    pack_num++;
  }

  int this_len = STREM_PACK_LEN;
  for (int cnt = 0; cnt < pack_num; cnt++)
  {
    if (cnt == pack_num - 1 && rest_length > 0)
    {
      this_len = rest_length;
    }
    else
    {
      this_len = STREM_PACK_LEN;
    }

    if(SUCCESS == stream_trans(id, map_offset, buffer + map_offset, this_len))
    {
      //The MCU is being upgraded and cannot be streamed
      //printf("is upgrade\n");
      return SUCCESS;
    }

    //while(stream_status == 0xff); //Received back
    
    if(stream_status != 0)
    {
      return ERROR;
    }
  }
  
  return SUCCESS;
}

/**
 * @brief  Multi-map stream service delivery
 * @param[in] {id} Map ID number
 * @param[in] {sub_id} Submap ID number
 * @param[in] {sub_id_pro_mode} Map ID data processing method
 * @ref       0x00: Continue to accumulate
 * @ref       0x01: Clear the data uploaded by the submap ID number
 * @param[in] {buffer} The address at which the packet is sent
 * @param[in] {buf_len} Length of packet sent
 * @return Stream_trans result
 * -           0(ERROR): failure
 * -           1(SUCCESS): success
 * @note   MCU needs to implement this function by itself
 */
unsigned char maps_stream_trans_send(unsigned int id, unsigned char sub_id, unsigned char sub_id_pro_mode, unsigned char *buffer, unsigned long buf_len)
{
  #error "Here is just an example. Please correct the multi-map stream service delivery processing code yourself. Please delete the line after you finish."
  unsigned long map_offset = 0;
  unsigned int pack_num = 0;
  unsigned int rest_length = 0;
  unsigned char pro_ver = 0; //Map service protocol version, default is 0

  if(stop_update_flag == ENABLE)
    return SUCCESS;

  pack_num = buf_len / STREM_PACK_LEN;
  rest_length = buf_len - pack_num * STREM_PACK_LEN;
  if (rest_length > 0)
  {
    pack_num++;
  }

  int this_len = STREM_PACK_LEN;
  for (int cnt = 0; cnt < pack_num; cnt++)
  {
    if (cnt == pack_num - 1 && rest_length > 0)
    {
      this_len = rest_length;
    }
    else
    {
      this_len = STREM_PACK_LEN;
    }

    if(SUCCESS == maps_stream_trans(pro_ver, id, sub_id, sub_id_pro_mode, map_offset, buffer + map_offset, this_len))
    {
      //The MCU is being upgraded and cannot be streamed
      //printf("is upgrade\n");
      return SUCCESS;
    }

    //while(stream_status == 0xff); //Received back
    
    if(stream_status != 0)
    {
      return ERROR;
    }
  }
  
  return SUCCESS;
}
#endif

#ifdef WIFI_CONNECT_TEST_ENABLE
/**
 * @brief  Routing information receives notification of results
 * @param[in] {result} Whether the module has successfully received the correct routing information
 * @ref       0x00: failure
 * @ref       0x01: success
 * @return Null
 * @note   MCU needs to implement this function by itself
 */
void wifi_connect_test_result(unsigned char result)
{
  #error "Please implement the successful/failed wifi function test processing code by yourself, and delete the line after completion"
  if(result == 0)
  {
    //The routing message failed to receive. Please check whether the routing packet sent is a complete JSON packet
  }
  else
  {
    //The routing information was received successfully. Please pay attention to the wifi working state of WIFI_STATE_CMD command
  }
}
#endif

#ifdef GET_MODULE_MAC_ENABLE
/**
 * @brief  Gets the module MAC result
 * @param[in] {mac} Module MAC data
 * @ref       mac[0]: is whether to get the MAC success flag,  0x00 success, 0x01 failure
 * @ref       mac[1]~mac[6]: When obtaining the MAC address flag bit MAC [0] is successful, it represents the valid MAC address of the module
 * @return Null
 * @note   MCU needs to implement this function by itself
 */
void mcu_get_mac(unsigned char mac[])
{
  #error "Complete the MAC fetch processing code yourself and delete the line"
  /*
  mac[0] is whether to get the MAC success flag,  0x00 success, 0x01 failure
  mac[1]~mac[6]:When obtaining the MAC address flag bit MAC [0] is successful, it represents the valid MAC address of the module
 */
 
  if(mac[0] == 1)
  {
  	//Error getting MAC
  }
  else
  {
    //Receive the MAC address returned by wifi module correctly
  }
}
#endif

#ifdef GET_IR_STATUS_ENABLE
/**
 * @brief  Obtain the infrared state results
 * @param[in] {result} Indicating infrared state
 * @ref       0x00: Infrared state 1 Sending infrared code
 * @ref       0x01: Infrared state 2 Send infrared code to end
 * @ref       0x02: Infrared state 3 Infrared learning begins
 * @ref       0x03: Infrared state 4 End of infrared learning
 * @return Null
 * @note   MCU needs to implement this function by itself
 */
void get_ir_status(unsigned char result)
{
  #error "Please complete the infrared status processing code by yourself and delete the line"
  unsigned short length = 0;
    
  switch(result) {
    case 0:
      //Infrared state 1
      break;
  
    case 1:
      //Infrared state 2
      break;
      
    case 2:
      //Infrared state 3
      break;
      
    case 3:
      //Infrared state 4
      break;
      
    default:
    break;
  }
  
  wifi_uart_write_frame(GET_IR_STATUS_CMD,MCU_SEND_VER,length);
}
#endif

#ifdef IR_TX_RX_TEST_ENABLE
/**
 * @brief  Notification of infrared incoming and outgoing test results
 * @param[in] {result} Whether the module successfully received the correct information
 * @ref       0x00: failure
 * @ref       0x01: success
 * @return Null
 * @note   MCU needs to implement this function by itself
 */
void ir_tx_rx_test_result(unsigned char result)
{
  #error "Please implement the infrared entry, send, receive, and test functions by yourself. Please delete this line after completion"
  if(result == 0)
  {
    //Infrared into the transceiver production test successful
  }
  else
  {
    //Infrared incoming transceiver production test failed, please check the sent packet
  }
}
#endif

#ifdef FILE_DOWNLOAD_ENABLE
/**
 * @brief  File download package size selection
 * @param[in] {package_sz} File download package size
 * @ref       0x00: 256 byte
 * @ref       0x01: 512 byte
 * @ref       0x02: 1024 byte
 * @return Null
 * @note   MCU needs to implement this function by itself
 */
void file_download_package_choose(unsigned char package_sz)
{
  #error "Please complete the file download package size selection processing code by yourself and delete the line"
  unsigned short length = 0;
  length = set_wifi_uart_byte(length,package_sz);
  wifi_uart_write_frame(FILE_DOWNLOAD_START_CMD,MCU_SEND_VER,length);
}

/**
 * @brief  Package download
 * @param[in] {value} Data buffer
 * @param[in] {position} The current packet is at the file location
 * @param[in] {length} Current file package length (when the length is 0, the file package is sent)
 * @return Data processing results
 * -           0(ERROR): failure
 * -           1(SUCCESS): success
 * @note   MCU needs to implement this function by itself
 */
unsigned char file_download_handle(const unsigned char value[],unsigned long position,unsigned short length)
{
  #error "Please complete the file download selection processing code by yourself and delete the line"
  if(length == 0)
  {
    //File package data sending completed
    
  }
  else
  {
    //File package data processing
  }
  
  return SUCCESS;
}
#endif

#ifdef MODULE_EXPANDING_SERVICE_ENABLE
/**
 * @brief  Module time service notification results
 * @param[in] {value} Data buffer
 * @param[in] {length} Data buffer length
 * @return Null
 * @note   MCU needs to implement this function by itself
 */
void open_module_time_serve_result(const unsigned char value[], unsigned short length)
{
  #error "Please complete the Module time service notification results processing code by yourself and delete the line"
  unsigned char sub_cmd = value[0];
  
  switch(sub_cmd)
  {
    case 0x01: { //
      if(0x02 != length)
      {
        //Data length error
        return;
      }
      
      if(value[1] == 0)
      {
        //Service started successfully
      }
      else
      {
        //Service startup failure
      }
    }
    break;
    case 0x02: {
      if(0x09 != length)
      {
        //Data length error
        return;
      }
      
      unsigned char time_type = value[1]; //0x00:Green time    0x01:Local time
      unsigned char time_data[7];
      
      my_memcpy(time_data, value + 2, length - 2);
      /*
      Data[0] is the year and 0x00 is the year 2000.
      Data[1] is the month, starting from 1 to ending at 12
      Data[2] is the date, starting from 1 to 31
      Data[3] is the clock, starting from 0 to ending at 23
      Data[4] is minutes, starting from 0 to ending at 59
      Data[5] is seconds, starting from 0 to ending at 59
      Data[6] is the week, starting from 1 to 7 and 1 is Monday.
      */
      
      //Add the time data handling code here, with time_type as the time type
      
      unsigned short length = 0;
      length = set_wifi_uart_byte(length,sub_cmd);
      wifi_uart_write_frame(OPEN_MODULE_TIME_SERVE_CMD,MCU_SEND_VER,length);
    }
    break;
    default:break;
  }
}
#endif

#ifdef BLE_RELATED_FUNCTION_ENABLE
/**
 * @brief  Bluetooth functional test results
 * @param[in] {value} Data buffer
 * @param[in] {length} Data buffer length
 * @return Null
 * @note   MCU needs to implement this function by itself
 */
void BLE_test_result(const unsigned char value[], unsigned short length)
{
  #error "Please complete the Bluetooth functional test results processing code by yourself and delete the line"
  unsigned char sub_cmd = value[0];
  
  if(0x03 != length)
  {
    //Data length error
    return;
  }
  
  if(0x01 != sub_cmd)
  {
    //Subcommand error
    return;
  }
  
  unsigned char result = value[1];
  unsigned char rssi = value[2];
      
  if(result == 0)
  {
    //test failure 
    if(rssi == 0x00)
    {
      //Bluetooth beacon named ty_mdev is not scanned, please check
    }
    else if(rssi == 0x01)
    {
      //Module is not authorized
    }
  }
  else if(result == 0x01)
  {
    //Test success
    //rssiis the signal strength (0-100, 0 signal is the worst, 100 signal is the strongest)
  }
}
#endif

#ifdef VOICE_MODULE_PROTOCOL_ENABLE
/**
 * @brief  Get voice status code result
 * @param[in] {result} Voice status code
 * @ref       0x00: idle
 * @ref       0x01: mic mute state
 * @ref       0x02: wake
 * @ref       0x03: recording
 * @ref       0x04: Recognizing
 * @ref       0x05: Identified successfully
 * @ref       0x06: Recognition failed
 * @return Null
 * @note   MCU needs to implement this function by itself
 */
void get_voice_state_result(unsigned char result)
{
  #error "Please complete the get voice status code result processing code by yourself and delete the line"
  switch(result) {
    case 0:
      //idle
      break;
  
    case 1:
      //mic mute state
      break;
      
    case 2:
      //wake
      break;
      
    case 3:
      //recording
      break;
      
    case 4:
      //Recognizing
      break;
  
    case 5:
      //Identified successfully
      break;
      
    case 6:
      //Recognition failed
      break;
      
    default:
    break;
  }
}

/**
 * @brief  MIC mute setting
 * @param[in] {result} Mute setting value
 * @ref       0x00: mic on
 * @ref       0x01: mic mute
 * @return Null
 * @note   MCU needs to implement this function by itself
 */
void set_voice_MIC_silence_result(unsigned char result)
{
  #error "Please complete the MIC mute setting processing code by yourself and delete the line"
  if(result == 0)
  {
    //mic on
  }
  else
  {
    //mic mute
  }
}

/**
 * @brief  speaker volume setting result
 * @param[in] {result} Volume value
 * @ref       0~10: Volume range
 * @return Null
 * @note   MCU needs to implement this function by itself
 */
void set_speaker_voice_result(unsigned char result)
{
  #error "Please complete the speaker volume setting result processing code by yourself and delete the line"
  
}

/**
 * @brief  Audio production test results
 * @param[in] {result} Audio production test status
 * @ref       0x00: Turn off audio production test
 * @ref       0x01: mic1 audio loop test
 * @ref       0x02: mic2 audio loop test
 * @return Null
 * @note   MCU needs to implement this function by itself
 */
void voice_test_result(unsigned char result)
{
  #error "Please complete the audio production test results processing code by yourself and delete the line"
  if(result == 0x00)
  {
    //Turn off audio production test
  }
  else if(result == 0x01)
  {
    //mic1 audio loop test
  }
  else if(result == 0x02)
  {
    //mic2 audio loop test
  }
}

/**
 * @brief  Get wake up test result
 * @param[in] {result} Wake-up return value
 * @ref       0x00: Wake up successfully
 * @ref       0x01: Wake up failure (10s timeout failure)
 * @return Null
 * @note   MCU needs to implement this function by itself
 */
void voice_awaken_test_result(unsigned char result)
{
  #error "Please complete the wake up the test results processing code by yourself and delete the line"
  if(result == 0x00)
  {
    //Wake up successfully
  }
  else if(result == 0x01)
  {
    //Wake up failure
  }
}
#endif



/******************************************************************************
                                WARNING!!!                     
The following function users do not modify!!
******************************************************************************/

/**
 * @brief  dp delivery processing function
 * @param[in] {dpid} DP number
 * @param[in] {value} dp data buffer address
 * @param[in] {length} dp data length
 * @return Dp processing results
 * -           0(ERROR): failure
 * -           1(SUCCESS): success
 * @note   The function user cannot modify
 */
unsigned char dp_download_handle(unsigned char dpid,const unsigned char value[], unsigned short length)
{
  /*********************************
  Current function processing can issue/report data calls                    
  Need to implement the data processing in the specific function
  The result of the processing needs to be fed back to the APP, otherwise the APP will consider the delivery failure.
  ***********************************/
  unsigned char ret;
  switch(dpid)
  {
    case DPID_SWITCH_LED_1:
      //开关1处理函数
      ret = dp_download_switch_led_1_handle(value,length);
      break;
    case DPID_BRIGHT_VALUE_1:
      //亮度值1处理函数
      ret = dp_download_bright_value_1_handle(value,length);
      break;
    case DPID_BRIGHTNESS_MIN_1:
      //最小亮度1处理函数
      ret = dp_download_brightness_min_1_handle(value,length);
      break;
    case DPID_LED_TYPE_1:
      //光源类型1处理函数
      ret = dp_download_led_type_1_handle(value,length);
      break;

  default:
    break;
  }
  return ret;
}

/**
 * @brief  Get the sum of all dp commands
 * @param[in] Null
 * @return Sent the sum of the commands
 * @note   The function user cannot modify
 */
unsigned char get_download_cmd_total(void)
{
  return(sizeof(download_cmd) / sizeof(download_cmd[0]));
}

