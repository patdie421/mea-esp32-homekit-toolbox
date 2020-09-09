#ifndef __status_led
#define __status_led

void status_led_init(uint16_t interval, uint8_t pin);
void status_led_set_interval(uint16_t interval);
uint16_t status_led_get_interval();

void status_led_off();
void status_led_on();
void status_led_set(uint16_t on, uint16_t off, uint16_t count, uint16_t wait);

#endif

