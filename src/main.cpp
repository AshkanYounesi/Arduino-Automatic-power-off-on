#include <Arduino.h>
#include <SoftwareSerial.h>
#include <String.h>
#include <Wire.h>
#include "RTClib.h"

RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
SoftwareSerial MySerial(4, 3); // RX, TX
unsigned int i = 0;
boolean Edit = false;
boolean Status = true;
boolean Save = true;
boolean Wifistate = false;
String serial_input;

boolean beforwifistatus = false;
boolean beforcheck = true;
boolean Relay = false;
boolean RelayOneTime = false;

int oldmin = 0;
int RelayPin = 9;
int test = 0;

long HourOn = 07, MinuteOn = 00, SecondOn = 00;
long HourOff = 23, MinuteOff = 00, SecondOff = 00;

int rec(int q);
void (*ResetFunc)(void) = 0;

void Send(String data)
{
  MySerial.println(data);
  MySerial.flush();
}

void setup()
{
  Serial.begin(9600);
  MySerial.begin(9600);
  pinMode(RelayPin, OUTPUT);
  digitalWrite(RelayPin, LOW);
  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    while (1)
      ;
  }

  if (!rtc.isrunning())
  {
    Serial.println("RTC lost power, lets set the time!");

    // Comment out below lines once you set the date & time.
    // Following line sets the RTC to the date & time this sketch was compiRelayPin
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

    // Following line sets the RTC with an explicit date & time
    // for example to set January 27 2017 at 12:56 you would call:
    //rtc.adjust(DateTime(2017, 1, 27, 01, 56, 0));
  }
}

void loop()
{
  DateTime now = rtc.now();
  if (MySerial.available())
  {
    serial_input = MySerial.readString();
    Serial.println(serial_input);

    if (serial_input[0] == 'C' && serial_input[4] == 'k')
    {
      if (Status == true)
      {
        if (digitalRead(RelayPin))
        {
          Send("AutoWifion");
          beforwifistatus = true;
        }
        else
        {
          Send("AutoWifioff");
          beforwifistatus = false;
        }
      }
      else
      {
        if (digitalRead(RelayPin))
        {
          Send("ManWifion");
        }
        else
        {
          Send("ManWifioff");
        }
      }
      // Send((String)((HourOn * 10000) + (MinuteOn * 100) + SecondOn));
      // delay(200);
      long Hour = ((rec(5) * 10) + rec(6));
      long Minute = ((rec(8) * 10) + rec(9));
      long Second = ((rec(11) * 10) + rec(12));
      rtc.adjust(DateTime(2017, 1, 27, Hour, Minute, Second + 2));
      serial_input[0] = 'i';
    }
    else if (serial_input == "Automatic")
    {
      beforcheck = true;
      Status = true;
      if (Relay)
      {
        Send("AutoWifion");
        digitalWrite(RelayPin, HIGH);
      }
      else
      {
        Send("AutoWifioff");
        digitalWrite(RelayPin, LOW);
      }
    }
    else if (serial_input == "Manual")
    {
      beforcheck = false;
      Status = false;
      if (digitalRead(RelayPin))
      {
        Send("Wifion");
      }
      else
      {
        Send("Wifioff");
      }
    }
    else if (serial_input == "Wifi on")
    {
      digitalWrite(RelayPin, HIGH);
      Send("Wifion");
    }
    else if (serial_input == "Wifi off")
    {
      digitalWrite(RelayPin, LOW);
      Send("Wifioff");
    }
    else if (serial_input == "Setting")
    {
      Edit = true;
      DateTime now = rtc.now();
      long Hour = now.hour();
      long Minute = now.minute();
      long Second = now.second();
      Send((String)((Hour * 10000) + (Minute * 100) + Second));
    }
    else if (serial_input == "Save")
    {
      Status = true;
      Edit = false;
      Save = true;
      Send("Save");
    }
    else if (serial_input == "Cansel")
    {
      Edit = false;
      Send("Cansel");
    }
    else if (serial_input == "Skip")
    {
      Edit = false;
      Send("Skip");
    }
    else if (serial_input == "HardwareReset")
    {
      Send("HardwareReset");
      ResetFunc();
    }
    else if (serial_input == "ModemReset")
    {
      Send("ModemReset");
      digitalWrite(RelayPin, LOW);
      delay(10000);
      digitalWrite(RelayPin, HIGH);
      Send("ModemResetOK");
    }
  }
  if (Save)
  {
  }

  if (serial_input[0] == 'O' && serial_input[1] == 'n')
  {
    Send((String)((HourOff * 10000) + (MinuteOff * 100) + SecondOff));
    delay(200);
    HourOn = ((rec(2) * 10) + rec(3));
    MinuteOn = ((rec(5) * 10) + rec(6));
    SecondOn = ((rec(8) * 10) + rec(9));
    serial_input[0] = 'i';
  }

  if (serial_input[0] == 'O' && serial_input[2] == 'f')
  {
    Status = true;
    Edit = false;
    Save = true;
    Send("Save");
    HourOff = ((rec(3) * 10) + rec(4));
    MinuteOff = ((rec(6) * 10) + rec(7));
    SecondOff = ((rec(9) * 10) + rec(10));
    serial_input[0] = 'i';
  }

  if (Status)
  {
    if (Relay && RelayOneTime == false)
    {
      digitalWrite(RelayPin, HIGH);
      RelayOneTime = true;
      Send("AutoWifion");
    }
    else if (!Relay && RelayOneTime == true)
    {
      digitalWrite(RelayPin, LOW);
      RelayOneTime = false;
      Send("AutoWifioff");
    }
  }

  long Hour = (int)(now.hour());
  long Minute = (int)(now.minute());
  if (HourOn < Hour && beforwifistatus == false)
  {
    Relay = true;
    beforwifistatus = true;
  }
  else if (HourOn == Hour && MinuteOn <= Minute && beforwifistatus == false)
  {
    Relay = true;
    beforwifistatus = true;
  }
  if ((HourOff < Hour || HourOn > Hour) && beforwifistatus == true)
  {
    Relay = false;
    beforwifistatus = false;
  }
  else if (((HourOff == Hour && MinuteOff <= Minute) || (HourOn == Hour && MinuteOn > Minute)) && beforwifistatus == true)
  {
    Relay = false;
    beforwifistatus = false;
  }
}

int rec(int q)
{
  switch (serial_input[q])
  {
  case '0':
    return 0;
    break;
  case '1':
    return 1;
    break;
  case '2':
    return 2;
    break;
  case '3':
    return 3;
    break;
  case '4':
    return 4;
    break;
  case '5':
    return 5;
    break;
  case '6':
    return 6;
    break;
  case '7':
    return 7;
    break;
  case '8':
    return 8;
    break;
  case '9':
    return 9;
    break;

  default:
    break;
  }
  return 12;
}
