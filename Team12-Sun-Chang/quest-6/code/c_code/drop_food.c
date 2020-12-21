//Created by Hairuo Sun & Chen-Yu Chang
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"

// For mcpwm
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

// For UDP client
#include <sys/param.h>
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


// For Timer
#include "esp_types.h"
#include "freertos/queue.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"

// For I2C
#include "driver/i2c.h"

// For adc - LIDAR
#include "driver/adc.h"
#include "esp_adc_cal.h"

// For alphanumeric display
#include "esp_vfs_dev.h"
#include "displaychars.h"

//You can get these value from the datasheet of servo you use, in general pulse width varies between 1000 to 2000 mocrosecond
#define SERVO_MIN_PULSEWIDTH 1000 //Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH 2000 //Maximum pulse width in microsecond
#define SERVO_MAX_DEGREE 90 //Maximum angle in degree upto which servo can rotate

// For RTOS
#define MAX_PRIORITIES                     2  // max priority for RTOS tasks

// For TIMER
#define TIMER_DIVIDER         16    //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // to seconds
#define TIMER_INTERVAL_SEC   (2)    // Sample test interval for the first timer
#define TEST_WITH_RELOAD      1     // Testing will be done with auto reload
#define MAX_COUNT             100

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

// For Alphanumeric Display
// different sdk/scl pins for alphanumeric display
#define I2C_EXAMPLE_MASTER_SCL_IO_DISPLAY 15
#define I2C_EXAMPLE_MASTER_SDA_IO_DISPLAY 32
#define I2C_EXAMPLE_MASTER_NUM_DISPLAY    I2C_NUM_1  // i2c port for alphanumeric
// 14-Segment Display
#define SLAVE_ADDR_ALPHA                   0x70 // alphanumeric address
#define OSC                                0x21 // oscillator cmd
#define HT16K33_BLINK_DISPLAYON            0x01 // Display on cmd
#define HT16K33_BLINK_OFF                  0    // Blink off cmd
#define HT16K33_BLINK_CMD                  0x80 // Blink cmd
#define HT16K33_CMD_BRIGHTNESS             0xE0 // Brightness cmd


// For adc
#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   10          //Multisampling   // modified by [hrsun] - Note: original: 64

// LIDARLite_v4LED slave address
#define SLAVE_ADDR                         0x62 // slave address

// For UDP Client
#define HOST_IP_ADDR "10.0.0.138"
#define PORT         1131
static const char *TAG = "ESP32";
// float temperature = 27.4, x = 1.2, y = 1.3, z = 9.8;
char payload[128] = " ";

// For UDP server
#define MY_IP_ADDR   "10.0.0.230"  // ESP32 1 IP Address
#define PORT1        1136   // ESP32 1 Listen Port - for receving

// global variables
double lidar_distance = 0.0;
double alpha_value = 0.0;
// Flags
bool drop_flag = false; // For mcpwm servo spin: drop food flag
bool udp_flag = false;
bool lidar_flag = false;
bool alpha_flag = false;


static void mcpwm_gpio_initialize(void)
{
    printf("initializing mcpwm servo control gpio......\n");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, 18);    //Set GPIO 18 as PWM0A, to which servo is connected
}

// Use this function to calcute pulse width for per degree rotation
// param  degree_of_rotation the angle in degree to which servo has to rotate
// return: calculated pulse width
static uint32_t servo_per_degree_init(uint32_t degree_of_rotation)
{
    uint32_t cal_pulsewidth = 0;
    cal_pulsewidth = (SERVO_MIN_PULSEWIDTH + (((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * (degree_of_rotation)) / (SERVO_MAX_DEGREE)));
    return cal_pulsewidth;
}

void mcpwm_servo_control(void *arg)
{
  printf("Testing servo motor.......\n");
  uint32_t angle;
  //1. mcpwm gpio initialization
  mcpwm_gpio_initialize();

  //2. initial mcpwm configuration
  printf("Configuring Initial Parameters of mcpwm......\n");
  mcpwm_config_t pwm_config;
  pwm_config.frequency = 50;    //frequency = 50Hz, i.e. for every servo motor time period should be 20ms
  pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
  pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
  pwm_config.counter_mode = MCPWM_UP_COUNTER;
  pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
  mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings

  while (1){
    if (drop_flag){
      printf("Drop\n");

      // servo roll down
      angle = servo_per_degree_init(SERVO_MAX_DEGREE);
      // printf("pulse width: %dus\n", angle);
      mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, angle);

      vTaskDelay(500 / portTICK_RATE_MS);

      // servo roll up
      angle = servo_per_degree_init(0);
      // printf("pulse width: %dus\n\n", angle);
      mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, angle);

      drop_flag = false;
    }

    vTaskDelay(500 / portTICK_RATE_MS);
  }
}


////////////////////////////////////////////////////////////////////////////////
// For UDP
static void udp_client_task(void *pvParameters)
{
    char rx_buffer[128];
    char host_ip[] = HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;
    // char payload[4] = "flag";  // need modification

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

                char drop_str[] = "d";
                if (strncmp(rx_buffer, drop_str, 1) == 0){
                  drop_flag = true;
                  printf("Drop Button Pushed\n");
                }

            }
            printf("DEBUG");
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

static void udp_server_task(void *pvParameters)
{
    char rx_buffer[128];
    char addr_str[128];
    int addr_family = (int)pvParameters;
    addr_family = AF_INET;  // I added this line
    int ip_protocol = 0;
    struct sockaddr_in6 dest_addr;

    //printf("\nI am in udp_server_task!\n");

    while (1) {
      //printf("\nI am in udp_server_task outer while loop!\n");
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = inet_addr(MY_IP_ADDR); // I modifiied this part. Originall: = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(PORT1);
        ip_protocol = IPPROTO_IP;


        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created");


        int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0) {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        }
        ESP_LOGI(TAG, "Socket bound, port %d", PORT1);

        while (1) {
            ESP_LOGI(TAG, "Waiting for data");
            struct sockaddr_in6 source_addr; // Large enough for both IPv4 or IPv6
            socklen_t socklen = sizeof(source_addr);
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            }
            // Data received
            else {
                // Get the sender's ip address as string
                if (source_addr.sin6_family == PF_INET) {
                    inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
                } else if (source_addr.sin6_family == PF_INET6) {
                    inet6_ntoa_r(source_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
                }

                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string...
                ESP_LOGI(TAG, "Received %d bytes from %s. Received Message: %s", len, addr_str, rx_buffer);

                sprintf(payload, "%s", rx_buffer);

            } // end of else loop - "Data received"
            // vTaskDelay(200 / portTICK_PERIOD_MS);   // I added later - need modification (potential watchdog issue)
        }

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
// For I2C
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
  int16_t distance;
  uint8_t busyflag = 1;

  while (1) {
    writeRegister(0x00, 0x04);  // start the read/write - modification? do I need to writeRegister everytime?

    // check busy flag
    do {
      busyflag = 0x01 & readRegister(0x01);   // modificatin? hex to dec in "while" loop?
    } while(busyflag == 1);

    // read LIDAR data
    distance = read16(0x10);
    printf("Distance: %d cm\n", distance);    // ? modification - should I convert it from HEX to DEC?

    // update alphanumeric display value
    alpha_value = distance;

    // reset busy flag
    busyflag = 1;
    vTaskDelay(500 / portTICK_RATE_MS);
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
        // temp = car_speed * 1000;
        temp = alpha_value;
        for(int x = 3; x >= 0; x--)
        {
            int rem = temp % 10;
            // printf("%d\n", rem);
            displaybuffer[x] = FourteenSegmentASCII[rem];
            temp = temp / 10;
        }
        // Add dot to the first number
        // displaybuffer[0] |= 1 << 14;

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
          printf("Action!\n");
          udp_flag = true;
          lidar_flag = true;
          alpha_flag = true;
        }

    }
}
////////////////////////////////////////////////////////////////////////////////

void init(){
  // I2C
  i2c_master_init();
  i2c_master_init2();
  i2c_scanner();


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
  init();
  timer_queue = xQueueCreate(10, sizeof(timer_event_t));  // Create a FIFO queue for timer-based

  xTaskCreate(udp_client_task, "udp_client_task", 4096, NULL, MAX_PRIORITIES-1, NULL);
  xTaskCreate(udp_server_task, "udp_server task", 4096, NULL, MAX_PRIORITIES-1, NULL);
  xTaskCreate(mcpwm_servo_control, "mcpwm_servo_control", 4096, NULL, MAX_PRIORITIES-1, NULL);
  xTaskCreate(test_LIDARLite_v4LED,"test_LIDARLite_v4LED", 4096, NULL, MAX_PRIORITIES-1, NULL);
  xTaskCreate(alphaTask,"alphanumeric display task", 4096, MAX_PRIORITIES-2, 1, NULL);

  xTaskCreate(timer_evt_task, "timer_evt_task", 2048, NULL, MAX_PRIORITIES, NULL);    // timer task
  alarm_init(); // Initiate alarm using timer API
}
