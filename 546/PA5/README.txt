# PA5 - EX1

To run the demo:
- change working directory to where this README resides
- change the Contiki-NG path in the make file
- make TARGET=cc26x0-cc13x0 BOARD=sensortag/cc2650 mTorch
- upload via uniflash
- sudo chmod 666 /dev/ttyACM*
- make TARGET=cc26x0-cc13x0 BOARD=sensortag/cc2650 PORT=/dev/ttyACM0 login


Notes:
- press any button on the SensorTag (left or right) to lit the torch
- the default setting for proximity detection is: RSSI > -45dBm, this could change depending on the testing environment, 10cm was the distance when we did the test
- the default extinguish (light) detection is: opt < 10lux, this should be dark enough for hand covering
- if logged in to the mote, the console will print (1) RSSI value and sequence number when other motes are close and they are in lit state; (2) OPT value when the light sensor reading is below 10lux; 


Synchronization method:
- utilizing a sequence number structure. "LIT_STATE" is the variable in the source code.
- every mote mentiains a number, init=0.
- when the button is pressed, number = 100.
- led toggling and nullnet broadcasting are taking place in the same function (under control of the same timer).
- when the mote is in lit state, every time it toggles&broadcast, LIT_STATE+=1
- so LIT_STATE(even)==toggle(on); LIT_STATE(odd)==toggle(off); ==> during lit state ==> i.e. number>=100
- when the unlit motes receive a number both (>=100 && even), it restarts its timer for toggling function (toggle on right away), and update its number to the received number.
- this strategy assumes: (1) no delay between transmission (since speed of light (radio) can be ignored when they are this close to each other); (2) no delay processing the lines of code on mote (the callback functions, the if-else statements, etc..)
