#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

#define GPIO_PWM01_OUT 18   //Set GPIO 18 as PWM01 - ESC
#define GPIO_PWM02_OUT 16   //Set GPIO 16 as PWM02 - Steering Servo

// In general pulse width: [1000, 2000] us
// For ESC
#define SERVO_MIN_DUTY_US 700 //Minimum duty cycle in microsecond
#define SERVO_MAX_DUTY_US 2100 //Maximum duty cycle in microsecond
#define SERVO_MID_DUTY_US 1400 //Medium duty cycle in microsecond

// In general pulse width: [1000, 2000] us
// For Steering Servo
#define SERVO_MIN_PULSEWIDTH 600 //Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH 2700 //Maximum pulse width in microsecond
#define SERVO_MAX_DEGREE 120 //Maximum angle in degree upto which servo can rotate

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
  printf("Testing ESC......\n");
  mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, SERVO_MID_DUTY_US); // NEUTRAL signal in microseconds
  vTaskDelay(3000 / portTICK_PERIOD_MS);  // Give yourself time to turn on crawler
  printf("ESC testing done.\n");
}

void mcpwm_esc_servo_control(void *arg) {
    // calibrate ESC
    calibrateESC();
    // vTaskDelay(2000 / portTICK_PERIOD_MS);

    while (1) {
      printf("forward\n");
      for (int duty_us = SERVO_MID_DUTY_US; duty_us <= SERVO_MAX_DUTY_US; duty_us += 10) {
          mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty_us);
          vTaskDelay(10 / portTICK_PERIOD_MS);     //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
      }
      // vTaskDelay(2000 / portTICK_PERIOD_MS);
      for (int duty_us = SERVO_MAX_DUTY_US; duty_us >= SERVO_MID_DUTY_US; duty_us -= 10) {
          mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty_us);
          vTaskDelay(10 / portTICK_PERIOD_MS);    //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
      }
      vTaskDelay(500 / portTICK_PERIOD_MS); // stop

      printf("backward\n");
      //  double clicking for backward implementation
      // backward 1st time
      for (int duty_us = SERVO_MID_DUTY_US; duty_us >= SERVO_MIN_DUTY_US; duty_us -= 10) {
          mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty_us);
          vTaskDelay(10 / portTICK_PERIOD_MS);     //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
      }
      for (int duty_us = SERVO_MIN_DUTY_US; duty_us <= SERVO_MID_DUTY_US; duty_us += 10) {
          mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty_us);
          vTaskDelay(10 / portTICK_PERIOD_MS);    //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
      }

      // backward 2nd time
      for (int duty_us = SERVO_MID_DUTY_US; duty_us >= SERVO_MIN_DUTY_US; duty_us -= 10) {
          mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty_us);
          vTaskDelay(10 / portTICK_PERIOD_MS);     //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
      }
      for (int duty_us = SERVO_MIN_DUTY_US; duty_us <= SERVO_MID_DUTY_US; duty_us += 10) {
          mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty_us);
          vTaskDelay(10 / portTICK_PERIOD_MS);    //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
      }
      vTaskDelay(1500 / portTICK_PERIOD_MS); // stop
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
  while (1) {
    printf("Center\n");
    count = center_count;
    angle = servo_per_degree_init(count);
    mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, angle);
    vTaskDelay(100);

    printf("Center to Left\n");
    for (count = center_count; count < SERVO_MAX_DEGREE - 20; count++) {
        // printf("Angle of rotation: %d\n", count);
        angle = servo_per_degree_init(count);
        // printf("pulse width: %dus\n", angle);
        mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, angle);
        vTaskDelay(2);     //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
    }

    printf("Left to Right\n");
    for (count = SERVO_MAX_DEGREE - 20; count > 0; count--) {
        // printf("Angle of rotation: %d\n", count);
        angle = servo_per_degree_init(count);
        // printf("pulse width: %dus\n", angle);
        mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, angle);
        vTaskDelay(2);     //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
    }

    printf("Right to Center\n");
    for (count = 0; count < center_count; count++) {
        // printf("Angle of rotation: %d\n", count);
        angle = servo_per_degree_init(count);
        // printf("pulse width: %dus\n", angle);
        mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, angle);
        vTaskDelay(2);     //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation at 5V
    }


  }
}

void app_main(void)
{
    printf("Testing servo motor.......\n");

    mcpwm_example_gpio_initialize();  //1. mcpwm gpio initialization
    mcpwm_setup();

    xTaskCreate(mcpwm_steering_servo_control, "mcpwm_steering_servo_control", 4096, NULL, 5, NULL);
    // xTaskCreate(mcpwm_esc_servo_control, "mcpwm_esc_servo_control", 4096, NULL, 5, NULL);
}
