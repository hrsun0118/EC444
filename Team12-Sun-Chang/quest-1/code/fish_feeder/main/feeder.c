#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"
#include "esp_vfs_dev.h"
#include "esp_attr.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

// For Timer
#include "esp_types.h"
#include "freertos/queue.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"

//defines for display
// 14-Segment Display
#define SLAVE_ADDR                         0x70 // alphanumeric address
#define OSC                                0x21 // oscillator cmd
#define HT16K33_BLINK_DISPLAYON            0x01 // Display on cmd
#define HT16K33_BLINK_OFF                  0    // Blink off cmd
#define HT16K33_BLINK_CMD                  0x80 // Blink cmd
#define HT16K33_CMD_BRIGHTNESS             0xE0 // Brightness cmd

// Master I2C
#define I2C_EXAMPLE_MASTER_SCL_IO          22   // gpio number for i2c clk
#define I2C_EXAMPLE_MASTER_SDA_IO          23   // gpio number for i2c data
#define I2C_EXAMPLE_MASTER_NUM             I2C_NUM_0  // i2c port
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE  0    // i2c master no buffer needed
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE  0    // i2c master no buffer needed
#define I2C_EXAMPLE_MASTER_FREQ_HZ         100000     // i2c master clock freq
#define WRITE_BIT                          I2C_MASTER_WRITE // i2c master write
#define READ_BIT                           I2C_MASTER_READ  // i2c master read
#define ACK_CHECK_EN                       true // i2c master will check ack
#define ACK_CHECK_DIS                      false// i2c master will not check ack
#define ACK_VAL                            0x00 // i2c ack value
#define NACK_VAL                           0xFF // i2c nack value
/////////////////////////////////////////////////////

// Defines for the servos
#define SERVO_MIN_PULSEWIDTH 550  //Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH 2700 //Maximum pulse width in microsecond
#define SERVO_MAX_DEGREE 120      //Maximum angle in degree upto which servo can rotate

//////////////////////////////////////////////////////
// For TIMER
#define TIMER_DIVIDER         16    //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // to seconds
#define TIMER_INTERVAL_SEC   (1)    // Sample test interval for the first timer
#define TEST_WITH_RELOAD      1     // Testing will be done with auto reload
#define MAX_COUNT             100
#define MAX_PRIORITIES        2

// Font table for the alphanumeric display
uint16_t font_table(int num) {
    uint16_t fonttable[10];
    fonttable[0] = 0b0000110000111111; // 0
    fonttable[1] = 0b0000000000000110; // 1
    fonttable[2] = 0b0000000011011011; // 2
    fonttable[3] = 0b0000000010001111; // 3
    fonttable[4] = 0b0000000011100110; // 4
    fonttable[5] = 0b0010000001101001; // 5
    fonttable[6] = 0b0000000011111101; // 6
    fonttable[7] = 0b0000000000000111; // 7
    fonttable[8] = 0b0000000011111111; // 8
    fonttable[9] = 0b0000000011101111; // 9
    return fonttable[num];
}

// Global variables
char hour[2];
char minute[2];
int int_hour = 0;
int int_minute = 0;

// For Servo
int tot_sec = 0;   // total seconds - calculated from input = cannot be changed
int tot_sec_cd = 0;  //total seconds countdown - updated every second; once reaching 0, restart everything
bool servo_flag = false;  // servo flag to indicate alarm for servo to spin
bool input_flag = false;

// timer count countdown display: hr, min, sec
int timer_hr = 0;
int timer_min = 0;
int timer_sec = 0;

// Checks if input are integers between 0 and 9
bool isitdigit(char str[2])
{
    int i = 0;
    int counter = 0;
    for(i = 0; i < 2 ; i++) {
        if ((str[i] >= '0' && str[i] <= '9') || (str[i] == '\0')){
            counter++;
        }
    }
    if(counter == 2) {
        return true;
    } else {
        return false;
    }
}


static void set_time() {
    // Asks the user to enter the hour
    printf(">> Enter the number of hours: \n");
    gets(hour);
    printf("%s\n", hour);
    while(isitdigit(hour) == false || atoi(hour) > 24) { // Error check the input
        if(isitdigit(hour) == false) {
            printf("Please enter numbers: \n");
            gets(hour);
            printf("%s\n", hour);
        }
        else if(atoi(hour) > 24) {
            printf("Please enter a number less than or equal to 24: \n");
            gets(hour);
            printf("%s\n", hour);
        }
    }
    printf("Hour is set to %s.\n", hour);
    int_hour = atoi(hour);

    // Asks the user to enter the minute
    printf(">> Enter the number of minute: \n");
    gets(minute);
    printf("%s\n", minute);
    while(isitdigit(minute) == false || atoi(minute) > 59) { // Error check the input
        if(isitdigit(minute) == false) {
            printf("Please enter numbers: \n"); // asks user for correct input on a loop
            gets(minute);
            printf("%s\n", minute);
        } else if( (int_hour == 24 && atoi(minute) != 0) || atoi(minute) > 59) {
            printf("Please enter a number less than 60: \n");
            gets(minute);
            printf("%s\n", minute);
        }
    }
    printf("Minute is set to %s.\n", minute);
    int_minute = atoi(minute);
    printf("Time is set to %02d:%02d.\n", int_hour, int_minute);

    // For SERVO
    tot_sec = int_hour * 3600 + int_minute * 60;    // calculate total second // need to be un-commented later
    // tot_sec = 15;  // ! need to commented out later
    tot_sec_cd = tot_sec;  // total second countdown
    input_flag = true;  // indicate inputs have been entered
}

// init for display
static void i2c_example_master_init(){
    // Debug
    int err;

    // Port configuration
    int i2c_master_port = I2C_EXAMPLE_MASTER_NUM;

    /// Define I2C configurations
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;                              // Master mode
    conf.sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO;              // Default SDA pin
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;                  // Internal pullup
    conf.scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO;              // Default SCL pin
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;                  // Internal pullup
    conf.master.clk_speed = I2C_EXAMPLE_MASTER_FREQ_HZ;       // CLK frequency
    err = i2c_param_config(i2c_master_port, &conf);           // Configure
    //if (err == ESP_OK) {printf("- parameters: ok\n");}

    // Install I2C driver
    err = i2c_driver_install(i2c_master_port, conf.mode,
                             I2C_EXAMPLE_MASTER_RX_BUF_DISABLE,
                             I2C_EXAMPLE_MASTER_TX_BUF_DISABLE, 0);
    // i2c_set_data_mode(i2c_master_port,I2C_DATA_MODE_LSB_FIRST,I2C_DATA_MODE_LSB_FIRST);
    //if (err == ESP_OK) {printf("- initialized: yes\n\n");}

    // Dat in MSB mode
    i2c_set_data_mode(i2c_master_port, I2C_DATA_MODE_MSB_FIRST, I2C_DATA_MODE_MSB_FIRST);
}

int testConnection(uint8_t devAddr, int32_t timeout) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (devAddr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    int err = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return err;
}

// Utility function to scan for i2c device
static void i2c_scanner() {
    int32_t scanTimeout = 1000;
    printf("\n>> I2C scanning ..."  "\n");
    uint8_t count = 0;
    for (uint8_t i = 1; i < 127; i++) {
        // printf("0x%X%s",i,"\n");
        if (testConnection(i, scanTimeout) == ESP_OK) {
            printf( "- Device found at address: 0x%X%s", i, "\n");
            count++;
        }
    }
    if (count == 0)
        printf("- No I2C devices found!" "\n");
    printf("\n");
}

// Turn on oscillator for alpha display
int alpha_oscillator() {
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, OSC, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    vTaskDelay(200 / portTICK_RATE_MS);
    return ret;
}
// Set blink rate to off for display
int no_blink() {
    int ret;
    i2c_cmd_handle_t cmd2 = i2c_cmd_link_create();
    i2c_master_start(cmd2);
    i2c_master_write_byte(cmd2, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd2, HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | (HT16K33_BLINK_OFF << 1), ACK_CHECK_EN);
    i2c_master_stop(cmd2);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd2, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd2);
    vTaskDelay(200 / portTICK_RATE_MS);
    return ret;
}
// Set Brightness for display
int set_brightness_max(uint8_t val) {
    int ret;
    i2c_cmd_handle_t cmd3 = i2c_cmd_link_create();
    i2c_master_start(cmd3);
    i2c_master_write_byte(cmd3, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd3, HT16K33_CMD_BRIGHTNESS | val, ACK_CHECK_EN);
    i2c_master_stop(cmd3);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd3, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd3);
    vTaskDelay(200 / portTICK_RATE_MS);
    return ret;
}

// main code for alphanumeric display
static void test_alpha_display() {
    int ret;
    int currenttime[] = {0,0,0,0};
    ret = alpha_oscillator();
    ret = no_blink();
    ret = set_brightness_max(0xF);

    uint16_t displaybuffer[8];

    while(1) {
        printf("The time left: %02d:%02d:%02d\n", timer_hr, timer_min, timer_sec);   // display on console
        currenttime[0] = ((int)(timer_hr / 10));
        currenttime[1] = timer_hr % 10;
        currenttime[2] = ((int)(timer_min / 10));
        currenttime[3] = timer_min % 10;

        // gets the correct mapping from the font table
        for (int i = 0; i <= 3; i++) {
            displaybuffer[i] = font_table(currenttime[i]);
        }

        // Send commands characters to display over I2C
        i2c_cmd_handle_t cmd4 = i2c_cmd_link_create();
        i2c_master_start(cmd4);
        i2c_master_write_byte(cmd4, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
        i2c_master_write_byte(cmd4, (uint8_t)0x00, ACK_CHECK_EN);
        for (uint8_t i=0; i<8; i++) {
            i2c_master_write_byte(cmd4, displaybuffer[i] & 0xFF, ACK_CHECK_EN);
            i2c_master_write_byte(cmd4, displaybuffer[i] >> 8, ACK_CHECK_EN);
        }
        i2c_master_stop(cmd4);
        ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd4, 1000 / portTICK_RATE_MS);
        i2c_cmd_link_delete(cmd4);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

////////////////////////////////////////////////////////////////////////////////
// SERVO Task
static void mcpwm_example_gpio_initialize(void)
{
    printf("initializing mcpwm servo control gpio......\n");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, 18);    //Set GPIO 18 as PWM0A, to which servo is connected
}

/**
 * @brief Use this function to calcute pulse width for per degree rotation
 *
 * @param  degree_of_rotation the angle in degree to which servo has to rotate
 *
 * @return
 *     - calculated pulse width
 */
static uint32_t servo_per_degree_init(uint32_t degree_of_rotation)
{
    uint32_t cal_pulsewidth = 0;
    cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (degree_of_rotation)) / (SERVO_MAX_DEGREE)));
    return cal_pulsewidth;
}

void servo_fn(void *arg)
{
    uint32_t angle, count;
    //1. mcpwm gpio initialization
    mcpwm_example_gpio_initialize();

    //2. initial mcpwm configuration
    printf("Configuring Initial Parameters of mcpwm......\n");
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 50;    //frequency = 50Hz, i.e. for every servo motor time period should be 20ms
    pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings

    // SERVO IDEL state:
    const TickType_t servo_idle_time = (int_hour * 3600 + int_minute * 60 - 3) * 1000 / portTICK_PERIOD_MS; // need to be uncommented later
    // const TickType_t servo_idle_time = 7000 / portTICK_PERIOD_MS;   // need to be commented out later
    vTaskDelay( servo_idle_time );

    while (1) {
      if (servo_flag) {   // if flag == true -->
        // SERVO spins right
        printf("Spin right\n");
        for (count = SERVO_MAX_DEGREE; count > 0; count--) {
            // printf("Angle of rotation: %d\n", count);
            angle = servo_per_degree_init(count);
            // printf("pulse width: %dus\n", angle);
            mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, angle);
            vTaskDelay(1);     //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
        }
        vTaskDelay(50);

        // SERVO spins left 3 times
        printf("Spin left 3 times\n");
        count = 0;
        for (int i = 0; i < 3; i++) {
          for (int j = 0; j < (SERVO_MAX_DEGREE/3); j++) {
              // printf("Angle of rotation: %d\n", count);
              angle = servo_per_degree_init(count);
              // printf("pulse width: %dus\n", angle);
              mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, angle);
              count++;
              vTaskDelay(1);     //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
          }
          vTaskDelay(50);
        }
        servo_flag = false;
        vTaskDelay( servo_idle_time );
      }   // end of "if(flag)" block
    }  // end of while loop
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

// task 1: FOR SERVO  - total seconds countdown & servo_flag update
void servo_update(){
  if (input_flag) {
    // for SERVO
    tot_sec_cd--;   // update number of secs left before servo spins
    if (tot_sec_cd == 0){    // if number of secs elapsed
      servo_flag = true;     // - set servo_flag to true
      tot_sec_cd = tot_sec;  // - restart secs countdown
    }
  } // end of "input_flag" loop
}

// initialize/reset the timer for countdown
void initialize_timer(){
  // 4 situations for intialization:
  if (int_hour > 0 && int_minute > 0){
    timer_hr = int_hour;
    timer_min = int_minute - 1;
    timer_sec = 59;
  }
  else if (int_hour > 0 && int_minute == 0){
    timer_hr = int_hour - 1;
    timer_min = 59;
    timer_sec = 59;
  }
  else if (int_hour == 0 && int_minute > 0){
    timer_hr = 0;
    timer_min = int_minute - 1;
    timer_sec = 59;
  }
  // else {; //do nothing}
}

// task 2: FOR DISPLAY - timer countdown update (alphanumeric & console)
void timer_update(){
  if (timer_sec > 0) {
    timer_sec--;
  }  // outer if loop (for sec)
  else {   // timer_sec == 0
    if (timer_min > 0){
      timer_sec = 59;
      timer_min--;
    }  // middle if loop (for min)
    else {  // timer_min == 0
      if (timer_hr > 0) {
        timer_min = 59;
        timer_hr--;
      }  // inner if loop (for hr)
      else {
        initialize_timer();    // countdown == 0 --> initialize/reset the timer
      }   // end of inner else loop (for hr)
    }   // end of middle else loop (for min)
  } // end of outer else loop (for sec)
}

// Timer: main task
static void timer_evt_task(void *arg) {
    while (1) {
        // Create dummy structure to store structure from queue
        timer_event_t evt;

        // Transfer from queue
        xQueueReceive(timer_queue, &evt, portMAX_DELAY);

        // Do something if triggered!
        if (evt.flag == 1) {
            servo_update();  // task 1: FOR SERVO  - total seconds countdown & servo_flag update
            timer_update();  // task 2: FOR DISPLAY - timer countdown
        }   // end of MAIN TASK LOOP
    }   // end of while loop
}
////////////////////////////////////////////////////////////////////////////////


void app_main(void)
{
    // For uart
    ESP_ERROR_CHECK( uart_driver_install(UART_NUM_0, 256, 0, 0, NULL, 0) );
    esp_vfs_dev_uart_use_driver(UART_NUM_0);

    // For timer
    set_time();
    // Create a FIFO queue for timer-based
    timer_queue = xQueueCreate(10, sizeof(timer_event_t));

    // For Alphanumeric display
    i2c_example_master_init();
    i2c_scanner();

    // Create task to handle timer-based events
    xTaskCreate(timer_evt_task, "timer_evt_task", 2048, NULL, MAX_PRIORITIES, NULL);
    xTaskCreate(test_alpha_display,"test_alpha_display", 4096, NULL, MAX_PRIORITIES-1, NULL);
    xTaskCreate(servo_fn, "servo function", 4096, NULL, MAX_PRIORITIES-1, NULL);

    // Initiate alarm using timer API
    alarm_init();
}
