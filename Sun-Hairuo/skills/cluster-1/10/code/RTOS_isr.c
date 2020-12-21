// Tasl A: for blink
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#define BLINK_GPIO14 14  // LED 1: green
#define BLINK_GPIO32 32  // LED 2: blue
#define BLINK_GPIO15 15  // LED 3: red
#define BLINK_GPIO33 33  // LED 4: yellow

// Task B: for pushbutton
#define BUTTON_GPIO27 27  // pushbutton
// #define BUTTON_A2 34

// Task C: for alphanumeric display
#include "driver/i2c.h"
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

// For RTOS
#define MAX_PRIORITIES                     2  // max priority for RTOS tasks

bool flag = true;   // global variable "flag": true: up; false: down

// TASK A
void LED_pattern(int num){
  printf("\nNumber: %d", num);
  int b_num[] = {0,0,0,0};
  int i = 0;  // used for indexing binary array b_num[]
  while (num > 0){  // convert number i from int to binary & store it in a char[] array
    b_num[i] = (num % 2);
    num /= 2;
    i++;
  }
  printf("\nBinary number: %d%d%d%d", b_num[3], b_num[2], b_num[1], b_num[0]);
  gpio_set_level(BLINK_GPIO14, b_num[0]);
  gpio_set_level(BLINK_GPIO32, b_num[1]);
  gpio_set_level(BLINK_GPIO15, b_num[2]);
  gpio_set_level(BLINK_GPIO33, b_num[3]);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}
static void task_A()  // LED binary counting
{
    while(1) {
      // LED_pattern();
      if (flag){   // COUNT UP
        for(int num = 0; num < 16; num++){  // integer pattern number: 0 - 15
          LED_pattern(num);
          if (!flag)   // if flag changed from "true" to "false" --> break
            break;
        }
      }
      else {    // COUNT DOWN
        for(int num = 15; num >= 0; num --){
          LED_pattern(num);
          if (flag)   // if flag changed from "false" to "true" --> break
            break;
        }
      }
    }
}


// TASK B: pushbutton
static void task_B() {    // pushbutton
  while(1){
    if ((gpio_get_level(BUTTON_GPIO27)) == 1){
      if (flag)
        flag = false;
      else
        flag = true;
    }
    vTaskDelay(100 / portTICK_RATE_MS);
  }
}






// TASK C
// Function to initiate i2c -- note the MSB declaration!
static void i2c_example_master_init(){
    // Debug
    printf("\n>> i2c Config\n");
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
    if (err == ESP_OK) {printf("- parameters: ok\n");}

    // Install I2C driver
    err = i2c_driver_install(i2c_master_port, conf.mode,
                       I2C_EXAMPLE_MASTER_RX_BUF_DISABLE,
                       I2C_EXAMPLE_MASTER_TX_BUF_DISABLE, 0);
    // i2c_set_data_mode(i2c_master_port,I2C_DATA_MODE_LSB_FIRST,I2C_DATA_MODE_LSB_FIRST);
    if (err == ESP_OK) {printf("- initialized: yes\n\n");}

    // Dat in MSB mode
    i2c_set_data_mode(i2c_master_port, I2C_DATA_MODE_MSB_FIRST, I2C_DATA_MODE_MSB_FIRST);
}

// Utility  Functions //////////////////////////////////////////////////////////

// Utility function to test for I2C device address -- not used in deploy
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

////////////////////////////////////////////////////////////////////////////////

// Alphanumeric Functions //////////////////////////////////////////////////////

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

// Set blink rate to off
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

// Set Brightness
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

////////////////////////////////////////////////////////////////////////////////

static void task_C() {
    // Debug
    int ret;
    printf(">> Test Alphanumeric Display: \n");

    // Set up routines
    // Turn on alpha oscillator
    ret = alpha_oscillator();
    if(ret == ESP_OK) {printf("- oscillator: ok \n");}
    // Set display blink off
    ret = no_blink();
    if(ret == ESP_OK) {printf("- blink: off \n");}
    ret = set_brightness_max(0xF);
    if(ret == ESP_OK) {printf("- brightness: max \n");}

    // Write to characters to buffer
    uint16_t displaybuffer[8];

    // Continually writes command
    while (1) {
      if (flag){  // UP
        displaybuffer[0] = 0b0100000000111110;  // U.
        displaybuffer[1] = 0b0100000011110011;  // P.
        displaybuffer[2] = 0b0000000000000000;  // blank
        displaybuffer[3] = 0b0000000000000000;  // blank
      }
      else{  // DOWN
        displaybuffer[0] = 0b0101001000001111;  // D.
        displaybuffer[1] = 0b0100000000111111;  // O.
        displaybuffer[2] = 0b0101001000111110;  // W.
        displaybuffer[3] = 0b0110000100110110;  // N.
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

      // for (int i = 0; i < 8; i++) {
      //     printf("%04x\n", displaybuffer[i]);
      // }

      if(ret == ESP_OK) {
        if (flag)
          printf("- wrote: U.P. \n\n");
        else
          printf("- wrote: D.O.W.N. \n\n");
      }
    }


}
void init(){
  // Task A config - LEDs
  gpio_reset_pin(BLINK_GPIO14);
  gpio_reset_pin(BLINK_GPIO32);
  gpio_reset_pin(BLINK_GPIO15);
  gpio_reset_pin(BLINK_GPIO33);

  gpio_set_direction(BLINK_GPIO14, GPIO_MODE_OUTPUT);
  gpio_set_direction(BLINK_GPIO32, GPIO_MODE_OUTPUT);
  gpio_set_direction(BLINK_GPIO15, GPIO_MODE_OUTPUT);
  gpio_set_direction(BLINK_GPIO33, GPIO_MODE_OUTPUT);

  // Task C config - pushbutton
  gpio_reset_pin(BUTTON_GPIO27);
  gpio_set_direction(BUTTON_GPIO27, GPIO_MODE_INPUT);

  // Task C config - Alphanumeric display
  i2c_example_master_init();
  i2c_scanner();
}

void app_main() {
  init();
  // Tasks
  xTaskCreate(task_A,"test_LEDs", 4096, NULL, MAX_PRIORITIES-2, NULL);
  xTaskCreate(task_B,"push_button", 4096, NULL, MAX_PRIORITIES, NULL);
  xTaskCreate(task_C,"test_alpha_display", 4096, MAX_PRIORITIES-1, 1, NULL);
}
