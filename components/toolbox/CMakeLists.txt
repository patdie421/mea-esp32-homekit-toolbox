set(srcs
   "contacts.c"
   "config.c"
   "ds18b20_add.c"
   "network.c"
   "status_led.c"
   "temperature_dht.c"
   "temperature_ds18b20.c"
   "temperature_ds18b20_2.c"
   "flags.c"
   "relays.c"
   "tcp_server.c"
   "xpl_server.c")
set(COMPONENT_SRCS ${srcs})
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_PRIV_REQUIRES main nvs_flash)
register_component()
