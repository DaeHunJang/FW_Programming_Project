#include <device_driver.h>

#define TIM2_FREQUENCY 72000000  // 타이머 클럭 (72MHz)
#define MOTOR_PWM_FREQ 100000    // PWM 주파수 (10kHz)

int prev_value_level = 0;
int prev_speed_level = 0;

void device_init() {
  Uart1_Init(115200);
  Uart1_RX_Interrupt_Enable(1);

  Clock_Init();
  Key_Poll_Init();  // KEY0,1 input floating init
  Adc_Cds_Init();   // CDS init
  TIM3_Out_Init();

  SCB->VTOR = 0x08003000;
  SCB->SHCSR = 0;

  RCC->APB1ENR |= (1 << 0);  // TIM2 클럭 활성화

  // TIM2 PWM 설정
  TIM2->PSC = (TIM2_FREQUENCY / MOTOR_PWM_FREQ) - 1;  // 프리스케일러 설정
  TIM2->ARR = 100;  // 자동 리로드 값 (듀티 사이클 0~100%)
  TIM2->CCR3 = 0;   // 채널 3 (PA2) 듀티 사이클 초기화
  TIM2->CCR4 = 0;   // 채널 4 (PA3) 듀티 사이클 초기화
  TIM2->CCMR2 |= (0x6 << 4) | (0x6 << 12);  // CH3, CH4: PWM 모드 1
  TIM2->CCER |= (1 << 8) | (1 << 12);
  TIM2->CR1 |= (1 << 0);  // TIM2 활성화

  // PortA Clock ON
  Macro_Set_Bit(RCC->APB2ENR, 2);
  // PA1(EN),PA2(1A),PA3(2A) ->General Purpose Output Push-Pull
  Macro_Write_Block(GPIOA->CRL, 0xff, 0xBB, 8);

  // SPI SC16IS752 Init
  SPI_SC16IS752_Init(32);  // PCLK2(36MHz)/32 = 1.125MHz (주파수 값이 아닌 분주비 전달)
  SPI_SC16IS752_Config_GPIO(0xFF);
}

void device_execute(char command, int speed) {
  switch (command) {
    case 'S':  // 정지
      Motor_Stop();
      Uart1_Printf("Motor Stopped\n");
      prev_speed_level = 1;
      break;
    case 'F':  // 정방향 회전
      // 이전 상황 체크 후 역방향이면 잠깐 멈췄다가 정방향
      if (prev_speed_level == 3) {
        Uart1_Printf("Motor Stopped Momentarily Reverse to Forward\n");
        Motor_Stop();
      }
      Motor_Forward(speed);
      Uart1_Printf("Motor Forward\n");
      prev_speed_level = 2;
      break;
    case 'R':  // 역방향 회전
      // 이전 상황 체크 후 정방향이면 잠깐 멈췄다가 역방향
      if (prev_speed_level == 2) {
        Uart1_Printf("Motor Stopped Momentarily Forward to Reverse\n");
        Motor_Stop();
      }
      Motor_Reserve(speed);
      Uart1_Printf("Motor Reverse\n");
      prev_speed_level = 3;
      break;
  }
}

void Motor_Stop(void) {
  TIM2->CCR3 = 0;  // PA2 (CH3) Low
  TIM2->CCR4 = 0;  // PA3 (CH4) Low
  control_led_with_SPI(0);
}

void Motor_Forward(int speed) {
  TIM2->CCR3 = speed + 30;  // PA2 (CH3): 속도 설정
  TIM2->CCR4 = 0;           // PA3 (CH4): Low
}
void Motor_Reserve(int speed) {
  TIM2->CCR3 = 0;           // PA2 (CH3): Low
  TIM2->CCR4 = speed + 30;  // PA3 (CH4): 속도 설정
}
extern int control_mode;

int get_Adc_Value_Level(void) {
  Cds_Start();                // CDS start
  while (!Cds_Get_Status());  // CDS 변화감지
  // 최소 0x0122 최대 0x0E3A
  if (Cds_Get_Data() >= 0x000 && Cds_Get_Data() < 0x500) {
    return 1;
  } else if (Cds_Get_Data() >= 0x500 && Cds_Get_Data() < 0x800) {
    return 2;
  } else if (Cds_Get_Data() >= 0x800 && Cds_Get_Data() < 0xB00) {
    return 3;
  } else
    return 4;
}

void device_cds_execute(void) {
  int adc_value_level = get_Adc_Value_Level();
  static int adc_value_count = 0;
  switch (adc_value_level) {
    case 1:
      if (prev_value_level != adc_value_level) {
        Uart1_Printf("CDS_Mode 1_Level\n");
        prev_value_level = adc_value_level;
      }
      Motor_Forward(15);  // 속도 30%로 모터 동작
      TIM3_Delay(30);     // 3초 지나면 모터 스탑되게 한다.
      adc_value_count++;
      if (adc_value_count >= 100) {
        Motor_Stop();
        control_mode = 2;  // mode 변경
      }
      break;

    case 2:
      if (prev_value_level != adc_value_level) {
        Uart1_Printf("CDS_Mode 2_Level\n");
        prev_value_level = adc_value_level;
      }
      Motor_Forward(25);  // 속도 50%로 모터 동작
      adc_value_count = 0;
      break;

    case 3:
      if (prev_value_level != adc_value_level) {
        Uart1_Printf("CDS_Mode 3_Level\n");
        prev_value_level = adc_value_level;
      }
      Motor_Forward(35);  // 속도 70%로 모터 동작
      adc_value_count = 0;
      break;

    case 4:
      if (prev_value_level != adc_value_level) {
        Uart1_Printf("CDS_Mode 4_Level\n");
        prev_value_level = adc_value_level;
      }
      Motor_Forward(45);  // 속도 90%로 모터 동작
      adc_value_count = 0;
      break;
  }
}

void control_led_with_SPI(int speed) {
  int data = ~((1u << ((speed / 5))) - 1);
  SPI_SC16IS752_Write_GPIO(data);
}