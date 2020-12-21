#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

// For Timer
#include "esp_types.h"
#include "freertos/queue.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"

//You can get these value from the datasheet of servo you use, in general pulse width varies between 1000 to 2000 mocrosecond
#define LED_MIN_PULSEWIDTH 0 //Minimum pulse width in microsecond
#define LED_MAX_PULSEWIDTH 90 //Maximum pulse width in microsecond
#define INTENSITY_TIME_PERIOD 250 //intensity time period = 250ms

// For RTOS
#define MAX_PRIORITIES                     1  // max priority for RTOS tasks

// For TIMER
#define TIMER_DIVIDER         16    //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // to seconds
#define TIMER_INTERVAL_SEC   (0.25)    // Sample test interval for the first timer
#define TEST_WITH_RELOAD      1     // Testing will be done with auto reload

bool flag = false;

static void mcpwm_example_gpio_initialize(void)
{
    printf("initializing mcpwm LED control gpio......\n");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, 18);    //Set GPIO 18 as PWM0A, to which LED is connected
}

void pwm_init(){
  //1. mcpwm gpio initialization
  mcpwm_example_gpio_initialize();

  //2. initial mcpwm configuration
  printf("Configuring Initial Parameters of mcpwm......\n");
  mcpwm_config_t pwm_config;
  pwm_config.frequency = 1000;    //frequency = 1k Hz, i.e. for every pwm time period should be 1ms
  pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
  pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
  pwm_config.counter_mode = MCPWM_UP_COUNTER;
  // pwm_config.duty_mode = MCPWM_DUTY_MODE_1;
  mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings
}

// Configure MCPWM module
void mcpwm_example_led_control(void *arg)
{
    while (1) {
        printf("LED Intensity Up.\n");
        int d = LED_MIN_PULSEWIDTH;
        while (d < LED_MAX_PULSEWIDTH) {
            if (flag){
              d+= 10;
              printf("Intensity Level = %d\n", d/10);
              mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, d);
              flag = false;
            }
        }
        printf("LED Intensity Down.\n");
        while (d > LED_MIN_PULSEWIDTH) {
            if (flag){
              d-=10;
              printf("Intensity Level = %d\n", d/10);
              mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, d);
              flag = false;
            }
        }
    }
}

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
            flag = true;
        }
    }
}
////////////////////////////////////////////////////////////////////////////////


void app_main(void)
{
  pwm_init();
  // Create a FIFO queue for timer-based
  timer_queue = xQueueCreate(10, sizeof(timer_event_t));

  xTaskCreate(mcpwm_example_led_control, "mcpwm_example_led_control", 4096, NULL, MAX_PRIORITIES-1, NULL);
  xTaskCreate(timer_evt_task, "timer_evt_task", 2048, NULL, 1, NULL);   // task for timer-based events
  alarm_init();   // Initiate alarm using timer API
}
