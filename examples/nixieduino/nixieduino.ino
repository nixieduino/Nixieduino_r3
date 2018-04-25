#include <Nixieduino_r3.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <EEPROM.h>

#define COR_ADDR  0
#define MAX_MODE  6

const long SDAY = 86400;	// seconds per day

long vCorr;
int Curr_sec;
volatile bool SecInterrupt = false;
int Mode;
int Correction;
boolean encA, encB, lastA;
boolean encButton;
int LastSec;
int Weekday;
TIME Time;
Nixieduino board(IN_8_2); // IN_14 or IN_8_2

// SETUP

void setup()
{
  Wire.begin();
  Serial.begin(9600);

  EEPROM.get(COR_ADDR, Correction);
  vCorr = SDAY/2;
  Curr_sec = 0;
  
  // setting interrupt int6:
  // (interrupt from clock ic)
  PCMSK0 = (1 << PCINT6);
  PCICR = (1 << PCIE0);
  EIMSK = (1 << INT6);
  EICRB = (1 << ISC61);
  
  board.Init();
  Mode = -1;

}

void loop()
{
  // encoder:
  encA = board.buttonA.GetState();
  encB = board.buttonB.GetState();
  if (encA != lastA)
  {
    if (encA != encB)
    {
      Serial.println("CW");
      switch (Mode)
      {
        case -1:  break;

        case 0:
          if (Time.sec >= 30)
            Time.min++;
          Time.sec = 0;
          SetTime(Time);
          break;

        case 1:
          if (++Time.min > 59)
            Time.min = 0;
          SetTime(Time);
          break;

        case 2:
          if (++Time.hour > 23)
            Time.hour = 0;
          SetTime(Time);
          break;

        case 3:
          if (++Time.day > MaxDate(Time.month, Time.year))
            Time.day = 1;
          SetTime(Time);
          break;

        case 4:
          if (++Time.month > 12)
            Time.month = 1;
          CheckDay(Time);
          SetTime(Time);
          break;

        case 5:
          if (++Time.year > 2199)
            Time.year = 2000;
          CheckDay(Time);
          SetTime(Time);
          break;
          
        case 6:
          if (++Correction > 99)
            Correction = 99;
          EEPROM.put(COR_ADDR, Correction);
          break;
      }
    }
    else
    {
      Serial.println("CCW");
      switch (Mode)
      {
        case -1:  break;

        case 0:   if (Time.sec >= 30)
            Time.min++;
          Time.sec = 0;
          SetTime(Time);
          break;

        case 1:   if (--Time.min < 0)
            Time.min = 59;
          SetTime(Time);
          break;

        case 2:   if (--Time.hour < 0)
            Time.hour = 23;
          SetTime(Time);
          break;

        case 3:   if (--Time.day < 1)
            Time.day = MaxDate(Time.month, Time.year);
          SetTime(Time);
          break;

        case 4:   if (--Time.month < 1)
            Time.month = 12;
          CheckDay(Time);
          SetTime(Time);
          break;

        case 5:   if (--Time.year < 2000)
            Time.year = 2199;
          CheckDay(Time);
          SetTime(Time);
          break;
          
        case 6:
          if (--Correction < -99)
            Correction = -99;
          EEPROM.put(COR_ADDR, Correction);
          break;
      }
    }
    lastA = encA;
  }

  // buttons:
  board.buttonE.Process();
  board.buttonFn.Process();
  if (board.buttonE.needActionPress)
  {
    board.buttonE.needActionPress = false;
    if (Mode != -1)
    {
      Mode++;
      if (Mode > MAX_MODE)
        Mode = 0;
    }
  }
  else if (board.buttonE.needActionHold)
  {
    board.buttonE.needActionHold = false;
    if (Mode == -1)
      Mode = 0;
    else if (Mode != -1)
    {
      //SetTime(Time);
      Mode = -1;
    }
  }

  if (board.buttonFn.needActionPress)
  {
    board.buttonFn.needActionPress = false;
  }

  // display:
  switch (Mode)
  {
    case -1:
      if (board.buttonFn.state == PRESSED)
      {
        // display weekday
        board.tube3.On();
        board.tube1.Write(Time.month % 10);
        board.tube2.Write(Time.month / 10);
        board.tube3.Write(Time.day % 10);
        board.tube4.Write(Time.day / 10);
      }
      else
      {
        // display time
        board.tube3.On();
        board.tube1.Write(Time.min % 10);
        board.tube2.Write(Time.min / 10);
        board.tube3.Write(Time.hour % 10);
        board.tube4.Write(Time.hour / 10);
        if (Time.hour / 10 == 0)
          board.tube4.Off();
        else
          board.tube4.On();
      }
      if (LastSec != Time.sec)
      {
        digitalWrite(DOTPIN, !digitalRead(DOTPIN));
        LastSec = Time.sec;
      }
      break;

    case 0:
      // seconds
      digitalWrite(DOTPIN, 0);
      board.tube3.Off();
      board.tube4.On();
      board.tube1.Write(Time.sec % 10);
      board.tube2.Write(Time.sec / 10);
      board.tube4.Write(Mode + 1);
      break;

    case 1:
	  // minutes
      digitalWrite(DOTPIN, 0);
      board.tube3.Off();
      board.tube1.Write(Time.min % 10);
      board.tube2.Write(Time.min / 10);
      board.tube4.Write(Mode + 1);
      break;

    case 2:
	  // hours
      digitalWrite(DOTPIN, 0);
      board.tube3.Off();
      board.tube1.Write(Time.hour % 10);
      board.tube2.Write(Time.hour / 10);
      board.tube4.Write(Mode + 1);
      break;

    case 3:
	  // day
      digitalWrite(DOTPIN, 0);
      board.tube3.Off();
      board.tube1.Write(Time.day % 10);
      board.tube2.Write(Time.day / 10);
      board.tube4.Write(Mode + 1);
      break;

    case 4:
      // month
      board.tube3.Off();
      board.tube1.Write(Time.month % 10);
      board.tube2.Write(Time.month / 10);
      board.tube4.Write(Mode + 1);
      break;

    case 5:
      // year
      board.tube3.On();
      board.tube1.Write(Time.year % 10);
      board.tube2.Write((Time.year / 10) % 10);
      board.tube3.Write((Time.year / 100) % 10);
      board.tube4.Write(Time.year / 1000);
      break;

    case 6:
      // correction value
      EEPROM.get(COR_ADDR, Correction);
      
      // negative or positive correction value
      if (Correction < 0)
        digitalWrite(DOTPIN, 1);
      else
        digitalWrite(DOTPIN, 0);

      board.tube3.Off();
      board.tube1.Write(abs(Correction) % 10);
      board.tube2.Write(abs(Correction) / 10);
      board.tube4.Write(Mode + 1);
  }

  if (SecInterrupt)
  {
    SecInterrupt = false;
    board.buttonE.IncCounter();
    board.buttonFn.IncCounter();

    // clear interrupt flag in control status 2:
    Wire.beginTransmission(0x51);
    Wire.write(0x1);
    Wire.write(0x01);
    Wire.endTransmission();

    // read time:
    Wire.requestFrom(0x51, 7);
    Time.sec = bcdToDec(Wire.read() & 0x7f);
    Time.min = bcdToDec(Wire.read() & 0x7f);
    Time.hour = bcdToDec(Wire.read() & 0x3f);
    Time.day = bcdToDec(Wire.read() & 0x3f);
    Weekday = Wire.read() & 0x7; // weekday
    char century_month = Wire.read();
    char century = (century_month & 0x80) != 0;
    Time.month = bcdToDec(century_month & 0x1F);
    Time.year = 2000 + 100 * century + bcdToDec(Wire.read());

    // TODO: date to EEPROM

    // time correction:
    if (Curr_sec != Time.sec)
    {
      vCorr += Correction;
      if(vCorr < 0)
        {
        vCorr += SDAY;
        // decrease time for 1 sec
        Time.sec--;
        
        Wire.beginTransmission(0x51);
        Wire.write(0x02);
        Wire.write(decToBcd(Time.sec));
        Wire.endTransmission();
        }
      else if(vCorr > SDAY)
        {
        vCorr -= SDAY;
        Time.sec++;

        Wire.beginTransmission(0x51);
        Wire.write(0x02);
        Wire.write(decToBcd(Time.sec));
        Wire.endTransmission();
        }
      Curr_sec = Time.sec;
    }

    
  }
}

ISR(INT6_vect)
{
  SecInterrupt = true;
}

byte decToBcd(byte val)
{
  return ( (val / 10 * 16) + (val % 10) );
}

byte bcdToDec(byte val)
{
  return ( (val / 16 * 10) + (val % 16) );
}

void SetTime (TIME& t)
{
  Wire.beginTransmission(0x51);	// Issue I2C start signal
  Wire.write(0x02);   	// send addr low byte, req'd

  Wire.write(decToBcd(t.sec));
  Wire.write(decToBcd(t.min));
  Wire.write(decToBcd(t.hour));
  Wire.write(decToBcd(t.day));
  Wire.endTransmission();

  Wire.beginTransmission(ADDR_PCF);
  Wire.write(0x07);
  char mn = t.month;
  if (t.year >= 2100)
    mn |= 0x80;
  Wire.write(decToBcd(mn));
  Wire.write(decToBcd(t.year % 100));
  Serial.println(t.year);
  Wire.endTransmission();

}

void CheckDay(TIME& t)
{
  int max = MaxDate(t.month, t.year);
  if (t.day > max)
    t.day = max;
}

int MaxDate(int month, int year)
{
  switch (month)
  {
    case 1:  return 31;
    case 2:  if (year % 4 == 0 && year != 2100)
        return 29;
      else
        return 28;
    case 3:  return 31;
    case 4:  return 30;
    case 5:  return 31;
    case 6:  return 30;
    case 7:  return 31;
    case 8:  return 31;
    case 9:  return 30;
    case 10: return 31;
    case 11: return 30;
    case 12: return 31;
    default: return 0;
  }
}

