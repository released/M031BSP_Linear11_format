/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>
#include <string.h>
#include "NuMicro.h"

#include "misc_config.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/

struct flag_32bit flag_PROJ_CTL;
#define FLAG_PROJ_TIMER_PERIOD_1000MS                 	(flag_PROJ_CTL.bit0)
#define FLAG_PROJ_REVERSE1                   			(flag_PROJ_CTL.bit1)
#define FLAG_PROJ_REVERSE2                 				(flag_PROJ_CTL.bit2)
#define FLAG_PROJ_REVERSE3                              (flag_PROJ_CTL.bit3)
#define FLAG_PROJ_REVERSE4                              (flag_PROJ_CTL.bit4)
#define FLAG_PROJ_REVERSE5                              (flag_PROJ_CTL.bit5)
#define FLAG_PROJ_REVERSE6                              (flag_PROJ_CTL.bit6)
#define FLAG_PROJ_REVERSE7                              (flag_PROJ_CTL.bit7)


#define FIST_MAGIC                                      ((((65536.0 * 65536.0 * 16)+(65536.0 * 0.5))* 65536.0))
#define MATH_2_N_6                              	    (-6)	
#define MATH_2_N_5                              	    (-5)	
#define MATH_2_N_4                              	    (-4)	
#define MATH_2_N_2                              	    (-2)	
#define MATH_2_N_1                              	    (-1)	
#define MATH_2_N_9                              	    (-9)
	
#define MATH_2_P_1                              	    (1)
#define MATH_2_P_2                              	    (2)
/*_____ D E F I N I T I O N S ______________________________________________*/

volatile unsigned int counter_systick = 0;
volatile uint32_t counter_tick = 0;

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

unsigned int get_systick(void)
{
	return (counter_systick);
}

void set_systick(unsigned int t)
{
	counter_systick = t;
}

void systick_counter(void)
{
	counter_systick++;
}

void SysTick_Handler(void)
{

    systick_counter();

    if (get_systick() >= 0xFFFFFFFF)
    {
        set_systick(0);      
    }

    // if ((get_systick() % 1000) == 0)
    // {
       
    // }

    #if defined (ENABLE_TICK_EVENT)
    TickCheckTickEvent();
    #endif    
}

void SysTick_delay(unsigned int delay)
{  
    
    unsigned int tickstart = get_systick(); 
    unsigned int wait = delay; 

    while((get_systick() - tickstart) < wait) 
    { 
    } 

}

void SysTick_enable(unsigned int ticks_per_second)
{
    set_systick(0);
    if (SysTick_Config(SystemCoreClock / ticks_per_second))
    {
        /* Setup SysTick Timer for 1 second interrupts  */
        printf("Set system tick error!!\n");
        while (1);
    }

    #if defined (ENABLE_TICK_EVENT)
    TickInitTickEvent();
    #endif
}

uint32_t get_tick(void)
{
	return (counter_tick);
}

void set_tick(uint32_t t)
{
	counter_tick = t;
}

void tick_counter(void)
{
	counter_tick++;
    if (get_tick() >= 60000)
    {
        set_tick(0);
    }
}

void delay_ms(uint16_t ms)
{
	#if 1
    uint32_t tickstart = get_tick();
    uint32_t wait = ms;
	uint32_t tmp = 0;
	
    while (1)
    {
		if (get_tick() > tickstart)	// tickstart = 59000 , tick_counter = 60000
		{
			tmp = get_tick() - tickstart;
		}
		else // tickstart = 59000 , tick_counter = 2048
		{
			tmp = 60000 -  tickstart + get_tick();
		}		
		
		if (tmp > wait)
			break;
    }
	
	#else
	TIMER_Delay(TIMER0, 1000*ms);
	#endif
}


int32_t float2Int(float inval)
{
	double dtemp = FIST_MAGIC + inval;
	return ((*(int32_t *)&dtemp) - 0x80000000);
}

float power(int x,int y)
{
    float p=1.00;
    float xx = (float)x;
    int i;
    if (y < 0)
    {
        y = -1*y;
        xx = 1/xx;
    }
    for (i = 1;i <= y; i++)
    {
        p = p*xx;
    }

    return p;
}

uint16_t linear11_rawdata_to_u16data(float raw , int mantissa)
{
    uint16_t res = 0;
    float tmp = 0.0f;

    res = mantissa << 11;
    tmp = (float) raw / power(2,mantissa);
    res |= (float2Int(tmp) & 0x7FF);

    // printf("%s:0x%4X\r\n" ,__FUNCTION__,res);

    return res;
}


float linear11_u16data_to_rawdata1(uint16_t input , int mantissa)
{
    /*
        linear11 = u16 & 0x7FF 
        cal data = linear11 * 2^N
    */
    float f_res = 0;

	signed char s_exponent;
	signed short s_mantissa;
	s_exponent = input >> 11;
	s_mantissa = input & 0x7FF;

    if(s_exponent > 0x0F) 
    {
        s_exponent |= 0xE0;
    }
	if(s_mantissa > 0x03FF) 
    {
		s_mantissa |= 0xF800;
	}

    f_res = s_mantissa * power(2.0,s_exponent);
    // printf("%s(**):%4.6f\r\n",__FUNCTION__,f_res);

    return f_res;
}

float linear11_u16data_to_rawdata2(uint16_t input , int mantissa)
{
    /*
        linear11 = u16 & 0x7FF 
        cal data = linear11 * 2^N
    */
    float f_res = 0;

    f_res = power(2,mantissa);
    f_res = f_res*(input & 0x7FF) ;
    // printf("%s(*):%4.6f\r\n",__FUNCTION__,f_res);

    return f_res;
}

void linear11_example_test3(void)
{
    int tmp_raw = 0;
    float tmp_float = 0.0f;
    
    /*
        P :
        1
        2
    */

    printf("\r\n%s\r\n",__FUNCTION__);
    printf("------------\r\n");
    tmp_float = 3.296*0.18 + 5.015*1.18 + 12.000*0.5;
    tmp_raw = linear11_rawdata_to_u16data(tmp_float,MATH_2_P_1);
    printf("P1-raw data:%2.4f,convert to linear11 INT:0x%02X(%4d)\r\n",tmp_float,tmp_raw,tmp_raw);

    tmp_float = linear11_u16data_to_rawdata1(tmp_raw,MATH_2_P_1);
    printf("P1-int data1 convert to linear11 FLOAT:%2.4f\r\n",tmp_float);
    tmp_float = linear11_u16data_to_rawdata2(tmp_raw,MATH_2_P_1);
    printf("P1-int data2 convert to linear11 FLOAT:%2.4f\r\n",tmp_float);

    printf("------------\r\n");
    tmp_float = 3.296*7.62 + 5.015*13.56 + 11.968*7.75;
    tmp_raw = linear11_rawdata_to_u16data(tmp_float,MATH_2_P_2);
    printf("P2-raw data:%2.4f,convert to linear11 INT:0x%02X(%4d)\r\n",tmp_float,tmp_raw,tmp_raw);

    tmp_float = linear11_u16data_to_rawdata1(tmp_raw,MATH_2_P_2);
    printf("P2-int data1 convert to linear11 FLOAT:%2.4f\r\n",tmp_float);
    tmp_float = linear11_u16data_to_rawdata2(tmp_raw,MATH_2_P_2);
    printf("P2-int data2 convert to linear11 FLOAT:%2.4f\r\n",tmp_float);


}

void linear11_example_test2(void)
{
    int tmp_raw = 0;
    float tmp_float = 0.0f;
    
    /*
        I
        3V3 : -4
        5V  : -4
        12V : -2
    */

    printf("\r\n%s\r\n",__FUNCTION__);
    printf("------------\r\n");
    tmp_float = 7.62;
    tmp_raw = linear11_rawdata_to_u16data(tmp_float,MATH_2_N_4);
    printf("3V3-raw data:%2.4f,convert to linear11 INT:0x%02X(%4d)\r\n",tmp_float,tmp_raw,tmp_raw);

    tmp_float = linear11_u16data_to_rawdata1(tmp_raw,MATH_2_N_4);
    printf("3V3-int data1 convert to linear11 FLOAT:%2.4f\r\n",tmp_float);
    tmp_float = linear11_u16data_to_rawdata2(tmp_raw,MATH_2_N_4);
    printf("3V3-int data2 convert to linear11 FLOAT:%2.4f\r\n",tmp_float);

    printf("------------\r\n");
    tmp_float = 13.56;
    tmp_raw = linear11_rawdata_to_u16data(tmp_float,MATH_2_N_4);
    printf("5V-raw data:%2.4f,convert to linear11 INT:0x%02X(%4d)\r\n",tmp_float,tmp_raw,tmp_raw);

    tmp_float = linear11_u16data_to_rawdata1(tmp_raw,MATH_2_N_4);
    printf("5V-int data1 convert to linear11 FLOAT:%2.4f\r\n",tmp_float);
    tmp_float = linear11_u16data_to_rawdata2(tmp_raw,MATH_2_N_4);
    printf("5V-int data2 convert to linear11 FLOAT:%2.4f\r\n",tmp_float);

    printf("------------\r\n");
    tmp_float = 7.75;
    tmp_raw = linear11_rawdata_to_u16data(tmp_float,MATH_2_N_2);
    printf("12V-raw data:%2.4f,convert to linear11 INT:0x%02X(%4d)\r\n",tmp_float,tmp_raw,tmp_raw);

    tmp_float = linear11_u16data_to_rawdata1(tmp_raw,MATH_2_N_2);
    printf("12V-int data1 convert to linear11 FLOAT:%2.4f\r\n",tmp_float);
    tmp_float = linear11_u16data_to_rawdata2(tmp_raw,MATH_2_N_2);
    printf("12V-int data2 convert to linear11 FLOAT:%2.4f\r\n",tmp_float);

}

void linear11_example_test1(void)
{
    int tmp_raw = 0;
    float tmp_float = 0.0f;
    
    /*
        V
        3V3 : -6
        5V  : -6
        12V : -6
    */

    printf("\r\n%s\r\n",__FUNCTION__);
    printf("------------\r\n");
    tmp_float = 3.296;
    tmp_raw = linear11_rawdata_to_u16data(tmp_float,MATH_2_N_6);
    printf("3V3-raw data:%2.4f,convert to linear11 INT:0x%02X(%4d)\r\n",tmp_float,tmp_raw,tmp_raw);

    tmp_float = linear11_u16data_to_rawdata1(tmp_raw,MATH_2_N_6);
    printf("3V3-int data1 convert to linear11 FLOAT:%2.4f\r\n",tmp_float);
    tmp_float = linear11_u16data_to_rawdata2(tmp_raw,MATH_2_N_6);
    printf("3V3-int data2 convert to linear11 FLOAT:%2.4f\r\n",tmp_float);

    printf("------------\r\n");
    tmp_float = 5.015;
    tmp_raw = linear11_rawdata_to_u16data(tmp_float,MATH_2_N_6);
    printf("5V-raw data:%2.4f,convert to linear11 INT:0x%02X(%4d)\r\n",tmp_float,tmp_raw,tmp_raw);

    tmp_float = linear11_u16data_to_rawdata1(tmp_raw,MATH_2_N_6);
    printf("5V-int data1 convert to linear11 FLOAT:%2.4f\r\n",tmp_float);
    tmp_float = linear11_u16data_to_rawdata2(tmp_raw,MATH_2_N_6);
    printf("5V-int data2 convert to linear11 FLOAT:%2.4f\r\n",tmp_float);

    printf("------------\r\n");
    tmp_float = 11.968;
    tmp_raw = linear11_rawdata_to_u16data(tmp_float,MATH_2_N_6);
    printf("12V-raw data:%2.4f,convert to linear11 INT:0x%02X(%4d)\r\n",tmp_float,tmp_raw,tmp_raw);

    tmp_float = linear11_u16data_to_rawdata1(tmp_raw,MATH_2_N_6);
    printf("12V-int data1 convert to linear11 FLOAT:%2.4f\r\n",tmp_float);
    tmp_float = linear11_u16data_to_rawdata2(tmp_raw,MATH_2_N_6);
    printf("12V-int data2 convert to linear11 FLOAT:%2.4f\r\n",tmp_float);

}

void linear11_example(void)
{
    linear11_example_test1();
    linear11_example_test2();
    linear11_example_test3();
}

//
// check_reset_source
//
uint8_t check_reset_source(void)
{
    uint32_t src = SYS_GetResetSrc();

    SYS->RSTSTS |= 0x1FF;
    printf("Reset Source <0x%08X>\r\n", src);

    #if 1   //DEBUG , list reset source
    if (src & BIT0)
    {
        printf("0)POR Reset Flag\r\n");       
    }
    if (src & BIT1)
    {
        printf("1)NRESET Pin Reset Flag\r\n");       
    }
    if (src & BIT2)
    {
        printf("2)WDT Reset Flag\r\n");       
    }
    if (src & BIT3)
    {
        printf("3)LVR Reset Flag\r\n");       
    }
    if (src & BIT4)
    {
        printf("4)BOD Reset Flag\r\n");       
    }
    if (src & BIT5)
    {
        printf("5)System Reset Flag \r\n");       
    }
    if (src & BIT6)
    {
        printf("6)Reserved.\r\n");       
    }
    if (src & BIT7)
    {
        printf("7)CPU Reset Flag\r\n");       
    }
    if (src & BIT8)
    {
        printf("8)CPU Lockup Reset Flag\r\n");       
    }
    #endif
    
    if (src & SYS_RSTSTS_PORF_Msk) {
        SYS_ClearResetSrc(SYS_RSTSTS_PORF_Msk);
        
        printf("power on from POR\r\n");
        return FALSE;
    }    
    else if (src & SYS_RSTSTS_PINRF_Msk)
    {
        SYS_ClearResetSrc(SYS_RSTSTS_PINRF_Msk);
        
        printf("power on from nRESET pin\r\n");
        return FALSE;
    } 
    else if (src & SYS_RSTSTS_WDTRF_Msk)
    {
        SYS_ClearResetSrc(SYS_RSTSTS_WDTRF_Msk);
        
        printf("power on from WDT Reset\r\n");
        return FALSE;
    }    
    else if (src & SYS_RSTSTS_LVRF_Msk)
    {
        SYS_ClearResetSrc(SYS_RSTSTS_LVRF_Msk);
        
        printf("power on from LVR Reset\r\n");
        return FALSE;
    }    
    else if (src & SYS_RSTSTS_BODRF_Msk)
    {
        SYS_ClearResetSrc(SYS_RSTSTS_BODRF_Msk);
        
        printf("power on from BOD Reset\r\n");
        return FALSE;
    }    
    else if (src & SYS_RSTSTS_SYSRF_Msk)
    {
        SYS_ClearResetSrc(SYS_RSTSTS_SYSRF_Msk);
        
        printf("power on from System Reset\r\n");
        return FALSE;
    } 
    else if (src & SYS_RSTSTS_CPURF_Msk)
    {
        SYS_ClearResetSrc(SYS_RSTSTS_CPURF_Msk);

        printf("power on from CPU reset\r\n");
        return FALSE;         
    }    
    else if (src & SYS_RSTSTS_CPULKRF_Msk)
    {
        SYS_ClearResetSrc(SYS_RSTSTS_CPULKRF_Msk);
        
        printf("power on from CPU Lockup Reset\r\n");
        return FALSE;
    }   
    
    printf("power on from unhandle reset source\r\n");
    return FALSE;
}

void TMR1_IRQHandler(void)
{
	
    if(TIMER_GetIntFlag(TIMER1) == 1)
    {
        TIMER_ClearIntFlag(TIMER1);
		tick_counter();

		if ((get_tick() % 1000) == 0)
		{
            FLAG_PROJ_TIMER_PERIOD_1000MS = 1;//set_flag(flag_timer_period_1000ms ,ENABLE);
		}

		if ((get_tick() % 50) == 0)
		{

		}	
    }
}

void TIMER1_Init(void)
{
    TIMER_Open(TIMER1, TIMER_PERIODIC_MODE, 1000);
    TIMER_EnableInt(TIMER1);
    NVIC_EnableIRQ(TMR1_IRQn);	
    TIMER_Start(TIMER1);
}

void loop(void)
{
	static uint32_t LOG1 = 0;
	// static uint32_t LOG2 = 0;

    if ((get_systick() % 1000) == 0)
    {
        // printf("%s(systick) : %4d\r\n",__FUNCTION__,LOG2++);    
    }

    if (FLAG_PROJ_TIMER_PERIOD_1000MS)//(is_flag_set(flag_timer_period_1000ms))
    {
        FLAG_PROJ_TIMER_PERIOD_1000MS = 0;//set_flag(flag_timer_period_1000ms ,DISABLE);

        printf("%s(timer) : %4d\r\n",__FUNCTION__,LOG1++);
        PB14 ^= 1;        
    }
}

void UARTx_Process(void)
{
	uint8_t res = 0;
	res = UART_READ(UART0);

	if (res > 0x7F)
	{
		printf("invalid command\r\n");
	}
	else
	{
		printf("press : %c\r\n" , res);
		switch(res)
		{
			case '1':
				break;

			case 'X':
			case 'x':
			case 'Z':
			case 'z':
                SYS_UnlockReg();
				// NVIC_SystemReset();	// Reset I/O and peripherals , only check BS(FMC_ISPCTL[1])
                // SYS_ResetCPU();     // Not reset I/O and peripherals
                SYS_ResetChip();    // Reset I/O and peripherals ,  BS(FMC_ISPCTL[1]) reload from CONFIG setting (CBS)	
				break;
		}
	}
}

void UART02_IRQHandler(void)
{
    if(UART_GET_INT_FLAG(UART0, UART_INTSTS_RDAINT_Msk | UART_INTSTS_RXTOINT_Msk))     /* UART receive data available flag */
    {
        while(UART_GET_RX_EMPTY(UART0) == 0)
        {
			UARTx_Process();
        }
    }

    if(UART0->FIFOSTS & (UART_FIFOSTS_BIF_Msk | UART_FIFOSTS_FEF_Msk | UART_FIFOSTS_PEF_Msk | UART_FIFOSTS_RXOVIF_Msk))
    {
        UART_ClearIntFlag(UART0, (UART_INTSTS_RLSINT_Msk| UART_INTSTS_BUFERRINT_Msk));
    }	
}

void UART0_Init(void)
{
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);
    UART_EnableInt(UART0, UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk);
    NVIC_EnableIRQ(UART02_IRQn);
	
	#if (_debug_log_UART_ == 1)	//debug
	dbg_printf("\r\nCLK_GetCPUFreq : %8d\r\n",CLK_GetCPUFreq());
	dbg_printf("CLK_GetHCLKFreq : %8d\r\n",CLK_GetHCLKFreq());
	dbg_printf("CLK_GetHXTFreq : %8d\r\n",CLK_GetHXTFreq());
	dbg_printf("CLK_GetLXTFreq : %8d\r\n",CLK_GetLXTFreq());	
	dbg_printf("CLK_GetPCLK0Freq : %8d\r\n",CLK_GetPCLK0Freq());
	dbg_printf("CLK_GetPCLK1Freq : %8d\r\n",CLK_GetPCLK1Freq());	
	#endif	

    #if 0
    dbg_printf("FLAG_PROJ_TIMER_PERIOD_1000MS : 0x%2X\r\n",FLAG_PROJ_TIMER_PERIOD_1000MS);
    dbg_printf("FLAG_PROJ_REVERSE1 : 0x%2X\r\n",FLAG_PROJ_REVERSE1);
    dbg_printf("FLAG_PROJ_REVERSE2 : 0x%2X\r\n",FLAG_PROJ_REVERSE2);
    dbg_printf("FLAG_PROJ_REVERSE3 : 0x%2X\r\n",FLAG_PROJ_REVERSE3);
    dbg_printf("FLAG_PROJ_REVERSE4 : 0x%2X\r\n",FLAG_PROJ_REVERSE4);
    dbg_printf("FLAG_PROJ_REVERSE5 : 0x%2X\r\n",FLAG_PROJ_REVERSE5);
    dbg_printf("FLAG_PROJ_REVERSE6 : 0x%2X\r\n",FLAG_PROJ_REVERSE6);
    dbg_printf("FLAG_PROJ_REVERSE7 : 0x%2X\r\n",FLAG_PROJ_REVERSE7);
    #endif

}

void GPIO_Init (void)
{
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB14MFP_Msk)) | (SYS_GPB_MFPH_PB14MFP_GPIO);
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB15MFP_Msk)) | (SYS_GPB_MFPH_PB15MFP_GPIO);

    GPIO_SetMode(PB, BIT14, GPIO_MODE_OUTPUT);
    GPIO_SetMode(PB, BIT15, GPIO_MODE_OUTPUT);	

}

void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Set XT1_OUT(PF.2) and XT1_IN(PF.3) to input mode */
//    PF->MODE &= ~(GPIO_MODE_MODE2_Msk | GPIO_MODE_MODE3_Msk);
    
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

//    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);
//    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);

//    CLK_EnableXtalRC(CLK_PWRCTL_LIRCEN_Msk);
//    CLK_WaitClockReady(CLK_STATUS_LIRCSTB_Msk);	

//    CLK_EnableXtalRC(CLK_PWRCTL_LXTEN_Msk);
//    CLK_WaitClockReady(CLK_STATUS_LXTSTB_Msk);	

    /* Select HCLK clock source as HIRC and HCLK source divider as 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));

    /***********************************/
    // CLK_EnableModuleClock(TMR0_MODULE);
  	// CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HIRC, 0);

    CLK_EnableModuleClock(TMR1_MODULE);
  	CLK_SetModuleClock(TMR1_MODULE, CLK_CLKSEL1_TMR1SEL_HIRC, 0);
    
	/***********************************/
    CLK_EnableModuleClock(UART0_MODULE);
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));
	
    /* Set PB multi-function pins for UART0 RXD=PB.12 and TXD=PB.13 */
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB12MFP_Msk | SYS_GPB_MFPH_PB13MFP_Msk)) |
                    (SYS_GPB_MFPH_PB12MFP_UART0_RXD | SYS_GPB_MFPH_PB13MFP_UART0_TXD);

	/***********************************/
   /* Update System Core Clock */
    SystemCoreClockUpdate();

    /* Lock protected registers */
    SYS_LockReg();
}

/*
 * This is a template project for M031 series MCU. Users could based on this project to create their
 * own application without worry about the IAR/Keil project settings.
 *
 * This template application uses external crystal as HCLK source and configures UART0 to print out
 * "Hello World", users may need to do extra system configuration based on their system design.
 */

int main()
{
    SYS_Init();

	GPIO_Init();
	UART0_Init();
	TIMER1_Init();
    check_reset_source();

    SysTick_enable(1000);
    #if defined (ENABLE_TICK_EVENT)
    TickSetTickEvent(1000, TickCallback_processA);  // 1000 ms
    TickSetTickEvent(5000, TickCallback_processB);  // 5000 ms
    #endif


    linear11_example();

    /* Got no where to go, just loop forever */
    while(1)
    {
        loop();

    }
}

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
