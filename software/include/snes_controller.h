/*
	this code is a stripped down version of the ArduinoGameController project by bitluni.
	original code can be found here:  https://github.com/bitluni/ArduinoGameController
*/



#pragma once

#include <Arduino.h>



class GamePad
{
	public:

	enum Button
	{
		B = 0,
		Y = 1,
		SELECT = 2,
		START = 3,
		UP = 4,
		DOWN = 5,
		LEFT = 6,
		RIGHT = 7,
		A = 8,
		X = 9,
		L = 10,
		R = 11,
	};


	int latchPin;
	int clockPin;
	int dataPin;
	
	long buttons[12];


	void init(int latch, int clock, int data)
	{
		latchPin = latch;
		clockPin = clock;
		dataPin = data;

		pinMode(latchPin, OUTPUT);
		digitalWrite(latchPin, LOW);

		pinMode(clockPin, OUTPUT);
		digitalWrite(clockPin, HIGH);

		for(int i = 0; i < 12; i++) buttons[i] = -1;

		pinMode(dataPin, INPUT_PULLUP);
	}
 
  
	void poll()
	{
		digitalWrite(latchPin, HIGH);
		delayMicroseconds(12);

		digitalWrite(latchPin, LOW);  
		delayMicroseconds(6);

		for(int i = 0; i < 12; i++)
		{
			if(dataPin > -1)
			{
				if(digitalRead(dataPin))
					buttons[i] = -1;
				else
					buttons[i]++;
			}
			
			digitalWrite(clockPin, LOW);
			delayMicroseconds(6);

			digitalWrite(clockPin, HIGH);
			delayMicroseconds(6);
		}
	}


	///button will be unpressed until released again
	void clear(int b)
	{
		buttons[b] = 0x80000000;
	}


	///returns if button is currently down
	bool down(int b) const
	{
		return buttons[b] >= 0;
	}


	///returns true if button state changed to down since previous poll. repeatAfterTics can be used to repeat after button was hold down for some time
	bool pressed(int b, long repeatAfterTics = 0x7fffffff) const
	{
		return buttons[b] == 0 || (buttons[b] >= repeatAfterTics);
	}

};


