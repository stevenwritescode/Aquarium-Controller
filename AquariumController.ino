#include <Time.h>
#include <TimeLib.h>
#include <DS3232RTC.h>
#include <sunMoon.h>

#define OUR_latitude 39.609824 // Centennial cordinates
#define OUR_longtitude -104.73716
#define OUR_timezone -420 // localtime with UTC difference in minutes

int sunPin = 2;
int moonPin = 3;
int fillPumpPin = 4;
int circulatorPin = 5;
int fillResValvePin = 6;
int fillValvePin = 7;
int drainValvePin = A2;
int relayEightPin = A3;
int fillButton = 8;
int drainButton = 9;
int changeButton = 10;
int sunButton = 11;
int moonButton = 12;
int fillResButton = 13;
int resFloatSensor = A0;
int tankFloatSensor = A1;

int fillTime = 60;
int fillResTime = 1600;
int drainTime = 60;

bool filling = false;
bool draining = false;
bool changing = false;
bool fillingRes = false;
bool erupting = false;

bool sunOverride = false;
bool moonOverride = false;
bool sunButtonPressed = false;
bool moonButtonPressed = false;
bool previousSunOverride = sunOverride;

time_t fillStopTime;
time_t fillResStopTime;
time_t drainStopTime;
time_t volcanoStopTime;

randomSeed(analogRead(0));

sunMoon sm;
tmElements_t tm; // specific time

void printDate(time_t date)
{
  char buff[20];
  sprintf(buff, "%2d-%02d-%4d %02d:%02d:%02d",
          day(date), month(date), year(date), hour(date), minute(date), second(date));
  Serial.print(buff);
}

void setup()
{

  tm.Second = 0;
  tm.Minute = 12;
  tm.Hour = 12;
  tm.Day = 3;
  tm.Month = 8;
  tm.Year = 2016 - 1970;
  time_t s_date = makeTime(tm);

  Serial.begin(9600);
  pinMode(sunPin, OUTPUT);
  pinMode(moonPin, OUTPUT);
  pinMode(fillPumpPin, OUTPUT);
  pinMode(circulatorPin, OUTPUT);
  pinMode(fillResValvePin, OUTPUT);
  pinMode(fillValvePin, OUTPUT);
  pinMode(drainValvePin, OUTPUT);
  pinMode(relayEightPin, OUTPUT);

  pinMode(fillButton, INPUT_PULLUP);
  pinMode(drainButton, INPUT_PULLUP);
  pinMode(changeButton, INPUT_PULLUP);
  pinMode(sunButton, INPUT_PULLUP);
  pinMode(moonButton, INPUT_PULLUP);
  pinMode(fillResButton, INPUT_PULLUP);
  pinMode(resFloatSensor, INPUT_PULLUP);
  pinMode(tankFloatSensor, INPUT_PULLUP);

  digitalWrite(sunPin, HIGH);
  digitalWrite(moonPin, HIGH);
  digitalWrite(fillPumpPin, HIGH);
  digitalWrite(circulatorPin, HIGH);
  digitalWrite(fillResValvePin, HIGH);
  digitalWrite(drainValvePin, HIGH);
  digitalWrite(fillValvePin, HIGH);
  digitalWrite(relayEightPin, HIGH);

  setSyncProvider(RTC.get); // the function to get the time from the RTC
  if (timeStatus() != timeSet)
    Serial.println("Unable to sync with the RTC");
  else
    Serial.println("RTC has set the system time");
  sm.init(OUR_timezone, OUR_latitude, OUR_longtitude);

  Serial.print("Today is ");
  printDate(RTC.get());
  Serial.println("");

  uint32_t jDay = sm.julianDay(); // Optional call
  byte mDay = sm.moonDay();
  time_t sRise = sm.sunRise();
  time_t sSet = sm.sunSet();
  Serial.print("Today is ");
  Serial.print(jDay);
  Serial.println(" Julian day");
  Serial.print("Moon age is ");
  Serial.print(mDay);
  Serial.println("day(s)");
  Serial.print("Today sunrise and sunset: ");
  printDate(sRise);
  Serial.print("; ");
  printDate(sSet);
  Serial.println("");

  Serial.print("Specific date was ");
  printDate(s_date);
  Serial.println("");
  jDay = sm.julianDay(s_date);
  mDay = sm.moonDay(s_date);
  sRise = sm.sunRise(s_date);
  sSet = sm.sunSet(s_date);
  Serial.print("Specific date sunrise and sunset was: ");
  Serial.print("Julian day of specific date was ");
  Serial.println(jDay);
  Serial.print("Moon age was ");
  Serial.print(mDay);
  Serial.println("day(s)");
}

void loop()
{

  // button presses
  if (digitalRead(changeButton) == LOW || changing == true)
  {
    change();
  }

  if (digitalRead(fillButton) == LOW || filling == true)
  {
    fill();
  }

  if (digitalRead(drainButton) == LOW || draining == true)
  {
    drain();
  }

  if (digitalRead(fillResButton) == LOW || fillingRes == true)
  {
    fillRes();
  }

  if (sunButtonPressed)
  {
  }
  else if (digitalRead(sunButton) == LOW && digitalRead(sunPin) == LOW)
  {
    sunOverride = !sunOverride;
    sunButtonPressed = true;
    digitalWrite(sunPin, HIGH);
  }
  else if (digitalRead(sunButton) == LOW && digitalRead(sunPin) == HIGH)
  {
    sunOverride = !sunOverride;
    sunButtonPressed = true;
    digitalWrite(sunPin, LOW);
  }

  if (moonButtonPressed)
  {
  }
  else if (digitalRead(moonButton) == LOW && digitalRead(moonPin) == LOW)
  {
    moonOverride = !moonOverride;
    moonButtonPressed = true;
    digitalWrite(moonPin, HIGH);
  }
  else if (digitalRead(moonButton) == LOW && digitalRead(moonPin) == HIGH)
  {
    moonOverride = !moonOverride;
    moonButtonPressed = true;
    digitalWrite(moonPin, LOW);
  }

  if (digitalRead(sunButton) == HIGH)
  {
    sunButtonPressed = false;
  }

  if (digitalRead(moonButton) == HIGH)
  {
    moonButtonPressed = false;
  }

  // daytime events
  if (RTC.get() > sm.sunRise() && RTC.get() < sm.sunSet())
  {
    sun();
    noMoon();
    volcano();
  }
  // nighttime events
  else if (RTC.get() < sm.sunRise() || RTC.get() > sm.sunSet())
  {
    moon();
    noSun();
    volcano();
  }
  // sunrise events
  else if (RTC.get() == sm.sunRise())
  {
    sunOverride = false;
    moonOverride = false;
    sun();
    moon();
    Serial.println("SUNRISE!!!!");
  }
  // sunset events
  else if (RTC.get() == sm.sunSet())
  {
    sunOverride = false;
    moonOverride = false;
    moon();
    sun();
    Serial.println("SUNSET!!!!");
  }
  else
  {
    Serial.println("ERROR");
  }
}

void sun()
{
  if (!sunOverride)
  {
    digitalWrite(sunPin, LOW);
  }
}

void noSun()
{
  if (!sunOverride)
  {
    digitalWrite(sunPin, HIGH);
  }
}

void moon()
{
  if (!moonOverride)
  {
    digitalWrite(moonPin, LOW);
  }
}

void noMoon()
{
  if (!moonOverride)
  {
    digitalWrite(moonPin, HIGH);
  }
}

void fill()
{
  if (!filling && digitalRead(tankFloatSensor) == LOW)
  {
    filling = true;
    fillStopTime = RTC.get() + fillTime;
    Serial.println("starting to fill");
    digitalWrite(fillPumpPin, LOW);
    digitalWrite(fillValvePin, LOW);
  }

  if (!filling && digitalRead(tankFloatSensor) == HIGH)
  {
    Serial.println("could not fill because tank is full");
    filling = false;
    return;
  }

  Serial.println("filling");

  if (filling && RTC.get() >= fillStopTime || digitalRead(tankFloatSensor) == HIGH)
  {
    filling = false;
    Serial.println("done filling");
    digitalWrite(fillPumpPin, HIGH);
    digitalWrite(fillValvePin, HIGH);
  }
}

void fillRes()
{

  if (!fillingRes && digitalRead(resFloatSensor) == LOW)
  {
    fillingRes = true;
    fillResStopTime = RTC.get() + fillResTime;
    Serial.println("starting to fill res");
    digitalWrite(fillResValvePin, LOW);
  }

  if (!fillingRes && digitalRead(resFloatSensor) == HIGH)
  {
    Serial.println("could not fill because res is full");
    fillingRes = false;
    return;
  }

  Serial.println("filling res");
  if (fillingRes && RTC.get() >= fillResStopTime || digitalRead(resFloatSensor) == HIGH)
  {
    fillingRes = false;
    Serial.println("done filling res");
    digitalWrite(fillResValvePin, HIGH);
  }
}

void drain()
{

  if (!draining)
  {
    draining = true;
    drainStopTime = RTC.get() + drainTime;
    Serial.println("starting to drain");
    digitalWrite(drainValvePin, LOW);
  }

  Serial.println("draining");

  if (draining && RTC.get() >= drainStopTime)
  {
    draining = false;
    Serial.println("done draining");
    digitalWrite(drainValvePin, HIGH);
  }
}

void change()
{

  if (!changing)
  {
    changing = true;
    drain();
  }

  if (changing && !draining && RTC.get() >= drainStopTime)
  {
    fill();
  }

  if (changing && !draining && !filling && RTC.get() >= fillStopTime && RTC.get() >= drainStopTime)
  {
    changing = false;
  }
}

void volcano()
{
  int mildEruptionChance = 25;
  int badEruptionChance = 5;
  int randomNum = random(1, 100);
  int duration = random(1, 300);

  if (!erupting && RTC.get() % 30 && randomNum <= mildEruptionChance)
  {
    digitalWrite(circulatorPin, LOW);
    erupting = true;
    volcanoStopTime = RTC.get() + duration);
  }

  else if (!erupting && RTC.get() % 30 && randomNum >= (100 - badEruptionChance))
  {
    digitalWrite(circulatorPin, LOW);
    erupting = true;
    previousSunOverride = sunOverride;
    sunOverride = true;
    noSun();
    volcanoStopTime = RTC.get() + duration);
  }

  else if (erupting && RTC.get() >= volcanoStopTime)
  {
    digitalWrite(circulatorPin, HIGH);
    erupting = false;
    sunOverride = previousSunOverride;
  }
}
