// Written by Hairuo Sun & Chen-Yu Chang
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt.h"
#include "soc/rmt_reg.h"
#include "driver/uart.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "soc/uart_reg.h"

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

// For initializeing an array of strings (also need: <stdio.h>, <string.h> included above already)
#include <stdlib.h>

// RMT definitions
#define RMT_TX_CHANNEL    1     // RMT channel for transmitter
#define RMT_TX_GPIO_NUM   25    // GPIO number for transmitter signal -- A1
#define RMT_CLK_DIV       100   // RMT counter clock divider
#define RMT_TICK_10_US    (80000000/RMT_CLK_DIV/100000)   // RMT counter value for 10 us.(Source clock is APB clock)
#define rmt_item32_tIMEOUT_US   9500     // RMT receiver timeout value(us)

// UART definitions
#define UART_TX_GPIO_NUM 26 // A0
#define UART_RX_GPIO_NUM 34 // A2 - connected with IR Receiver Diode - TSOP38238
#define BUF_SIZE (1024)

// Hardware interrupt definitions
#define GPIO_INPUT_IO_1       4 // A5
#define ESP_INTR_FLAG_DEFAULT 0
#define GPIO_INPUT_PIN_SEL    1ULL<<GPIO_INPUT_IO_1

// Button Define for changing led - I added
#define BUTTON_GPIO27 27  // pushbutton2

// LED Output pins definitions
#define BLUEPIN   14
#define GREENPIN  32
#define REDPIN    15
#define ONBOARD   13

#define TIMER_DIVIDER         16    //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // to seconds
#define TIMER_INTERVAL_2_SEC  (2)
#define TIMER_INTERVAL_10_SEC (10)
#define TIMER_INTERVAL_1_SEC (1)
#define TEST_WITH_RELOAD      1     // Testing will be done with auto reload

// For RTOS
#define MAX_PRIORITIES                     5  // max priority for RTOS tasks  // !!! need modification

// For FSM
// 3 states
#define ELECTION_STATE        0
#define NON_LEADER_STATE      1
#define LEADER_STATE          2
#define INIT_ELECTION_STATE   3
// Timeouts counts (in sec)
#define ELECTION_TOUT         13    // timer1 timeout #
#define LEADER_TOUT           9    // timer2 timeout #
#define HEARTBEAT_TOUT        3    // timer3 timeout #

// For UDP Client
// ESP32 devices (devices name): IP: Listening PORT <- (Port Forward(!MUST AVOID LISTENING ON THESE PORTS))
// ESP32 1 (espressif): "10.0.0.230": 1136 <- (AVOID 1140)
// ESP32 2 (espressif-5): "10.0.0.31": 1137 <- (AVOID 1138)
// ESP32 3 (espressif-6): "10.0.0.175": 1139 <- (AVOID 1134)
// Note: PORT1 RECEIVING & other ports SENDING


// Uncomment for ESP32 1
#define ID            0
#define COLOR         'G'
#define MY_IP_ADDR   "10.0.0.230"  // ESP32 1 IP Address
#define HOST_IP_ADDR2 "10.0.0.31" // ESP32 2 IP Address
#define HOST_IP_ADDR3 "10.0.0.175"  // ESP32 3 IP Address
#define PORT1        1136   // ESP32 1 Listen Port - for receving
const int port_send_array[] = {1137, 1139};
char **ip_send_array;
static const char *TAG = "ESP32 1"; // send from: ESP32 1


/*
// Uncomment for ESP32 2
#define ID            1
#define COLOR         'G'
#define MY_IP_ADDR    "10.0.0.31"  // ESP32 2 IP Address
#define HOST_IP_ADDR2 "10.0.0.230"  // ESP32 1 IP Address
#define HOST_IP_ADDR3 "10.0.0.175"  // ESP32 3 IP Address
#define PORT1        1137   // ESP32 2 Listen Port - for receving
const int port_send_array[] = {1136, 1139};
char **ip_send_array;
static const char *TAG = "ESP32 2"; // send from: ESP32 2
*/

/*
// Uncomment for ESP32 3
#define ID            2
#define COLOR         'G'
#define MY_IP_ADDR   "10.0.0.175"  // ESP32 3 IP Address
#define HOST_IP_ADDR2 "10.0.0.230"  // ESP32 1 IP Address
#define HOST_IP_ADDR3 "10.0.0.31" // ESP32 2 IP Address
#define PORT1        1139   // ESP32 3 Listen Port - for receving
const int port_send_array[] = {1136, 1137};
char **ip_send_array;
static const char *TAG = "ESP32 3"; // send from: ESP32 3
*/

// For UDP Client
#define HOST_IP_JS      "10.0.0.138"
#define PORT_JS         1131

char payload[40] = " ";
char recv_payload[128] = " ";
char **ip_array;
int port_array[3];

// Variables for my ID, minID and status plus string fragments
char start = 0x1B;
char myID = (char) ID;
int minID = ID;   // initialize minID with myID
char myColor = (char) COLOR;
int len_out = 4;
bool button_flag = false;
bool led_change_flag = false;

// FSM current state & timer initialization
// initialize current state
int state = INIT_ELECTION_STATE;    // initialize current state to "INIT_ELECTION_STATE" - (modification) - initially (ELECTION_STATE)
// Current Time for each timer
int election_timer = ELECTION_TOUT;   // timer1
int leader_timer = LEADER_TOUT;     // timer2
int heartbeat_timer = HEARTBEAT_TOUT;  // timer3

// FSM Flags
// init flags
bool init_election_send_flag = true; // for initial ip sending - FLOODING
bool init_election_recv_flag = true; // for updating ip_array - flag to set once "count" reaches a #
int init_update_count = 0;   // for updating ip_array - only update ip_array once
// election flags
bool send_lowerIDs_flag = false;  // send messages to all lower IDs
bool recv_election_flag = false;
bool send_okay_flag = false;
bool recv_okay_flag = false;
// ledaer flag
bool recv_from_leader_flag = false;
bool broadcast_flag = false;


// For voting
char voteID = (char) ID;    // initialization - this value need modification
bool vote_send_leader_flag = false;
bool vote_send_server_flag = false;
char udp_vote_str[10];


// Button Define for changing led - I added
#define BUTTON_GPIO27 27  // pushbutton2

// Mutex (for resources), and Queues (for button)
SemaphoreHandle_t mux = NULL;
static xQueueHandle gpio_evt_queue = NULL;
static xQueueHandle timer_queue;

// A simple structure to pass "events" to main task
typedef struct {
    int flag;     // flag for enabling stuff in timer task
} timer_event_t;

// System tags
static const char *TAG_SYSTEM = "system";       // For debug logs

// Button interrupt handler -- add to queue
static void IRAM_ATTR gpio_isr_handler(void* arg){
  uint32_t gpio_num = (uint32_t) arg;
  xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

// ISR handler
void IRAM_ATTR timer_group0_isr(void *para) {

    // Prepare basic event data, aka set flag
    timer_event_t evt;
    evt.flag = 1;

    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, TIMER_INTERVAL_1_SEC * TIMER_SCALE);
    /*
    // BLUE is shorter
    if (myColor == 'G') {
      timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, TIMER_INTERVAL_2_SEC * TIMER_SCALE);
    }
    else {
      timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, TIMER_INTERVAL_10_SEC * TIMER_SCALE);
    }
*/
    // Clear the interrupt, Timer 0 in group 0
    TIMERG0.int_clr_timers.t0 = 1;

    // After the alarm triggers, we need to re-enable it to trigger it next time
    TIMERG0.hw_timer[TIMER_0].config.alarm_en = TIMER_ALARM_EN;

    // Send the event data back to the main program task
    xQueueSendFromISR(timer_queue, &evt, NULL);
}

// Utilities ///////////////////////////////////////////////////////////////////

// Checksum
char genCheckSum(char *p, int len) {
  char temp = 0;
  for (int i = 0; i < len; i++){
    temp = temp^p[i];
  }
  // printf("%X\n",temp);

  return temp;
}
bool checkCheckSum(uint8_t *p, int len) {
  char temp = (char) 0;
  bool isValid;
  for (int i = 0; i < len-1; i++){
    temp = temp^p[i];
  }
  // printf("Check: %02X ", temp);
  if (temp == p[len-1]) {
    isValid = true; }
  else {
    isValid = false; }
  return isValid;
}


// Tasks ///////////////////////////////////////////////////////////////////////
// Button task -- rotate through myIDs
void button_task(){
  uint32_t io_num;
  while(1) {
    if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
      xSemaphoreTake(mux, portMAX_DELAY);
      xSemaphoreGive(mux);
      button_flag = true;
      printf("Button pressed.\n\n");
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }

}

// TASK B: pushbutton
void toggle_led_button_task() {    // pushbutton
  while(1){
    if ((gpio_get_level(BUTTON_GPIO27)) == 0){
      vTaskDelay(150 / portTICK_RATE_MS);
      if ((gpio_get_level(BUTTON_GPIO27)) == 0){
        if (led_change_flag)
          led_change_flag = false;
        else
          led_change_flag = true;
      }
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void led_helper(){
  switch((int)myColor){
    case 'R' : // Red
      gpio_set_level(GREENPIN, 0);
      gpio_set_level(REDPIN, 1);
      gpio_set_level(BLUEPIN, 0);
      // printf("Current state: %c\n",status);
      break;
    case 'B' : // BLUE
      gpio_set_level(GREENPIN, 0);
      gpio_set_level(REDPIN, 0);
      gpio_set_level(BLUEPIN, 1);
      // printf("Current state: %c\n",status);
      break;
    case 'G' : // Green
      gpio_set_level(GREENPIN, 1);
      gpio_set_level(REDPIN, 0);
      gpio_set_level(BLUEPIN, 0);
      // printf("Current state: %c\n",status);
      break;
  }
}

// LED task to light LED based on traffic state
void led_task(){
  while(1) {
    if (led_change_flag){
      // For LED - can be deleted later
      if (myColor == 'R') {
        myColor = 'G';
      }
      else if (myColor == 'G') {
        myColor = 'B';
      }
      else if (myColor == 'B') {
        myColor = 'R';
      }
      led_helper();
      led_change_flag = false;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// Send task -- sends payload | Start | myID | Start | myID
void send_task(){
  while(1) {
    if (button_flag){
      char *data_out = (char *) malloc(len_out);
      xSemaphoreTake(mux, portMAX_DELAY);
      data_out[0] = start;
      data_out[1] = (char) myColor;
      data_out[2] = (char) myID;
      data_out[3] = genCheckSum(data_out,len_out-1);
      // ESP_LOG_BUFFER_HEXDUMP(TAG_SYSTEM, data_out, len_out, ESP_LOG_INFO);

      uart_write_bytes(UART_NUM_1, data_out, len_out);
      xSemaphoreGive(mux);
      // printf("sending data");
      button_flag = false;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// Receives task -- looks for Start byte then stores received values
void recv_task(){
  // Buffer for input data
  uint8_t *data_in = (uint8_t *) malloc(BUF_SIZE);
  while (1) {
    int len_in = uart_read_bytes(UART_NUM_1, data_in, BUF_SIZE, 20 / portTICK_RATE_MS);
    if (len_in >0) {
      if (data_in[0] == start) {
        if (checkCheckSum(data_in,len_out)) {
           printf("\nData Received!\n");
          ESP_LOG_BUFFER_HEXDUMP(TAG_SYSTEM, data_in, len_out, ESP_LOG_INFO);
          if (data_in[1] != myColor && (data_in[1] == 'R' || data_in[1] == 'B' || data_in[1] == 'G' )){
            printf("Color Changed!\n\n");
            myColor = data_in[1]; // match the receiver led light of the transmitter led
            voteID = (char)data_in[2];

            if (minID == (int)(myID)) {        // send data to server directly as poll leader
              sprintf(udp_vote_str, "%d %c", data_in[2], myColor);
              vote_send_server_flag = true;
              printf("\nPoll Leader received vote. Vote Info [VoteID ColorVoted]: %d %c\n", data_in[2], myColor);
            }
            else                              // send data to poll leader
              vote_send_leader_flag = true;

            led_helper();
          } // end of checking different data

        }
      }
    }
    else{
      // printf("Nothing received.\n");
    }


    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
  free(data_in);
}

// Timer task -- R (10 seconds), G (10 seconds), Y (2 seconds)
static void timer_evt_task(void *arg) {
    while (1) {
        // Create dummy structure to store structure from queue
        timer_event_t evt;

        // Transfer from queue
        xQueueReceive(timer_queue, &evt, portMAX_DELAY);

        // Do something if triggered!
        if (evt.flag == 1) {
            printf("Action!\n");
            election_timer -= 1;
            leader_timer -= 1;
            heartbeat_timer -= 1;
        }
    }
}

// UDP tasks////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void update_payload_std_msg(){
  sprintf(payload, "%d %d %d", myID, minID, state);
  // printf("\nDEBUG - strlen(payload): %d\n",strlen(payload));
  // printf("\nDEBUG - payload: %s\n", payload);
}

static void udp_client_task(void *pvParameters)
{
    char rx_buffer[128];
    // char host_ip[] = HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;

    while (1) {
        struct sockaddr_in dest_addr;
        // dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
        dest_addr.sin_family = AF_INET;
        // dest_addr.sin_port = htons(PORT2);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        // ESP_LOGI(TAG, "Socket created, sending to %s:%d", HOST_IP_ADDR, PORT2);
        ESP_LOGI(TAG, "Socket created!");

        int init_sending_count = 3;

        while (1) {
            // initial IP message sending
            if (init_election_send_flag){
              update_payload_std_msg();
              int send_array_size = (sizeof(port_send_array) / sizeof(int));

              if (init_sending_count >=0){
                for (int i = 0; i < send_array_size; i++){
                  // reconfigure sending IP & Port
                  dest_addr.sin_addr.s_addr = inet_addr(ip_send_array[i]);
                  dest_addr.sin_port = htons(port_send_array[i]);
                  int err = sendto(sock, &payload, strlen(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                  if (err < 0) {
                      ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                      break;
                  }
                  ESP_LOGI(TAG, "Initialization Sending - Message sent to %s:%d. Message: %s",  ip_send_array[i], port_send_array[i], payload);  // I modified
                } // end of for loop
                init_sending_count -= 1;

              }
              else{
                init_election_send_flag = false; // uncommented later (modification)
                printf("\nNew Device Initial Election Done!\n");
              }
              vTaskDelay(2000 / portTICK_PERIOD_MS);
            }

            // 2nd or later times of message sending - send Msgs to all lower IDs
            else if (send_lowerIDs_flag){
              update_payload_std_msg();
              for (int i = 0; i < myID; i++){
                // reconfigure sending IP & Port
                dest_addr.sin_addr.s_addr = inet_addr(ip_array[i]);
                dest_addr.sin_port = htons(port_array[i]);
                int err = sendto(sock, &payload, strlen(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                if (err < 0) {
                    ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                    break;
                }
                ESP_LOGI(TAG, "LOWER ID MSG SENT - Message sent to %s:%d. Message: %s",  ip_array[i], port_array[i], payload);  // I modified
              } // end of for loop
              send_lowerIDs_flag = false;
            }

            // 2nd or later times of message sending - Broadcast messages to all higher fobs - as a leader
            else if (broadcast_flag){
              update_payload_std_msg();
              for (int i = myID + 1; i < (sizeof(ip_array) - 1); i++){
                // reconfigure sending IP & Port
                dest_addr.sin_addr.s_addr = inet_addr(ip_array[i]);
                dest_addr.sin_port = htons(port_array[i]);
                int err = sendto(sock, &payload, strlen(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                if (err < 0) {
                    ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                    break;
                }
                ESP_LOGI(TAG, "BROADCAST (to all higher IDs) - Message sent to %s:%d. Message: %s",  ip_array[i], port_array[i], payload);  // I modified
              } // end of for loop
              broadcast_flag = false;
            }

            // 2nd or later times of message sending - send "ok" to all higher IDs
            if (send_okay_flag){
              sprintf(payload, "ok %d", ((int)(myID)));   // update payload = "ok"
              for (int i = (int)(myID) + 1; i < (sizeof(ip_array) - 1); i++){
                // reconfigure sending IP & Port
                dest_addr.sin_addr.s_addr = inet_addr(ip_array[i]);
                dest_addr.sin_port = htons(port_array[i]);
                int err = sendto(sock, &payload, strlen(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                if (err < 0) {
                    ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                    break;
                }
                ESP_LOGI(TAG, "SEND OKAY! - Message sent to %s:%d. Message: %s",  ip_array[i], port_array[i], payload);  // I modified
              } // end of for loop
              send_okay_flag = false;
            }   // (modification) - needs to become "else if" again - or does it???

            // voting conditions
            // send vote to only poll leader
            if (vote_send_leader_flag){
              sprintf(payload, "vote %d %c", ((int)(voteID)), myColor);   // update payload = "vote [myID] [colorVoted]"
              // reconfigure sending IP & Port
              dest_addr.sin_addr.s_addr = inet_addr(ip_array[minID]);
              dest_addr.sin_port = htons(port_array[minID]);
              int err = sendto(sock, &payload, strlen(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
              if (err < 0) {
                  ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                  break;
              }
              ESP_LOGI(TAG, "SEND VOTE! - Message sent to %s:%d. Message: %s",  ip_array[minID], port_array[minID], payload);  // I modified
              printf("\nVote is sent to Poll Leader!\n");
              vote_send_leader_flag = false;
            }


            if (vote_send_server_flag){
              sprintf(payload, udp_vote_str);   // update payload = "vote [myID] [colorVoted]"
              // reconfigure sending IP & Port
              dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_JS);
              dest_addr.sin_port = htons(PORT_JS);
              int err = sendto(sock, &payload, strlen(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
              if (err < 0) {
                  ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                  break;
              }
              ESP_LOGI(TAG, "SEND VOTE to Node.js DataBase! - Message sent to %s:%d. Message: %s",  HOST_IP_JS, PORT_JS, payload);  // I modified
              printf("\nVote is sent to Node.js Database!\n");
              vote_send_server_flag = false;
            }
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}

// get other_minID from "ok myID" message recevied from other fobs
void parse_ok_msg(char *ok_str_recvd){
  char delim[] = " ";
  char *ptr = strtok(ok_str_recvd, delim);  // get "ok" message
  ptr = strtok(NULL, delim);  // get otherID
  int otherID = atoi(ptr);
  if (minID > otherID){
    minID = otherID;
  }
}

// pase incoming messages, update ip_array & port_array
void parse_recv_payload(char *ip_str){
  // parse the recv_payload
  char delim[] = " ";

  // 1. get other esp32's ID
  char *ptr = strtok(recv_payload, delim);
  int otherID = atoi(ptr);

  // 2. update minID
  ptr = strtok(NULL, delim);  // get minID & update minID - leader
  int other_minID = atoi(ptr);
  if (minID > other_minID){
    minID = other_minID;
  }
  else if (minID < other_minID){
    if (!init_election_send_flag){
      send_okay_flag = true;        // send okay msg to confirm that I have received lower ID - might need modification?
    }
  }

  // 3. update election & leader flags??? -- (modification)
  ptr = strtok(NULL, delim);  // get state
  int other_state = atoi(ptr);
  if (other_state == LEADER_STATE){   // check if received from leader - (modification maybe)
    recv_from_leader_flag = true;
  }
  else if (other_state == ELECTION_STATE || other_state == INIT_ELECTION_STATE){  // modification - initially there is no condition for INIT_ELECTION_STATE
    recv_election_flag = true;
  }
  // printf("\nDEBUG - other_state: %s", ptr);

  // update initial ip_array & port_array
  if (init_election_recv_flag){
    // update ip_array
    sprintf(ip_array[otherID], ip_str);
    // update port_array
    for (int i = 0; i < (sizeof(port_send_array) / sizeof(int)); i++){
      if (strcmp(ip_array[otherID], ip_send_array[i]) == 0){
        port_array[otherID] = port_send_array[i];
      }
    }
    /*
    // print ip_array - (modification) need to be commented out later
    for (int i = 0; i < sizeof(ip_array) - 1; i++){
      printf("ip_array: %s\n", ip_array[i]);
    }
    // print port_array - (modification) need to be commented out later
    for (int i = 0; i < (sizeof(port_array) / sizeof(int)); i++){
      printf("port_array: %d\n", port_array[i]);
    }
    */

    // only update IP array # of times: # = number of IP addresses to receive from
    init_update_count += 1;
    if (init_update_count == ((sizeof(ip_array) - 2) * 3)){
      init_election_recv_flag = false;
    }

  } // end of if loop ("init_election_recv_flag")



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
            sprintf(recv_payload, "%s", rx_buffer); // receive payload
            // printf("\nDEBUG - rx_buffer len: %d", len);
            // printf("\nDEBUG - rx_buffer: %s\n", rx_buffer);

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

                // check if recieving okay
                char ok_str[] = "ok";
                char vote_str[] = "vote";
                if (strncmp(rx_buffer, ok_str, 2) == 0){   // okay received
                  parse_ok_msg(rx_buffer);    // parse okay_string & reset minID
                  recv_okay_flag = true;
                  //printf("DEBUG - received okay!");
                }
                else if (strncmp(rx_buffer, vote_str, 4) == 0){   // vote received
                  strncpy(udp_vote_str,rx_buffer+5, 3);
                  vote_send_server_flag = true; // (modification) - uncomment later
                  printf("\nPoll Leader received vote. Vote Info [VoteID ColorVoted]: %s\n", udp_vote_str);

                  // update poll leader led
                  myColor = rx_buffer[7];
                  led_helper();
                }

                else{   // okay not received, just received standard msg
                  parse_recv_payload(addr_str); // send ip & parse data: IP array, port array, other_state & minID(leader)
                }

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

// FSM Task ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void fsm_task(){
  // while loop
  while (1){
    if (init_election_send_flag){
      vTaskDelay(100 / portTICK_PERIOD_MS);
      continue;
    } // check if initilization is done
    vTaskDelay(2000 / portTICK_PERIOD_MS);    // wait 2 seconds in case there are additional msg to receive

    printf("\nFSM: myID = %d, minID = %d, ", (int)(myID), minID);    // print out my current status
    recv_okay_flag = false;                                          // ! reset "recv_okay_flag" (important)

    // FSM states conditional if/elseif blocks start
    // 0. ELECTION_STATE
    if (state == ELECTION_STATE){
      printf("myState = ELECTION_STATE\n");                   // print out my current state

      // resets
      election_timer = ELECTION_TOUT;                         // reinitialize election_timer
      minID = ID;                                             // reinitialize minID
      // actions
      send_lowerIDs_flag = true;                              // send all lowerID fobs Messages

      // election_timer time_out check
      while (election_timer > 0){
        // do nothing
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }

      // update next state
      // wait for "ok" massage back & record lowerest "ok" msg id: minID'
      if (minID == (int)(myID)){   //determine next state
        state = LEADER_STATE;
        printf("\nI am leader & my ID is: %d\n", minID);    // debug only - comment out later (modification)

        // Update Leader LED Color
        myColor = 'B';
        led_helper();
      }
      else{
        state = NON_LEADER_STATE;
        printf("\nI am non_leader. My leader ID is: %d\n", minID);    // debug only - comment out later (modification)
        // Update Leader LED Color
        myColor = 'R';
        led_helper();
      }

    }

    // 1. LEADER_STATE
    else if (state == LEADER_STATE){
      printf("myState = LEADER_STATE\n");                     // print out my current state
      heartbeat_timer = HEARTBEAT_TOUT;                       // reset heartbeat_timer
      recv_election_flag = false;                             // ??? reset recv_election_flag (modification?)

      while (heartbeat_timer > 0){                            // heartbeat_timer wait period
        if (recv_election_flag){break;}                       // check if receiving re-election message
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }

      // reset next state
      if (recv_election_flag){
        printf("\nRESTART ELECTION!\n");
        state = ELECTION_STATE;

        // Update Election LED Color
        myColor = 'G';
        led_helper();
      }
      else{
        broadcast_flag = true;                                // send to everyone
        state = LEADER_STATE;
      }

    }

    // 2. NON_LEADER_STATE
    else if (state == NON_LEADER_STATE){
      printf("myState = NON_LEADER_STATE\n");                 // print out my current state
      leader_timer = LEADER_TOUT;
      recv_election_flag = false;                             // ??? reset recv_election_flag (modification?)
      recv_from_leader_flag = false;


      while (leader_timer > 0){
        if (recv_election_flag){break;}                       // check if receiving re-election message
        if (recv_from_leader_flag){break;}                    // check if received from leader
        vTaskDelay(100 / portTICK_PERIOD_MS);
        // do nothing
      }

      if (recv_election_flag || (!recv_from_leader_flag)){                               //  receive election
        printf("\nRESTART ELECTION!\n");
        state = ELECTION_STATE;

        // Update Election LED Color
        myColor = 'G';
        led_helper();
      }
      else{
        printf("\nReceived from leader!\n");
        state = NON_LEADER_STATE;
      }

    }

    // 3. INIT_ELECTION_STATE
    else if (state == INIT_ELECTION_STATE){
      printf("myState = INIT_ELECTION_STATE\n");                   // print out my current state
      if (minID == (int)(myID)){
        state = LEADER_STATE;
        printf("\nI am leader & my ID is: %d\n", minID);    // debug only - comment out later (modification)
        // Update Leader LED Color
        myColor = 'B';
        led_helper();
      }
      else{
        state = NON_LEADER_STATE;
        printf("\nI am non_leader. My leader ID is: %d\n", minID);    // debug only - comment out later (modification)
        // Update Leader LED Color
        myColor = 'R';
        led_helper();
      }
    }



  }   // end of FSM outer while loop

}

////////////////////////////////////////////////////////////////////////////////

// Init Functions //////////////////////////////////////////////////////////////
// RMT tx init
static void rmt_tx_init() {
    rmt_config_t rmt_tx;
    rmt_tx.channel = RMT_TX_CHANNEL;
    rmt_tx.gpio_num = RMT_TX_GPIO_NUM;
    rmt_tx.mem_block_num = 1;
    rmt_tx.clk_div = RMT_CLK_DIV;
    rmt_tx.tx_config.loop_en = false;
    rmt_tx.tx_config.carrier_duty_percent = 50;
    // Carrier Frequency of the IR receiver
    rmt_tx.tx_config.carrier_freq_hz = 38000;
    rmt_tx.tx_config.carrier_level = 1;
    rmt_tx.tx_config.carrier_en = 1;
    // Never idle -> aka ontinuous TX of 38kHz pulses
    rmt_tx.tx_config.idle_level = 1;
    rmt_tx.tx_config.idle_output_en = true;
    rmt_tx.rmt_mode = 0;
    rmt_config(&rmt_tx);
    rmt_driver_install(rmt_tx.channel, 0, 0);
}

// Configure UART
static void uart_init() {
  // Basic configs
  uart_config_t uart_config = {
      .baud_rate = 1200, // Slow BAUD rate
      .data_bits = UART_DATA_8_BITS,
      .parity    = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
  };
  uart_param_config(UART_NUM_1, &uart_config);

  // Set UART pins using UART0 default pins
  uart_set_pin(UART_NUM_1, UART_TX_GPIO_NUM, UART_RX_GPIO_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

  // Reverse receive logic line
  uart_set_line_inverse(UART_NUM_1,UART_SIGNAL_RXD_INV);

  // Install UART driver
  uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, 0);
}

// GPIO init for LEDs
static void led_init() {
    gpio_pad_select_gpio(BLUEPIN);
    gpio_pad_select_gpio(GREENPIN);
    gpio_pad_select_gpio(REDPIN);
    gpio_pad_select_gpio(ONBOARD);
    gpio_set_direction(BLUEPIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(GREENPIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(REDPIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(ONBOARD, GPIO_MODE_OUTPUT);

    led_helper(); // initialize the led
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
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, TIMER_INTERVAL_1_SEC * TIMER_SCALE); // original: TIMER_INTERVAL_10_SEC
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

    // button init for toggle led
    gpio_reset_pin(BUTTON_GPIO27);
    gpio_set_direction(BUTTON_GPIO27, GPIO_MODE_INPUT);
}

// UDP init
static void udp_init(){
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
  * Read "Establishing Wi-Fi or Ethernet Connection" section in
  * examples/protocols/README.md for more information about this function.
  */
  ESP_ERROR_CHECK(example_connect());
}

// ip array init
static void ip_array_init(){
  // need modification - temporarily change from 4 fobs to 3 fobs
  ip_array = (char**)malloc(3*sizeof(char*));
  for(int i = 0; i < 3; i++){
        ip_array[i] = (char*)malloc(10*sizeof(char));
    }
  sprintf(ip_array[(int)(myID)], MY_IP_ADDR);
}

// port array init
static void port_array_init(){
  // initialize port array
  port_array[(int)(myID)] = PORT1;
}

// ip send array init
static void ip_send_array_init(){
  ip_send_array = (char**)malloc(2*sizeof(char*));
  for(int i = 0; i < 2; i++){
        ip_send_array[i] = (char*)malloc(15*sizeof(char));
  }
  sprintf(ip_send_array[0], HOST_IP_ADDR2);
  sprintf(ip_send_array[1], HOST_IP_ADDR3);
}


////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
void app_main() {

    // Mutex for current values when sending
    mux = xSemaphoreCreateMutex();

    // Create a FIFO queue for timer-based events
    timer_queue = xQueueCreate(10, sizeof(timer_event_t));

    // Create task to handle timer-based events
    xTaskCreate(timer_evt_task, "timer_evt_task", 2048, NULL, 5, NULL);

    // Initialize all the things
    rmt_tx_init();
    uart_init();
    led_init();
    alarm_init();
    button_init();
    udp_init();
    ip_array_init();
    port_array_init();
    ip_send_array_init();

    // Create tasks for receive, send, set gpio, and button
    xTaskCreate(recv_task, "uart_rx_task", 1024*4, NULL, MAX_PRIORITIES-1, NULL);
    xTaskCreate(send_task, "uart_tx_task", 1024*2, NULL, MAX_PRIORITIES-1, NULL);
    xTaskCreate(led_task, "led_task", 1024*2, NULL, MAX_PRIORITIES-1, NULL);
    xTaskCreate(button_task, "button_task", 1024*2, NULL, MAX_PRIORITIES-1, NULL);
    xTaskCreate(toggle_led_button_task, "toggle_led_button_task", 1024*2, NULL, MAX_PRIORITIES-1, NULL);
    xTaskCreate(udp_client_task, "udp_client task", 4096, NULL, MAX_PRIORITIES, NULL);
    xTaskCreate(udp_server_task, "udp_server task", 4096, NULL, MAX_PRIORITIES, NULL);
    xTaskCreate(fsm_task, "FSM task", 2048, NULL, MAX_PRIORITIES, NULL);
}
