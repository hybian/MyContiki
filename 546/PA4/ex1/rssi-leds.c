/*
 * Modified nullnet broadcast example
 * named as rssi-leds
 *
 */
#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "net/packetbuf.h"
#include <string.h>
#include <stdio.h> /* For printf() */
#include <math.h>
#include "dev/leds.h"


/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

/* send interval configuration */
#define SEND_INTERVAL (CLOCK_SECOND / 4)

/*---------------------------------------------------------------------------*/
PROCESS(rssi_leds_process, "rssi-leds");
AUTOSTART_PROCESSES(&rssi_leds_process);

int msg_counter=0;
int arr_counter=0;
int16_t prev_rssi[4];
/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  if(len == sizeof(unsigned)) 
  {
    unsigned count;
    memcpy(&count, data, sizeof(count));
    int16_t curr_RSSI = (int16_t)packetbuf_attr(PACKETBUF_ATTR_RSSI);    

    int i = 0;
    int16_t sum = 0;
    int16_t avg = 0;

    if(msg_counter < 4) // don't have four values of RSSI yet
    {
        prev_rssi[arr_counter] = curr_RSSI;
        arr_counter++;
    }
    else
    {
        /* calculate the running average */
        for(i=0; i<4; i++){
          sum = sum + prev_rssi[i];
        }
        avg = sum / 4;

        if( fabs(curr_RSSI - avg) <= 1 ) // RSSI within +/- 1 dBm of the avg
        {
          leds_single_off(LEDS_LED1); // RED OFF
          leds_single_off(LEDS_LED2); // GREEN OFF
          printf("msg_counter= %d, RED=off, GREEN=off ", msg_counter);
        }
        else if( curr_RSSI > avg ) // RSSI greater than avg
        {
          leds_single_on(LEDS_LED1); // RED ON
          leds_single_off(LEDS_LED2); // GREEN OFF
          printf("msg_counter= %d, RED=on, GREEN=off ", msg_counter);
        }
        else // RSSI less than avg
        {
          leds_single_off(LEDS_LED1); // RED OFF
          leds_single_on(LEDS_LED2); // GREEN ON
          printf("msg_counter= %d, RED=off, GREEN=on ", msg_counter);
        }

        /* update RSSI array */
        if(arr_counter > 3)
        {
          arr_counter = 0;
          prev_rssi[0] = curr_RSSI;
          arr_counter++;
        }
        else
        {
          prev_rssi[arr_counter] = curr_RSSI;
          arr_counter++;
        }
    }

    msg_counter++;
    printf("curr_RSSI=%d, sum=%d, avg=%d, arr_counter=%d\n", curr_RSSI, sum, avg, arr_counter);
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(rssi_leds_process, ev, data)
{
  static struct etimer periodic_timer;
  static unsigned count = 0;

  PROCESS_BEGIN();

  /* Initialize NullNet */
  nullnet_buf = (uint8_t *)&count;
  nullnet_len = sizeof(count);
  nullnet_set_input_callback(input_callback);

  etimer_set(&periodic_timer, SEND_INTERVAL);
  while(1) 
  {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    
    memcpy(nullnet_buf, &count, sizeof(count));
    nullnet_len = sizeof(count);

    NETSTACK_NETWORK.output(NULL);
    count++;
    etimer_reset(&periodic_timer);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
