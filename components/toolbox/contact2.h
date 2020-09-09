#ifndef __contacts_int_h
#define __contacts_int_h

typedef void (*contact_int_callback_t)(int8_t value, int8_t prev, int8_t id, void *userdata);

struct contact_int_s {
   int8_t   state;
   int8_t   prev;
   uint32_t debounceTimout;
   int8_t   gpio_pin;
   char    *name;
   void    *contact;
   uint8_t  flag;
   uint32_t counter;
   contact_int_callback_t callback;
};

void contacts_int_init(struct contact_int_s contacts[], int nb_contacts);
int8_t contacts_int_get(int8_t id);

#endif
