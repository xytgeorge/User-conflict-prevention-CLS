all: broadcast-server broadcast-room-controller broadcast-desk-controller broadcast-example
APPS=servreg-hack
CONTIKI=../../../contiki-2.7

WITH_UIP6=1
UIP_CONF_IPV6=1
CFLAGS+= -DUIP_CONF_IPV6_RPL

TARGET_LIBFILES+= -lm
CFLAGS+= -lm
CFLAGS+= -ffunction-sections


DEFINES=PROJECT_CONF_H=\"project-conf.h\"

APPS += httpd-ws json

include $(CONTIKI)/Makefile.include
