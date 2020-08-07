#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <esp_log.h>

#include <ds18x20.h>
#include "ds18b20_add.h"

#include "temperature_ds18b20_2.h"


#define TASK_NAME "ds18b20"
#define TASK_PRIORITY 2
#define TASK_STACK_SIZE 2560+sizeof(_temperature_ds18b20_data_2)

#define RESCAN_INTERVAL 720
#define MAX_SENSORS 8

#define TEMP_OK 0
#define TEMP_NOT_FOUND 1
#define TEMP_NOT_AVAILABLE 2
#define TEMP_NOT_SET 3
#define TEMP_NOT_READ 99


static char *TAG = "ds18x20_2";
static TaskHandle_t task_handle = NULL;
static const uint32_t LOOP_DELAY_MS = 10000;

static gpio_num_t SENSOR_GPIO = 5;

struct temperature_ds18b20_data_2_s {
   uint32_t addr0;
   uint32_t addr1;
   float current;
   float last;
   int8_t userid;
   void *userdata;
   temperature_ds18b20_callback_2_t cb;
   int8_t state;
};
static struct temperature_ds18b20_data_2_s _temperature_ds18b20_data_2[MAX_SENSORS];


static int _add_new(uint32_t addr0, uint32_t addr1)
{
   for(int i=0;i<MAX_SENSORS;i++) {
      if(_temperature_ds18b20_data_2[i]==NOT_SET) {
         _temperature_ds18b20_data_2[i]={ .cb=NULL, .userdata=NULL, userid=-1, .addr0=addr0, .addr1=addr1, .current=-999.0, .last=-999.0, .state=TEMP_NOT_READ };
         return i;
      }
   }
   return -1;
}


static void _force_status_not_found()
{
   for(int i=0;i<MAX_SENSORS;i++) {
      if(_temperature_ds18b20_data_2[i].status!=TEMP_NOT_SET) {
         temperature_ds18b20_data_2[i].status=TEMP_NOT_FOUND;
      }
   }
}


static int _find_addr(uint32_t addr0, uint32_t addr1)
{
   for(int i=0;i<MAX_SENSORS;i++) {
      if(_temperature_ds18b20_data_2[i].addr0==addr0 && _temperature_ds18b20_data_2[i].addr1=addr1) {
         return i;
      }
   }
   return -1;
}


static int _find_userid(int8_t userid)
{
   for(int i=0;i<MAX_SENSORS;i++) {
      if(_temperature_ds18b20_data_2[i].userid==userid) {
         return i;
      }
   }
   return -1;
}


int temperature_ds18b20_init_2()
{
   int sensor_count = ds18x20_scan_devices(SENSOR_GPIO, addrs, MAX_SENSORS);

   for(int i=0;i<MAX_SENSORS;i++) {
      if(i<sensor_count) {
         _temperature_ds18b20_data_2[i]={ .cb=NULL, .userdata=NULL, .userid=-1, .addr0=addrs[i]>>32, .addr1=addrs[i], .current=0.0, .last=0.0, .state=TEMP_NOT_READ };
      }
      else {
         _temperature_ds18b20_data_2[i]={ .cb=NULL, .userdata=NULL, .userid=-1, .addr0=NULL, .addr1=NULL, .current=0.0, .last=0.0, .state=NOT_SET };
      }
   }
   return sensor_count;
}


int temperature_ds18b20_remove_by_addr_2(uint32_t addr0, uint32_t addr1)
{
   int id=_find_addr(addr0, addr1);
   if(id!=-1) {
      _temperature_ds18b20_data_2[id]={ .cb=NULL, .userdata=NULL, .userid=-1, .addr0=NULL, .addr1=NULL, .current=0.0, .last=0.0, .state=NOT_SET };
      return 1;
   }
   return 0;
}


int temperature_ds18b20_set_cb_by_addr_2(uint32_t addr0, uint32_t addr1, temperature_ds18b20_callback_t _cb, void *_userdata, int8_t _userid)
{
   int id=_find_addr(addr0, addr1);
   if(id!=-1) {
      _temperature_ds18b20_data_2[id].cb=_cb;
      _temperature_ds18b20_data_2[id].userdata=_userdata;
      _temperature_ds18b20_data_2[id].userid=_userid;
      return 1;
   }
   return 0;
}


int temperature_ds18b20_get_addr_by_id_2(int8_t id, uint32_t *addr0, uint32_t *addr1)
{
  if(id<MAX_SENSORS && _temperature_ds18b20_data_2[id].status!=TEMP_NOT_SET) {
     *addr0=_temperature_ds18b20_data_2[id].addr0;
     *addr1=_temperature_ds18b20_data_2[id].addr1;
     return 1;
  }
  return 0;
}


int temperature_ds18b20_get_current_by_userid_2(int8_t userid, float *_current)
{
    int id=_find_userid(userid);
    if(id!=-1) {
       *_current=_temperature_ds18b20_data_2[id].current;
       return _temperature_ds18b20_data_2[id].state;
    }
    return -1;
}


int temperature_ds18b20_get_current_by_addr_2(uint32_t addr0, uint32_t addr1, float *_current)
{
    int id=_find_addr(addr0, addr1);
    if(id!=-1) {
       *_current=_temperature_ds18b20_data_2[id].current;
       return _temperature_ds18b20_data_2[id].state;
    }
    return -1;
}


float temperature_ds18b20_get()
{
   return _temperature_ds18b20_data_2[0].current;
}


void temperature_ds18b20_task(void *_args)
{
   int sensor_count;
   ds18x20_addr_t addrs[MAX_SENSORS];
   float temps[MAX_SENSORS];

   while (1) {
      sensor_count = ds18x20_scan_devices(SENSOR_GPIO, addrs, MAX_SENSORS);

      if(sensor_count <= 0) {
         vTaskDelay(LOOP_DELAY_MS*5 / portTICK_PERIOD_MS);
         continue;
      }

      ds18b20_set_resolution(SENSOR_GPIO, ds18x20_ANY, DS18B20_RESOLUTION_12_BIT);

      if (sensor_count < 1) {
         ESP_LOGI(TAG, "No sensors detected!");
      }
      else {
         ESP_LOGI(TAG, "%d sensors detected:", sensor_count);
         if (sensor_count > MAX_SENSORS) {
            sensor_count = MAX_SENSORS;
         }

         for (int i = 0; i < RESCAN_INTERVAL; i++) {
            ds18x20_measure_and_read_multi(SENSOR_GPIO, addrs, sensor_count, temps);

            _force_status_not_found();
            for(int j = 0; j<sensor_count; i++) {
               uint32_t addr0 = addrs[i] >> 32;
               uint32_t addr1 = addrs[i];

               ESP_LOGI(TAG, "Sensor %08x%08x reports %.1f deg C", addr0, addr1, temps[i]);

               int id=_find_addr(addr0, addr1);
               if(id!=-1) {
                  _temperature_ds18b20_data_2[id].last=_temperature_ds18b20_data_2[id].current;
                  _temperature_ds18b20_data_2[id].current=temps[i];
                  _temperature_ds18b20_data_2[id].state=TEMP_OK;
                  if(_temperature_ds18b20_data_2[id].cb) {
                     _temperature_ds18b20_data_2[id].cb(_temperature_ds18b20_data_2[id].current, temperature_ds18b20_data_2[id].last, temperature_ds18b20_data_2[id].userdata)
                  }
               }
               else {
                  id=_add_new(addr0, addr1);
                  if(id!=-1) {
                     _temperature_ds18b20_data_2[id].last=-999.0;
                     _temperature_ds18b20_data_2[id].current=temps[i];
                     _temperature_ds18b20_data_2[id].state=TEMP_OK;
                     _temperature_ds18b20_data_2[id].cb=NULL;
                  }
               }
            }
            vTaskDelay(LOOP_DELAY_MS / portTICK_PERIOD_MS);
         }
      }
   }
}


void temperature_ds18b20_start(gpio_num_t sensor_gpio)
{
   SENSOR_GPIO=sensor_gpio;

   xTaskCreate(temperature_ds18b20_task, "ds18b20", TASK_STACK_SIZE, NULL, TASK_PRIORITY, &task_handle);
}
