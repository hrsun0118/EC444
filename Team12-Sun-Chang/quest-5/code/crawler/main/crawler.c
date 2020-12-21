#include <stdio.h>
#include <stdlib.h>
#include "driver/gpio.h"
#include "sdkconfig.h"

// For ir range sensor
#include <math.h>

// For adc
#include "driver/adc.h"
#include "esp_adc_cal.h"

// For RTOS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// For Timer
#include "esp_types.h"
#include "freertos/queue.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"

// For alphanumeric display
#include "driver/i2c.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include "displaychars.h"

// For Wheel Speed - PCNT
#include "freertos/portmacro.h"
#include "driver/ledc.h"
#include "driver/pcnt.h"
#include "esp_attr.h"
#include "esp_log.h"

// For MCPWM(ESC & steering)
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

// For ultrasonic sensor
#include <string.h>
#include "freertos/semphr.h"
#include "esp_err.h"
#include "driver/rmt.h"
#include "soc/rmt_reg.h"

// For UDP Client

#include <sys/param.h>
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "addr_from_stdin.h"


// For MCPWM
#define GPIO_PWM01_OUT                     18   //Set GPIO 18 as PWM01 - ESC
#define GPIO_PWM02_OUT                     16   //Set GPIO 16 as PWM02 - Steering Servo
// In general pulse width: [1000, 2000] us
// For ESC
#define SERVO_MAX_DUTY_US                  1500 //Maximum duty cycle in microsecond
#define SERVO_MID_DUTY_US                  1400 //Medium duty cycle in microsecond
// For Steering Servo
#define SERVO_MIN_PULSEWIDTH               600 //Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH               2700 //Maximum pulse width in microsecond
#define SERVO_MAX_DEGREE                   80 //Maximum angle in degree upto which servo can rotate    // modification - previously = 120


// For Alphanumeric Display
// 14-Segment Display
#define SLAVE_ADDR_ALPHA                   0x70 // alphanumeric address
#define OSC                                0x21 // oscillator cmd
#define HT16K33_BLINK_DISPLAYON            0x01 // Display on cmd
#define HT16K33_BLINK_OFF                  0    // Blink off cmd
#define HT16K33_BLINK_CMD                  0x80 // Blink cmd
#define HT16K33_CMD_BRIGHTNESS             0xE0 // Brightness cmd

// different sdk/scl pins for alphanumeric display
#define I2C_EXAMPLE_MASTER_SCL_IO_DISPLAY 15
#define I2C_EXAMPLE_MASTER_SDA_IO_DISPLAY 32
#define I2C_EXAMPLE_MASTER_NUM_DISPLAY    I2C_NUM_1  // i2c port for alphanumeric

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

// For adc
#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   10          //Multisampling   // modified by [hrsun] - Note: original: 64

// For RTOS
#define MAX_PRIORITIES                     2  // max priority for RTOS tasks

// LIDARLite_v4LED slave address
#define SLAVE_ADDR                         0x62 // slave address

// For adc
static esp_adc_cal_characteristics_t *adc_chars;
#if CONFIG_IDF_TARGET_ESP32
static const adc_channel_t channel = ADC_CHANNEL_6;     //GPIO34 if ADC1, GPIO14 if ADC2  // ADC1 (GPIO14/A6) [hrsun]
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
#elif CONFIG_IDF_TARGET_ESP32S2
static const adc_channel_t channel = ADC_CHANNEL_6;     // GPIO7 if ADC1, GPIO17 if ADC2
static const adc_bits_width_t width = ADC_WIDTH_BIT_13;
#endif
static const adc_atten_t atten = ADC_ATTEN_DB_11;    // modified by [hrsun] - Note: original: ADC_ATTEN_DB_0
static const adc_unit_t unit = ADC_UNIT_2;    // ADC 1 [hrsun]

// For Ultrasonic sensor
#define RMT_TX_CHANNEL                        1     /*!< RMT channel for transmitter */
#define RMT_TX_GPIO_NUM                       12     /*!< GPIO number for transmitter signal */
#define RMT_RX_CHANNEL                        0     /*!< RMT channel for receiver */
#define RMT_RX_GPIO_NUM                       27     /*!< GPIO number for receiver */
#define RMT_CLK_DIV                           100    /*!< RMT counter clock divider */
#define RMT_TICK_10_US                        (80000000/RMT_CLK_DIV/100000)   /*!< RMT counter value for 10 us.(Source clock is APB clock) */
#define HCSR04_MAX_TIMEOUT_US                 25000   /*!< RMT receiver timeout value(us) */
#define US2TICKS(us)                          (us / 10 * RMT_TICK_10_US)
#define TICKS2US(ticks)                       (ticks * 10 / RMT_TICK_10_US)

// For Counter
#define PCNT_TEST_UNIT                          PCNT_UNIT_0
#define PCNT_INPUT_SIG_IO                       4  // Pulse Input GPIO - A5 & use ADC2
#define PCNT_INPUT_CTRL_IO                      5  // Control GPIO HIGH=count up, LOW=count down - SCK
// For Counter Calculation
#define M_PI                                    3.14159265358979323846

// For PID
// Setpoints & dt
#define DIST_ERROR_SET_PT                       0.3   // dist_error distance set point [m]
#define V_ERROR_SET_PT                          0.8   // speed error set point [m/s]
#define DT                                      1 // dt = 1s
// PID Distance constants
#define KP_D                                    1   // Kp
#define KI_D                                    0   // Ki
#define KD_D                                    0   // Kd
// PID Speed constants
#define KP_V                                    1   // Kp
#define KI_V                                    0   // Ki
#define KD_V                                    0   // Kd

// For TIMER
#define TIMER_DIVIDER                           16    //  Hardware timer clock divider
#define TIMER_SCALE                             (TIMER_BASE_CLK / TIMER_DIVIDER)  // to seconds
#define TIMER_INTERVAL_SEC                     (1)    // Sample test interval for the first timer
#define TEST_WITH_RELOAD                        1     // Testing will be done with auto reload
#define MAX_COUNT                               100

// For UDP Client
#define HOST_IP_ADDR "10.0.0.138"
#define PORT         1131
static const char *TAG = "ESP32";
// float temperature = 27.4, x = 1.2, y = 1.3, z = 9.8;
char payload[40] = " ";
int start_index = 0;  // "start_index" for the next string to append


// global variables
double ir_distance = 0.0;
float ultra_distance = 0.0;
double lidar_distance = 0.0;
double alpha_value = 0.0;
// flags
bool ir_adc_flag = false;
bool ultra_flag = false;
bool stop_flag = false;   // need modification
bool web_flag = false;
bool alpha_flag = false;
bool lidar_flag = false;
bool Vwheel_flag = false;
bool PID_flag = false;
// For PCNT & Wheel speed
xQueueHandle pcnt_evt_queue;   // A queue to handle pulse counter events
int16_t pcnt_count = 0;    // moved out of main fn for timer to use as a global varaible
double car_speed = 0.0;

// For Ultrasonic Sensor
static const char* NEC_TAG = "HCSR04";

// For PID
// distance
double dist_integral = 0.0;
double prev_dist_error = 0.0;
double PID_dist_output = 0.0;
// speed
double v_integral = 0.0;
double prev_v_error = 0.0;  // modification - not sure if needed, can be uncommented later if needed
double PID_v_output = 0.0;


////////////////////////////////////////////////////////////////////////////////
// For I2C
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

static void i2c_master_init2(){
    // Debug
    printf("\n>> i2c Config\n");
    int err;

    // Port configuration (alphanumeric)
    int i2c_master_port_display = I2C_EXAMPLE_MASTER_NUM_DISPLAY;

    i2c_config_t conf;
    /// Define I2C configurations for alphanumeric
    conf.mode = I2C_MODE_MASTER;                              // Master mode
    conf.sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO_DISPLAY;              // Default SDA pin
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;                  // Internal pullup
    conf.scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO_DISPLAY;              // Default SCL pin
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;                  // Internal pullup
    conf.master.clk_speed = I2C_EXAMPLE_MASTER_FREQ_HZ;       // CLK frequency
    err = i2c_param_config(i2c_master_port_display, &conf);           // Configure
    if (err == ESP_OK) {printf("- parameters: ok\n");}

    // Install I2C driver
    err = i2c_driver_install(i2c_master_port_display, conf.mode,
                             I2C_EXAMPLE_MASTER_RX_BUF_DISABLE,
                             I2C_EXAMPLE_MASTER_TX_BUF_DISABLE, 0);
    if (err == ESP_OK) {printf("- initialized: yes\n");}

    // Data in MSB mode
    i2c_set_data_mode(i2c_master_port_display, I2C_DATA_MODE_MSB_FIRST, I2C_DATA_MODE_MSB_FIRST);
}

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

////////////////////////////////////////////////////////////////////////////////
// For IR sensor
static void check_efuse(void)
{
#if CONFIG_IDF_TARGET_ESP32
    //Check if TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }
    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }
#elif CONFIG_IDF_TARGET_ESP32S2
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("Cannot retrieve eFuse Two Point calibration values. Default calibration values will be used.\n");
    }
#else
#error "This example is configured for ESP32/ESP32S2."
#endif
}

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}

// IR Task
void ir_adc_task(){
  //Characterize ADC
  adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_chars);
  print_char_val_type(val_type);

  //Continuously sample ADC1
  while (1) {
    if (ir_adc_flag) {
      uint32_t adc_reading = 0;
      //Multisampling
      for (int i = 0; i < NO_OF_SAMPLES; i++) {
          if (unit == ADC_UNIT_1) {
              adc_reading += adc1_get_raw((adc1_channel_t)channel);
          } else {
              int raw;
              adc2_get_raw((adc2_channel_t)channel, width, &raw);
              adc_reading += raw;
          }
      }
      adc_reading /= NO_OF_SAMPLES;
      // Convert adc_reading to voltage in mV
      uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);  // UNIT: V
      ir_distance = -0.0241 * voltage + 73.471;

      printf("ir: distance:%.2f", ir_distance);
      // printf("Raw ADC Reading: %d\tVout: %dmV\n", adc_reading, voltage);
      if (ir_distance < 20){
        ir_distance = 20;
        // printf("\ndistance < 20.\n\n"); // modification - uncommented later
      }
      else if (ir_distance > 150){
        ir_distance = 150;
        // printf("\ndistance > 150.\n\n");   // modification - uncommented later
      }
      else {
        // printf("IR sensor distance: %.2fcm\n\n", ir_distance);   // modification - uncommented later
      }
      alpha_value = ir_distance;
      ir_adc_flag = false;
      vTaskDelay(pdMS_TO_TICKS(TIMER_INTERVAL_SEC * 1000 * 0.9)); // avoid watchdog problem - originally "pdMS_TO_TICKS(1000)"
    }
  }
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// For Ultrasonic Sensor
static inline void set_item_edge(rmt_item32_t* item, int low_us, int high_us)
{
    item->level0 = 0;
    item->duration0 = US2TICKS(low_us);
    item->level1 = 1;
    item->duration1 = US2TICKS(high_us);
}

// @brief RMT transmitter initialization
static void nec_tx_init()
{
    rmt_config_t rmt_tx;
    rmt_tx.channel = RMT_TX_CHANNEL;
    rmt_tx.gpio_num = RMT_TX_GPIO_NUM;
    rmt_tx.mem_block_num = 1;
    rmt_tx.clk_div = RMT_CLK_DIV;
    rmt_tx.tx_config.loop_en = false;

    rmt_tx.tx_config.carrier_duty_percent = 50;
    rmt_tx.tx_config.carrier_freq_hz = 38000;
    rmt_tx.tx_config.carrier_level = 1;
    rmt_tx.tx_config.carrier_en = 0;    // off

    rmt_tx.tx_config.idle_level = 0;
    rmt_tx.tx_config.idle_output_en = true;
    rmt_tx.rmt_mode = 0;
    rmt_config(&rmt_tx);
    rmt_driver_install(rmt_tx.channel, 0, 0);
}

// @brief RMT receiver initialization
static void nec_rx_init()
{
    rmt_config_t rmt_rx;
    rmt_rx.channel = RMT_RX_CHANNEL;
    rmt_rx.gpio_num = RMT_RX_GPIO_NUM;
    rmt_rx.clk_div = RMT_CLK_DIV;
    rmt_rx.mem_block_num = 1;
    rmt_rx.rmt_mode = RMT_MODE_RX;
    rmt_rx.rx_config.filter_en = false;
    rmt_rx.rx_config.filter_ticks_thresh = 100;
    rmt_rx.rx_config.idle_threshold = US2TICKS(HCSR04_MAX_TIMEOUT_US);
    rmt_config(&rmt_rx);
    rmt_driver_install(rmt_rx.channel, 1000, 0);
}

// @brief RMT receiver demo, this task will print each received NEC data.
static void ultrasonic_rx_task()
{
    int channel = RMT_RX_CHANNEL;

    nec_rx_init();

    RingbufHandle_t rb = NULL;

    //get RMT RX ringbuffer
    rmt_get_ringbuf_handle(channel, &rb);
    rmt_rx_start(channel, 1);

    while (rb)
    {
        size_t rx_size = 0;
        //try to receive data from ringbuffer.
        //RMT driver will push all the data it receives to its ringbuffer.
        //We just need to parse the value and return the spaces of ringbuffer.
        rmt_item32_t* item = (rmt_item32_t*) xRingbufferReceive(rb, &rx_size, 1000);
        if (item)
        { int i;
          for (i=0; i<rx_size / sizeof(rmt_item32_t); ++i){
            ultra_distance = (float)TICKS2US(item[i].duration0) / 58.2;
            // printf("DEBUG: Ultra value: %.2f\n", value);
            // ultra_distance = -0.0332 * pow(value, 2) + 3.6356 * value - 50.156;
            if (ultra_distance <= 40)
              stop_flag = true;
            else
              stop_flag = false;
            // ultra_distance = 44.759 * log(value) - 124.23;
            ESP_LOGI(NEC_TAG, "RMT RCV -- %d:%d | %d:%d : %.1fcm",
                     item[i].level0, TICKS2US(item[i].duration0),
                     item[i].level1, TICKS2US(item[i].duration1),
                     ultra_distance);
          }
          //after parsing the data, return spaces to ringbuffer.
          vRingbufferReturnItem(rb, (void*) item);
        } else {
            vTaskDelay(10 / portTICK_PERIOD_MS);
            //            break;
        }


    }

    vTaskDelete(NULL);
}

// @brief RMT transmitter demo, this task will periodically send NEC data. (100 * 32 bits each time.)
static void ultrasonic_tx_task()
{
    vTaskDelay(10);
    nec_tx_init();

    int channel = RMT_TX_CHANNEL;

    int item_num = 1;
    rmt_item32_t item[item_num];
    for (int i=0; i<item_num; ++i)
        set_item_edge(&item[i], 20, 180);
    //    set_item_edge(&item[1], factor * 70, factor * 30);

    for (;;)
    {
      if (ultra_flag){
        ESP_LOGI(NEC_TAG, "RMT TX DATA");

        // To send data according to the waveform items.
        rmt_write_items(channel, item, item_num, true);
        // Wait until sending is done.
        rmt_wait_tx_done(channel, portMAX_DELAY);
        // before we free the data, make sure sending is already done.
        ultra_flag = false;
      }

      vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
// LIDARLite_v4LED Function
// Write one byte to register
int writeRegister(uint8_t reg, uint8_t data) {
  // YOUR CODE HERE
  int ret;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  return ret;
}

// Read register
uint8_t readRegister(uint8_t reg) {
  // YOUR CODE HERE
  int ret;
  uint8_t data;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ( SLAVE_ADDR << 1 ) | READ_BIT, ACK_CHECK_EN);
  i2c_master_read_byte(cmd, &data, ACK_CHECK_DIS);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  return data;
}

// read 16 bits (2 bytes)
int16_t read16(uint8_t reg) {
  // YOUR CODE HERE
  int ret;
  uint8_t data, data2;
  uint16_t result;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ( SLAVE_ADDR << 1 ) | READ_BIT, ACK_CHECK_EN);
  i2c_master_read_byte(cmd, &data, ACK_VAL);
  i2c_master_read_byte(cmd, &data2, ACK_CHECK_DIS);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  result = (data2 << 8) | data;
  // printf("\nread16 Data: %d\n", result);
  return result;
}

// LIDAR task
static void test_LIDARLite_v4LED(){
  printf("\n>> Polling LIDARLite_v4LED!\n");
  // variables
  uint8_t busyflag = 1;
  int16_t distance = 0;

  while (1) {
    while(lidar_flag){
      double sum = 0.0;
      int for_loop_count = 6;
      for (int i = 0; i < for_loop_count; i++){
        writeRegister(0x00, 0x04);

        // check busy flag
        do {
          busyflag = 0x01 & readRegister(0x01);
        } while(busyflag == 1);

        // read LIDAR data
        distance = read16(0x10) - 2;
        sum += (double)(distance);

        busyflag = 1; // reset busy flag
      }
      lidar_distance = (double)(sum / (double)(for_loop_count));
      printf("\nLIDAR distance to the Wall: %.1f cm\n", lidar_distance); // potential modification - can be uncommented or not later, doesn't matter

      // print distance from the center
      /*
      if (lidar_distance < DIST_ERROR_SET_PT)
        printf("\nCar is %.1f cm to the right of the Center Line\n", DIST_ERROR_SET_PT * 100 - lidar_distance);
      else if (lidar_distance > DIST_ERROR_SET_PT)
        printf("\nCar is %.1f cm to the left of the Center Line\n", lidar_distance - DIST_ERROR_SET_PT * 100);
      else
        printf("\nCar is on the Center Line\n");
        */

      lidar_flag = false; // reset lidar_flag
    }
    vTaskDelay(10 / portTICK_RATE_MS);
  }
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Speed Calculation & PCNT task
/* A sample structure to pass events from the PCNT
 * interrupt handler to the main program.
 */
typedef struct {
    int unit;  // the PCNT unit that originated an interrupt
    uint32_t status; // information on the event type that caused the interrupt
} pcnt_evt_t;

/* Initialize PCNT functions:
 *  - configure and initialize PCNT
 *  - set up the input filter
 */
static void pcnt_example_init(void)
{
    /* Prepare configuration for the PCNT unit */
    pcnt_config_t pcnt_config = {
        // Set PCNT input signal and control GPIOs
        .pulse_gpio_num = PCNT_INPUT_SIG_IO,
        .ctrl_gpio_num = PCNT_INPUT_CTRL_IO,
        .channel = PCNT_CHANNEL_0,
        .unit = PCNT_TEST_UNIT,
        // What to do on the positive / negative edge of pulse input?
        .pos_mode = PCNT_COUNT_INC,   // Count up on the positive edge
        .neg_mode = PCNT_COUNT_DIS,   // Keep the counter value on the negative edge
        // What to do when control input is low or high?
        .lctrl_mode = PCNT_MODE_REVERSE, // Reverse counting direction if low
        .hctrl_mode = PCNT_MODE_KEEP,    // Keep the primary counter mode if high
    };
    /* Initialize PCNT unit */
    pcnt_unit_config(&pcnt_config);

    /* Configure and enable the input filter */
    pcnt_set_filter_value(PCNT_TEST_UNIT, 300);   // - poential modification?
    pcnt_filter_enable(PCNT_TEST_UNIT);

    /* Initialize PCNT's counter */
    pcnt_counter_pause(PCNT_TEST_UNIT);
    pcnt_counter_clear(PCNT_TEST_UNIT);
    pcnt_counter_resume(PCNT_TEST_UNIT);
}

// PCNT task
void pcnt(){
  pcnt_evt_t evt;
  int16_t count;
  count = 0;
  /* Wait for the event information passed from PCNT's interrupt handler.
   * Once received, decode the event type and print it on the serial monitor.
   */
  xQueueReceive(pcnt_evt_queue, &evt, 20 / portTICK_PERIOD_MS);
  pcnt_get_counter_value(PCNT_TEST_UNIT, &count);
  pcnt_count = count;
  pcnt_counter_clear(PCNT_TEST_UNIT);

  // printf("\nDEBUG - count: %d\n", pcnt_count);
}

// For Speed Calculation task
void speed_cal_task(){
  double rps = 0.0;
  double Cwheel = 2 * M_PI * 2.98;  // [cm]

  while(1){
    if (Vwheel_flag){
      pcnt();
      rps = (double)((pcnt_count) / 6.0);
      car_speed = rps * Cwheel / 100.0; // [m/s]
      /*
      if (car_speed > V_ERROR_SET_PT)
        printf("\nCar Speed = %.3f m/s is too FAST\n", car_speed);
      else if (car_speed < V_ERROR_SET_PT)
        printf("\nCar Speed = %.3f m/s is too SLOW\n", car_speed);
      else
        printf("Car Speed = Desired Car Speed = %.1f", car_speed);
        */
      printf("\nCar Speed = %.1f\n", car_speed);
      // pcnt_counter_clear(PCNT_TEST_UNIT);
      Vwheel_flag = false;
    }
    vTaskDelay(100 / portTICK_RATE_MS);
  }
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// PID Function
void PID_distance(){
  double error = DIST_ERROR_SET_PT - (double)(lidar_distance / 100.0); // distance error unit: [m]
  dist_integral = dist_integral + error * DT;
  double derivative = (error - prev_dist_error) / DT;
  PID_dist_output = KP_D * error + KI_D *  dist_integral + KD_D * derivative;
  // printf("Distance PID = %.3f\n", PID_dist_output); // modification - uncomment later!
  prev_dist_error = error;
}

void PID_speed(){
  double error = V_ERROR_SET_PT - car_speed;
  v_integral = v_integral + error * DT;
  double derivative = (error - prev_v_error) / DT;
  PID_v_output = KP_V * error + KI_V *  v_integral + KD_V * derivative;
  // printf("Car Speed PID = %.3f\n", PID_v_output);
  prev_dist_error = error;
}

void PID_task(){
  while(1){
    if (PID_flag){
      PID_distance();
      PID_speed();
      PID_flag = false;
    }
    vTaskDelay(100 / portTICK_RATE_MS);
  }
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// For MCPWM(ESC & steering)
static void mcpwm_example_gpio_initialize(void)
{
    printf("initializing mcpwm servo control gpio......\n");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, GPIO_PWM01_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, GPIO_PWM02_OUT);
}

void mcpwm_setup() {
  // initial mcpwm configuration
  printf("Configuring Initial Parameters of mcpwm......\n");
  mcpwm_config_t pwm_config;
  pwm_config.frequency = 50;    //frequency = 50Hz, i.e. for every servo motor time period should be 20ms
  pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
  pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
  pwm_config.counter_mode = MCPWM_UP_COUNTER;
  pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
  mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings

}

void calibrateESC() {
  printf("Calibrating ESC......\n");
  mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, SERVO_MID_DUTY_US); // NEUTRAL signal in microseconds
  vTaskDelay(3000 / portTICK_PERIOD_MS);  // Give yourself time to turn on crawler
  printf("ESC calibration done.\n");
}

void mcpwm_esc_servo_control(void *arg) {
    // calibrate ESC
    calibrateESC();

    int duty_us;
    // 1st start up
    while(!web_flag){
      if (web_flag)
        break;
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    for (duty_us = SERVO_MID_DUTY_US; duty_us <= SERVO_MAX_DUTY_US; duty_us += 1) {
        mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty_us);
        vTaskDelay(2 / portTICK_PERIOD_MS);     //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
    }

    // after start up
    bool prev_stop_flag = false;
    while (1) {

      if ((!stop_flag) && web_flag) {
        // restart forward and set the wheel to previous speed with "duty_us"
        if (prev_stop_flag){
          for (int i = SERVO_MID_DUTY_US; i <= duty_us; i += 1) {
              mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, i);
              vTaskDelay(2 / portTICK_PERIOD_MS);     //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
          }
          prev_stop_flag = false;
        }

        // PID tuning
        if (PID_v_output < 0){
          // too fast, need to reduce speed
          duty_us -= 2;
          mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty_us);
        }
        else if (PID_v_output > 0){
          // too slow, need to increase speed
          duty_us += 2;
          mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty_us);
        }
      }
      else {
        // reduce speed from current "duty_us" to SERVO_MID_DUTY_US
        for (int i = duty_us; i >= SERVO_MID_DUTY_US; i -= 1) {
          mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, i);
          vTaskDelay(2 / portTICK_PERIOD_MS);    //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
        }
        prev_stop_flag = true;
      }
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      /*
      // STOP
      for (int duty_us = SERVO_MAX_DUTY_US; duty_us >= SERVO_MID_DUTY_US; duty_us -= 1) {
          mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty_us);
          vTaskDelay(2 / portTICK_PERIOD_MS);    //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
      }
*/
    }


}

static uint32_t servo_per_degree_init(uint32_t degree_of_rotation)
{
    uint32_t cal_pulsewidth = 0;
    cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (degree_of_rotation)) / (SERVO_MAX_DEGREE)));
    return cal_pulsewidth;
}

void mcpwm_steering_servo_control(void *arg){
  uint32_t angle, count;
  uint32_t center_count = 0.375 * SERVO_MAX_DEGREE;
  printf("Wheels at Center\n");
  count = center_count;
  angle = servo_per_degree_init(count);
  mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, angle);

  while (1) {
    // steering
    if (PID_dist_output < 0){ // steer right
      // reduce count
      count -= 2;
      if (count < 0)  // avoid count < 0
        count = 0;

      // set angle
      angle = servo_per_degree_init(count);
      mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, angle);
    }
    else if (PID_dist_output > 0){  // steer left
      // increase count
      count += 2;
      if (count > SERVO_MAX_DEGREE - 20)   // avoid count > max count
        count = SERVO_MAX_DEGREE - 20;

      // set angle
      angle = servo_per_degree_init(count);
      mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, angle);
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Alphanumeric Functions //////////////////////////////////////////////////////
//Hex values for the alphanumeric display numbers
const uint16_t FourteenSegmentASCII[10] = {
    0b000110000111111, /* 0 */
    0b000010000000110, /* 1 */
    0b000000011011011, /* 2 */
    0b000000010001111, /* 3 */
    0b000000011100110, /* 4 */
    0b010000001101001, /* 5 */
    0b000000011111101, /* 6 */
    0b000000000000111, /* 7 */
    0b000000011111111, /* 8 */
    0b000000011101111, /* 9 */
};

int alpha_oscillator() {
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( SLAVE_ADDR_ALPHA << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, OSC, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM_DISPLAY, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    vTaskDelay(200 / portTICK_RATE_MS);
    return ret;
}

// Set blink rate to off
int no_blink() {
    int ret;
    i2c_cmd_handle_t cmd2 = i2c_cmd_link_create();
    i2c_master_start(cmd2);
    i2c_master_write_byte(cmd2, ( SLAVE_ADDR_ALPHA << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd2, HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | (HT16K33_BLINK_OFF << 1), ACK_CHECK_EN);
    i2c_master_stop(cmd2);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM_DISPLAY, cmd2, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd2);
    vTaskDelay(200 / portTICK_RATE_MS);
    return ret;
}

// Set Brightness
int set_brightness_max(uint8_t val) {
    int ret;
    i2c_cmd_handle_t cmd3 = i2c_cmd_link_create();
    i2c_master_start(cmd3);
    i2c_master_write_byte(cmd3, ( SLAVE_ADDR_ALPHA << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd3, HT16K33_CMD_BRIGHTNESS | val, ACK_CHECK_EN);
    i2c_master_stop(cmd3);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM_DISPLAY, cmd3, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd3);
    vTaskDelay(200 / portTICK_RATE_MS);
    return ret;
}

// Task for running the alphanumeric display
static void alphaTask(){
    // Debug
    int ret;
    printf(">> Test Alphanumeric Display: \n");
    int temp;
    // Set up routines
    // Turn on alpha oscillator
    ret = alpha_oscillator();
    if(ret == ESP_OK) {printf("- oscillator: ok \n");}
    else printf("ret is: %d\n", ret);
    // Set display blink off
    ret = no_blink();
    if(ret == ESP_OK) {printf("- blink: off \n");}
    else printf("ret is: %d\n", ret);
    ret = set_brightness_max(0xF);
    if(ret == ESP_OK) {printf("- brightness: max \n");}
    else printf("ret is: %d\n", ret);
    // Write to characters to buffer
    uint16_t displaybuffer[8];
    // Continually writes the same command
    while (1) {
      if (alpha_flag){
        temp = car_speed * 1000;
        for(int x = 3; x >= 0; x--)
        {
            int rem = temp % 10;
            // printf("%d\n", rem);
            displaybuffer[x] = FourteenSegmentASCII[rem];
            temp = temp / 10;
        }
        //Add dot to the first number
        displaybuffer[0] |= 1 << 14;

        // Send commands characters to display over I2C
        i2c_cmd_handle_t cmd4 = i2c_cmd_link_create();
        i2c_master_start(cmd4);
        i2c_master_write_byte(cmd4, ( SLAVE_ADDR_ALPHA << 1 ) | WRITE_BIT, ACK_CHECK_EN);
        i2c_master_write_byte(cmd4, (uint8_t)0x00, ACK_CHECK_EN);
        for (uint8_t i=0; i<8; i++) {
            i2c_master_write_byte(cmd4, displaybuffer[i] & 0xFF, ACK_CHECK_EN);
            i2c_master_write_byte(cmd4, displaybuffer[i] >> 8, ACK_CHECK_EN);
        }
        i2c_master_stop(cmd4);
        ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM_DISPLAY, cmd4, 1000 / portTICK_RATE_MS);
        i2c_cmd_link_delete(cmd4);
        alpha_flag = false;
      }

      vTaskDelay(100 / portTICK_RATE_MS);
    }
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// For UDP
static void udp_client_task(void *pvParameters)
{
    char rx_buffer[128];
    char host_ip[] = HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;
    char payload[4] = "flag";

    while (1) {

        // IPV4 Configuration
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;

        // create socket
        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created, sending to %s:%d", HOST_IP_ADDR, PORT);

        while (1) {
            int err = sendto(sock, &payload, strlen(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                break;
            }
            ESP_LOGI(TAG, "Message sent");

            struct sockaddr_in source_addr; // Large enough for both IPv4 or IPv6
            socklen_t socklen = sizeof(source_addr);
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            }
            // Data received
            else {
                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                ESP_LOGI(TAG, "Received %d bytes from %s. Message Received: %s", len, host_ip, rx_buffer);

                char start_str[] = "0";
                char stop_str[] = "1";
                if (strncmp(rx_buffer, start_str, 1) == 0){
                  web_flag = true;
                  printf("START (1)\n");
                }
                else if (strncmp(rx_buffer, stop_str, 1) == 0){
                  web_flag = false;
                  printf("STOP (0)\n");
                }

            }

            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }   // end of while loop

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Timer & Alarm
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
            ir_adc_flag = true;
            ultra_flag = true;
            alpha_flag = true;
            lidar_flag = true;
            Vwheel_flag = true;
            PID_flag = true;
            alpha_flag = true;
        }

    }
}
////////////////////////////////////////////////////////////////////////////////

void init(){
  // For ADC
  //Check if Two Point or Vref are burned into eFuse
  check_efuse();
  //Configure ADC
  if (unit == ADC_UNIT_1) {
      adc1_config_width(width);
      adc1_config_channel_atten(channel, atten);
  } else {
      adc2_config_channel_atten((adc2_channel_t)channel, atten);
  }


  // For Alphanumeric Display Config
  i2c_example_master_init();
  i2c_scanner();
  i2c_master_init2();

  // For PCNT
  pcnt_example_init();  // Initialize PCNT functions
  // PCNT Input Signal Pin init - GPIO4 / A5
  gpio_reset_pin(PCNT_INPUT_SIG_IO);
  gpio_set_direction(PCNT_INPUT_SIG_IO, GPIO_MODE_INPUT);

  // For MCPWM(ESC & steering)
  mcpwm_example_gpio_initialize();  //1. mcpwm gpio initialization
  mcpwm_setup();

  // For udp_client
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
   * Read "Establishing Wi-Fi or Ethernet Connection" section in
   * examples/protocols/README.md for more information about this function.
   */
  ESP_ERROR_CHECK(example_connect());
}

void app_main(void)
{
  timer_queue = xQueueCreate(10, sizeof(timer_event_t));  // Create a FIFO queue for timer-based
  pcnt_evt_queue = xQueueCreate(10, sizeof(pcnt_evt_t));  // Initialize PCNT event queue
  init();

  // create tasks
  // xTaskCreate(ir_adc_task,"IR Range Finder task", 4096, NULL, MAX_PRIORITIES-1, NULL);
  xTaskCreate(ultrasonic_rx_task, "rx_task", 2048, NULL, MAX_PRIORITIES-1, NULL);
  xTaskCreate(ultrasonic_tx_task, "tx_task", 2048, NULL, MAX_PRIORITIES, NULL);
  xTaskCreate(test_LIDARLite_v4LED,"test_LIDARLite_v4LED", 4096, NULL, MAX_PRIORITIES-1, NULL);
  xTaskCreate(alphaTask,"alphanumeric display task", 4096, MAX_PRIORITIES-2, 1, NULL);
  xTaskCreate(speed_cal_task, "speed_cal_task", 2048, NULL, MAX_PRIORITIES-1, NULL);
  xTaskCreate(PID_task,"pid task", 4096, NULL, MAX_PRIORITIES-1, NULL);
  xTaskCreate(mcpwm_steering_servo_control, "mcpwm_steering_servo_control", 4096, NULL, MAX_PRIORITIES-1, NULL);
  xTaskCreate(mcpwm_esc_servo_control, "mcpwm_esc_servo_control", 4096, NULL, MAX_PRIORITIES-1, NULL);
  xTaskCreate(udp_client_task, "udp_client_task", 4096, NULL, MAX_PRIORITIES, NULL);

  // timer & alarm tasks
  xTaskCreate(timer_evt_task, "timer_evt_task", 2048, NULL, MAX_PRIORITIES, NULL);
  alarm_init(); // Initiate alarm using timer API
}
