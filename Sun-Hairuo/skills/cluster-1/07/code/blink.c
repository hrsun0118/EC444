/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

/* Can use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define BLINK_GPIO1 14  // LED 1: green
#define BLINK_GPIO2 32  // LED 2: blue
#define BLINK_GPIO3 15  // LED 3: red
#define BLINK_GPIO4 33  // LED 4: yellow

void LED_pattern(){
  for(int num = 0; num < 16; num++){  // integer pattern number: 0 - 15
    int b_num[] = {0,0,0,0};
    int i = 0;  // used for indexing binary array b_num[]
    int temp_num = num;   // used for calculating binary number
    while (temp_num > 0){  // convert number i from int to binary & store it in a char[] array
      b_num[i] = (temp_num % 2);
      temp_num /= 2;
      i++;
    }
    printf("\nNumber: %d", num);
    printf("\nBinary number: %d%d%d%d", b_num[3], b_num[2], b_num[1], b_num[0]);
    gpio_set_level(BLINK_GPIO1, b_num[0]);
    gpio_set_level(BLINK_GPIO2, b_num[1]);
    gpio_set_level(BLINK_GPIO3, b_num[2]);
    gpio_set_level(BLINK_GPIO4, b_num[3]);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void app_main(void)
{
    /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. Consult the
       Technical Reference for a list of pads and their default
       functions.)
    */
    gpio_reset_pin(BLINK_GPIO1);
    gpio_reset_pin(BLINK_GPIO2);
    gpio_reset_pin(BLINK_GPIO3);
    gpio_reset_pin(BLINK_GPIO4);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO1, GPIO_MODE_OUTPUT);
    gpio_set_direction(BLINK_GPIO2, GPIO_MODE_OUTPUT);
    gpio_set_direction(BLINK_GPIO3, GPIO_MODE_OUTPUT);
    gpio_set_direction(BLINK_GPIO4, GPIO_MODE_OUTPUT);
    while(1) {
      LED_pattern();
    }
}
