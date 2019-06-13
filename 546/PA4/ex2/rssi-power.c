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
    printf("%d\n", RSSI);
    msg_counter++;
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

    radio_value_t tx_power = -20;

    if(count <= 120)
    {
        tx_power = -20;
        leds_single_toggle(LEDS_LED1); // RED toggle
    }
    else if(count>120 && count<=240)
    {
        tx_power = -15;
        leds_single_off(LEDS_LED1); // RED off
        leds_single_toggle(LEDS_LED2); // GREEN toggle
    }
    else if(count>240 && count<=360)
    {
        tx_power = -10;
        leds_single_off(LEDS_LED2); // GREEN off
        leds_single_toggle(LEDS_LED1); // RED toggle
    }
    else if(count>360 && count<=480)
    {
        tx_power = -5;
        leds_single_off(LEDS_LED1); // RED off
        leds_single_toggle(LEDS_LED2); // GREEN toggle
    }
    else if(count>480 && count<=600)
    {
        tx_power = 0;
        leds_single_off(LEDS_LED2); // GREEN off
        leds_single_toggle(LEDS_LED1); // RED toggle
    }
    else
    {
        leds_single_off(LEDS_LED1); // RED off
        leds_single_off(LEDS_LED2); // GREEN off
    }

    NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, tx_power);
    NETSTACK_NETWORK.output(NULL);


    count++;
    etimer_reset(&periodic_timer);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
