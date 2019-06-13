/*
 * Modified Sensor example
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "sys/etimer.h"
#include "sys/ctimer.h"
#include "dev/leds.h"
#include "dev/watchdog.h"
#include "dev/button-hal.h"
#include "random.h"
#include "button-sensor.h"
#include "batmon-sensor.h"
#include "board-peripherals.h"
#include "rf-core/rf-ble.h"

#include "ti-lib.h"

#include <stdio.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
#define SENSOR_READING_PERIOD (CLOCK_SECOND * 5)
/*---------------------------------------------------------------------------*/
PROCESS(ex2_process, "ex2 process");
AUTOSTART_PROCESSES(&ex2_process);
/*---------------------------------------------------------------------------*/
static int current_tmp;
static int current_opt;

static struct ctimer opt_timer, tmp_timer;
/*---------------------------------------------------------------------------*/
static void init_opt_reading(void *not_used);
static void init_tmp_reading(void *not_used);
/*---------------------------------------------------------------------------*/
static void
get_tmp_reading()
{
  int value;
  clock_time_t next = SENSOR_READING_PERIOD;

  value = tmp_007_sensor.value(TMP_007_SENSOR_TYPE_ALL);

  if(value == CC26XX_SENSOR_READING_ERROR) {
    printf("TMP: Ambient Read Error\n");
    return;
  }

  value = tmp_007_sensor.value(TMP_007_SENSOR_TYPE_AMBIENT);
  current_tmp = value / 1000;
  printf("TMP: Ambient=%d.%03d C\n", value / 1000, value % 1000);

  if( current_tmp > 25 )
    {
      //hot, red LED on
      printf("RED ON\n");
      leds_single_on(LEDS_LED1);
      // leds_single_toggle(LEDS_LED1);
    }
    else{
      // red LED off
      printf("RED OFF\n");
      leds_single_off(LEDS_LED1);
    }


  SENSORS_DEACTIVATE(tmp_007_sensor);

  ctimer_set(&tmp_timer, next, init_tmp_reading, NULL);
}
/*---------------------------------------------------------------------------*/
static void
get_light_reading()
{
  int value;
  clock_time_t next = SENSOR_READING_PERIOD;

  value = opt_3001_sensor.value(0);
  if(value != CC26XX_SENSOR_READING_ERROR) 
  {
    printf("OPT: Light=%d.%02d lux\n", value / 100, value % 100);
    current_opt = value / 100;
    if( current_opt < 10 )
    {
      //darkness, green LED on
      printf("GREEN ON\n");
      leds_single_on(LEDS_LED2);
      // leds_single_toggle(LEDS_LED2);
    }
    else{
      // green LED off
      printf("GREEN OFF\n");
      leds_single_off(LEDS_LED2);
    }


  } else {
    printf("OPT: Light Read Error\n");
  }

  /* The OPT will turn itself off, so we don't need to call its DEACTIVATE */
  ctimer_set(&opt_timer, next, init_opt_reading, NULL);
}
/*---------------------------------------------------------------------------*/
static void
init_opt_reading(void *not_used)
{
  SENSORS_ACTIVATE(opt_3001_sensor);
}
/*---------------------------------------------------------------------------*/
static void
init_tmp_reading(void *not_used)
{
  SENSORS_ACTIVATE(tmp_007_sensor);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ex2_process, ev, data)
{

  PROCESS_BEGIN();
  printf("ex2 demo\n");

  leds_set(LEDS_ALL);
  SENSORS_ACTIVATE(tmp_007_sensor);
  SENSORS_ACTIVATE(opt_3001_sensor);

  while(1) 
  {

    PROCESS_YIELD();

    if(ev == sensors_event) 
    {
      if(data == &opt_3001_sensor) {
        get_light_reading();
      } else if(data == &tmp_007_sensor) {
        get_tmp_reading();
      }
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
