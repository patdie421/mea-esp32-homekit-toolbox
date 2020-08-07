#ifndef __temperature_ds18b20_2_h
#define __temperature_ds18b20_2_h

#define TEMP_OK 0
#define TEMP_NOT_FOUND 1
#define TEMP_NOT_AVAILABLE 2
#define TEMP_NOT_SET 3
#define TEMP_NOT_READ 99

typedef void (*temperature_ds18b20_callback_2_t)(float value, float prev, void *userdata);

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

int  temperature_ds18b20_init_2();
int  temperature_ds18b20_remove_by_addr_2(uint32_t addr0, uint32_t addr1);
int  temperature_ds18b20_set_cb_by_addr_2(uint32_t addr0, uint32_t addr1, temperature_ds18b20_callback_t _cb, void *_u
serdata, int8_t _userid);
int  temperature_ds18b20_get_addr_by_id_2(int8_t id, uint32_t *addr0, uint32_t *addr1);
int  temperature_ds18b20_get_current_by_userid_2(int8_t userid, float *_current);
int  temperature_ds18b20_get_current_by_addr_2(uint32_t addr0, uint32_t addr1, float *_current);
void temperature_ds18b20_start_2(gpio_num_t sensor_gpio);

#endif
