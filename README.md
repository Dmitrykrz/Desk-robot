Project structure:
PCB - files and photos for the pcb
Hardware - files from Fusion 360 for the robot
Software - Code for esp32-c6
Dashboard - Code and images for node-red where dashboard runs

What is it for? As with all my projects I first do them and then ask questionn What is the point.
Now after making it I ca answer that this cute companion sits on my desk, waves his hands on me and generally being nice code byddy.

The robot rotates one one big bearing. Inside the bearing there is a 3mm metal shaft connecting the base with upper body. Around this shaft the hollow slip ring is installed that transmits power from usb connector in the base to upper body. The shaft is connected to the stepped motor with a  1:34 gearbox for smooth rotation movement.

For both arm the small stepper motor is chosen with integrated worm motor that allows tthe reduction.

3 steppers are controlled with tmc2209 drivers installed on a custom made pcb with esp32-c6 board and a dc-dc converter.


Radar module hidden under the screen behind the plastic can detect human movement and presence. as well as measuring the distance to the human for up to 8 meters. the spread of the radar is rather wide.

The screen is a charliplexed module of individual SMD leds made by adafruit. This display was the inspiration for the whole project. I stumbled opon it, liked its yellow retro aestaetics and immideately got the idea for a desk robot with a cute face on that screen.