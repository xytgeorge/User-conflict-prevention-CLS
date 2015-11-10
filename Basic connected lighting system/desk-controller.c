/*
 * Copyright (c) 2011, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

#include "contiki.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "sys/etimer.h"
#include "net/uip.h"
#include "net/uip-ds6.h"
#include "net/uip-debug.h"
#include "dev/button-sensor.h"
//#include "dev/light-sensor.h"
#include "dev/leds.h"
#include "energest.h"

#include "sys/node-id.h"

#include "simple-udp.h"
#include "servreg-hack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UDP_PORT 1234
#define SERVICE_ID 190

#define SEND_INTERVAL		(2 * CLOCK_SECOND)
#define SEND_TIME		(random_rand() % (SEND_INTERVAL))

static struct simple_udp_connection unicast_connection;

static unsigned long rx_start_duration;

/*---------------------------------------------------------------------------*/
PROCESS(unicast_sender_process, "desk controller process");
AUTOSTART_PROCESSES(&unicast_sender_process);
/*---------------------------------------------------------------------------*/
static void
receiver(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
 // printf("Data received on port %d from port %d with length %d\n",
  //       receiver_port, sender_port, datalen);
}
/*---------------------------------------------------------------------------*/
static void
set_global_address(void)
{
  uip_ipaddr_t ipaddr;
  int i;
  uint8_t state;

  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

  printf("IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
      printf("\n");
    }
  }
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(unicast_sender_process, ev, data)
{
  static struct etimer periodic_timer;
  static struct etimer send_timer;

  uip_ipaddr_t *addr;

  PROCESS_BEGIN();

  SENSORS_ACTIVATE(button_sensor);
 // SENSORS_ACTIVATE(light_sensor);

  servreg_hack_init();

  set_global_address();

  simple_udp_register(&unicast_connection, UDP_PORT,
                      NULL, UDP_PORT, receiver);
  rx_start_duration = energest_type_time(ENERGEST_TYPE_LISTEN);
  //etimer_set(&periodic_timer, SEND_INTERVAL);
  while(1) {

   // PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);
   // etimer_reset(&periodic_timer);
    etimer_set(&send_timer, SEND_TIME);

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&send_timer));
    addr = servreg_hack_lookup(SERVICE_ID);

    if(addr != NULL) {
      static unsigned int button_number;
      char buf[50];

      if(button_number == 0) {
	button_number = 1;
	leds_toggle(LEDS_ALL);
      } else {
	button_number = 0;
	leds_toggle(LEDS_ALL);
      }

      printf("Sending unicast to ");
      uip_debug_ipaddr_print(addr);
      printf("\n");

      //sprintf(buf, "Occupancy Detector: %d, Luminaire Illuminance: %d", button_number, LUMINAIRE_ILLUMINANCE);
      sprintf(buf, "Occupancy Detector: %d, Luminous Intensity: %d", button_number, /*(int)(random() % 180 + 320 )*/ 1500 );      

      simple_udp_sendto(&unicast_connection, buf, strlen(buf) + 1, addr);
	    //  printf("energy rx: %lu\n", energest_type_time(ENERGEST_TYPE_LISTEN) - rx_start_duration);
		//printf("energy tx: %lu\n", energest_type_time(ENERGEST_TYPE_TRANSMIT) - rx_start_duration);
		//printf("cpu: %lu\n", energest_type_time(ENERGEST_TYPE_CPU) - rx_start_duration);
		//printf("lpm: %lu\n", energest_type_time(ENERGEST_TYPE_LPM) - rx_start_duration);
    } else {
      printf("Service %d not found\n", SERVICE_ID);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
