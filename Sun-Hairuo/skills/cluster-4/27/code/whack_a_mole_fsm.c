#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

// For UART
#include <string.h>
#include "driver/uart.h"
#include "esp_vfs_dev.h"

// For Timer
#include "esp_types.h"
#include "freertos/queue.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"

// For RTOS
#define MAX_PRIORITIES                     2  // max priority for RTOS tasks

// For TIMER
#define TIMER_DIVIDER         16    //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // to seconds
#define TIMER_INTERVAL_SEC    (1)    // Sample test interval for the first timer
#define TEST_WITH_RELOAD      1     // Testing will be done with auto reload
#define MAX_COUNT             100

// For FSM
// states
#define GAMEOVER_STATE             0
#define IDLE_STATE            1
#define MOLE_APPEAR_STATE     2
// events
// #define EVENT_START           0
// #define EVENT_HIT             1
// timeouts
#define GAME_TIMEOUT            20
#define IDLE_TIMEOUT            2
#define MOLE_TIMEOUT            5

// Global variables
// For FSM
int state = GAMEOVER_STATE;
int game_timer = GAME_TIMEOUT;
int idle_timer = IDLE_TIMEOUT;
int mole_timer = MOLE_TIMEOUT;
int score = 0;
bool game_started_flag = false;


// TIMER
////////////////////////////////////////////////////////////////////////////////
// A simple structure to pass "events" to main task
typedef struct {
    int flag;     // flag for enabling stuff in main code
} timer_event_t;

// Initialize queue handler for timer-based events
xQueueHandle timer_queue;

// ISR handler
void IRAM_ATTR timer_group0_isr(void *para) {

    // Prepare basic event data, aka set flag
    timer_event_t evt;
    evt.flag = 1;

    // Clear the interrupt, Timer 0 in group 0
    TIMERG0.int_clr_timers.t0 = 1;

    // After the alarm triggers, we need to re-enable it to trigger it next time
    TIMERG0.hw_timer[TIMER_0].config.alarm_en = TIMER_ALARM_EN;

    // Send the event data back to the main program task
    xQueueSendFromISR(timer_queue, &evt, NULL);
}

// Initialize timer 0 in group 0 for 1 sec alarm interval and auto reload
static void alarm_init() {
    /* Select and initialize basic parameters of the timer */
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
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, TIMER_INTERVAL_SEC * TIMER_SCALE);
    timer_enable_intr(TIMER_GROUP_0, TIMER_0);
    timer_isr_register(TIMER_GROUP_0, TIMER_0, timer_group0_isr,
        (void *) TIMER_0, ESP_INTR_FLAG_IRAM, NULL);

    // Start timer
    timer_start(TIMER_GROUP_0, TIMER_0);
}

// The main task of this example program
static void timer_evt_task(void *arg) {
    while (1) {
        // Create dummy structure to store structure from queue
        timer_event_t evt;

        // Transfer from queue
        xQueueReceive(timer_queue, &evt, portMAX_DELAY);

        // Do something if triggered!
        if (evt.flag == 1) {
            // printf("Action!\n");
            game_timer -= 1;
            if (game_started_flag && (game_timer < 3))
              printf("\nTime Left: %d\n", game_timer);
            idle_timer -= 1;
            mole_timer -= 1;
        }
    }
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// FSM Functions
void fsm_task(){
  while (1) {

    // 1. GAMEOVER_STATE
    if (state == GAMEOVER_STATE){
      //printf("\nDEBUG - GAMEOVER_STATE!\n");
      game_started_flag = false;
      while (1){
        printf("\nDo you want to start the game? (y/n):");  // print input prompt
        char buf[3];   // ??? modification - Can I get a single char using buf like this?
        gets(buf);  // get event: use get() function to READ INPUT from UART

        // start Game
        if (buf[0] == 'y'){
          game_started_flag = true;
          printf("\nGame Starts!\n");
          break;
        }

        vTaskDelay(99 / portTICK_PERIOD_MS);
      }

      state = IDLE_STATE;             // next state
      game_timer = GAME_TIMEOUT;      // start new GAME timer countdown
      score = 0;                      // new score
      printf("\n GAME STARTED!");
      printf("\n Total Game Time = %d seconds.\n", game_timer);
    }

    // 2. IDLE_STATE
    else if (state == IDLE_STATE) {
      //printf("\nDEBUG - IDLE_STATE!\n");
      idle_timer = IDLE_TIMEOUT;      // re-initialize idle_timer

      // wait for idle_timer/game_timer timeout
      while (idle_timer > 0){
        // game time & idle time conditions
        if (game_timer <= 0){             // GAME OVER
          printf("\nGAME OVER!\n");
          break;
        }
        // printf("\nDEBUG - idle_timer: %d\n", idle_timer);
        vTaskDelay(99 / portTICK_PERIOD_MS);
      }


      // assign new states
      if (game_timer <= 0){
        state = GAMEOVER_STATE;
        printf("\n FINAL SCORE: %d", score);
      }
      else // if (idle_timer <= 0)
        state = MOLE_APPEAR_STATE;
    }

    // 3. MOLE_APPEAR_STATE
    else if (state == MOLE_APPEAR_STATE){
      //printf("\nDEBUG - MOLE_APPEAR_STATE!\n");
      mole_timer = MOLE_TIMEOUT;  // re-initialize mole_timer
      printf("\nA mole appear!\n\n");
      printf("\nHit the mole within 5 seconds to get 10 points (h: hit):");

      while (mole_timer > 0){
        if (game_timer <= 0){break;}    // check game time

        char buf[3];   // ??? modification - Can I get a single char using buf like this?
        gets(buf);  // get event: use get() function to READ INPUT from UART
        if (buf[0] == 'h'){                                    // mole is hit
          printf("\n\nGood job! A mole is hit!\n");
          score += 10;
          printf("NEW SCORE: %d\n", score);
          break;
        }

        printf("\nMole Not Hit, Try again (h: hit):");     // mole not hit
      }

      // set next state
      if (game_timer == 0){
        state = GAMEOVER_STATE;
        printf("\n FINAL SCORE: %d\n", score);
      }
      else{
        state = IDLE_STATE;
        idle_timer = IDLE_TIMEOUT;        // re-initialize idle_timer
      }
    } // end of state 3 if block


  }   // end of while loop
}

////////////////////////////////////////////////////////////////////////////////

void init(){
  /* Install UART driver for interrupt-driven reads and writes */
  ESP_ERROR_CHECK( uart_driver_install(UART_NUM_0, 256, 0, 0, NULL, 0) );
  esp_vfs_dev_uart_use_driver(UART_NUM_0);  // Tell VFS to use UART driver

}

void app_main() {
  init();

  timer_queue = xQueueCreate(10, sizeof(timer_event_t));  // Create a FIFO queue for timer-based
  xTaskCreate(timer_evt_task, "timer_evt_task", 2048, NULL, 1, NULL); // Create task to handle timer-based events
  alarm_init(); // Initiate alarm using timer API
  xTaskCreate(fsm_task, "FSM task", 2048, NULL, 0, NULL);

}
