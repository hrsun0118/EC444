/* Button Interrupt Example #2 - The Timer & Queues Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/rmt.h"
#include "driver/timer.h"

#define TIMER_DIVIDER         16    //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // to seconds
#define TIMER_INTERVAL_1_SEC  (1)
#define TEST_WITH_RELOAD      1     // Testing will be done with auto reload

// Hardware interrupt definitions
#define GPIO_INPUT_IO_1       4
#define ESP_INTR_FLAG_DEFAULT 0
#define GPIO_INPUT_PIN_SEL    1ULL<<GPIO_INPUT_IO_1

// Queues
static xQueueHandle gpio_evt_queue = NULL;
static xQueueHandle timer_queue;

// Global vars
int increment = 1;     // Variable used to increment the displayed counter
int counter = 0;      // Variable to hold counter value

// Button interrupt handler -- add to button queue
static void IRAM_ATTR gpio_isr_handler(void* arg){
  uint32_t gpio_num = (uint32_t) arg;
  xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

// Timer interrupt handler -- add to timer queue
void IRAM_ATTR timer_group0_isr(void *para) {
  int evt;
  // Clear the interrupt, Timer 0 in group 0
  TIMERG0.int_clr_timers.t0 = 1;
  // After the alarm triggers, we need to re-enable it to trigger it next time
  TIMERG0.hw_timer[TIMER_0].config.alarm_en = TIMER_ALARM_EN;
  // Send the event data back to the main program task
  xQueueSendFromISR(timer_queue, &evt, NULL);
}

// Configure timer
static void alarm_init() {
  // Select and initialize basic parameters of the timer
  timer_config_t config;
  config.divider = TIMER_DIVIDER;
  config.counter_dir = TIMER_COUNT_UP;
  config.counter_en = TIMER_PAUSE;
  config.alarm_en = TIMER_ALARM_EN;
  config.intr_type = TIMER_INTR_LEVEL;
  config.auto_reload = TEST_WITH_RELOAD;
  timer_init(TIMER_GROUP_0, TIMER_0, &config);

  // Timer's counter will initially start from value below
  timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x00000000ULL);

  // Configure the alarm value and the interrupt on alarm
  timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, TIMER_INTERVAL_1_SEC * TIMER_SCALE);
  timer_enable_intr(TIMER_GROUP_0, TIMER_0);
  timer_isr_register(TIMER_GROUP_0, TIMER_0, timer_group0_isr,
      (void *) TIMER_0, ESP_INTR_FLAG_IRAM, NULL);

  // Start timer
  timer_start(TIMER_GROUP_0, TIMER_0);
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
  //create a queue to handle gpio event from isr
  gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
  //start gpio task
}

// Button task -- check for button press and change counting direction
void button_task(){
  uint32_t io_num;
  while(1) {
    if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
      printf("Button pressed - changing counting direction.\n");
      increment *= -1;  // Change the counting direction
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// Timer task -- receives something on queue every 1 second
static void timer_evt_task(void *arg) {
  while (1) {
    // Create dummy variable to store value from queue
    int evt;
    // Transfer from queue
    xQueueReceive(timer_queue, &evt, portMAX_DELAY);
    // Do something if triggered
    if (evt) {
      counter += increment;
      printf("Count: %d\n", counter);
    }
  }
}

void app_main(void)
{
  // Create a FIFO queue for timer-based events
  timer_queue = xQueueCreate(10, sizeof(uint32_t));

  // Initialize timer and button
  alarm_init();
  button_init();

  // Create task to handle timer-based events
  xTaskCreate(timer_evt_task, "timer_evt_task", 2048, NULL, 5, NULL);

  // Create task for button
  xTaskCreate(button_task, "button_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);

  printf("Everything initialized. Waiting for button presses...\n");
}
