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

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

/*---------------------------------------------------------------------------*/
/* interval configurations */
#define SENSOR_READING_PERIOD (CLOCK_SECOND * 1)
#define TOGGLE_PERIOD (CLOCK_SECOND * 1)
#define EXTINGUISH_THRESHOLD 10
#define RSSI_THRESHOLD -30

/* function prototypes */
static void my_toggle_func(void * data);
static void get_light_reading();
static void init_opt_reading(void *not_used);

/* global variables */
static struct ctimer opt_timer;
static struct ctimer toggle_timer;
signed LIT_STATE = 0; // 0->UNLIT; >=2->LIT; even==toggle(on); odd==toggle(off)
/*---------------------------------------------------------------------------*/
PROCESS(mTorch_process, "mTorch");
AUTOSTART_PROCESSES(&mTorch_process);
/*---------------------------------------------------------------------------*/
void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  if(len == sizeof(signed)) 
  {
    signed count;
    memcpy(&count, data, sizeof(count));
    int16_t RSSI = (int16_t)packetbuf_attr(PACKETBUF_ATTR_RSSI);  // get RSSI reading

    if( RSSI > RSSI_THRESHOLD && count > 0 ) // if close enough && that one is on
    {
      LOG_INFO("Received %u, my_seq = %u, RSSI=%d\n", count, LIT_STATE, RSSI);

      if(LIT_STATE == 0) // if myself is unlit
      {
        if(count%2 == 0) // if the other one is toggle(on) right now
        {
          printf("restarted timer from IF\n");
          ctimer_restart(&toggle_timer); // restart right now to synchronize
          LIT_STATE = count; // update my sequnce number to his
        }
      }
      else // if myself is lit
      {
        if( (count-LIT_STATE) > 4 ) // if his bigger && we are different enough 
        {   
            printf("restarted timer from ELSE\n");
            LIT_STATE = 0; // make myself unlit for now
        }
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
static void my_toggle_func(void * data)
{
  if( LIT_STATE > 0 ) // lit state
  {
    leds_single_toggle(LEDS_LED1); // RED toggle
    LIT_STATE++; // increment counter based on the rand, even==toggle(on), odd==toggle(off)
  }
  else
  {
    leds_single_off(LEDS_LED1); // RED OFF
  }

  /* send data */
  memcpy(nullnet_buf, &LIT_STATE, sizeof(LIT_STATE));
  nullnet_len = sizeof(LIT_STATE);
  NETSTACK_NETWORK.output(NULL);

  ctimer_reset(&toggle_timer);
}
/*---------------------------------------------------------------------------*/
static void get_light_reading()
{
  int value;
  clock_time_t next = SENSOR_READING_PERIOD;

  value = opt_3001_sensor.value(0);
  if(value != CC26XX_SENSOR_READING_ERROR) 
  {
    if( (value / 100) < EXTINGUISH_THRESHOLD ){
      printf("OPT: Light=%d.%02d lux\n", value / 100, value % 100);
      LIT_STATE = 0; // extinguish
    }
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
  nullnet_buf = (uint8_t *)&LIT_STATE;
  nullnet_len = sizeof(LIT_STATE);
  nullnet_set_input_callback(input_callback);

  /* Toggle timer set */
  ctimer_set(&toggle_timer, TOGGLE_PERIOD, my_toggle_func, NULL);

  while(1) 
  {
    PROCESS_YIELD();

    /* button event to lit */
    if(ev == button_hal_press_event) 
    {
      btn = (button_hal_button_t *)data;
      printf("Press event (%s)\n", BUTTON_HAL_GET_DESCRIPTION(btn));
      if(LIT_STATE == 0)
      {
        // any button pressed will trigger to lit the torch
        LIT_STATE = 2 * (rand() % 100 + 1); // random generate a even num between 2->200
      }
    }

    /* light sensor event to extinguish */
    if(ev == sensors_event && data == &opt_3001_sensor) 
    {
       get_light_reading();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/