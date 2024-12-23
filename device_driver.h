#include "stm32f10x.h"
#include "option.h"
#include "macro.h"
#include "malloc.h"

// Uart.c

#define Uart_Init			Uart1_Init
#define Uart_Send_Byte		Uart1_Send_Byte
#define Uart_Send_String	Uart1_Send_String
#define Uart_Printf			Uart1_Printf

extern void Uart1_Init(int baud);
extern void Uart1_Send_Byte(char data);
extern void Uart1_Send_String(char *pt);
extern void Uart1_Printf(char *fmt,...);
extern char Uart1_Get_Char(void);
extern char Uart1_Get_Pressed(void);
extern void Uart1_RX_Interrupt_Enable(int en);
extern void Uart1_EchoBack(void);

// Led.c

extern void LED_Init(void);
extern void LED_Display(unsigned int num);
extern void LED_All_On(void);
extern void LED_All_Off(void);

// Clock.c

extern void Clock_Init(void);

// Key.c

extern void Key_Poll_Init(void);
extern int Key_Get_Pressed(void);
extern void Key_Wait_Key_Released(void);
extern int Key_Wait_Key_Pressed(void);
extern void Key_ISR_Enable(int en);

// SysTick.c

extern void SysTick_Run(unsigned int msec);
extern int SysTick_Check_Timeout(void);
extern unsigned int SysTick_Get_Time(void);
extern unsigned int SysTick_Get_Load_Time(void);
extern void SysTick_Stop(void);
extern void SysTick_OS_Tick(unsigned int msec);

// Timer.c

extern void TIM2_Delay(int time);
extern void TIM2_Stopwatch_Start(void);
extern unsigned int TIM2_Stopwatch_Stop(void);
extern void TIM4_Repeat(int time);
extern int TIM4_Check_Timeout(void);
extern void TIM4_Stop(void);
extern void TIM4_Change_Value(int time);
extern void TIM3_Out_Init(void);
extern void TIM3_Out_Freq_Generation(unsigned short freq);
extern void TIM3_Out_Stop(void);
extern void TIM3_Delay(int time);

// Moter.c

void device_init();
void device_execute(char command, int speed);
void Motor_Stop(void);
void Motor_Forward(int speed);
void Motor_Reserve(int speed);
void device_cds_execute(void);
int get_Adc_Value_Level(void);
void control_led_with_SPI(int speed);

//adc.c
void Adc_Cds_Init(void);
void Cds_Start(void);
void Cds_Stop(void);
int Cds_Get_Status(void);
int Cds_Get_Data(void);

//spi.c

#define SC16IS752_IODIR				0x0A
#define SC16IS752_IOSTATE			0x0B
extern void SPI_SC16IS752_Init(unsigned int div);
extern void SPI_SC16IS752_Write_Reg(unsigned int addr, unsigned int data);
extern void SPI_SC16IS752_Config_GPIO(unsigned int config);
extern void SPI_SC16IS752_Write_GPIO(unsigned int data);
