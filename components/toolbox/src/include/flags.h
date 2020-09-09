#ifndef __flags_h
#define __flags_h

typedef void (*flags_callback_t)(int8_t value, int8_t prev, int8_t id, void *userdata);

struct flag_s {
   int8_t last_state;
   char *name;
   void *flag;
   flags_callback_t callback;
};

void flags_init(struct flag_s flags[], int nb_flags);
int8_t flags_get(int8_t id);
int8_t flags_set(int8_t id, int8_t v);

#endif
