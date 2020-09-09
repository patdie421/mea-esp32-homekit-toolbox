#include <stdio.h>
#include <esp_log.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <driver/gpio.h>

#include "contacts2.h"


#define ESP_INTR_FLAG_DEFAULT 0
#define DEBOUNCETIMEOUT 50
// #define DEBUG


static char *TAG = "contacts2";

static int _nb_contacts = 0;
static struct contact2_s *_contacts = NULL;
static TaskHandle_t taskHandle = NULL;
static xQueueHandle gpio_evt_queue = NULL;


static void IRAM_ATTR gpio_isr_handler(void* arg)
{
   uint32_t id = (uint32_t)arg;

   xQueueSendFromISR(gpio_evt_queue, (void *)&id, NULL);
}


static void _contacts2_task_sub(void *arg)
{
   uint32_t id = (uint32_t)arg;
   vTaskDelay(DEBOUNCETIMEOUT / portTICK_PERIOD_MS);
   _contacts[id].prev = _contacts[id].state;
   _contacts[id].state = gpio_get_level(_contacts[id].gpio_pin);

   if(_contacts[id].state != _contacts[id].prev) {
      if(_contacts[id].callback) {
         _contacts[id].callback(_contacts[id].state, _contacts[id].prev, id, _contacts[id].contact);
      }
   }
   gpio_intr_enable(_contacts[id].gpio_pin);
   _contacts[id].flag=0;
   vTaskDelete(NULL);
}


static void _contacts2_task(void* arg)
{
   for(;;) {
      uint32_t id;
      if(xQueueReceive(gpio_evt_queue, &id, (TickType_t)(portMAX_DELAY))) {
         if(_contacts[id].flag==0) {
            gpio_intr_disable(_contacts[id].gpio_pin);
            _contacts[id].flag=1;
#ifndef DEBUG
            xTaskCreate(_contacts2_task_sub, "contacts2_sub", 2048, (void *)id, 50, NULL);
#else
            xTaskCreate(_contacts2_task_sub, "contacts2_sub", 4096, (void *)id, 50, NULL);
#endif
         }
      }
   }
}


int8_t contacts2_get(int8_t id)
{
   if(id<_nb_contacts) {
      return _contacts[id].state;
   }
   else {
      return -1;
   }
}


void contacts2_init(struct contact2_s my_contacts[], int nb_contacts) {
   gpio_config_t io_conf;
   uint32_t gpio_input_pin_sel = 0;

   // cleaning
   TaskHandle_t _taskHandle = taskHandle;
   if(_taskHandle) {
     taskHandle=NULL;
     vTaskDelete(_taskHandle);
   }
   gpio_uninstall_isr_service();
   if(gpio_evt_queue) {
      vQueueDelete(gpio_evt_queue);
   }

   // store new config
   _contacts = my_contacts;
   _nb_contacts = nb_contacts;

   // init data and construct gpio mask
   for(int i=0;i<_nb_contacts;i++) {
      _contacts[i].prev=-1;
      _contacts[i].flag=0;
      gpio_input_pin_sel = gpio_input_pin_sel | (1ULL<<_contacts[i].gpio_pin);
   }

   // gpio configuration
   io_conf.pin_bit_mask = gpio_input_pin_sel;
   io_conf.intr_type = GPIO_INTR_ANYEDGE;
   io_conf.mode = GPIO_MODE_INPUT;
   io_conf.pull_up_en = 1; //enable pull-up mode
   io_conf.pull_down_en = 0; //disable pull-down mode
   gpio_config(&io_conf);

   // queue and isr configuration
   gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
   gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
   for(uint32_t i=0;i<_nb_contacts;i++) {
      gpio_isr_handler_add(_contacts[i].gpio_pin, gpio_isr_handler, (void *)i);
   }

   xTaskCreate(_contacts2_task, "contacts2_task", 1024, NULL, 10, taskHandle);
}
