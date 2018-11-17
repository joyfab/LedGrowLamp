/*(C) nov 2018. by joy. LedGrowLamp Controller Master.
This is to use on the firth bluetooth controled lamp, for
scheduling and lightning to drive other slave lamps on RF24.
(not a beerware)*/
#include <EEPROM.h>
#include <SPI.h>
#include "RF24.h"
#include <Wire.h>
#include "SparkFunHTU21D.h"
#include <LM75.h>
#include <DS1107H.h>
#include <SoftwareSerial.h>
RF24 myRadio (8, 7);     // CE et CS pins
SoftwareSerial Bl(2, 4); // RX, TX
DS1107H clock; //RTC horloge
HTU21D myHumidity; //Humidité et température extérieur
LM75 sensor(LM75_ADDRESS | 0b000); //Température lampe
byte addresses[][6] = {"1Node"};
int dataTransmitted;
const int Uv = 10;
const int Wh = 9;
const int Fan = 3;
const int Lamp135 = 5;
const int Lamp24 = 6;
unsigned int Mois = 0;
unsigned int Matin = 0;
unsigned int Soir = 0;
char junk;
String inputString = "";
void setup() {
  pinMode(Fan, OUTPUT);
  pinMode(Wh, OUTPUT);
  pinMode(Uv, OUTPUT);
  pinMode(Lamp135, OUTPUT);
  pinMode(Lamp24, OUTPUT);
  Wire.begin();
  Serial.begin(9600);
  Bl.begin(9600);
  myHumidity.begin();
  myRadio.begin();
  myRadio.setChannel(108);
  myRadio.setPALevel(RF24_PA_MAX);
  myRadio.openWritingPipe( addresses[0]);
  //Mise à l'heure et date
  /*byte second = 0;
  byte minute = 41;
  byte hour = 1;
  byte dayOfWeek = 7;
  byte dayOfMonth = 27;
  byte month = 10;
  byte year = 18;
  clock.setDate(second,minute,hour,dayOfWeek,dayOfMonth,month,year);
  */
  Mois = EEPROM.read(0);  //1-12 mois de l'année
  Matin = EEPROM.read(1);  // heure de démarrage
  Soir = EEPROM.read(2);   // heure d'extinction
  digitalWrite(Fan, LOW);
  digitalWrite(Lamp135, LOW);
  digitalWrite(Lamp24, LOW);
  digitalWrite(Uv, LOW);
  digitalWrite(Wh, LOW);
  delay(200);
}
void loop() {
  clock.getDate();
  showTime(clock);
  //Protection surchauffe
  if (sensor.temp() > 62.00) {
    Bl.println("Over 62C: Stop All!!");
    Bl.println("");
    digitalWrite(Lamp135, LOW);
    digitalWrite(Lamp24, LOW);
    digitalWrite(Uv, LOW);
    digitalWrite(Wh, LOW);
    digitalWrite(Fan, HIGH);
  }
  //Protection préchauffe
  if (sensor.temp() > 58.00) {
    Bl.println("Over 58C: Stop 2 and 4");
    Bl.println("");
    digitalWrite(Lamp24, LOW);
    digitalWrite(Uv, LOW);
    digitalWrite(Wh, LOW);
    digitalWrite(Fan, HIGH);
  }
} // programmation calendrier
void showTime(DS1107H timerChip) {
  float humd = myHumidity.readHumidity();
  float temp = myHumidity.readTemperature();
  if ((timerChip.month == Mois) && (timerChip.hour >= Matin || timerChip.hour < Soir) && (timerChip.minute == 30))  {
    Bl.println("");
    Bl.print("Matin: ");
    Bl.print("Croissance ");
    //Bl.print("Floraison ");
    digitalWrite(Fan, HIGH);
    myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
    dataTransmitted = 0;
    delay(10000);
    digitalWrite(Lamp24, HIGH);
    myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
    dataTransmitted = 6;
    delay(10000);
    digitalWrite(Lamp135, HIGH);
    myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
    dataTransmitted = 7;
    delay(10000);
    digitalWrite(Uv, LOW);  // (Croissance)
    //analogWrite(Uv, 60);  // (Floraison)
    myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
    dataTransmitted = 5;
    delay(35000);
    Bl.println("Matin");
  }
  if ((timerChip.month == Mois) && (timerChip.hour >= Soir || timerChip.hour < Matin) && (timerChip.minute == 30))  {
    Bl.println("");
    Bl.print("Soir");
    digitalWrite(Lamp135, LOW);
    myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
    dataTransmitted = 9;
    delay(15000);
    digitalWrite(Lamp24, LOW);
    delay(15000);
    digitalWrite(Uv, LOW);
    myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
    dataTransmitted = 5;
    delay(1200000);
    digitalWrite(Fan, LOW);
    myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
    dataTransmitted = 1;
    delay(100);
    Bl.println("Nuit");
  }
  //Données BlueTooth entrantes
  if (Bl.available()) {
    while (Bl.available() > 0) {
      char inChar = (char)Bl.read();
      inputString += inChar;
    }
    if (inputString.length() <= 8) {
      Bl.println(""); {
        junk = Bl.read();
      }
      if (inputString == "a") {
        Bl.println("Mois:");
        Mois = Mois + 1 ;
        Bl.print(Mois, DEC);
        Bl.println("");
        EEPROM.write(0, Mois);
      }
      if (inputString == "b") {
        Bl.println("Matin:");
        Matin = Matin + 1 ;
        Bl.print(Matin, DEC);
        Bl.print("h ");
        Bl.println("");
        EEPROM.write(1, Matin);
      }
      if (inputString == "c") {
        Bl.println("Soir:");
        Soir = Soir + 1 ;
        Bl.print(Soir, DEC);
        Bl.print("h ");
        Bl.println("");
        EEPROM.write(2, Soir);
      }
      if (inputString == "d") {
        Bl.println("programme:");
        Bl.print("mois: ");
        Bl.println(Mois, DEC);
        Bl.print("matin: ");
        Bl.print(Matin, DEC);
        Bl.println("h ");
        Bl.print(" soir : ");
        Bl.print(Soir, DEC);
        Bl.println("h ");
        Bl.println("");
      }
      if (inputString == "e") {
        Bl.println("raz programme:");
        Mois = 0;
        Matin = 0;
        Soir = 0;
        Bl.print("mois: ");
        Bl.println(Mois, DEC);
        Bl.print("matin: ");
        Bl.print(Matin, DEC);
        Bl.println("h ");
        Bl.print(" soir : ");
        Bl.print(Soir, DEC);
        Bl.println("h ");
        Bl.println("");
      }
      if (inputString == "Fanon") {
        Bl.println("Fan: ON");
        digitalWrite(Fan, HIGH);
        myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
        dataTransmitted = 0;
        delay(100);
        myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
        dataTransmitted = 0;
        delay(100);
      }
      if (inputString == "Fanoff") {
        Bl.println("Fan: OFF");
        digitalWrite(Fan, LOW);
        myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
        dataTransmitted = 1;
        delay(100);
      }
      if (inputString == "Uvon") {
        Bl.println("Uv: ON");
        analogWrite(Uv, 80);
        digitalWrite(Wh, LOW);
        digitalWrite(Fan, HIGH);
        myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
        dataTransmitted = 4;
        delay(100);
        myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
        dataTransmitted = 0;
        delay(100);
      }
      if (inputString == "Uvoff") {
        Bl.println("Uv: OFF");
        digitalWrite(Uv, LOW);
        digitalWrite(Fan, HIGH);
        myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
        dataTransmitted = 5;
        delay(100);
        myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
        dataTransmitted = 0;
        delay(100);
      }
      if (inputString == "Whon") {
        Bl.println("Wh: ON");
        analogWrite(Wh, 80);
        digitalWrite(Lamp135, LOW);
        digitalWrite(Lamp24, LOW);
        digitalWrite(Uv, LOW);
        digitalWrite(Fan, HIGH);
        myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
        dataTransmitted = 2;
        delay(100);
        myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
        dataTransmitted = 0;
        delay(100);
      }
      if (inputString == "Whoff") {
        Bl.println("Wh: OFF");
        digitalWrite(Wh, LOW);
        digitalWrite(Fan, HIGH);
        myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
        dataTransmitted = 3;
        delay(100);
        myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
        dataTransmitted = 0;
        delay(100);
      }
      if (inputString == "1") {
        Bl.println("2,4 : ON 40% ");
        digitalWrite(Lamp24, HIGH);
        digitalWrite(Fan, HIGH);
        myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
        dataTransmitted = 6;
        delay(100);
        myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
        dataTransmitted = 0;
        delay(100);
      }
      if (inputString == "2") {
        Bl.println("1,3,5 : ON 60%");
        digitalWrite(Lamp135, HIGH);
        digitalWrite(Fan, HIGH);
        myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
        dataTransmitted = 7;
        delay(100);
        myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
        dataTransmitted = 0;
        delay(100);
      }
      if (inputString == "Lampon") {
        Bl.println("Lamp : ON 100%");
        digitalWrite(Lamp135, HIGH);
        digitalWrite(Lamp24, HIGH);
        digitalWrite(Fan, HIGH);
        myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
        dataTransmitted = 8;
        delay(100);
        myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
        dataTransmitted = 0;
        delay(100);
      }
      if (inputString == "Lampoff") {
        Bl.println("Lamp: OFF");
        digitalWrite(Lamp135, LOW);
        digitalWrite(Lamp24, LOW);
        digitalWrite(Uv, LOW);
        digitalWrite(Wh, LOW);
        digitalWrite(Fan, HIGH);
        myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
        dataTransmitted = 9;
        delay(100);
        myRadio.write( &dataTransmitted, sizeof(dataTransmitted) );
        dataTransmitted = 0;
        delay(100);
      }
      if (inputString == "TempLed") {
        Bl.println("");
        Bl.print("Temperature lampe: ");
        Bl.print(sensor.temp());
        Bl.println(" C");
        Bl.print("Temperature exterieure: ");
        Bl.print(temp, 1);
        Bl.println(" C");
        Bl.print("Humidite: ");
        Bl.print(humd, 1);
        Bl.print("%");
      }
      if (inputString == "Time") {
        if (timerChip.dayOfWeek == 1 ) {
          Bl.print("lundi ");
        }
        if (timerChip.dayOfWeek == 2 ) {
          Bl.print("mardi ");
        }
        if (timerChip.dayOfWeek == 3 ) {
          Bl.print("mercredi ");
        }
        if (timerChip.dayOfWeek == 4 ) {
          Bl.print("jeudi ");
        }
        if (timerChip.dayOfWeek == 5 ) {
          Bl.print("vendredi ");
        }
        if (timerChip.dayOfWeek == 6 ) {
          Bl.print("samedi ");
        }
        if (timerChip.dayOfWeek == 7 ) {
          Bl.print("dimanche ");
        }
        Bl.print(timerChip.dayOfMonth, DEC);
        if (timerChip.month == 1 ) {
          Bl.print(" janvier ");
        }
        if (timerChip.month == 2 ) {
          Bl.print(" fevrier ");
        }
        if (timerChip.month == 3 ) {
          Bl.print(" mars ");
        }
        if (timerChip.month == 4 ) {
          Bl.print(" avril ");
        }
        if (timerChip.month == 5 ) {
          Bl.print(" mai ");
        }
        if (timerChip.month == 6 ) {
          Bl.print(" juin ");
        }
        if (timerChip.month == 7 ) {
          Bl.print(" juillet ");
        }
        if (timerChip.month == 8 ) {
          Bl.print(" aout ");
        }
        if (timerChip.month == 9 ) {
          Bl.print(" septembre ");
        }
        if (timerChip.month == 10 ) {
          Bl.print(" octobre ");
        }
        if (timerChip.month == 11 ) {
          Bl.print(" novembre ");
        }
        if (timerChip.month == 12 ) {
          Bl.print(" decembre ");
        }
        Bl.print("20");
        Bl.println(timerChip.year, DEC);
        Bl.print(timerChip.hour, DEC);
        Bl.print("h:");
        Bl.print(timerChip.minute, DEC);
        Bl.print("mn:");
        Bl.print(timerChip.second, DEC);
        Bl.print("s");
        Bl.println(" UT GMT+1");
      }
      inputString = "";
    }
    delay(100);
  }
}
