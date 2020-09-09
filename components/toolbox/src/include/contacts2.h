#ifndef __contacts2_h
#define __contacts2_h

typedef void (*contact2_callback_t)(int8_t value, int8_t prev, int8_t id, void *userdata);

struct contact2_s {
   int8_t   state;
   int8_t   prev;
   int8_t   gpio_pin;
   char    *name;
   void    *contact;
   uint8_t  flag;
   contact2_callback_t callback;
};

void contacts2_init(struct contact2_s contacts[], int nb_contacts);
int8_t contacts2_get(int8_t id);

#endif
