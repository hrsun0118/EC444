#include <stdio.h>
#include <math.h>
#include "driver/i2c.h"
#include "./ADXL343.h"
#include "driver/gpio.h"
#include "sdkconfig.h" // not sure if this is necessary to include

// For adc
#include "driver/adc.h"
#include "esp_adc_cal.h"

// For lED
#include "driver/gpio.h"
#include <stdlib.h>

// For Timer
#include "esp_types.h"
#include "freertos/queue.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"

// For UDP Client
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "addr_from_stdin.h"

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

// ADXL343
#define SLAVE_ADDR                         ADXL343_ADDRESS // 0x53

// For adc
#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   10          //Multisampling   // modified by [hrsun] - Note: original: 64
static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_6;     //GPIO34 if ADC1, GPIO14 if ADC2  // ADC1 (GPIO34/A2) [hrsun]
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
static const adc_atten_t atten = ADC_ATTEN_DB_11;    // modified by [hrsun] - Note: original: ADC_ATTEN_DB_0
static const adc_unit_t unit = ADC_UNIT_1;    // ADC 1 [hrsun]

// For LED
#define BLINK_GPIO 18

// For RTOS
#define MAX_PRIORITIES                     2  // max priority for RTOS tasks

// For TIMER
#define TIMER_DIVIDER         16    //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // to seconds
#define TIMER_INTERVAL_SEC   (2)    // Sample test interval for the first timer
#define TEST_WITH_RELOAD      1     // Testing will be done with auto reload
#define MAX_COUNT             100

// For UDP Client
#define HOST_IP_ADDR "10.0.0.138"
#define PORT         1131
static const char *TAG = "ESP32";
// float temperature = 27.4, x = 1.2, y = 1.3, z = 9.8;
char payload[40] = " ";
int start_index = 0;  // "start_index" for the next string to append

// global variables
bool adc_flag = false;
bool accel_flag = false;
// bool alpha_flag = false;
// float_t alpha_value = 0.0;
char *prev_led_in = "false";  // LED
char *led_in = "false";       // LED
int led = 0;
float temperature = 0.0;
float accel_x = 0.0;
float accel_y = 0.0;
float accel_z = 0.0;

// Function to initiate i2c -- note the MSB declaration!
static void i2c_master_init(){
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
  if (err == ESP_OK) {printf("- initialized: yes\n");}

  // Data in MSB mode
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
  if (count == 0) {printf("- No I2C devices found!" "\n");}
}

////////////////////////////////////////////////////////////////////////////////

// ADXL343 Functions ///////////////////////////////////////////////////////////

// Get Device ID
int getDeviceID(uint8_t *data) {
  int ret;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, ADXL343_REG_DEVID, ACK_CHECK_EN);
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ( SLAVE_ADDR << 1 ) | READ_BIT, ACK_CHECK_EN);
  i2c_master_read_byte(cmd, data, ACK_CHECK_DIS);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  return ret;
}

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

void setRange(range_t range) {
  /* Red the data format register to preserve bits */
  uint8_t format = readRegister(ADXL343_REG_DATA_FORMAT);
  /* Update the data rate */
  format &= ~0x0F;
  format |= range;

  /* Make sure that the FULL-RES bit is enabled for range scaling */
  format |= 0x08;

  /* Write the register back to the IC */
  writeRegister(ADXL343_REG_DATA_FORMAT, format);

}

range_t getRange(void) {
  /* Red the data format register to preserve bits */
  return (range_t)(readRegister(ADXL343_REG_DATA_FORMAT) & 0x03);
}

dataRate_t getDataRate(void) {
  return (dataRate_t)(readRegister(ADXL343_REG_BW_RATE) & 0x0F);
}

////////////////////////////////////////////////////////////////////////////////

// function to get acceleration
void getAccel(float * xp, float *yp, float *zp) {
  *xp = read16(ADXL343_REG_DATAX0) * ADXL343_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;
  *yp = read16(ADXL343_REG_DATAY0) * ADXL343_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;
  *zp = read16(ADXL343_REG_DATAZ0) * ADXL343_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;
  printf("\n Accelerometor Data: X: %.2f \t Y: %.2f \t Z: %.2f\n", *xp, *yp, *zp);
}

// Task to continuously poll acceleration and calculate roll and pitch
static void test_adxl343() {
  printf("\n>> Polling ADAXL343\n");
  while (1) {
    if (accel_flag){
      float xVal, yVal, zVal;
      getAccel(&xVal, &yVal, &zVal);
      // calcRP(xVal, yVal, zVal);
      accel_x = roundf(xVal * 100)/ 100.0;
      accel_y = roundf(yVal * 100)/ 100.0;
      accel_z = roundf(zVal * 100)/ 100.0;
      accel_flag = false;
      vTaskDelay(pdMS_TO_TICKS(TIMER_INTERVAL_SEC * 1000 * 0.9));
      // vTaskDelay(500 / portTICK_RATE_MS);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// ADC task
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
    printf("- ADC ");
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}

void thermistor_adc_task(){
  //Characterize ADC
  adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_chars);
  print_char_val_type(val_type);
  printf("\n");

  //Continuously sample ADC1
  while (1) {
    if (adc_flag) {
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
      //Convert adc_reading to voltage in mV
      uint32_t voltage_mV = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);  // UNIT: mV
      double voltage_V = (double)voltage_mV / 1000.0;
      double Rt = (3.3 - voltage_V) / (voltage_V /10000.0);  // calculate thermistor resistance
      // alternative if Vadc_ref == Vsource:
      // double Rt = 10000 * ((double)(4095 / (double)adc_reading) - 1);
      double Tt = 1.0 / (1.0/298.15 + (1.0 / 3435.0) * log(Rt / 10000.0)) - 273.15;
      // printf("Raw ADC Reading: %d\tVout: %.3fV\n", adc_reading, voltage_V);
      printf("Thermistor Temperature: %.2f C\n", Tt);
      temperature = roundf(Tt * 100)/ 100.0;

      // alpha_value = Tt;
      adc_flag = false;
      vTaskDelay(pdMS_TO_TICKS(TIMER_INTERVAL_SEC * 1000 * 0.9)); // avoid watchdog problem - originally "pdMS_TO_TICKS(1000)"
    }


  }
}

////////////////////////////////////////////////////////////////////////////////

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
            adc_flag = true;
            accel_flag = true;
            // alpha_flag = true;
        }

    }
}
////////////////////////////////////////////////////////////////////////////////

// LED task
void led_task(){
  int led = 0;
  // char *led_cmp = "false";
  char *true_str = "true";
  // char *false_str = "false";
  while(1) {
    int ret_true = strncmp(led_in, true_str, 1);
    // int ret_false = strncmp(led_in, false_str, 1);
    printf("\nret_true: %d\n", ret_true);
    //printf("\nret_false: %d\n", ret_false);

    if (ret_true == 0){
      led = 1;
      printf("Turning on the LED\n");
    }
    else{
      led = 0;
      printf("Turning off the LED\n");
    }
    gpio_set_level(BLINK_GPIO, led);
    /*
    if (strncmp(*prev_led_in, *led_in, 1) != 0) {
      if (strncmp(*led_in, "false", 1) == 0){
        led = 0;
        printf("Turning off the LED\n");
      }
      else if (strncmp(*led_in, "true", 1) == 0) {
        led = 1;
        printf("Turning on the LED\n");
      }
      //else
      //  break;
      gpio_set_level(BLINK_GPIO, led);
      prev_led_in = led_in;
      // vTaskDelay(0.95 * 2000 / portTICK_PERIOD_MS);
    }*/
    vTaskDelay(0.95 * 2000 / portTICK_PERIOD_MS);
  }
}

////////////////////////////////////////////////////////////////////////////////

// For UDP Client
////////////////////////////////////////////////////////////////////////////////
// UDP client
int valueX100(float value){
  return (int)(100 * value);
}

int count_int_digit(int value){
  int num = 0;
  while(value > 0){
    value /= 10;
    num++;
  }
  return num;
}

char int2char(int value){
  char ch = value + '0';
  return ch;
}

void value2str(float temp){
  int tempX100;
  if (temp < 0)
    tempX100 = valueX100(-temp);   // conver "temp x 100" to an integer
  else
    tempX100 = valueX100(temp);
  int count_temp_size = count_int_digit(tempX100);   // count # of digit in "temp"
  int temp_int = tempX100 / 100;     // get the integer part of temp
  int temp_dec = tempX100 - (tempX100 / 100) * 100;     // get the decimal part of temp
  int str_size;   // string size: # of char needed

  // convert integer part into a string
  int count_int;
  if (temp_int == 0) {
    if (temp < 0)
      count_int = 2;
    else
      count_int = 1;
  }
  else{ // temp_int > 0
    if (temp < 0)
      count_int = count_int_digit(temp_int) + 1;
    else
      count_int =count_int_digit(temp_int);
  }
  char dec[count_int];
  memset(dec, 0, 10);
  char c;
  int i = count_int - 1;
  if (temp_int == 0) {
    if (temp < 0){
      dec[0] = '-';
      dec[1] = '0';
      str_size = 6; // add: '-', '0', '.', ','
    }
    else if (temp > 0){
      dec[0] ='0';
      str_size = count_temp_size + 3; // add: '0', '.', ','
    }
    else{ // temp == 0
      dec[0] = '0';
      str_size = 5;
    }
  }
  else{   // temp_int > 0
    if (temp < 0){
      dec[0] = '-';
      str_size = count_temp_size + 3; // add: '-', '.', ','
    }
    else{
      str_size = count_temp_size + 2; // add: '.', ','
    }

    while(i >= 0 && temp_int > 0){
      c = int2char(temp_int%10);
      temp_int = temp_int / 10;
      dec[i] = c;
      i--;
    }
  }

  // add .
  c = '.';
  strncat(dec, &c, 1);

  // add decimals
  c = int2char(temp_dec / 10);
  strncat(dec, &c, 1);
  c = int2char(temp_dec % 10);
  strncat(dec, &c, 1);

  // add ","
  c = ',';
  strncat(dec, &c, 1);

  // update global variable payload
  for (int i = start_index; i < start_index + str_size; i++){
    payload[i] = dec[i - start_index];
  }
  start_index += str_size;
}

void update_payload(){
  value2str(temperature);
  value2str(accel_x);
  value2str(accel_y);
  value2str(accel_z);
}

static void udp_client_task(void *pvParameters)
{
    char rx_buffer[128];
    char host_ip[] = HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;

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
            update_payload();
            int err = sendto(sock, &payload, start_index-1, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
            start_index = 0;  // reset start_index to 0
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                break;
            }
            ESP_LOGI(TAG, "Message sent");

            struct sockaddr_in source_addr; // Large enough for both IPv4 or IPv6
            socklen_t socklen = sizeof(source_addr);
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);
            led_in = rx_buffer;
            //printf("\nrx_buffer: %s\n", rx_buffer);
            //printf("\nled_in: %s:\n", led_in);
            // printf("rx_buffer: %s:", rx_buffer);

            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            }
            // Data received
            else {
                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
                ESP_LOGI(TAG, "%s", rx_buffer);
                if (strncmp(rx_buffer, "OK: ", 4) == 0) {
                    ESP_LOGI(TAG, "Received expected message, reconnecting");
                    break;
                }
            }
            // char *trial_c = "01";
            // int size =
            // char ch = *rx_buffer;  // !!! get the LED value
            // led_in = atoi(*trial_c);

            vTaskDelay(2000 / portTICK_PERIOD_MS);
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

    // For Accelerometor
    // Routine
    // i2c
    i2c_master_init();
    i2c_scanner();

    // Check for ADXL343
    uint8_t deviceID;
    getDeviceID(&deviceID);
    if (deviceID == 0xE5) {
      printf("\n>> Found ADAXL343\n");
    }

    // Disable interrupts
    writeRegister(ADXL343_REG_INT_ENABLE, 0);

    // Set range
    setRange(ADXL343_RANGE_16_G);
    // Display range
    printf  ("- Range:         +/- ");
    switch(getRange()) {
      case ADXL343_RANGE_16_G:
        printf  ("16 ");
        break;
      case ADXL343_RANGE_8_G:
        printf  ("8 ");
        break;
      case ADXL343_RANGE_4_G:
        printf  ("4 ");
        break;
      case ADXL343_RANGE_2_G:
        printf  ("2 ");
        break;
      default:
        printf  ("?? ");
        break;
    }
    printf(" g\n");

    // Display data rate
    printf ("- Data Rate:    ");
    switch(getDataRate()) {
      case ADXL343_DATARATE_3200_HZ:
        printf  ("3200 ");
        break;
      case ADXL343_DATARATE_1600_HZ:
        printf  ("1600 ");
        break;
      case ADXL343_DATARATE_800_HZ:
        printf  ("800 ");
        break;
      case ADXL343_DATARATE_400_HZ:
        printf  ("400 ");
        break;
      case ADXL343_DATARATE_200_HZ:
        printf  ("200 ");
        break;
      case ADXL343_DATARATE_100_HZ:
        printf  ("100 ");
        break;
      case ADXL343_DATARATE_50_HZ:
        printf  ("50 ");
        break;
      case ADXL343_DATARATE_25_HZ:
        printf  ("25 ");
        break;
      case ADXL343_DATARATE_12_5_HZ:
        printf  ("12.5 ");
        break;
      case ADXL343_DATARATE_6_25HZ:
        printf  ("6.25 ");
        break;
      case ADXL343_DATARATE_3_13_HZ:
        printf  ("3.13 ");
        break;
      case ADXL343_DATARATE_1_56_HZ:
        printf  ("1.56 ");
        break;
      case ADXL343_DATARATE_0_78_HZ:
        printf  ("0.78 ");
        break;
      case ADXL343_DATARATE_0_39_HZ:
        printf  ("0.39 ");
        break;
      case ADXL343_DATARATE_0_20_HZ:
        printf  ("0.20 ");
        break;
      case ADXL343_DATARATE_0_10_HZ:
        printf  ("0.10 ");
        break;
      default:
        printf  ("???? ");
        break;
    }
    printf(" Hz\n\n");

    // For LED:
    gpio_reset_pin(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    // initialize the LED with low power
    gpio_set_level(BLINK_GPIO, led);

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

void app_main() {
  init();

  // Enable measurements
  writeRegister(ADXL343_REG_POWER_CTL, 0x08);
  // Create a FIFO queue for timer-based
  timer_queue = xQueueCreate(10, sizeof(timer_event_t));

  // Create tasks
  xTaskCreate(test_adxl343,"test_adxl343", 4096, NULL, MAX_PRIORITIES-1, NULL);    // poll ADXL343
  xTaskCreate(timer_evt_task, "timer_evt_task", 2048, NULL, MAX_PRIORITIES, NULL);
  xTaskCreate(thermistor_adc_task,"adc task", 4096, NULL, MAX_PRIORITIES-1, NULL);
  // xTaskCreate(alpha_task,"alphanumeric display task", 4096, MAX_PRIORITIES-2, 1, NULL);
  xTaskCreate(led_task,"LED task", 4096, NULL, MAX_PRIORITIES-1, NULL);
  xTaskCreate(udp_client_task, "udp_client", 4096, NULL, MAX_PRIORITIES, NULL);

  alarm_init();   // Initiate alarm using timer API
}
