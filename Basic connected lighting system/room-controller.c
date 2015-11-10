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
#include "energest.h"

#include <stdio.h>
#include <string.h>

#define UDP_PORT 1234

#define SEND_INTERVAL		(2 * CLOCK_SECOND)
#define SEND_TIME		(random_rand() % (SEND_INTERVAL))

static struct simple_udp_connection broadcast_connection;

static unsigned long rx_start_duration;

/*---------------------------------------------------------------------------*/
PROCESS(broadcast_process, "room controller process");
AUTOSTART_PROCESSES(&broadcast_process);
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
  if(node_id == 1){
  	//printf("Data received from ");
  	//uip_debug_ipaddr_print(sender_addr);
  	//printf(" on port %d from port %d with length %d: '%s'\n", receiver_port, sender_port, datalen, data);
  	printf("%s", data);
  }
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(broadcast_process, ev, data)
{
  static struct etimer periodic_timer;
  static struct etimer send_timer;

static unsigned int entry_number;
/*----profiles defined-----*/
const char *profile1="id:432,T:19,C:5000,E:465,L:1,AT:0.30,AC:0.25,AE:0.10,AL:0.35";
const char *profile2="id:235,T:20,C:4500,E:335,L:0,AT:0.10,AC:0.15,AE:0.15,AL:0.60";
const char *profile3="id:679,T:21,C:4000,E:355,L:0,AT:0.30,AC:0.30,AE:0.20,AL:0.20";
const char *profile4="id:614,T:22,C:3500,E:375,L:0,AT:0.25,AC:0.25,AE:0.25,AL:0.25";
const char *profile5="id:918,T:23,C:3000,E:475,L:0,AT:0.20,AC:0.30,AE:0.15,AL:0.35";
const char *profile6="id:119,T:24,C:3500,E:405,L:1,AT:0.30,AC:0.20,AE:0.40,AL:0.10";
const char *profile7="id:203,T:25,C:4000,E:425,L:0,AT:0.60,AC:0.20,AE:0.15,AL:0.05";
const char *profile8="id:485,T:26,C:4500,E:345,L:1,AT:0.30,AC:0.25,AE:0.15,AL:0.30";
const char *profile9="id:459,T:27,C:5000,E:365,L:0,AT:0.30,AC:0.30,AE:0.20,AL:0.20";
const char *profile10="id:570,T:27,C:4500,E:385,L:0,AT:0.45,AC:0.25,AE:0.15,AL:0.15";
const char *profile11="id:487,T:26,C:4000,E:485,L:1,AT:0.30,AC:0.25,AE:0.10,AL:0.35";
const char *profile12="id:236,T:25,C:3500,E:495,L:0,AT:0.10,AC:0.15,AE:0.15,AL:0.60";
const char *profile13="id:677,T:24,C:3000,E:445,L:1,AT:0.30,AC:0.30,AE:0.20,AL:0.20";
const char *profile14="id:414,T:23,C:3500,E:395,L:0,AT:0.25,AC:0.25,AE:0.25,AL:0.25";
const char *profile15="id:581,T:22,C:4000,E:415,L:0,AT:0.20,AC:0.30,AE:0.15,AL:0.35";
const char *profile16="id:910,T:21,C:4500,E:325,L:0,AT:0.30,AC:0.20,AE:0.40,AL:0.10";
const char *profile17="id:333,T:20,C:5000,E:435,L:0,AT:0.60,AC:0.20,AE:0.15,AL:0.05";
const char *profile18="id:105,T:19,C:4000,E:455,L:1,AT:0.30,AC:0.25,AE:0.15,AL:0.30";

  const char *profile19 = "id:449,T:25,C:4500,E:400,L:1,AT:0.30,AC:0.20,AE:0.10,AL:0.40";
  const char *profile20 = "id:762,T:25,C:4500,E:400,L:1,AT:0.30,AC:0.20,AE:0.10,AL:0.40";

  const char *profile_default = "no profile";
  uip_ipaddr_t addr;

  PROCESS_BEGIN();

  SENSORS_ACTIVATE(button_sensor);

  simple_udp_register(&broadcast_connection, UDP_PORT,
                      NULL, UDP_PORT,
                      receiver);
  rx_start_duration = energest_type_time(ENERGEST_TYPE_LISTEN);
  //etimer_set(&periodic_timer, SEND_INTERVAL);
  while(1) {
/*-----button pressed: generate a new profile-----*/

   // PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);
   // etimer_reset(&periodic_timer);

    etimer_set(&send_timer, SEND_TIME);

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&send_timer));

      
      char buf[61];
      int i;
      
      entry_number++;
      switch(entry_number){
	case 1: 
      		for(i = 0; i < 61; i++){
        		buf[i] = *(profile1++);
      			printf("%c", buf[i]);
      		}
	break;
	case 2: 
      		for(i = 0; i < 61; i++){
        		buf[i] = *(profile2++);
      			printf("%c", buf[i]);
      		}
	break;
        case 3: 
      		for(i = 0; i < 61; i++){
        		buf[i] = *(profile3++);
      			printf("%c", buf[i]);
      		}
	break;
	case 4: 
      		for(i = 0; i < 61; i++){
        		buf[i] = *(profile4++);
      			printf("%c", buf[i]);
      		}
	break;
	case 5: 
      		for(i = 0; i < 61; i++){
        		buf[i] = *(profile5++);
      			printf("%c", buf[i]);
      		}
	break;
	case 6: 
      		for(i = 0; i < 61; i++){
        		buf[i] = *(profile6++);
      			printf("%c", buf[i]);
      		}
	break;
	case 7: 
      		for(i = 0; i < 61; i++){
        		buf[i] = *(profile7++);
      			printf("%c", buf[i]);
      		}
	break;
	case 8: 
      		for(i = 0; i < 61; i++){
        		buf[i] = *(profile8++);
      			printf("%c", buf[i]);
      		}
	break;
	case 9: 
      		for(i = 0; i < 61; i++){
        		buf[i] = *(profile9++);
      			printf("%c", buf[i]);
      		}
	break;
	case 10: 
      		for(i = 0; i < 61; i++){
        		buf[i] = *(profile10++);
      			printf("%c", buf[i]);
      		}
	break;
	case 11: 
      		for(i = 0; i < 61; i++){
        		buf[i] = *(profile11++);
      			printf("%c", buf[i]);
      		}
	break;
	case 12: 
      		for(i = 0; i < 61; i++){
        		buf[i] = *(profile12++);
      			printf("%c", buf[i]);
      		}
	break;
	case 13: 
      		for(i = 0; i < 61; i++){
        		buf[i] = *(profile13++);
      			printf("%c", buf[i]);
      		}
	break;
	case 14: 
      		for(i = 0; i < 61; i++){
        		buf[i] = *(profile14++);
      			printf("%c", buf[i]);
      		}
	break;
	case 15: 
      		for(i = 0; i < 61; i++){
        		buf[i] = *(profile15++);
      			printf("%c", buf[i]);
      		}
	break;
	case 16: 
      		for(i = 0; i < 61; i++){
        		buf[i] = *(profile16++);
      			printf("%c", buf[i]);
      		}
	break;
	case 17: 
      		for(i = 0; i < 61; i++){
        		buf[i] = *(profile17++);
      			printf("%c", buf[i]);
      		}
	break;
	case 18: 
      		for(i = 0; i < 61; i++){
        		buf[i] = *(profile18++);
      			printf("%c", buf[i]);
      		}
	break;
	case 19: 
      		for(i = 0; i < 61; i++){
        		buf[i] = *(profile19++);
      			printf("%c", buf[i]);
      		}
	break;
	case 20: 
      		for(i = 0; i < 61; i++){
        		buf[i] = *(profile20++);
      			printf("%c", buf[i]);
      		}
	break;
	default:
		printf("No more profile.\n"); 
      		for(i = 0; i < 61; i++){
        		buf[i] = *(profile_default++);
      			printf("%c", buf[i]);
      		}
	break;
      }

    printf(" \n");
    uip_create_linklocal_allnodes_mcast(&addr);
    simple_udp_sendto(&broadcast_connection, buf, strlen(buf) + 1, &addr);
	//printf("energy rx: %lu\n", energest_type_time(ENERGEST_TYPE_LISTEN) - rx_start_duration);
	//	printf("energy tx: %lu\n", energest_type_time(ENERGEST_TYPE_TRANSMIT) - rx_start_duration);
	//	printf("cpu: %lu\n", energest_type_time(ENERGEST_TYPE_CPU) - rx_start_duration);
	//	printf("lpm: %lu\n", energest_type_time(ENERGEST_TYPE_LPM) - rx_start_duration);
  
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
