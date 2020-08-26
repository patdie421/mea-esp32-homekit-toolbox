#include <stdio.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>

#include "status_led.h"


static uint8_t  status_led_gpio = 2;
static uint16_t status_led_interval_on = 50; // ms
static uint16_t status_led_interval_off = 50; // ms
static uint16_t status_led_wait = 0; // ms : wait after on/off cycle
static uint16_t status_led_count = 1; // nb off on/off cycle before wait
static uint8_t  status_led_off = 0;

static uint8_t  status_led_is_init = 0;
TickType_t xLastWakeTime;


void status_led_write(bool on) {
   gpio_set_level(status_led_gpio, on ? 1 : 0);
}


void status_led_task(void *_args) {
   
   while(1) {
      if(status_led_off) {
         vTaskDelay(1);
         continue;
      }

      xLastWakeTime = xTaskGetTickCount();  
      for(int i=0;i<status_led_count;i++) {
         status_led_write(true);
//         vTaskDelay(status_led_interval_on / portTICK_PERIOD_MS);
         vTaskDelayUntil( &xLastWakeTime, status_led_interval_on / portTICK_PERIOD_MS );
         status_led_write(false);
//         vTaskDelay(status_led_interval_off / portTICK_PERIOD_MS);
         vTaskDelayUntil( &xLastWakeTime, status_led_interval_off / portTICK_PERIOD_MS );
      }
   }
   if(status_led_wait>0) {
//      vTaskDelay(status_led_wait / portTICK_PERIOD_MS);
      vTaskDelayUntil( &xLastWakeTime, status_led_wait / portTICK_PERIOD_MS );
   }
}


void status_led_init(uint16_t interval, uint8_t pin)
{
   status_led_interval_on=interval;
   status_led_interval_off=interval;
   if(!status_led_is_init) {
      status_led_is_init=1;
      status_led_gpio=pin;
      gpio_set_direction(status_led_gpio, GPIO_MODE_OUTPUT);
      xTaskCreate(status_led_task, "status led", 1024, NULL, 2, NULL);
   }
}


void status_led_off()
{
   status_led_off=1;
}


void status_led_on()
{
   status_led_off=0;
}


void status_led_set_interval(uint16_t interval)
{
   status_led_interval_on=interval;
   status_led_interval_off=interval;
   status_led_count=1;
   status_led_wait=0;
   status_led_off=0;
}


void status_led_set(uint16_t on, uint16_t off, uint16_t count, uint16_t wait)
{
   status_led_interval_on=on;
   status_led_interval_off=off;
   status_led_count=count;
   status_led_wait=wait;
   status_led_off=0;
}


uint16_t status_led_get_interval()
{
   return status_led_interval_on;
}
