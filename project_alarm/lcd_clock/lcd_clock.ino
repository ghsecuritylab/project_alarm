#include <LiquidCrystal.h>
#include <LCDKeypad.h>
#include <Wire.h>

//DS3231 address bus:
#define DS3231_I2C_ADDRESS 0x68
#define DS3231_TEMPERATURE_ADDR 0x11

//Date:
#define YEAR 0
#define MONTH 1
#define DAYS 2
#define WEEKDAY 3

//weekday state:
#define MIN 0
#define SEN 1
#define SEL 2
#define RAB 3
#define KAM 4
#define JUM 5
#define SAB 6

//Time:
#define HOURS 0
#define MINUTES 1
#define SECONDS 2

//menu list:
#define IDLEMENU 0
#define SETDATE 1
#define SETTIME 2
#define SET_ALARM 3
#define SHOW 4
#define DONE 5

// The LCD screen
LCDKeypad lcd;

// The time model
byte year = 0;
byte month = 1;
byte days = 1;
byte hours = 0;
byte minutes = 0;
byte seconds = 0;
byte weekday = 0;
byte datesetting = 0;
byte timesetting = 0;
byte menustate = 0;
byte al_hour = 0;
byte al_min = 0;
byte al_sec = 0;
byte pp = 2;
String stat_alarm = "OFF";
bool act_alarm = false;

int buzzerPin = 22;


void setup() {
  //power up ZS-042
  pinMode(53, OUTPUT);
  pinMode (buzzerPin, OUTPUT);
  digitalWrite(53, HIGH);
  //initialize I2C and lcd:
  lcd.begin(16, 2);
  Wire.begin();
  /*lcd.setCursor(0, 0);

    // Print a text in the first row
    lcd.print("PRESS SELECT    ");
    lcd.setCursor(0, 1);
    lcd.print("TO SET DATETIME ");*/
  //setDS3231time(30,06,17,3,11,5,16);
  //delay(1000);
  readDS3231time(&seconds, &minutes, &hours, &weekday, &days, &month, &year);
}

void loop() {
  // Increase the time model by one second
  incTime();
  //readDS3231time(&seconds, &minutes, &hours, &weekday, &days, &month, &year);
  // Print the time on the LCD
  printTime();
  // Listen for buttons for 1 second
  buttonListen();

}

void buttonListen() {
  // Read the buttons five times in a second
  for (int i = 0; i < 5; i++) {

    // Read the buttons value
    int button = lcd.button();
    switch (menustate) {
      case SETDATE:
        switch (button) {
          // Right button was pushed
          case KEYPAD_RIGHT:
            datesetting++;
            break;

          // Left button was pushed
          case KEYPAD_LEFT:
            datesetting--;
            if (datesetting == -1) datesetting = 3;
            break;

          // Up button was pushed
          case KEYPAD_UP:
            switch (datesetting) {
              case YEAR:
                year++;
                break;
              case MONTH:
                month++;
                break;
              case DAYS:
                days++;
                break;
              case WEEKDAY:
                weekday++;
            }
            break;

          // Down button was pushed
          case KEYPAD_DOWN:
            switch (datesetting) {
              case YEAR:
                year--;
                if (year == -1) year = 99;
                break;
              case MONTH:
                month--;
                if (month == 0) month = 12;
                break;
              case DAYS:
                days--;
                if (days == 0) days = 31;
                break;
              case WEEKDAY:
                weekday--;
                if (weekday == 0) days = 6;
            }
        }
        break;
      case SETTIME:
        switch (button) {
          // Right button was pushed
          case KEYPAD_RIGHT:
            timesetting++;
            break;

          // Left button was pushed
          case KEYPAD_LEFT:
            timesetting--;
            if (timesetting == -1) timesetting = 2;
            break;

          // Up button was pushed
          case KEYPAD_UP:
            switch (timesetting) {
              case HOURS:
                hours++;
                break;
              case MINUTES:
                minutes++;
                break;
              case SECONDS:
                seconds++;
            }
            break;

          // Down button was pushed
          case KEYPAD_DOWN:
            switch (timesetting) {
              case HOURS:
                hours--;
                if (hours == -1) hours = 23;
                break;
              case MINUTES:
                minutes--;
                if (minutes == -1) minutes = 59;
                break;
              case SECONDS:
                seconds--;
                if (seconds == -1) seconds = 59;
            }
        }
        break;

      case SET_ALARM:
        switch (button) {
          case KEYPAD_SELECT:
            setDS3231time(seconds, minutes, hours, weekday, days, month, year);
            stat_alarm = "ON";
            act_alarm = true;
            break;

          // Right button was pushed
          case KEYPAD_RIGHT:
            timesetting++;
            break;

          // Left button was pushed
          case KEYPAD_LEFT:
            timesetting--;
            if (timesetting == -1) timesetting = 1;
            break;

          // Up button was pushed
          case KEYPAD_UP:
            switch (timesetting) {
              case HOURS:
                al_hour++;
                break;
              case MINUTES:
                al_min++;
                break;

            }
            break;

          // Down button was pushed
          case KEYPAD_DOWN:
            switch (timesetting) {
              case HOURS:
                al_hour--;
                if (al_hour == -1) al_hour = 23;
                break;
              case MINUTES:
                al_min--;
                if (al_min == -1) al_min = 59;
            }
        }
        break;
      case DONE:
        // DS3231 seconds, minutes, hours, day, date, month, year
        String stat = "OKE";
        /*if (button == KEYPAD_SELECT) {
          setDS3231time(seconds, minutes, hours, weekday, days, month, year);
          stat_alarm = "ON";
          }*/
    }
    if (button == KEYPAD_SELECT) {
      menustate++;
    }
    datesetting %= 4;
    timesetting %= 3;
    menustate %= 5;
    printSetting();

    year %= 100;
    month %= 13;
    days %= 32;
    hours %= 24;
    al_hour %= 24;
    al_min %= 60;
    minutes %= 60;
    seconds %= 60;
    weekday %= 7;
    printTime();

    // Wait one fifth of a second to complete
    while (millis() % 200 != 0);
  }
}

// Print the current setting
void printSetting() {
  int button = lcd.button();
  int tempC = DS3231_get_treg();
  char time[17];
  switch (menustate) {
    case SETDATE:
      lcd.setCursor(0, 0);
      lcd.print("SETTING:        ");
      lcd.setCursor(9, 0);
      switch (datesetting) {
        case YEAR:
          lcd.print("YEAR   ");
          break;
        case MONTH:
          lcd.print("MONTH  ");
          break;
        case DAYS:
          lcd.print("Days   ");
          break;
        case WEEKDAY:
          lcd.print("WeekDay");
      }
      break;
    case SETTIME:
      lcd.setCursor(0, 0);
      lcd.print("SETTING:        ");
      lcd.setCursor(9, 0);
      switch (timesetting) {
        case HOURS:
          lcd.print("Hours  ");
          break;
        case MINUTES:
          lcd.print("Minutes");
          break;
        case SECONDS:
          lcd.print("Seconds");
      }
      break;
    case SET_ALARM:
      lcd.setCursor(0, 0);
      lcd.print("SET ALRM:        ");
      lcd.setCursor(9, 0);
      switch (timesetting) {
        case HOURS:
          lcd.print("Hours  ");
          break;
        case MINUTES:
          lcd.print("Minutes");
      }
      break;

    case IDLEMENU:
      if (hours == al_hour && minutes == al_min && act_alarm == true) {
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print("ALARM IS ACTIVE");
        bunyi();
        if (button == KEYPAD_DOWN) {
          stat_alarm = "OFF";
          act_alarm = false;
          digitalWrite (buzzerPin, LOW);
          break;
        } else {
          postpone();
        }
      } else {

        lcd.clear();
        lcd.setCursor(0, 0);
        sprintf(time, "%02i:%02i:%02i        ", hours, minutes, seconds);
        lcd.print(time);
        lcd.setCursor(12, 0);
        lcd.print(tempC);
        lcd.print((char)223);
        lcd.print("C ");
        lcd.setCursor(13, 1);
        lcd.print(stat_alarm);
        lcd.setCursor(0, 1);
        sprintf(time, "%02i/%02i/%02i", year, month, days);
        lcd.print(time);
      }

      break;
    case DONE:
      lcd.setCursor(0, 0);
      lcd.print("DONE            ");
      lcd.setCursor(0, 1);
      lcd.print("................");
  }
}

// Increase the time model by one second
void incTime() {
  // Increase seconds
  seconds++;

  if (seconds == 60) {
    // Reset seconds
    seconds = 0;

    // Increase minutes
    minutes++;

    if (minutes == 60) {
      // Reset minutes
      minutes = 0;

      // Increase hours
      hours++;

      if (hours == 24) {
        // Reset hours
        hours = 0;

        // Increase days
        days++;
        weekday++;
      }
    }
  }
}

// Print the time on the LCD
void printTime() {
  char time[17];
  //check which state right now:
  switch (menustate) {
    case SETDATE:
      lcd.setCursor(0, 1);
      switch (weekday) {
        case SUN:
          sprintf(time, "%02i/%02i/%02i     MIN", year, month, days);
          break;
        case MON:
          sprintf(time, "%02i/%02i/%02i     SEN", year, month, days);
          break;
        case TUE:
          sprintf(time, "%02i/%02i/%02i     SEL", year, month, days);
          break;
        case WED:
          sprintf(time, "%02i/%02i/%02i     RAB", year, month, days);
          break;
        case THU:
          sprintf(time, "%02i/%02i/%02i     KAM", year, month, days);
          break;
        case FRI:
          sprintf(time, "%02i/%02i/%02i     JUM", year, month, days);
          break;
        case SAT:
          sprintf(time, "%02i/%02i/%02i     SAB", year, month, days);
      }
      lcd.print(time);
      break;
    case SETTIME:
      lcd.setCursor(0, 1);
      sprintf(time, "%02i:%02i:%02i        ", hours, minutes, seconds);
      lcd.print(time);
      break;
    case SET_ALARM:
      lcd.setCursor(0, 1);
      sprintf(time, "%02i:%02i:%02i        ", al_hour, al_min, al_sec);
      lcd.print(time);
      break;

  }
  // Set the cursor at the begining of the second row

}
// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return ( (val / 10 * 16) + (val % 10) );
}
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return ( (val / 16 * 10) + (val % 16) );
}

void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year)
{
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (0=Sunday, 6=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.endTransmission();
}
void readDS3231time(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year)
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}

float DS3231_get_treg()
{
  int rv;  // Reads the temperature as an int, to save memory
  //  float rv;

  uint8_t temp_msb, temp_lsb;
  int8_t nint;

  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(DS3231_TEMPERATURE_ADDR);
  Wire.endTransmission();

  Wire.requestFrom(DS3231_I2C_ADDRESS, 2);
  temp_msb = Wire.read();
  temp_lsb = Wire.read() >> 6;

  if ((temp_msb & 0x80) != 0)
    nint = temp_msb | ~((1 << 8) - 1);      // if negative get two's complement
  else
    nint = temp_msb;

  rv = 0.25 * temp_lsb + nint;

  return rv;
}

void bunyi() {
  digitalWrite (buzzerPin, HIGH);
  delay (100);
  digitalWrite (buzzerPin, LOW);
  delay (1000);
}

byte postpone() {
  if (hours == al_hour && minutes - 1 == al_min && act_alarm == true) {
    al_min = al_min + pp;
    return postpone;
  }
} //untuk batalin alarm

