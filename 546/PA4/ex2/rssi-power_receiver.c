/*
 * Modified nullnet broadcast example
 * named as rssi-power
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
int msg_counter=0;
/*---------------------------------------------------------------------------*/
PROCESS(rssi_power_process, "rssi-power");
AUTOSTART_PROCESSES(&rssi_power_process);
/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  if(len == sizeof(unsigned)) 
  {
    unsigned count;
    memcpy(&count, data, sizeof(count));
    int16_t RSSI = (int16_t)packetbuf_attr(PACKETBUF_ATTR_RSSI);    
    printf("%d\t%d\n", msg_counter, RSSI);
    msg_counter++;
    leds_single_toggle(LEDS_LED1); // RED toggle
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(rssi_power_process, ev, data)
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

    // radio_value_t tx_power = -20;

    // NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, tx_power);
    // NETSTACK_NETWORK.output(NULL);


    count++;
    etimer_reset(&periodic_timer);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
