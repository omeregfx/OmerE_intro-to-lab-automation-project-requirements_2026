# Project8.1 - Two way communication between the computer and the Arduino board using firmataexpress

## Goals
1. Understand the firmataexpress package and its usage in actual project
2. Understand the use of jupyter notebooks for exploration

**IMPORTANT:** the Jupyter notebook is useful for exploration, and the notebook file is a demonstration only. The project should be in a regular *.py file.

## Instructions
1. Install the firmataexpress package in the Arduino IDE. upload the firmataexpress sketch to the Arduino board.
2. Install the [pymata4 package](https://mryslab.github.io/pymata4/install_pymata4/) in the python environment - pip install pymata4 
3. follow along with the jupyter notebook to explore pymata usage. 

**IMPORTANT!** the pymata4 package sometimes causes crashes. The computers reset when booting and all your code will be lost in this case. Therefore, **commit and push often**.

## Requirements

1. Develop a Python script that communicates with an Arduino device using the pymata4 library.
2. Set up a pin for digital input and monitor its changes.
3. React to button presses on the Arduino. When the button is pressed, perform an action (e.g., print a message, turn on an LED).
5. use threading.Timer (from the threading python package) to turn off the led. This will allow the script to perform other tasks while waiting for the button press. This version of the script should be named Reacting_button_press_pymata4_using_timer.py.
6. Add a Graphical User Interface (GUI) to the timer version of the script. The GUI should display the state of the button (pressed or not pressed) and any actions performed in response to the button press. plus, it should have a textbox input to accept different time intervals for the timer.
7. light the led for 30 ms. Measure in scopy the length of the led light a 5-10 times and measure the average and standard deviation. Compare to previous arduino and non firmata python code.
8. Include comments explaining the functionality of each part of the code.
9. Write the script in a clean, organized, and efficient manner following good programming practices.


## Exercises
Comparison of AI changes if any:

Paste a screenshot of the GUI here:
![alt text](<Screenshot GUI.png>)

Paste a screenshot of the logic analyzer here that presents the time the LED is ON when pressing the button.
![alt text](<Screenshot logic analyzer30ms.png>)

```
What is the difference between the two versions (with pymata4 and firmata and without) of the script? Which one was easier to code? Where was thread handling easier?

Answer: The pymata4 version was easier to code and the thread handling in it was also easier in my opinion.

What can you say about the accuracy of the timing between the firmata version and your arduino code version?
which one is more accurate and why?

What are the advantages and disadvantages of using the pymata4 package?

Answer: The Arduino C++ code is much more accurate.
This is because that in the native Arduino approach, the timer and button detection logic reside directly on the microcontroller. 
When the button is pressed, a hardware interrupt is triggered instantly, and the MsTimer2 library schedules the turning off of the LED down to millisecond precision.
Advantages of using pymata4:
- You do not need to write, compile, or upload C++ code every time you want to change your project's logic or pin assignments.
- You upload the FirmataExpress sketch to the Arduino once, and the board effectively becomes a plug-and-play peripheral for any Python script.

Disadvantages of using pymata4:
- As noted above, pymata4 cannot guarantee strict real-time execution. If your application relies on microsecond precision, fast PWM generation, or tight time-critical sensor readings, pymata4 is unsuitable.

