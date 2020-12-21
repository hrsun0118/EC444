#include <stdio.h>
// For UART
#include <string.h>
#include "driver/uart.h"
#include "esp_vfs_dev.h"
// For blink & echo
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#define BLINK_GPIO 13

void printPrompt(char mode){
  // print "prompt" for different mode
  if (mode == 't')                // toggle mode
    printf("Read: ");
  else if (mode == 'h')           // int2hex mode
    printf("Enter an integer: ");
  else                            // echo mode
    printf("echo:");
}

char switch_Mode(char mode){
  printf("s\n");
  if (mode == 't'){  // toggle mode --> echo mode
    mode = 'e';
    printf("echo mode\n");
  }
  else if (mode == 'e'){  // echo mode --> int2hex mode
    mode = 'h';
    printf("echo dec to hex mode\n");
  }
  else{  // int2hex mode --> toggle mode
    mode = 't';
    printf("toggle mode\n");
  }
  return mode;
}

void toggle_LED(int pinout){
  gpio_set_level(BLINK_GPIO, pinout);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}

int toggle_Mode(int pinout) {
  printf("t");
  if (pinout == 0)
    pinout = 1;
    else
    pinout = 0;
  toggle_LED(pinout);
  printf("\n");
  return pinout;
}

bool isInt(char *buf){
  for (int i = 0; buf[i] != '\0'; i++){
    if (!(buf[i] >= '0' && buf[i] <= '9'))
      return false;
  }
  return true;
}

void int2hex_Mode(char *buf){
  if (buf[0] == '\0' || !isInt(buf))  // if buf empty || buf is not an integer
    printf("\n");
  else   // if this is a valid digit
    printf("\n\nHex: %x\n", atoi(buf));
}

void app_main()
{
    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK( uart_driver_install(UART_NUM_0, 256, 0, 0, NULL, 0) );
    esp_vfs_dev_uart_use_driver(UART_NUM_0);  // Tell VFS to use UART driver
    gpio_reset_pin(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    char mode = 't';  // default: toggle mode
    printf("toggle mode\n");
    int pinout = 0;   // declare & initialize LED pin_out

    // while loop
    while(1) {
      printPrompt(mode);  // print input prompt

      char buf[5];
      gets(buf);  // use get() function to READ INPUT from UART

      // switch mode
      if (buf[0] == 's' && buf[1] == '\0'){
        mode = switch_Mode(mode);
        continue;
      }

      // execute instructions based on mode
      if (mode == 't' && (*buf) == 't' && buf[1] == '\0') {      // toggle_LED w/ input = 't'
        // printf("toggle_LED");
        pinout = toggle_Mode(pinout);
      }
      else if (mode == 'e'){  // mode 1: echo string
        printf("%s\n", buf);
      }
      else if (mode == 'h') {  // int2hex mode
        int2hex_Mode(buf);
      }
      else  // print a newline if in toggle mode & the input isn't right
        printf("\n");
    }
}
