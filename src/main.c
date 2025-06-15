#include "device_driver.h"

char received_command;
int speed_level = 50;  // 기본 속도 (%)
extern volatile int Uart1_Rx_In;
extern volatile int Uart1_Rx_Data;
int control_mode = 0;  // 2: FRS 모드, 1: CDS 모드
int print_speed_level;

void Main(void) {
  device_init();
  Uart1_Printf("Motor Control System Ready\n");
  static int prev_keystate = 4;
  for (;;) {
    int keystate = Key_Get_Pressed();  // KEY1을 누르면 2, KEY0누르면 1

    if (keystate != prev_keystate)  // 키 상태가 변경된 경우만 실행
    {
      prev_keystate = keystate;  // 현재 상태를 저장
      if (keystate == 2) {
        control_mode = 2;  // KEY1:F,R,Smode
        Uart1_Printf("Mode: FRS (Terminal Control)\n");
      } else if (keystate == 1) {
        control_mode = 1;  //  KEY0:CDSmode
        Uart1_Printf("Mode: CDS (Light Sensor Control)\n");
      }
    }

    switch (control_mode) {
      case 1:
        device_cds_execute();
        break;

      case 2:
        if (Uart1_Rx_In) {
          received_command = Uart1_Rx_Data;
          Uart1_Rx_In = 0;
          if (received_command >= '1' && received_command <= '8') {
            speed_level = ((received_command - '0') * 5);
            print_speed_level = speed_level * 2;
            Uart1_Printf("Speed Level: %d\n", print_speed_level);
          } else {
            device_execute(received_command, speed_level);
            if (received_command != 'S') {
              control_led_with_SPI(speed_level);
            }
          }
        }
        break;

      default:
        Motor_Stop();
        break;
    }
  }
}
