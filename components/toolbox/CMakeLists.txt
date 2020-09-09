set(srcs
   "src/contacts.c"
   "src/contacts2.c"
   "src/config.c"
   "src/ds18b20_add.c"
   "src/network.c"
   "src/status_led.c"
   "src/temperature_dht.c"
   "src/temperature_ds18b20.c"
   "src/temperature_ds18b20_2.c"
   "src/flags.c"
   "src/relays.c"
   "src/tcp_server.c"
   "src/xpl_server.c")
set(COMPONENT_SRCS ${srcs})
set(COMPONENT_ADD_INCLUDEDIRS "src/include")
set(COMPONENT_PRIV_REQUIRES main nvs_flash)
if(CONFIG_MEATOOLBOX_HOMEKIT)
   set(CMAKE_C_FLAGS "-DMEATOOLBOX_HOMEKIT")
endif()
register_component()
