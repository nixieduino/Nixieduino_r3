/*
Nixieduino_r3.cpp - Library for Nixieduino clock, revision 3
more information about clock on Nixieduino.com
Created by Voloshchuk Sergey, March 2018
Released into public domain
*/
#include "Nixieduino_r3.h"

Nixieduino::Nixieduino(int tt): tube1(tt), tube2(tt), tube3(tt), tube4(tt)
{
	
}


void Nixieduino::Init(void)
{
	// Tubes:
	tube1.Init();
	tube2.Init();
	tube3.Init();
	tube4.Init();

	pinMode(T40, OUTPUT);
	pinMode(T41, OUTPUT);
	pinMode(T42, OUTPUT);
	pinMode(T43, OUTPUT);

	pinMode(T30, OUTPUT);
	pinMode(T31, OUTPUT);
	pinMode(T32, OUTPUT);
	pinMode(T33, OUTPUT);

	pinMode(T20, OUTPUT);
	pinMode(T21, OUTPUT);
	pinMode(T22, OUTPUT);
	pinMode(T23, OUTPUT);

	pinMode(T10, OUTPUT);
	pinMode(T11, OUTPUT);
	pinMode(T12, OUTPUT);
	pinMode(T13, OUTPUT);

	// Dot:  
	pinMode(DOTPIN, OUTPUT);

	// Encoder:
	pinMode(ENC_BUTTON, INPUT);
	pinMode(ENC_A, INPUT_PULLUP);
	pinMode(ENC_B, INPUT_PULLUP);

	// Fn button:
	DDRE = DDRE & ~0x80;

	// PCF8563 init
	Wire.beginTransmission(ADDR_PCF);
	Wire.write(0x0E);
	Wire.endTransmission();
	Wire.requestFrom(ADDR_PCF, 1);
	int t_control = Wire.read() & ~0x7C;

	if (t_control != 0x81)
	{
		Wire.beginTransmission(ADDR_PCF);	// Issue I2C start signal
		Wire.write(0x0);					// start address

		Wire.write(0x0); 	// control/status1
		Wire.write(0x11); 	// control/status2
		Wire.write(0x00); 	//set seconds
		Wire.write(0x00);	//set minutes
		Wire.write(0x00);	//set hour
		Wire.write(0x01);	//set day
		Wire.write(0x01);	//set weekday
		Wire.write(0x01); 	//set month, century to 1
		Wire.write(0x01);	//set year to 99
		Wire.write(0x80);	//minute alarm value reset to 00
		Wire.write(0x80);	//hour alarm value reset to 00
		Wire.write(0x80);	//day alarm value reset to 00
		Wire.write(0x80);	//weekday alarm value reset to 00
		Wire.write(0x0); 	//CLKOUT
		Wire.write(0x81); 	//timer control
		Wire.write(0x1);	//timer
		Wire.endTransmission();
	}
	
}

// Buttons ////////////////////////////////////////////////////////////////////

template<>
void Button<ENC_BUTTON>::Process(void)
{
	int curstate = digitalRead(ENC_BUTTON);
	if (curstate != laststate)
	{
		debcounter++;
		if (debcounter > 10)
		{
			debcounter = 0;
			state = curstate;
			laststate = curstate;
			if (state == PRESSED)
				needActionPress = true;
		}
	}
	else
	{
		debcounter = 0;
	}
}

template<>
void Button<FN_BUTTON>::Process(void)
{
	int curstate = (PINE & 0x80)>>7;
	if (curstate != laststate)
	{
		debcounter++;
		if (debcounter > 10)
		{
			debcounter = 0;
			state = curstate;
			laststate = curstate;
			if (state == PRESSED)
				needActionPress = true;
		}
	}
	else
	{
		debcounter = 0;
	}
}

// Tubes ///////////////////////////////////////////////////////////////////////

template<>
void Tube<1, 37>::Write(int num)
{
	int number;
	if(t_TubeType == IN_8_2)
		number = m_IN_8_2[num];
	else
		number = num;
		
	PORTA = (PORTA & 0xF0) | (number);
}

template<>
void Tube<2, 36>::Write(int num)
{
	int number;
	if(t_TubeType == IN_8_2)
		number = m_IN_8_2[num];
	else
		number = num;
	
	PORTA = (PORTA & 0x0F) | (number << 4);
}

template<>
void Tube<3, 35>::Write(int num)
{
	int number;
	if(t_TubeType == IN_8_2)
		number = m_IN_8_2[num];
	else
		number = num;
	
	DDRD = (DDRD & 0x0F) | (0x0F << 4);
	PORTD = (PORTD & 0x0F) | (number << 4);
}

template<>
void Tube<4, 34>::Write(int num)
{
	int number;
	if(t_TubeType == IN_8_2)
		number = m_IN_8_2[num];
	else
		number = num;
	
	digitalWrite(T43, (number & 8) != 0);
	digitalWrite(T42, (number & 4) != 0);
	digitalWrite(T41, (number & 2) != 0);
	digitalWrite(T40, (number & 1) != 0);
}
