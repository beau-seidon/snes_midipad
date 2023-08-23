#include <Arduino.h>
#include <SNES_Controller.h>


const int LATCH_PIN = 8;
const int CLOCK_PIN = 12;
const int DATA_PIN = 11;  

GamePad controller;



void setup()
{
	Serial.begin(115200);
	controller.init(LATCH_PIN, CLOCK_PIN, DATA_PIN); 
}



void loop()
{
	controller.poll();  // read controller

	if(controller.pressed(GamePad::START))  // check if Start was pressed since last loop
		Serial.println("Start was pressed.");

	if(controller.pressed(GamePad::A, 20))  // if A button is hold down repeat after 20 loops
		Serial.print("A");

	if(controller.down(GamePad::B))  // check if B button is currently pressed down
		Serial.print("B");
			
	delay(50);  // read controller ~20 times a second
}


