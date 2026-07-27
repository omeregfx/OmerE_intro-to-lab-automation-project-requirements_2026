# Project 6: LED turns ON for 5 seconds on button press - the use of timers in Arduino

1. Understand the use of timers in Arduino and problems that they can solve

## Write a program that does the following:
- Turn on an LED on pin 4 when a button is pressed using interrupts
- The LED should turn off after 5 seconds
- Do not use a delay() function here. Please use the system clock to measure the time. look for the millis() function in the Arduino reference.
Test the code and make sure it works as expected
paste a screen shot from the logic analyzer below:
 
## update the code to add a delay in the loop function
- Add the same for loop as in the previous exercise to simulate a long process. Does the LED still turn off after 5 seconds? Why or why not?
answer here: __________
add a screen shot from the logic analyzer below:

## Write a second program. The proper way to solve this problem is to use a timer
- install package mstimer2 from the library manager
- read the readme file of the package and note the package limitations
- open an example of the package, examine the code and its functions and how to use them.
- implement a timer to turn off the LED after 5 seconds
- note the callback in the timer. When is it called?

## Exercises
- check that although there is delay in the loop function, the LED now turns off after 5 seconds

- change the LED time ON from 5 seconds to 30 ms, measure in the scope the time the LED is ON. is it 30 ms? Why or why not?
answer here: __________
paste a screen shot from the scope below:

## Answers
- Screenshot of the log analyzer without the for loop:
![alt text](<Screenshot no delay.png>)

- The LED doesn't turn off after 5 seconds. This is because the calculation in the main loop is preventing the Arduino from checking if millis() - startTime >= 5000 until it is done.
![alt text](<Screenshot with delay.png>)

- The time that the LED is on is ~28ms. This is because the Arduino Uno's Timer 2 generates its timebase using fixed hardware prescaler ratios. Because the system clock frequency cannot be divided down to yield a perfect whole number for every millisecond value, the library rounds the requested tick count to the nearest attainable hardware match. For very short durations like 30ms, this quantization rounding introduces a noticeable percentage of error.
![alt text](<Screenshot 30ms.png>)