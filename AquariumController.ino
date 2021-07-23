#include <Time.h>
#include <TimeLib.h>
#include <DS3232RTC.h>
#include <sunMoon.h>
#include <Bounce2.h>


#define OUR_latitude 39.609824 // Centennial cordinates
#define OUR_longtitude -104.73716
#define OUR_timezone -420 // localtime with UTC difference in minutes

int drainTime = 60 * 2;
int fillTime = 60 * 4.25;
int fillResTime = 60 * 30;
int waterChangeDay = 7; // Sunday = 1 thru Saturday = 7

// 12v relay outs
int sunPin = A0; //K1
int moonPin = A1; //K2
int fillResValvePin = A3; //K4
int drainValvePin = A4; //K5
int fillValvePin = A5; //K6

//120v relay outs
int fillPumpPin = A6; //K7
int volcanoPin = A7; //K8

// LED relay outs
int feedLedPin = A8; //K9
int sunLedPin = A9; //K10
int moonLedPin = A10; //K11
int changeLedPin = A11; //K12
int fillLedPin = A12; //K13
int drainLedPin = A13; //K14
int fillResLedPin = A14; //K15


int feedButton = 0;
int sunButton = 22;
int moonButton = 2;
int changeButton = 3;
int fillButton = 4;
int drainButton = 5;
int fillResButton = 6;
int resFloatSensor = 7;
int tankFloatSensor = 8;

Bounce feedButtonBounce = Bounce();
Bounce fillButtonBounce = Bounce();
Bounce drainButtonBounce = Bounce();
Bounce changeButtonBounce = Bounce();
Bounce sunButtonBounce = Bounce();
Bounce moonButtonBounce = Bounce();
Bounce fillResBounce = Bounce();
Bounce resFloatBounce = Bounce();
Bounce tankFloatBounce = Bounce();

bool filling = false;
bool draining = false;
bool changing = false;
bool fillingRes = false;
bool erupting = false;

bool sunOverride = false;
bool moonOverride = false;
bool doneOnce;

time_t fillStopTime;
time_t drainStopTime;
time_t fillResStopTime;

time_t volcanoStopTime;
time_t volcanoDelay;


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
  randomSeed(analogRead(4));

  tm.Second = 0;
  tm.Minute = 12;
  tm.Hour = 12;
  tm.Day = 3;
  tm.Month = 8;
  tm.Year = 2016 - 1970;
  time_t s_date = makeTime(tm);

  Serial.begin(19200);

  pinMode(sunPin, OUTPUT); // K1
  pinMode(moonPin, OUTPUT); // K2
  pinMode(fillResValvePin, OUTPUT); //K4
  pinMode(drainValvePin, OUTPUT); //K5
  pinMode(fillValvePin, OUTPUT); //K6
  pinMode(fillPumpPin, OUTPUT); //K7
  pinMode(volcanoPin, OUTPUT); //K8

  pinMode(feedLedPin, OUTPUT); //K9
  pinMode(sunLedPin, OUTPUT); //K10
  pinMode(moonLedPin, OUTPUT); //K11
  pinMode(changeLedPin, OUTPUT); //K12
  pinMode(fillLedPin, OUTPUT); //K13
  pinMode(drainLedPin, OUTPUT); //K14
  pinMode(fillResLedPin, OUTPUT); //K15

  feedButtonBounce.attach(feedButton, INPUT_PULLUP);
  feedButtonBounce.interval(100);

  sunButtonBounce.attach(sunButton, INPUT_PULLUP);
  sunButtonBounce.interval(100);

  moonButtonBounce.attach(moonButton, INPUT_PULLUP);
  moonButtonBounce.interval(100);

  changeButtonBounce.attach(changeButton, INPUT_PULLUP);
  changeButtonBounce.interval(100);

  fillButtonBounce.attach(fillButton, INPUT_PULLUP);
  fillButtonBounce.interval(100);

  drainButtonBounce.attach(drainButton, INPUT_PULLUP);
  drainButtonBounce.interval(100);

  fillResBounce.attach(fillResButton, INPUT_PULLUP);
  fillResBounce.interval(100);

  resFloatBounce.attach(resFloatSensor, INPUT_PULLUP);
  resFloatBounce.interval(100);

  tankFloatBounce.attach(tankFloatSensor, INPUT_PULLUP);
  tankFloatBounce.interval(100);

  digitalWrite(sunPin, HIGH); // K1
  digitalWrite(moonPin, HIGH); // K2
  digitalWrite(fillResValvePin, HIGH); //K4
  digitalWrite(drainValvePin, HIGH); //K5
  digitalWrite(fillValvePin, HIGH); //K6
  digitalWrite(fillPumpPin, HIGH); //K7
  digitalWrite(volcanoPin, HIGH); //K8

  digitalWrite(feedLedPin, LOW); //K9
  digitalWrite(sunLedPin, HIGH); //K10
  digitalWrite(moonLedPin, HIGH); //K11
  digitalWrite(changeLedPin, HIGH); //K12
  digitalWrite(fillLedPin, HIGH); //K13
  digitalWrite(drainLedPin, HIGH); //K14
  digitalWrite(fillResLedPin, HIGH); //K15

  setSyncProvider(RTC.get); // the function to get the time from the RTC
  if (timeStatus() != timeSet) {
    Serial.println("Unable to sync with the RTC");
  } else {
    Serial.println("RTC has set the system time");
    sm.init(OUR_timezone, OUR_latitude, OUR_longtitude);
  }

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

  //  Serial.print("Specific date was ");
  //  printDate(s_date);
  //  Serial.println("");
  //  jDay = sm.julianDay(s_date);
  //  mDay = sm.moonDay(s_date);
  //  sRise = sm.sunRise(s_date);
  //  sSet = sm.sunSet(s_date);
  //  Serial.print("Specific date sunrise and sunset was: ");
  //  Serial.print("Julian day of specific date was ");
  //  Serial.println(jDay);
  //  Serial.print("Moon age was ");
  //  Serial.print(mDay);
  //  Serial.println("day(s)");

  delay(500);
}

void loop()
{
  feedButtonBounce.update();
  sunButtonBounce.update();
  moonButtonBounce.update();
  changeButtonBounce.update();
  fillButtonBounce.update();
  drainButtonBounce.update();
  fillResBounce.update();
  resFloatBounce.update();
  tankFloatBounce.update();

  // button presses
  if (feedButtonBounce.fell())
  {
    Serial.println("Feed button pressed");
    int feedLedState = digitalRead(feedLedPin);
    digitalWrite(feedLedPin, !feedLedState);
  }

  if (sunButtonBounce.fell())
  {
    Serial.println("Sun button pressed");
    sunOverride = !sunOverride;
    int sunPinState = digitalRead(sunPin);
    digitalWrite(sunPin, !sunPinState);
  }

  if (moonButtonBounce.fell())
  {
    Serial.println("Moon button pressed");
    moonOverride = !moonOverride;
    int moonPinState = digitalRead(moonPin);
    digitalWrite(moonPin, !moonPinState);
  }

if (changing && changeButtonBounce.fell())
  {
    cancelChange();
    return;
  }
  
  if (changeButtonBounce.fell() || changing)
  {
    change();
  }
  
if (draining && drainButtonBounce.fell())
  {
    cancelDrain();
    return;
  }

   if (filling && fillButtonBounce.fell())
  {
    cancelFill();
    return;
  }

 if (fillingRes && fillResBounce.fell())
  {
    cancelFillRes();
    return;
  }
  if (fillButtonBounce.fell() || filling)
  {
    fill();
  }
  
  if ((drainButtonBounce.fell() && !draining) || draining)
  {
    drain();
  }

  if (fillResBounce.fell() || fillingRes)
  {
    fillRes();
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
    digitalWrite(feedLedPin, LOW);
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

    if (dayOfWeek(RTC.get()) == waterChangeDay) {
      change();
      Serial.println("Performing a water change.");
    }
  }
  else
  {
    //    Serial.println("ERROR");
  }
}

void sun()
{
  digitalWrite(sunLedPin, !sunOverride);
  if (!sunOverride)
  {
    digitalWrite(sunPin, LOW);
  }
}

void noSun()
{
  digitalWrite(sunLedPin, !sunOverride);
  if (!sunOverride)
  {
    digitalWrite(sunPin, HIGH);
  }
}

void moon()
{
  digitalWrite(moonLedPin, !moonOverride);

  if (!moonOverride)
  {
    digitalWrite(moonPin, LOW);
  }
}

void noMoon()
{
  digitalWrite(moonLedPin, !moonOverride);

  if (!moonOverride)
  {
    digitalWrite(moonPin, HIGH);
  }
}

void fill()
{
  if (!filling)
  {
    filling = true;
    fillStopTime = RTC.get() + fillTime;
    Serial.println("starting to fill");
    digitalWrite(fillPumpPin, LOW);
    digitalWrite(fillValvePin, LOW);
    digitalWrite(fillLedPin, LOW);
  }

  if (!filling && tankFloatBounce.read() == HIGH || tankFloatBounce.rose())
  {
    Serial.println("could not fill because tank is full");
    filling = false;
    digitalWrite(fillPumpPin, HIGH);
    digitalWrite(fillValvePin, HIGH);
    digitalWrite(fillLedPin, HIGH);
    fillResStopTime = RTC.get();
    return;
  }

  if (filling && RTC.get() >= fillStopTime || tankFloatBounce.read() == HIGH || tankFloatBounce.rose())
  {
    filling = false;
    Serial.println("done filling");
    digitalWrite(fillPumpPin, HIGH);
    digitalWrite(fillValvePin, HIGH);
    digitalWrite(fillLedPin, HIGH);
  }
}

void fillRes() {
  if (!fillingRes)
  {


    filling = false;
    fillingRes = true;
    fillResStopTime = RTC.get() + fillResTime;
    Serial.println("starting to fill res");
    digitalWrite(fillResValvePin, LOW);
    digitalWrite(fillResLedPin, LOW);
  }

  if (!fillingRes && resFloatBounce.read() == HIGH  || resFloatBounce.rose())
  {
    Serial.println("could not fill because res tank is full");
    filling = false;
    digitalWrite(fillResValvePin, HIGH);
    digitalWrite(fillResLedPin, HIGH);
    fillResStopTime = RTC.get();
    return;
  }

  if (fillingRes && RTC.get() >= fillResStopTime || resFloatBounce.read() == HIGH || resFloatBounce.rose())
  {
    fillingRes = false;
    Serial.println("done filling res");
    digitalWrite(fillResValvePin, HIGH);
    digitalWrite(fillResLedPin, HIGH);
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
    digitalWrite(drainLedPin, LOW);
  }

  if (draining && RTC.get() >= drainStopTime)
  {
    draining = false;
    Serial.println("done draining");
    digitalWrite(drainValvePin, HIGH);
    digitalWrite(drainLedPin, HIGH);
  }
}

void cancelChange() {
  changing = false;
  digitalWrite(changeLedPin, HIGH);
  cancelDrain();
  cancelFill();
  cancelFillRes();
}

void cancelDrain() {
  draining = false;
  Serial.println("draining canceled");
  digitalWrite(drainValvePin, HIGH);
  digitalWrite(drainLedPin, HIGH);
}

void cancelFill() {
  filling = false;
  Serial.println("filling canceled");
  digitalWrite(fillValvePin, HIGH);
  digitalWrite(fillLedPin, HIGH);
}

void cancelFillRes() {
  fillingRes = false;
  Serial.println("filling res canceled");
  digitalWrite(fillResValvePin, HIGH);
  digitalWrite(fillResLedPin, HIGH);
}

void change() {
  if (!changing) {
    changing = true;
    digitalWrite(changeLedPin, LOW);
    drain();
  }

  if (changing && !draining && RTC.get() >= drainStopTime) {
    fill();
  }
  
  if (changing && !draining && !filling && RTC.get() >= drainStopTime && RTC.get() >= fillStopTime) {
    fillRes();
  }
  
  if (changing && !draining && !filling && RTC.get() >= fillStopTime && RTC.get() >= drainStopTime) {
    changing = false;
    digitalWrite(changeLedPin, HIGH);
  }
}

void volcano()
{

  if (erupting && RTC.get() >= volcanoStopTime) {
    stopErupting();
  }
  else if (second() == 0 && !doneOnce) {
    doneOnce = true;
  } else if (second() == 0 && doneOnce) {
    return;
  }
  else if (second() >= 1) {
    doneOnce = false;
    return;
  }

  if (volcanoDelay >= RTC.get() && (volcanoDelay - RTC.get()) > 90) {
    Serial.println("The sea sleeps.");
    return;

  } else if (volcanoDelay >= RTC.get() && (volcanoDelay - RTC.get()) <= 90) {
    Serial.print(volcanoDelay - RTC.get());
    Serial.println(" seconds until potential eruption.");
    return;
  }


  if (second() != 0) {
    return;
  }


  int eruptionChance = 25;
  int randomNum = random(1, 100);

  //  Serial.print(randomNum);
  //  Serial.print(" ");

  if (!erupting && randomNum <= eruptionChance) {
    //    Serial.print("  Eruption!  ");
    startErupting();
  } else {
    //    Serial.println("  The sea rumbles...  ");
  }



}

void startErupting() {
  int volcanoDuration = random(1, 60);
  digitalWrite(volcanoPin, LOW);
  erupting = true;
  //  Serial.print(" It will last for "); Serial.print(volcanoDuration); Serial.println(" seconds.");

  volcanoStopTime = RTC.get() + volcanoDuration;
}

void stopErupting() {
  digitalWrite(volcanoPin, HIGH);
  erupting = false;
  //  Serial.println("done erupting.");
  volcanoDelay = volcanoStopTime + 360;
}
