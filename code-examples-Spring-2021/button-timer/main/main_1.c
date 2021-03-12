/* Button Interrupt Example #1 - The Simple Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

// Hardware interrupt definitions
#define GPIO_INPUT_IO_1       4
#define ESP_INTR_FLAG_DEFAULT 0
#define GPIO_INPUT_PIN_SEL    1ULL<<GPIO_INPUT_IO_1


int flag = 0;     // Flag for signaling from ISR

// Button interrupt handler
static void IRAM_ATTR gpio_isr_handler(void* arg){
  flag = 1;
}

// Button interrupt init
static void button_init() {
  gpio_config_t io_conf;
  //interrupt of rising edge
  io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
  //bit mask of the pins, use GPIO4 here
  io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
  //set as input mode
  io_conf.mode = GPIO_MODE_INPUT;
  //enable pull-up mode
  io_conf.pull_up_en = 1;
  gpio_config(&io_conf);
  gpio_intr_enable(GPIO_INPUT_IO_1 );
  //install gpio isr service
  gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);
  //hook isr handler for specific gpio pin
  gpio_isr_handler_add(GPIO_INPUT_IO_1, gpio_isr_handler, (void*) GPIO_INPUT_IO_1);
}

// Button task
void button_task(){
  while(1) {
    if(flag) {
      printf("Button pressed.\n");
      flag = 0;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }

}


void app_main(void)
{
  // Initialize button config 
  button_init();

  // Create task for button
  xTaskCreate(button_task, "button_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
  printf("Everything initialized. Waiting for button presses...\n");
}
