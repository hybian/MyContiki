/*
 * CPRE546 PA5 - Torch Game
 * named as mTorch.c
 *
 */
#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "net/packetbuf.h"
#include "dev/button-hal.h"
#include "button-sensor.h"
#include "batmon-sensor.h"
#include "board-peripherals.h"
#include "dev/leds.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

/*---------------------------------------------------------------------------*/
/* interval configurations */
#define SEND_INTERVAL (CLOCK_SECOND * 1)
#define SENSOR_READING_PERIOD (CLOCK_SECOND * 1)
#define LED_TOGGLE_PERIOD (CLOCK_SECOND * 1)
#define EXTINGUISH_THRESHOLD 10
#define RSSI_THRESHOLD -30

/* function prototypes */
static void my_tx_func(void * data);
static void my_led_func(void * data);
static void get_light_reading();
static void init_opt_reading(void *not_used);

/* global variables */
static struct ctimer opt_timer;
static struct ctimer led_timer;
static struct ctimer tx_timer;
int LIT_STATE = 0; // 0 refers to UNLIT, 1 refers to LIT
static unsigned count = 0; // just simple data in packets
/*---------------------------------------------------------------------------*/
PROCESS(mTorch_process, "mTorch");
AUTOSTART_PROCESSES(&mTorch_process);
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

    if( RSSI > RSSI_THRESHOLD && count == 1 ) // if close enough && that one is on
    {
      LIT_STATE = 1;
    }
  }
}
/*---------------------------------------------------------------------------*/
static void my_tx_func(void * data)
{
  ctimer_reset(&tx_timer);

  /* send data representing current state */
  if(LIT_STATE == 1)
    count = 1;
  else
    count = 0;

  /* send data */
  memcpy(nullnet_buf, &count, sizeof(count));
  nullnet_len = sizeof(count);
  NETSTACK_NETWORK.output(NULL);
}
/*---------------------------------------------------------------------------*/
static void my_led_func(void * data)
{
  ctimer_reset(&led_timer);

  if(LIT_STATE == 1)
    leds_single_toggle(LEDS_LED1); // RED toggle
  else
    leds_single_off(LEDS_LED1); // RED OFF
}
/*---------------------------------------------------------------------------*/
static void get_light_reading()
{
  int value;
  clock_time_t next = SENSOR_READING_PERIOD;

  value = opt_3001_sensor.value(0);
  if(value != CC26XX_SENSOR_READING_ERROR) 
  {
    printf("OPT: Light=%d.%02d lux\n", value / 100, value % 100);
    if( (value / 100) < EXTINGUISH_THRESHOLD ){
      LIT_STATE = 0; // extinguish
    }
  } 
  else {
    printf("OPT: Light Read Error\n");
  }

  /* The OPT will turn itself off, so we don't need to call its DEACTIVATE */
  ctimer_set(&opt_timer, next, init_opt_reading, NULL);
}
/*---------------------------------------------------------------------------*/
static void init_opt_reading(void *not_used)
{
  SENSORS_ACTIVATE(opt_3001_sensor);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mTorch_process, ev, data)
{
  button_hal_button_t *btn;

  PROCESS_BEGIN();

  /* Initialize button and OPT_sensor */
  SENSORS_ACTIVATE(opt_3001_sensor);
  btn = button_hal_get_by_index(0);

  /* Initialize NullNet */
  nullnet_buf = (uint8_t *)&count;
  nullnet_len = sizeof(count);
  nullnet_set_input_callback(input_callback);

  /* LED and TX timers */
  ctimer_set(&led_timer, LED_TOGGLE_PERIOD, my_led_func, NULL);
  ctimer_set(&tx_timer, SEND_INTERVAL, my_tx_func, NULL);

  while(1) 
  {
    PROCESS_YIELD();

    if(ev == button_hal_press_event) 
    {
      btn = (button_hal_button_t *)data;
      printf("Press event (%s)\n", BUTTON_HAL_GET_DESCRIPTION(btn));
      if(LIT_STATE == 0)
      {
        LIT_STATE = 1; // any button pressed will trigger to lit the torch
      }
    }

    if(ev == sensors_event && data == &opt_3001_sensor) 
    {
       get_light_reading();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

