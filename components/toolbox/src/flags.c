#include <stdio.h>
#include <esp_log.h>


#include "flags.h"


static char *TAG = "flags";

static int _nb_flags = 0;
static struct flag_s *_flags = NULL;


int8_t flags_get(int8_t id)
{
   if(_flags && id<_nb_flags) {
      return _flags[id].last_state;
   }
   else {
      return -1;
   }
}


int8_t flags_set(int8_t id, int8_t v)
{
   if(_flags && id<_nb_flags) {
      int8_t _v=(v > 0) ? 1 : 0;
      if(_v!=_flags[id].last_state) {
         if(_flags[id].callback) {
            _flags[id].callback(_v, _flags[id].last_state, id, _flags[id].flag);
         }
         _flags[id].last_state = _v;
      }
      return 0;
   }
   else {
      return -1;
   }
}


void flags_init(struct flag_s my_flags[], int nb_flags) {
   _flags = my_flags;
   _nb_flags = nb_flags;
}
