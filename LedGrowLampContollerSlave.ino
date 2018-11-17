/*(C) nov 2018. by joy. LedGrowLamp Controller Slave.
This is to use on the following RF24 controled Slave lamps, 
to repeat the Master. 
(not a beerware)*/
#include <SPI.h>
#include <Wire.h>
#include "RF24.h"
RF24 myRadio (8, 7);
#include <LM75.h>
LM75 sensor(LM75_ADDRESS | 0b000);
byte addresses[][6] = {"1Node"};
int dataReceived;
const int Uv = 10;
const int Wh = 9;
const int Fan = 3;
const int Lamp135 = 5;
const int Lamp24 = 6;
void setup()  {
  pinMode(Fan, OUTPUT);
  pinMode(Wh, OUTPUT);
  pinMode(Uv, OUTPUT);
  pinMode(Lamp135, OUTPUT);
  pinMode(Lamp24, OUTPUT);
  Wire.begin();
  Serial.begin(9600);
  myRadio.begin();
  myRadio.setChannel(108);
  myRadio.openReadingPipe(1, addresses[0]);
  myRadio.startListening();
  delay(1000);
}
void loop()  {
  if ( myRadio.available())  {
    while (myRadio.available())  {
      myRadio.read( &dataReceived, sizeof(dataReceived) );
      if (dataReceived == 0 )  {
        Serial.println("Fan: ON");
        digitalWrite(Fan, HIGH);
      }
      if (dataReceived == 1 )  {
        Serial.println("Fan: OFF");
        digitalWrite(Fan, LOW);
      }
      if (dataReceived == 2 )  {
        Serial.println("White: ON");
        digitalWrite(Wh, HIGH);
      }
      if (dataReceived == 3 )  {
        Serial.println("White: OFF");
        digitalWrite(Wh, LOW);
      }
      if (dataReceived == 4 )  {
        Serial.println("Uv: ON");
        digitalWrite(Uv, HIGH);
      }
      if (dataReceived == 5 )  {
        Serial.println("Uv: OFF");
        digitalWrite(Uv, LOW);
      }
      if (dataReceived == 6 )  {
        Serial.println("Led 40%: ");
        digitalWrite(Lamp24, HIGH);
      }
      if (dataReceived == 7 )  {
        Serial.println("Led 60%: ");
        digitalWrite(Lamp135, HIGH);
      }
      if (dataReceived == 8 )  {
        Serial.println("Led 100%: ");
        digitalWrite(Lamp24, HIGH);
        digitalWrite(Lamp135, HIGH);
      }
      if (dataReceived == 9 )  {
        Serial.println("Led OFF");
        digitalWrite(Lamp24, LOW);
        digitalWrite(Lamp135, LOW);
      }
      delay(50);
    }
  }
}

