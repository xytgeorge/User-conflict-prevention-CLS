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
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "simple-udp.h"

#include <stdio.h>
#include <string.h>

#define UDP_PORT 1234

#define SEND_INTERVAL		(2 * CLOCK_SECOND)
#define SEND_TIME		(random_rand() % (SEND_INTERVAL))

static struct simple_udp_connection broadcast_connection;

static uip_ipaddr_t prefix;
static uint8_t prefix_set;

/*---------------------------------------------------------------------------*/
PROCESS(broadcast_process, "room controller process");
AUTOSTART_PROCESSES(&broadcast_process);
/*---------------------------------------------------------------------------*/
void
set_prefix_64(uip_ipaddr_t *prefix_64)
{
  uip_ipaddr_t ipaddr;
  memcpy(&prefix, prefix_64, 16);
  memcpy(&ipaddr, prefix_64, 16);
  prefix_set = 1;
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
}
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
  static int re_id;
  static int node_id;
  re_id = receiver_addr->u8[15];
  node_id = sender_addr->u8[15];
  if(node_id == 2){
  	//printf("Data received from ");
  	//uip_debug_ipaddr_print(sender_addr);
  	//printf(" on port %d from port %d with length %d: '%s'\n", receiver_port, sender_port, datalen, data);
  	printf("%s\n", data);
  }
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(broadcast_process, ev, data)
{
  static struct etimer periodic_timer;
  static struct etimer send_timer;

static unsigned int entry_number;
/*----profiles defined-----*/
 const char *profile1 = "{\"profile\": {\"id\": \"432\", \"T\": \"19\", \"C\": \"4500\", \"I\": \"400\", \"L\": \"0\", \"weight\": [ {\"AT\": \"0.30\"}, {\"AC\": \"0.25\"}, {\"AI\": \"0.10\"}, {\"AL\": \"0.35\"} ] } }";
  const char *profile2 = "{\"profile\": {\"id\": \"235\", \"T\": \"22\", \"C\": \"4300\", \"I\": \"460\", \"L\": \"1\", \"weight\": [ {\"AT\": \"0.10\"}, {\"AC\": \"0.15\"}, {\"AI\": \"0.15\"}, {\"AL\": \"0.60\"} ] } }";
  const char *profile3 = "{\"profile\": {\"id\": \"679\", \"T\": \"20\", \"C\": \"4460\", \"I\": \"340\", \"L\": \"1\", \"weight\": [ {\"AT\": \"0.30\"}, {\"AC\": \"0.30\"}, {\"AI\": \"0.20\"}, {\"AL\": \"0.20\"} ] } }";
  const char *profile4 = "{\"profile\": {\"id\": \"614\", \"T\": \"19\", \"C\": \"4800\", \"I\": \"370\", \"L\": \"1\", \"weight\": [ {\"AT\": \"0.25\"}, {\"AC\": \"0.25\"}, {\"AI\": \"0.25\"}, {\"AL\": \"0.25\"} ] } }";
  const char *profile5 = "{\"profile\": {\"id\": \"918\", \"T\": \"25\", \"C\": \"5500\", \"I\": \"480\", \"L\": \"0\", \"weight\": [ {\"AT\": \"0.20\"}, {\"AC\": \"0.30\"}, {\"AI\": \"0.15\"}, {\"AL\": \"0.35\"} ] } }";
  const char *profile6 = "{\"profile\": {\"id\": \"019\", \"T\": \"27\", \"C\": \"5800\", \"I\": \"420\", \"L\": \"1\", \"weight\": [ {\"AT\": \"0.30\"}, {\"AC\": \"0.20\"}, {\"AI\": \"0.40\"}, {\"AL\": \"0.10\"} ] } }";
  const char *profile7 = "{\"profile\": {\"id\": \"203\", \"T\": \"26\", \"C\": \"3500\", \"I\": \"470\", \"L\": \"0\", \"weight\": [ {\"AT\": \"0.60\"}, {\"AC\": \"0.20\"}, {\"AI\": \"0.15\"}, {\"AL\": \"0.05\"} ] } }";
  const char *profile8 = "{\"profile\": {\"id\": \"485\", \"T\": \"25\", \"C\": \"6000\", \"I\": \"430\", \"L\": \"1\", \"weight\": [ {\"AT\": \"0.30\"}, {\"AC\": \"0.25\"}, {\"AI\": \"0.15\"}, {\"AL\": \"0.30\"} ] } }";
  const char *profile9 = "{\"profile\": {\"id\": \"459\", \"T\": \"23\", \"C\": \"5600\", \"I\": \"330\", \"L\": \"0\", \"weight\": [ {\"AT\": \"0.35\"}, {\"AC\": \"0.35\"}, {\"AI\": \"0.10\"}, {\"AL\": \"0.20\"} ] } }";
  const char *profile10 = "{\"profile\": {\"id\": \"570\", \"T\": \"22\", \"C\": \"3900\", \"I\": \"370\", \"L\": \"1\", \"weight\": [ {\"AT\": \"0.45\"}, {\"AC\": \"0.45\"}, {\"AI\": \"0.05\"}, {\"AL\": \"0.05\"} ] } }";
  const char *profile_default = "{ profile empty }";

  uip_ipaddr_t addr;

  PROCESS_BEGIN();

  SENSORS_ACTIVATE(button_sensor);

  simple_udp_register(&broadcast_connection, UDP_PORT,
                      NULL, UDP_PORT,
                      receiver);

  //etimer_set(&periodic_timer, SEND_INTERVAL);
  while(1) {
/*-----button pressed: generate a new profile-----*/

   // PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);
   // etimer_reset(&periodic_timer);

    etimer_set(&send_timer, SEND_TIME);

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&send_timer));

      
      char buf[152];
      int i;
      
      entry_number++;
      switch(entry_number){
	case 1: 
      		for(i = 0; i < 152; i++){
        		buf[i] = *(profile1++);
      			printf("%c", *profile1);
      		}
	break;
	case 2: 
      		for(i = 0; i < 152; i++){
        		buf[i] = *(profile2++);
      			printf("%c", *profile2);
      		}
	break;
        case 3: 
      		for(i = 0; i < 152; i++){
        		buf[i] = *(profile3++);
      			printf("%c", *profile3);
      		}
	break;
	case 4: 
      		for(i = 0; i < 152; i++){
        		buf[i] = *(profile4++);
      			printf("%c", *profile4);
      		}
	break;
	case 5: 
      		for(i = 0; i < 152; i++){
        		buf[i] = *(profile5++);
      			printf("%c", *profile5);
      		}
	break;
	case 6: 
      		for(i = 0; i < 152; i++){
        		buf[i] = *(profile6++);
      			printf("%c", *profile6);
      		}
	break;
	case 7: 
      		for(i = 0; i < 152; i++){
        		buf[i] = *(profile7++);
      			printf("%c", *profile7);
      		}
	break;
	case 8: 
      		for(i = 0; i < 152; i++){
        		buf[i] = *(profile8++);
      			printf("%c", *profile8);
      		}
	break;
	case 9: 
      		for(i = 0; i < 152; i++){
        		buf[i] = *(profile9++);
      			printf("%c", *profile9);
      		}
	break;
	case 10: 
      		for(i = 0; i < 152; i++){
        		buf[i] = *(profile10++);
      			printf("%c", *profile10);
      		}
	break;
	default:
		printf("No more profile.\n"); 
      		for(i = 0; i < 152; i++){
        		buf[i] = *(profile_default++);
      			printf("%c", *profile_default);
      		}
	break;
      }

    printf("Sending broadcast\n");
    uip_create_linklocal_allnodes_mcast(&addr);
    simple_udp_sendto(&broadcast_connection, buf, strlen(buf) + 1, &addr);
  
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
