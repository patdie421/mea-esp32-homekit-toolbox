#include <stdio.h>
#include <esp_log.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>

#include "contacts.h"

#define DEBOUNCETIMEOUT 25;

static char *TAG = "contacts_int";

static int _nb_contacts = 0;
static struct contact_int_s *_contacts = NULL;
static TaskHandle_t taskHandle = NULL;
static xQueueHandle gpio_evt_queue = NULL;


int8_t contacts_int_get(int8_t id)
{
   if(id<_nb_contacts) {
      return _contacts[id].state;
   }
   else {
      return -1;
   }
}


static uint32_t millis()
{
   return xTaskGetTickCount()*portTICK_PERIOD_MS;
}


static void IRAM_ATTR gpio_isr_handler(void* arg)
{
   uint32_t state=gpio_get_level(_contact_ptr->gpio_pin);
   uint32_t id = (uint32_t)arg;
   static struct contact_int_s *_contact_ptr = &_contacts[id];

   uint8_t now = millis();

   if(now-DEBOUNCETIMOUT>_contact_ptr->debounceTimout) {
      _contact_ptr->debounceTimout=now;
      _contact_ptr->prev=_contact_ptr->state;
      _contact_ptr->state=state;
      xQueueSendFromISR(gpio_evt_queue, (void *)id, NULL);
   }
}


static void IRAM_ATTR gpio_isr_handler_2(void* arg)
{
   uint32_t id = (uint32_t)arg;
   static struct contact_int_s *_contact_ptr = &_contacts[id];

   uint8_t now = millis();

   if(now-DEBOUNCETIMOUT>_contact_ptr->debounceTimout) {
      _contact_ptr->debounceTimout=now;
      _contact_ptr->prev=_contact_ptr->state;
      if(_contact_ptr->state){
         _contact_ptr->state=0;
         gpio_set_intr_type(_contact_ptr->gpio_pin, GPIO_INTR_HIGH_LEVEL);
      }
      else {
         _contact_ptr->state=1;
         gpio_set_intr_type(_contact_ptr->gpio_pin, GPIO_INTR_LOW_LEVEL);
      }
      xQueueSendFromISR(gpio_evt_queue, (void *)id, NULL);
   }
}


static void IRAM_ATTR gpio_isr_handler_3(void* arg)
{
   uint32_t id = (uint32_t)arg;

#ifdef DEBUG
   _contacts[id].counter++;
   _contacts[i].debounceTimout=millis();
#endif

   xQueueSendFromISR(gpio_evt_queue, (void *)id, NULL);
}


static void _contacts_int_sync()
{
   now = millis();
   for(uint32_t i=0; i<nb_contacts;i++) {
      gpio_intr_disable(_contacts[i].gpio_pin);
      int l = gpio_get_level(_contacts[i].gpio_pin);

      if(now-DEBOUNCETIMOUT>_contacts[i].debounceTimout) {
         _contacts[i].debounceTimout=now;
         _contacts[i].prev=_contacts[i].state;
         _contacts[i].state=l;
#ifdef INT2
         if(_contacts[i].state) {
            gpio_set_intr_type(_contact_ptr->gpio_pin, GPIO_INTR_LOW_LEVEL);
         }
         else {
            gpio_set_intr_type(_contact_ptr->gpio_pin, GPIO_INTR_HIGH_LEVEL);
         }
#endif
         if(_contacts[i].callback) {
            _contacts[i].callback(_contacts[i].state, _contacts[i].prev, id, _contacts[i].contact);
         }
      }
      gpio_intr_enable(_contacts[i].gpio_pin);
   }
}


static void _contacts_int_task_2(void* arg)
{

   for(;;) {
      uint32_t id = 0xFF;
      if(xQueueReceive(gpio_evt_queue, &id, (TickType_t)(5000/portTICK_PERIOD_MS))) {
         if(_contacts[id].callback) {
            _contacts[id].callback(_contacts[id].state, _contacts[id].prev, id, _contacts[id].contact);
         }
      }
      else {
         _contacts_int_sync();
      }
   }
}


static void _contacts_int_task_3_sub(void *arg)
{
   uint32_t id = (uint32_t)arg;
#ifdef DEBUG
   ESP_LOGI(TAG, "pin: %d start wait (%d/%d)", millis(),_contacts[i].debounceTimout);
#endif
   vTaskDelay(DEBOUNCETIMEOUT / portTICK_PERIOD_MS);
#ifdef DEBUG
   ESP_LOGI(TAG, "pin: %d end wait (%d/%d)", millis(),_contacts[i].debounceTimout);
#endif

   _contacts[id].prev = _contacts[id].state;
   _contacts[id].state = gpio_get_level(_contacts[i].gpio_pin);
#ifdef DEBUG
   ESP_LOGI(TAG, "pin: %d state:%d (prev: %d counter: %d)", _contacts[id].state, _contacts[id].prev, _contacts[id].counter);
#endif

   if(_contacts[id].callback) {
      _contacts[id].callback(_contacts[id].state, _contacts[id].prev, id, _contacts[id].contact);
   }

//   gpio_intr_enable(_contacts[i].gpio_pin);
#ifdef DEBUG
   _contacts[id].flag=0;
   _contacts[id].counter=0;
#endif
}


static void _contacts_int_task_3(void* arg)
{
   for(;;) {
      uint32_t id = 0xFF;
      if(xQueueReceive(gpio_evt_queue, &id, (TickType_t)(portMAX_DELAY))) {
         if(_contacts[id].flag==0) {
//            gpio_intr_disable(_contacts[i].gpio_pin);
            _contacts[id].flag=1;
#ifndef DEBUG
            xTaskCreate(_contacts_int_task_3_sub, "contacts_int_sub", 64, (void *)id, 1, NULL);
#else
            xTaskCreate(_contacts_int_task_3_sub, "contacts_int_sub", 512, (void *)id, 1, NULL);
#endif
         }
      }
   }
}


static void _contacts_int_task(void* arg)
{
   _contacts_int_sync();
   for(;;) {
      uint32_t id = 0xFF;
      if(xQueueReceive(gpio_evt_queue, &id, (TickType_t)(1000/portTICK_PERIOD_MS))) {
         if(_contacts[id].callback) {
            _contacts[id].callback(_contacts[id].state, _contacts[id].prev, id, _contacts[id].contact);
         }
      }
      else {
         now = millis();
         for(uint32_t i=0; i<nb_contacts;i++) {
            gpio_intr_disable(_contacts[i].gpio_pin);
            int s = gpio_get_level(_contacts[i].gpio_pin);
            if(s != _contacts[i].state) {
               if(now-DEBOUNCETIMOUT>_contacts[i].debounceTimout) {
                  _contacts[i].debounceTimout=now;
                  _contacts[i].prev=_contacts[i].state;
                  _contacts[i].state=s;
                  if(_contacts[i].callback) {
                     _contacts[i].callback(_contacts[i].state, _contacts[i].prev, id, _contacts[i].contact);
               }
            }
            gpio_intr_enable(_contacts[i].gpio_pin);
         }
      }
   }
}


void contacts_int_init(struct contact_int_s my_contacts[], int nb_contacts) {
   gpio_config_t io_conf;
   uint32_t gpio_input_pin_sel = 0;

   _contacts = my_contacts;
   _nb_contacts = nb_contacts;

   TaskHandle_t _taskHandle = taskHandle;
   if(_taskHandle) {
     taskHandle=NULL;
     vTaskDelete(_taskHandle);
   }

   // init data and gpio mask
   for(int i=0;i<_nb_contacts;i++) {
      _contacts[i].debounceTimeout=0;
      _contacts[i].prev=-1;
#ifdef INT3
      _contacts[i].flag=0;
      _contacts[i].counter=0;
#endif
      gpio_input_pin_sel = gpio_input_pin_sel | (1ULL<<_contacts[i].gpio_pin);
   }

   // gpio configuration
   io_conf.pin_bit_mask = gpio_input_pin_sel;
#ifdef INT3
   io_conf.intr_type = GPIO_INTR_ANYEDGE;
#elif INT2
   io_conf.intr_type = GPIO_INTR_GPIO_INTR_HIGH_LEVEL;
#else
   io_conf.intr_type = GPIO_INTR_ANYEDGE;
#endif
   io_conf.mode = GPIO_MODE_INPUT;
   //enable pull-up mode
   io_conf.pull_up_en = 1;
   //disable pull-down mode
   io_conf.pull_down_en = 0;
   gpio_config(&io_conf);

   // isr configuration
   gpio_uninstall_isr_service();
   if(gpio_evt_queue) {
      vQueueDelete(gpio_evt_queue);
   }
   gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
   gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
   for(uint32_t i=0;i<_nb_contacts:i++) {
#ifdef INT3
      gpio_isr_handler_add(_contacts[i].gpio_pin, gpio_isr_handler_3, (void *)i);
#elif INT2
      gpio_isr_handler_add(_contacts[i].gpio_pin, gpio_isr_handler_2, (void *)i);
#else
      gpio_isr_handler_add(_contacts[i].gpio_pin, gpio_isr_handler, (void *)i);
#endif
   }

#ifdef INT3
   xTaskCreate(_contacts_int_task_3, "contacts_int_task", 1024, NULL, 1, taskHandle);
#elif INT2
   xTaskCreate(_contacts_int_task_2, "contacts_int_task", 2560, NULL, 10, taskHandle);
#else
   xTaskCreate(_contacts_int_task, "contacts_int_task", 2560, NULL, 10, taskHandle);
#endif
}
