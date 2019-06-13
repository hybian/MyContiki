# PA3 - EX1

to run the demo:
- change the Contiki-NG path in the make file.
- make TARGET=cc26x0-cc13x0 BOARD=sensortag/cc2650 reception-counter
- upload via uniflash

note: to achieve bullet point 4 in description, we implemented the LEDs so that the green LED toggles for every "unit digit" counter, and the red LED toggles for every "tens digit" counter.