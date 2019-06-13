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


/* send interval configuration */
#define SEND_INTERVAL (CLOCK_SECOND / 4)

/*---------------------------------------------------------------------------*/
PROCESS(nullnet_test_process, "nullnet_test");
AUTOSTART_PROCESSES(&nullnet_test_process);

static struct ctimer send_timer;
static unsigned count = 0;
/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  if(len == sizeof(unsigned)) 
  {
    unsigned count;
    memcpy(&count, data, sizeof(count));
    int16_t RSSI = (int16_t)packetbuf_attr(PACKETBUF_ATTR_RSSI);    
    printf("%d: RSSI=%d\n", count, RSSI);
  }
}
/*---------------------------------------------------------------------------*/
static void my_send_func(void * data)
{
  ctimer_reset(&send_timer);

  memcpy(nullnet_buf, &count, sizeof(count));
  nullnet_len = sizeof(count);
  NETSTACK_NETWORK.output(NULL);
  count++;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nullnet_test_process, ev, data)
{
  // static struct etimer tx_timer;
  // static unsigned count = 0;

  PROCESS_BEGIN();

  /* Initialize NullNet */
  nullnet_buf = (uint8_t *)&count;
  nullnet_len = sizeof(count);
  nullnet_set_input_callback(input_callback);

  ctimer_set(&send_timer, CLOCK_SECOND*3, my_send_func, NULL);

  // etimer_set(&tx_timer, SEND_INTERVAL);
  // while(1) 
  // {
  //   PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&tx_timer));
    
  //   memcpy(nullnet_buf, &count, sizeof(count));
  //   nullnet_len = sizeof(count);

  //   NETSTACK_NETWORK.output(NULL);
  //   count++;
  //   etimer_reset(&tx_timer);
  // }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
