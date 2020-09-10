/******************** (C) COPYRIGHT 2020 INNOTECH **************************
* COMPANY:			INNOTECH
* DATE:					2020/05
* AUTHOR:				Sean
* IC:						SN32F700B
* DESCRIPTION:	SysTick related functions.
*____________________________________________________________________________
* REVISION	Date				User		Description
* 1.0				2020/05/7	  Sean			First release
*
*____________________________________________________________________________

*****************************************************************************/

/*_____ I N C L U D E S ____________________________________________________*/
#include "SysTick.h"

#include "GPIO.h"
#include "userApp.h"
#include "zigbee.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/
static uint8_t  LedFlashStep = 0;         //???????(2?1?)
volatile uint8_t FadeProcessFlag = FALSE;
volatile uint8_t SetTimeoutFlag = FALSE;

volatile uint8_t FactoryTestFlag = FALSE;
volatile uint8_t FactoryTestStep = 0;
volatile uint8_t FactoryStepFlag = FALSE;
volatile uint16_t FactoryTickCount = 0;
volatile uint8_t SwitchProcessFlag;

uint8_t Step_Flag = 0;

extern uint8_t zigbee_reset_flag;
/*_____ D E F I N I T I O N S ______________________________________________*/


/*_____ M A C R O S ________________________________________________________*/


/*_____ F U N C T I O N S __________________________________________________*/
/*****************************************************************************
* Function		: SysTick_Init
* Description	: Initialization of SysTick timer
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
void SysTick_Init (void)
{
	SystemCoreClockUpdate();
	
	__SYSTICK_SET_TIMER_PERIOD(1);			//RELOAD = (system tick clock frequency �� 10 ms)/1000 -1
  //SysTick->LOAD = 6*100 - 1;
	
	__SYSTICK_CLEAR_COUNTER_AND_FLAG;

#if SYSTICK_IRQ == INTERRUPT_METHOD
	SysTick->CTRL = 0x7;			//Enable SysTick timer and interrupt
#else
	SysTick->CTRL = 0x5;			//Enable SysTick timer ONLY
#endif	
}


/*****************************************************************************
* Function		: SysTick_Handler
* Description	: ISR of SysTick interrupt
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
__irq void SysTick_Handler(void)
{
	static uint32_t SysTickCount = 0;
	static uint32_t Timer0_Count = 0;
	static uint32_t Timer0_Value = 0;
	static uint8_t Tick = 0;
	
	__SYSTICK_CLEAR_COUNTER_AND_FLAG;
	
	// 厂测计数
	FactoryTickCount++;
	if(FactoryTickCount == 4000)
	{
		FactoryTickCount = 0;
		FactoryStepFlag = TRUE;
	}
	Tick++;
	if(Tick == 5)
	{
		Tick = 0;
		//ZCD_Process();
		Step_Flag = 1;
	}
	KeyPowerTime++;
	KeyDimUpTime++;
	KeyDimDownTime++;
		
	
	if(IsSwitch == TRUE && NormalSwitch == TRUE)
	{
		TimeoutMs++;
		if(TimeoutMs >= 50)
		{
			SwitchProcessFlag = TRUE;		
		}
	}
	
	Timer0_Count++;
	SysTickCount++;
	if(SysTickCount == 10)
	{
		SysTickCount = 0;
		FadeProcessFlag = TRUE;

		{
			if(WorkMode == MODE_BRIGHT_SET)
			{
					SetTimeout++;
					// 1 min timeout
					if(SetTimeout >= 6000)
					{
							SetTimeout = 0;
						  SetTimeoutFlag = TRUE;
					}
					if(SetTimeout % 25 == 0)
					{
						if(LedFlashStep < 4)
						{
							LedFlashStep++;
							SN_GPIO0->DATA_b.DATA9 ^= 0x1;		//Toggle P0.9
						}
					}
					if(SetTimeout % 100 == 0)
					{
							LedFlashStep++;
							if(LedFlashStep < 8 && LedFlashStep > 5)
							{
								SN_GPIO0->DATA_b.DATA9 ^= 0x1;		//Toggle P0.9
							}else if (LedFlashStep == 8){
								LedFlashStep = 0;
							}
					}		
			}
			else if(WorkMode == MODE_NORMAL && FactoryTestFlag == FALSE)
			{
				if(Timer0_Count > 250)
				{
					Timer0_Count = 0;
					Timer0_Value++;
							
					if(Timer0_Value == 16)
					{
						Timer0_Value = 0;
					}
					if(zigbee_reset_flag == 1)
					{
						switch(zigbee_work_state)
						{
							case 1:										//connect
								zigbee_reset_flag = 0;
								SN_GPIO0->DATA_b.DATA9 = 0x0;		//Toggle P0.9
							break;
							default:
									SN_GPIO0->DATA_b.DATA9 ^= 0x1;		//Toggle P0.9
									//mcu_reset_zigbee();
								break;
						}
					}	
				}
			}
		} 
	}
}

 
