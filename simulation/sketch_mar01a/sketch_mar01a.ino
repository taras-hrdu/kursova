#include <Arduino.h>
#include <LiquidCrystal.h>

#define C0 37
#define C1 36
#define C2 35
#define C3 34
#define C4 33
#define C5 32 
#define C6 31
#define C7 30

#define RS 5
#define RW 6
#define E 7

#define BUZZER 11 // signal

#define SHORT_SIGNAL_DELAY 50
#define LONG_SIGNAL_DELAY 500
#define INTERMEDIATE_VALUES 60

LiquidCrystal lcd(RS, RW, E, C0, C1, C2, C3, C4, C5, C6, C7);

bool isSignalBeep = false;
bool stop = false;


struct Time
{
  uint8_t hour, minute, second;
};
Time time = {0, 0, 0};

Time memoryRegister[INTERMEDIATE_VALUES];

uint8_t sizeOfMemoryRegister = 0;

inline void stopwatch(const Time &time);
ISR(TIMER4_COMPA_vect)
{
  if (!stop)
  {
    if (++time.second == 60)
    {
      time.second = 0;
      if (++time.minute == 60)
      {
        time.minute = 0;
        if (++time.hour == 24)
          time.hour = 0;
      }
    }

    if (time.second == 0)
    {
      isSignalBeep = true;
    }
  }
}

void sendTime(){
  Serial.print(time.hour);
  Serial.print(":");
  Serial.print(time.minute);
  Serial.print(":");
  Serial.print(time.second);
  }



inline void buttonA();
inline void buttonB(uint8_t &recentTimeSavedId);
inline void buttonC();
inline void buttonD();

bool isApply(String data);

inline void shortSignal();
inline void longSignal();
inline void doubleShortSignal();

inline bool checkId(const uint8_t &index);
inline void resetMemoryRegister();
inline void saveToMemoryRegister();

void setup()
{
  Serial.begin(9600);
  noInterrupts();

  pinMode(BUZZER, OUTPUT);
  lcd.begin(16, 2);

//T4 settings of timer, частота
  TCCR4A = WGM10;
  TCCR4B = (1 << WGM12) | (1 << CS12) | (1 << CS10);
  TIMSK4 = (1 << OCIE4A);                            
  OCR4A = 0x3D08;   
                                   

  interrupts(); 
}

void loop()
{
    char data = Serial.read();
    if(data != 'a'){
      stopwatch(time);
      }
  
  if(data == 'a'){
    shortSignal;
    buttonA();
}

  if(data == 'c'){
    shortSignal;
    buttonC();}

  if(data == 'd'){
    shortSignal;
    buttonD();}

  if(data == '*'){
    shortSignal;
    resetMemoryRegister();}
  
  if(data == '#'){
    shortSignal;
    saveToMemoryRegister();
    delay(200);
    sendTime();
    }
  
  if (isSignalBeep)
  {
    longSignal();
    isSignalBeep = false;
  }
    
}



inline void resetMemoryRegister()
{
  sizeOfMemoryRegister = 0;
  doubleShortSignal();
}



inline void saveToMemoryRegister()
{
  if ((sizeOfMemoryRegister + 1) <= INTERMEDIATE_VALUES)
  {
    memoryRegister[++sizeOfMemoryRegister - 1] = time;
    doubleShortSignal();
  }
  else
  {
    longSignal();
  }
}

inline void buttonA()
{
  lcd.clear();
  if (sizeOfMemoryRegister > 0)
  {
    uint8_t input[2];
    uint8_t inputSize = 0;
    uint8_t recentTimeSavedId = 0;
    lcd.setCursor(0, 0);
    lcd.print("0");
    lcd.print(recentTimeSavedId + 1);
    lcd.setCursor(3, 0);
    stopwatch(memoryRegister[recentTimeSavedId]);
    lcd.setCursor(13, 0);
    lcd.print("M");
    if (sizeOfMemoryRegister < 10)
    {
      lcd.print("0");
    }
    lcd.print(sizeOfMemoryRegister);
    lcd.setCursor(5, 1);
    lcd.print("<-A Y-# B->");
    lcd.setCursor(0, 1);

    
    while (true)
    {
        char currentButton = Serial.read();  
      
        if (currentButton == 'a')
        {
          shortSignal;
          lcd.clear();
          break;
        }
        else if (currentButton == 'b')
        {
          shortSignal;
          buttonB(recentTimeSavedId);
        }
        else if (currentButton == '#')
        {
          shortSignal;
          if (inputSize == 0)
          {
            lcd.setCursor(0, 1);
            lcd.print("BAN");
            longSignal();
            lcd.setCursor(0, 1);
            lcd.print("   ");
          }
          else if (checkId(inputSize == 2 ? (input[0] * 10 + input[1] - 1) : input[0] - 1)) // переводжу символи в цілі числа
          {
            recentTimeSavedId = inputSize == 2 ? (input[0] * 10 + input[1] - 1) : input[0] - 1;
            lcd.setCursor(0, 0);
            if (recentTimeSavedId < 10)
            {
              lcd.print("0");
            }
            lcd.print(recentTimeSavedId + 1);
            lcd.setCursor(3, 0);
            stopwatch(memoryRegister[recentTimeSavedId]);
            inputSize = 0;

            lcd.setCursor(0, 1);
            lcd.print("DONE");
            doubleShortSignal();
            delay(200);
            lcd.setCursor(0, 1);
            lcd.print("    ");
          }
          else
          {
            lcd.setCursor(0, 1);
            lcd.print("BAN");
            longSignal();
            lcd.setCursor(0, 1);
            lcd.print("   ");
            inputSize = 0;
          }
        }
        else if (int(currentButton) <= int('9') && int(currentButton) >= int('0'))
        {
          shortSignal;
          if (inputSize < 2)
          {
            lcd.setCursor(inputSize, 1);
            lcd.print(currentButton);
            inputSize++;
            input[inputSize - 1] = currentButton - '0';
          }
          else
          {
            lcd.setCursor(0, 1);
            lcd.print("BAN");
            longSignal();
            lcd.setCursor(0, 1);
            lcd.print("   ");
            inputSize = 0;
          }
        }
      
    }
  }
  else
  {
    lcd.print("Register's empty");

    lcd.setCursor(10, 1);
    lcd.print("Back-*");

    while (true)
    {
      char data = Serial.read();
      if (data == '*')
      {
        lcd.clear();
        break;
      }
      
    }
    
  }
}

inline void buttonB(uint8_t &recentTimeSavedId)
{
  if (checkId(recentTimeSavedId + 1))
  {
    recentTimeSavedId++;
  }
  lcd.setCursor(0, 0);
  if (recentTimeSavedId < 10)
  {
    lcd.print("0");
  }
  lcd.print(recentTimeSavedId + 1);
  stopwatch(memoryRegister[recentTimeSavedId]);
}

inline void buttonC()
{
  time = {0, 0, 0};
  stop = true;
}

inline void buttonD()
{
  stop = !stop;
}

inline bool checkId(const uint8_t &index)
{
  return index >= 0 && index < sizeOfMemoryRegister;
}

bool isApply(String data)
{
  lcd.setCursor(11, 1);
  lcd.print("Yes-#");

  while (true)
  {
    if (data == "pin_#")
    {
      return true;
    }
    delay(50);
  }
}

inline void shortSignal()
{
  digitalWrite(BUZZER, HIGH);
  delay(SHORT_SIGNAL_DELAY);
  digitalWrite(BUZZER, LOW);
}

inline void doubleShortSignal()
{
  digitalWrite(BUZZER, HIGH);
  delay(SHORT_SIGNAL_DELAY);
  digitalWrite(BUZZER, LOW);
  delay(SHORT_SIGNAL_DELAY / 2);
  digitalWrite(BUZZER, HIGH);
  delay(SHORT_SIGNAL_DELAY);
  digitalWrite(BUZZER, LOW);
}

inline void longSignal()
{
  digitalWrite(BUZZER, HIGH);
  delay(LONG_SIGNAL_DELAY);
  digitalWrite(BUZZER, LOW);
}

inline void stopwatch(const Time &time)
{
  lcd.setCursor(3, 0);
  if (time.hour < 10)
    lcd.print("0");
  lcd.print(time.hour);
  lcd.print(":");
  if (time.minute < 10)
    lcd.print("0");
  lcd.print(time.minute);
  lcd.print(":");
  if (time.second < 10)
    lcd.print("0");
  lcd.print(time.second);
}
