# Soccer_on_De1-SoC
This is a simple soccer game written in C language that is meant to run on the De1-SoC board, a particular FPGA board. It uses vga graphics to draw 
entities such as a player, a ball, and a goal.

To run (in virtual):
1. download the file
2. go to https://cpulator.01xz.net/?sys=arm-de1soc
3. select language c
4. upload the file
5. press continue to run
6. If a edge-capture warning pops, press continue to dismiss

Alteratively, you could run it physically with an actually De1-SoC board with the intel fgpa program. You should find the instructions here
https://www.intel.com/content/www/us/en/programmable/downloads/download-center.html

The objective of the game is to pass all the levels by kicking the ball into the goal to score. You will automatically lose if the ball hits any of the edges
(out of bounds), or if the opponents touch the ball. The green line is the trajectory of the ball. You can change the angle of the trajectory of the shot by pressing KEY1
and KEY0. Note that you have to double click the key as it uses the lower-edge of the keypress. To shoot or pass to the teammate, press KEY2. If you score, you will move on to
the next level. If you lose, you can reset the game by clicking KEY3.

Controls:
(Doubleclick)
KEY0: Rotate kick clockwise
KEY1: Rotate kick counter-clockwise
KEY2: Kick
KEY3: Reset game
