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
#include "contiki-lib.h"
#include "contiki-net.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "sys/etimer.h"
#include "net/uip.h"
#include "net/uip-ds6.h"
#include "net/rpl/rpl.h"
#include "net/netstack.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "dev/slip.h"
#include "simple-udp.h"
//#include "json.h"
//#include "../apps/json/jsonparse.h"
#include "energest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define DEBUG DEBUG_NONE
#include "net/uip-debug.h"

#define UDP_PORT 1234
#define SERVICE_ID 190

#define HEIGHT 2.5
#define TOTAL_SENDER 18
#define SIGMA_T 3.2683
#define SIGMA_C 1429.88
#define SIGMA_I 0.1823

#define SEND_INTERVAL		(0.1 * CLOCK_SECOND)
//#define SEND_TIME		(random_rand() % (SEND_INTERVAL))

  static struct simple_udp_connection server_connection;
  static struct simple_udp_connection broadcast_connection;

  static unsigned long rx_start_duration;

  static int occupancy_info[TOTAL_SENDER] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
 // static int illuminance_info[TOTAL_SENDER] = {325, 335, 345, 355, 365, 375, 385, 395, 405, 415, 425, 435, 445, 455, 465, 475, 485, 495};
  static int temperature_desk[TOTAL_SENDER] = {25, 23, 21, 25, 23, 21, 25, 23, 21, 25, 23, 21, 25, 23, 21, 25, 23, 21};
  static int color_temperature_desk[TOTAL_SENDER] = {3000, 3000, 3000, 3000, 3000, 3000, 4000, 4000, 4000, 4000, 4000, 4000, 5000, 5000, 5000, 5000, 5000, 5000};
  static int illuminance_desk[TOTAL_SENDER] = {325, 335, 345, 355, 365, 375, 385, 395, 405, 415, 425, 435, 445, 455, 465, 475, 485, 495};
  static int location_desk[TOTAL_SENDER] = {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0};
  static float lagrange[TOTAL_SENDER] = {0};

  static float max_lagrange;
  static int max_desk_number;
 // static float sum_lagrange = 0.0;
  static int id_person;
  static int flag = 0;
/*
double distance_square(int k, int j) {
  double d2;
  double s;
  int axis_x[TOTAL_SENDER+1];
  int axis_y[TOTAL_SENDER+1];
  
  int i;
  for(i = 3; i <= TOTAL_SENDER; i = i + 3) {
	axis_x[i] = 0;
  }
  for(i = 4; i <= TOTAL_SENDER; i = i + 3) {
	axis_x[i] = 2.4;
  }
  for(i = 5; i <= TOTAL_SENDER; i = i + 3) {
	axis_x[i] = 4.8;
  }
  for(i = 3; i <= 5; i = i++) {
	axis_y[i] = 0;
  }
  for(i = 6; i <= 8; i = i++) {
	axis_y[i] = 1.8;
  }
  for(i = 9; i <= 11; i = i++) {
	axis_y[i] = 5.4;
  }
  for(i = 12; i <= 14; i = i++) {
	axis_y[i] = 7.2;
  }
  for(i = 15; i <= 17; i = i++) {
	axis_y[i] = 10.8;
  }
  for(i = 18; i <= 20; i = i++) {
	axis_y[i] = 12.6;
  }
  
  s = sqrt(pow((axis_x[j] - axis_x[k]), 2) + pow((axis_y[j] - axis_y[k]), 2));
  d2 = s * s + HEIGHT * HEIGHT;
  return d2;
}

double cosine(int k, int j)
{
  return HEIGHT / sqrt(distance_square(k, j));
}
*/
/*---------------------------------------------------------------------------*/
PROCESS(server_process, "Server process");
PROCESS(broadcast_process, "Server process: broadcast");
AUTOSTART_PROCESSES(&server_process, &broadcast_process);

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
  //printf("Data received from ");
  //uip_debug_ipaddr_print(sender_addr);
  //printf(" on port %d from port %d with length %d: '%s'\n",
  //       	receiver_port, sender_port, datalen, data);

  int node_id;
  int temperature_person;
  int color_temperature_person;
  int illuminance_person;
  int location_person;
  float weight_temperature;
  float weight_color_temperature;
  float weight_illuminance;
  float weight_location;
  float delta_temperature;
  float delta_color_temperature;
  float delta_illuminance;
  float delta_location;
 
  //struct jsonparse_state state;
  
  node_id = sender_addr->u8[15];

 // printf("Data received from ");
 // uip_debug_ipaddr_print(sender_addr);
//  printf(" on port %d from port %d with length %d: '%s'\n",
//         	receiver_port, sender_port, datalen, data);

  if(node_id == 2){
	printf("Data received from Room controller...");
/*-------if received from node 2 (coordinator), parse JSON file-------*/
	//jsonparse_setup(&state, data, datalen);

   // while((char)jsonparse_next(&state) != '"'){}
    //char buf1[3];
   // if(jsonparse_copy_value(&state, &buf1, 4)){
	id_person = (data[3] - 48) * 100 + (data[4] - 48) * 10 + (data[5] - 48);
	//id_person = (buf1[0] - 48) * 100 + (buf1[1] - 48) * 10 + (buf1[2] - 48);
	//printf("Profile is parsed - ID: %d; ", id_person);
  //  }
   // while((char)jsonparse_next(&state) != '"'){}
   // char buf2[2];
   // if(jsonparse_copy_value(&state, &buf2, 3)){
	temperature_person = (data[9] - 48) * 10 + data[10] - 48;
	//temperature_person = (buf2[0] - 48) * 10 + (buf2[1] - 48);
	//printf("Temperature: %d oC; ", temperature_person);
  //  }
   // while((char)jsonparse_next(&state) != '"'){}
   // char buf3[4];
   // if(jsonparse_copy_value(&state, &buf3, 5)){
	color_temperature_person = (data[14] - 48) * 1000 + (data[15] - 48) * 100 + (data[16] - 48) * 10 + (data[17] - 48);
	//color_temperature_person = (buf3[0] - 48) * 1000 + (buf3[1] - 48) * 100 + (buf3[2] - 48) * 10 + (buf3[3] - 48);
	//printf("Color Temperature: %d K; ", color_temperature_person);
   // }
   // while((char)jsonparse_next(&state) != '"'){}
   // char buf4[3];
    //if(jsonparse_copy_value(&state, &buf4, 4)){
	illuminance_person = (data[21] - 48) * 100 + (data[22] - 48) * 10 + (data[23] - 48);	
	//illuminance_person = (buf4[0] - 48) * 100 + (buf4[1] - 48) * 10 + (buf4[2] - 48);
	//printf("Illuminance: %d lux; ", illuminance_person);
   // }
   // while((char)jsonparse_next(&state) != '"'){}
   // char buf5[1];
   // if(jsonparse_copy_value(&state, &buf5, 2)){
	location_person = data[27] - 48;	
	//location_person = buf5[0] - 48;
	//printf("Location: %d; ", location_person);
   // }
 	//weight_temperature = 0;
	weight_temperature = (data[34] - 48) * 0.1 + (data[35] - 48) * 0.01;
	//printf("Weight(t): %ld.%02u; ", (long)weight_temperature, (unsigned)((weight_temperature-floor(weight_temperature)) * 100));
   	//weight_color_temperature = 0;
	weight_color_temperature =  (data[42] - 48) * 0.1 + (data[43] - 48) * 0.01;
	//printf("Weight(c): %ld.%02u; ", (long)weight_color_temperature, (unsigned)((weight_color_temperature-floor(weight_color_temperature)) * 100));
    	//weight_illuminance = 0;
	weight_illuminance =  (data[50] - 48) * 0.1 + (data[51] - 48) * 0.01;
	//printf("Weight(i): %ld.%02u; ", (long)weight_illuminance, (unsigned)((weight_illuminance-floor(weight_illuminance)) * 100));
   	//weight_location = 1;
	weight_location =  (data[58] - 48) * 0.1 + (data[59] - 48) * 0.01;
	//printf("Weight(l): %ld.%02u; ", (long)weight_location, (unsigned)((weight_location-floor(weight_location)) * 100));

/*------Lagrange Optimization Calculation-------*/
   /* if(temperature_person > 27 ) {
	temperature_person = 27;
    }else if(temperature_person < 19) {
	temperature_person = 19;
    }
    if(color_temperature_person > 6500) {
	color_temperature_person = 6500;
    }else if(color_temperature_person < 3000) {
	color_temperature_person = 3000;
    }
    if(illuminance_person > 500) {
	illuminance_person = 500;
    }else if(illuminance_person < 320) {
	illuminance_person = 320;
    }
    if(location_person < 0) {
	location_person = 0;
    }else if(location_person > 1) {
	location_person = 1;
    }
    if(weight_temperature < 0 || weight_temperature > 1) {
	printf("The value of temperature weight is wrong!\n");
	return;
    }
    if(weight_color_temperature < 0 || weight_color_temperature > 1) {
	printf("The value of color temperature weight is wrong!\n");
	return;
    }
    if(weight_illuminance < 0 || weight_illuminance > 1) {
	printf("The value of illuminance weight is wrong!\n");
	return;
    }
    if(weight_location < 0 || weight_location > 1) {
	printf("The value of location weight is wrong!\n");
	return;
    }
    if( (int)( weight_temperature + weight_color_temperature + weight_illuminance + weight_location ) != 1 ) {
	printf("Sum of weight is not 1!\n");
	return;
    }*/
    int i;
    int sum = 0;
    for(i = 0; i < TOTAL_SENDER; i++) {
	sum = sum + occupancy_info[i];
    }
    if(sum == 18) {
	printf("All desks are occupied!\n");
	return;
    }
   
    flag = 1;
    max_lagrange = 0;
    max_desk_number = 0;
    
    for(i = 0; i < TOTAL_SENDER; i++) {	
	lagrange[i] = 0;
	if(occupancy_info[i] == 0) {
		delta_temperature = weight_temperature * expf( -( temperature_person - temperature_desk[i] ) * ( temperature_person - temperature_desk[i] ) / ( 2 * SIGMA_T * SIGMA_T ) );
		//printf("d t: %ld.%04u-", (long)delta_temperature, (unsigned)((delta_temperature - floor(delta_temperature)) * 10000));
		delta_color_temperature = weight_color_temperature * expf( -( color_temperature_person - color_temperature_desk[i] ) * ( color_temperature_person - color_temperature_desk[i] ) / ( 2 * SIGMA_C * SIGMA_C ) );
		//printf("d c: %ld.%04u-", (long)delta_color_temperature, (unsigned)((delta_color_temperature - floor(delta_color_temperature)) * 10000));
		delta_illuminance = weight_illuminance * expf( -( logf(illuminance_person) - logf(illuminance_desk[i]) ) * ( logf(illuminance_person) - logf(illuminance_desk[i]) ) / ( 2 * SIGMA_I * SIGMA_I ) );
		//printf("d e: %ld.%04u-", (long)delta_illuminance, (unsigned)((delta_illuminance - floor(delta_illuminance)) * 10000));
		//printf("my location:%d,location:%d,", location_person, location_desk[i]);
		if(location_person == location_desk[i]) {
			delta_location = weight_location;
		}
		else {
			delta_location = 0;
		}
		lagrange[i] = delta_temperature + delta_color_temperature + delta_illuminance + delta_location;
		//printf("User satisfaction: %ld.%04u; \n", (long)lagrange[i], (unsigned)((lagrange[i] - floor(lagrange[i])) * 10000));
		if(lagrange[i] > max_lagrange) {
			max_lagrange = lagrange[i];
			max_desk_number = i;
		}
	}
    }
   //sum_lagrange = sum_lagrange + max_lagrange;
   printf("Calculating conflict resolution algorithm......\n");
   //printf("sum lagrange: %f\n", sum_lagrange);
   // printf("Result:%d\n", max_desk_number);
    occupancy_info[max_desk_number] = 1;
   // printf("The optimal for this profile is Desk Number %d, with User satisfaction value %ld.%04u.\n", max_desk_number, (long)max_lagrange, (unsigned)((max_lagrange-floor(max_lagrange)) * 10000));

  } else {
/*------if from client controller, update occupancy & illuminance information------*/
	occupancy_info[node_id] = data[20] - 48;
  	//illuminance_info[node_id] = (data[46] - 48) * 100 + (data[47] - 48)* 10 + (data[48] - 48);
	printf("Occupancy & illuminance info is updated.\n");
  	/*int i,j;
  	for(i = 3; i <= TOTAL_SENDER; i++) {
  		printf("node %d's occu: %d & illu: %d lux;\n", i, occupancy_info[i], illuminance_info[i]);
  	}
	printf("\n");
	*/
	/*
	for(i = 3; i <= TOTAL_SENDER; i++) {
		for(j = 3; j <= TOTAL_SENDER; j++) {
			illuminance_desk[i] =+ illuminance_info[j] * cosine(i, j) / distance_square(i, j);
		}	
		printf("Illuminance on desk %d: %f\n", i, illuminance_desk[i]);
	}*/
  }
}


/*---------------------------------------------------------------------------*/
static uip_ipaddr_t *
set_global_address(void)
{
  static uip_ipaddr_t ipaddr;
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

  return &ipaddr;
}
/*---------------------------------------------------------------------------*/
static void
create_rpl_dag(uip_ipaddr_t *ipaddr)
{
  struct uip_ds6_addr *root_if;

  root_if = uip_ds6_addr_lookup(ipaddr);
  if(root_if != NULL) {
    rpl_dag_t *dag;
    uip_ipaddr_t prefix;
    
    rpl_set_root(RPL_DEFAULT_INSTANCE, ipaddr);
    dag = rpl_get_any_dag();
    uip_ip6addr(&prefix, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
    rpl_set_prefix(dag, &prefix, 64);
    PRINTF("created a new RPL dag\n");
  } else {
    PRINTF("failed to create a new RPL DAG\n");
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(server_process, ev, data)
{
  uip_ipaddr_t *ipaddr;

  PROCESS_BEGIN();

  servreg_hack_init();

  ipaddr = set_global_address();

  create_rpl_dag(ipaddr);

  servreg_hack_register(SERVICE_ID, ipaddr);

  simple_udp_register(&server_connection, UDP_PORT,
                      NULL, UDP_PORT, receiver);
 
  while(1) {
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END();
}

/*----------initialization and sending of broadcast to other nodes---------------------------------*/
PROCESS_THREAD(broadcast_process, ev, data)
{
  static struct etimer periodic_timer;
 // static struct etimer send_timer;
  uip_ipaddr_t addr;

  PROCESS_BEGIN();

  simple_udp_register(&broadcast_connection, UDP_PORT,
                      NULL, UDP_PORT,
                      receiver);

  etimer_set(&periodic_timer, SEND_INTERVAL);

  //rx_start_duration = energest_type_time(ENERGEST_TYPE_LISTEN);

  while(1) {
    char buf[43];
    static int id_person_old;
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    etimer_reset(&periodic_timer);
  //  etimer_set(&send_timer, SEND_TIME);
  //  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&send_timer));
  //  printf("Sending broadcast\n");
    if(flag == 1) {
 	   if(id_person_old != id_person){
		sprintf(buf, "ID:%d,Result:%d,User Satisfaction:%ld.%04u\n", id_person, max_desk_number+3, (long)max_lagrange, (unsigned)((max_lagrange-floor(max_lagrange)) * 10000)); 
    		uip_create_linklocal_allnodes_mcast(&addr);
    		simple_udp_sendto(&broadcast_connection, buf, strlen(buf) + 1, &addr);
	//	printf("energy rx: %lu\n", energest_type_time(ENERGEST_TYPE_LISTEN) - rx_start_duration);
	//	printf("energy tx: %lu\n", energest_type_time(ENERGEST_TYPE_TRANSMIT) - rx_start_duration);
	//	printf("cpu: %lu\n", energest_type_time(ENERGEST_TYPE_CPU) - rx_start_duration);
	//	printf("lpm: %lu\n", energest_type_time(ENERGEST_TYPE_LPM) - rx_start_duration);
		id_person_old = id_person;
    		}
  	  }
	flag = 0;
   }

  PROCESS_END();
}
/*-------------------------------------------------------*/

