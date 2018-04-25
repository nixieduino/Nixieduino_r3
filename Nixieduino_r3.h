#include "Arduino.h"
#include "Wire.h"

#define IN_14		0
#define IN_8_2		1

#define DOTPIN		33

#define T40			49
#define T41			48
#define T42			47
#define T43			46

#define T30			45
#define T31			44
#define T32			43
#define T33			38

#define T20			26
#define T21			27
#define T22			28
#define T23			29

#define T10			22
#define T11			23
#define T12			24
#define T13			25

#define ENC_BUTTON	32
#define FN_BUTTON	0xE7

#define ENC_A		31
#define ENC_B		30

#define PRESSED		0
#define NOTPRESSED	1
#define HOLDED		2

#define ADDR_PCF	0x51



struct TIME
{
	TIME()
	{
		sec = min = hour = 0;
		day = month = 1;
		year = 2018;
	}

	int sec;
	int min;
	int hour;
	int day;
	int month;
	int year;
};

template<int PIN_NUMBER> class EncChan
{
public:
	int GetState()
	{
		int curstate = digitalRead(PIN_NUMBER);

		// debouncing
		if (curstate != laststate)
			counter = 0;
		else
			counter++;
		if (counter > 10)
		{
			counter = 0;
			state = curstate;
		}
		laststate = curstate;

		return state;
	}

private:
	int counter;
	int state;
	int laststate;
};

template<int PIN_NUMBER> class Button
{
	int debcounter;
	int holdcounter;
	
	int laststate;

public:
	boolean needActionPress;
	boolean needActionHold;
	int state;

	void Process();
	void IncCounter()
	{
		if (state == PRESSED)
		{
			holdcounter++;
			if (holdcounter > 3 * 64)
			{
				holdcounter = 0;
				state = HOLDED;
				laststate = PRESSED;
				needActionPress = false;
				needActionHold = true;
			}
		}
		else
			holdcounter = 0;
	}
};

template<int TUBE_NUMBER, int PIN_ENABLE> class Tube
{
public:
	
	int t_TubeType;
	int m_IN_8_2[10]	= {0,7,1,2,3,4,5,6,8,9};

	Tube(int tt)
	{
		t_TubeType = tt;
	}
	
	void Write(int num);
	void Init(void)
	{
		pinMode(PIN_ENABLE, OUTPUT);
		digitalWrite(PIN_ENABLE, HIGH);
	}
	void On(void)
	{
		digitalWrite(PIN_ENABLE, HIGH);
	}
	void Off(void)
	{
		digitalWrite(PIN_ENABLE, LOW);
	}
	
};



class Nixieduino 
{
public:
	Nixieduino(int tt);
	void Init(void);

	EncChan<ENC_A> buttonA;
	EncChan<ENC_B> buttonB;
	Button<ENC_BUTTON> buttonE;
	Button<FN_BUTTON> buttonFn;
	
	int TubeType;
	
	Tube<1, 37> tube1;
	Tube<2, 36> tube2;
	Tube<3, 35> tube3;
	Tube<4, 34> tube4;
};
