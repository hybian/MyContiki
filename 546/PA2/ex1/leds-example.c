/*
 * Modified LED example using ctimer
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "dev/leds.h"
#include "sys/etimer.h"

#include <stdio.h>

static void my_led_func(void * data);
/*---------------------------------------------------------------------------*/
// static struct etimer et;
static struct ctimer ct;
static uint8_t counter;
/*---------------------------------------------------------------------------*/
PROCESS(leds_example, "LED HAL Example");
AUTOSTART_PROCESSES(&leds_example);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(leds_example, ev, data)
{
  PROCESS_BEGIN();
  counter = 0;
  ctimer_set(&ct, 5 * CLOCK_SECOND, my_led_func, NULL);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

static void my_led_func(void * data)
{
  ctimer_reset(&ct);

  if((counter & 7) == 0) {
    printf("set LEDS__ALL\n");
    leds_set(LEDS_ALL);
  } else if((counter & 7) == 1) {
    printf("off LEDS__ALL\n");
    leds_off(LEDS_ALL);
  } else if((counter & 7) == 2) {
    printf("on LEDS__ALL\n");
    leds_on(LEDS_ALL);
  } else if((counter & 7) == 3) {
    printf("toggle LEDS__ALL\n");
    leds_toggle(LEDS_ALL);
#if !LEDS_LEGACY_API
  } else if((counter & 7) == 4) {
    printf("single_on LEDS__LED1\n");
    leds_single_on(LEDS_LED1);
  } else if((counter & 7) == 5) {
    printf("single_off LEDS__LED1\n");
    leds_single_off(LEDS_LED1);
  } else if((counter & 7) == 6) {
    printf("single_toggle LEDS__LED1\n");
    leds_single_toggle(LEDS_LED1);
#endif /* LEDS_LEGACY_API */
  } else if((counter & 7) == 7) {
    printf("single_toggle LEDS__ALL\n");
    leds_toggle(LEDS_ALL);
  }
  counter++;
}
