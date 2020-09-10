/******************** (C) COPYRIGHT 2020 INNOTECH **************************
* COMPANY:			INNOTECH
* DATE:					2020/05
* AUTHOR:				Sean
* IC:						SN32F700B
* DESCRIPTION:	Main related functions.
*____________________________________________________________________________
* REVISION	Date				User		Description
* 1.0				2020/05/7	  Sean			First release
*
*____________________________________________________________________________

*****************************************************************************/

/*_____ I N C L U D E S ____________________________________________________*/
#include <SN32F700B.h>
#include <SN32F700_Def.h>

#include "GPIO.h"
#include "CT16B1.h"
#include "CT16B2.h"
#include "SysTick.h"
#include "UART.h"
#include "Flash.h"
#include "PMU_drive.h"
#include "Utility.h"
#include "WDT.h"
#include "userApp.h"
#include "mcu_api.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/

uint8_t PMU_Status = 0;
uint8_t Step_Cnt = 0;
uint8_t IsUpdate = FALSE;
uint8_t backup_value = 0;

extern volatile uint8_t SwitchProcessFlag;

/******************************/
void	NotPinOut_GPIO_init(void);

/*_____ D E F I N I T I O N S ______________________________________________*/

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

/*****************************************************************************
* Function		: Read_Parameter
* Description	: Read parameter 
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
static void Read_Parameter(void)
{
	FLASH_Read_Data(ISP_TARGET_ADDR, (uint8_t *)(&Device_Param), sizeof(Device_Param));
	
	// 默认参数
	if(Device_Param.flag != 0xAA)
	{
		Device_Param.flag = 0xAA;
		Device_Param.led_type = 0x01;
		Device_Param.led_switch = OFF;
		Device_Param.brightness_en = FALSE;
		Device_Param.led_angle = MAX_BR;
		Device_Param.led_angle_min = MIN_BR;
		Device_Param.bright_value = 1000;
		Device_Param.brightness_min = 10;
    	
		//1. Backup data @ISP_TARGET_ADDR if needed	

		//2. Erase page address @ISP_TARGET_ADDR
		//==>Not Necessary to erase for SN32F700B series!!!

		//3. Program content of wFLASH_PGRAM[0] & [1] to User ROM address ISP_TARGET_ADDR & (ISP_TARGET_ADDR+4)
		if (FLASH_ProgramPage(ISP_TARGET_ADDR, sizeof(Device_Param), (uint8_t *)(&Device_Param)) == FAIL){
				while(1);			//Program Fail
		}
		FLASH_Read_Data(ISP_TARGET_ADDR, (uint8_t *)(&Device_Param), sizeof(Device_Param));
  }  
}

/*****************************************************************************
* Function		: main
* Description	: Add description of this project Here. 
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
int	main (void)
{
	//User can configure System Clock with Configuration Wizard in system_SN32F700B.c
	SystemInit();
	SystemCoreClockUpdate();				//Must call for SN32F700B, Please do NOT remove!!!

	//1. User SHALL define PKG on demand.
	//2. User SHALL set the status of the GPIO which are NOT pin-out to input pull-up.
	//NotPinOut_GPIO_init();
	//--------------------------------------------------------------------------
	//User Code starts HERE!!!
	WDT_Init();
  GPIO_Init();
	UART0_Init();
	UART1_Init();
	//MN_CT16B1_Init();
	MN_CT16B2_Init();
	SysTick_Init();
	Read_Parameter();
  UT_DelayNms(200);
	//Fre_Cnt_Flag = TRUE;  // frequency acquisition mark
	backup_value = SN_GPIO0->DATA_b.DATA4;
	
	// zigbee init
	zigbee_protocol_init();
	current_light_angle = Device_Param.led_angle;
	
	//Factory_Test_Mode();
	while(1)
	{
		WDT_ReloadValue(250);
		if(IsUpdate == FALSE)
		{
			if(Step_Flag)
			{
				Step_Flag = 0;
				switch(Step_Cnt++)
				{
					case 0:
						Key_Process();
						Led_Process();
						break;
					case 1:
						zigbee_uart_service();
						break;
					case 2:
						Fade_Process();
						break;
					case 3:
						UartB_Process();
						break;
					case 4:
						if(SwitchProcessFlag == TRUE)
						{
							SwitchProcessFlag = FALSE;
							Connect_With_Switch();
						}
						break;
					case 5:
						Step_Cnt = 0;
						SetTimeout_Process();
						break;

					default: 
						break;
					}
				}
		}
		else if(IsUpdate == TRUE)
		{
			SysTick->CTRL = 0x5; // close systick interrupt
			NVIC_DisableIRQ(CT16B2_IRQn);
			zigbee_uart_service();
		}
	}
}


/*****************************************************************************
* Function		: NotPinOut_GPIO_init
* Description	: Set the status of the GPIO which are NOT pin-out to input pull-up. 
* Input				: None
* Output			: None
* Return			: None
* Note				: 1. User SHALL define PKG on demand.
*****************************************************************************/
void	NotPinOut_GPIO_init(void)
{
	// pull-up no use
	SN_GPIO0->CFG = 0x00A2A0AA;
	SN_GPIO1->CFG = 0x00AAA802;
	SN_GPIO2->CFG = 0x00AAA02A;
	SN_GPIO3->CFG = 0x00A28AAA;
}

/*****************************************************************************
* Function		: HardFault_Handler
* Description	: ISR of Hard fault interrupt
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
__irq void HardFault_Handler(void)
{
	NVIC_SystemReset();
}

