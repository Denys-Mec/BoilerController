#include "microDS18B20.h"
#include "PWMrelay.h"

#ifndef DENSOLIDFUELBOILER
#define DENSOLIDFUELBOILER

#include "dataStructure.h"

#define PUMP_MIN_TEMPERATURE 25
#define PUMP_MAX_TEMPERATURE 50

#define WORK_MODE1_TEMPERATURE 55
#define MIN_MODE2_TEMPERATURE 50
#define MAX_TEMPERATURE 90
#define CRITICAL_TEMPERATURE 95

#define HYSTERESIS_PERIOD 10 //percent
#define MINIMAL_FAN_PWM 30
#define MINIMAL_AUGER_TIME 500 //millis

class DenSolidfuelBoiler
{  
private:
// піни
  const byte m_pump_pin; //пін на насос
  const byte m_fan_pin; //пін на вертилятор
  const byte m_auger_pin; //пін на шнек
//  PWMrelay *m_fan; //реле для вентилятор на шим
  
// статус роботи елементів
  bool is_pump_work; //робота насоса
  bool is_fan_work; //робота вертилятора
  bool is_auger_work; //робота шнека
  bool m_start_work; //робота на початку не залежна від температури
  bool m_auger_state; //робота шнека під час горіння котла
  
// налаштування
  float m_fan_pwm; //шим вентилятора

  //насос
  float m_pump_temperature; //температура включення насоса
  
  //ручний режим
  unsigned int m_fan_percent_mode1; //процент нагрузки вентилятора(обороти)
  
  //автоматичний режим
  bool m_overheated_temperature; //аварійна(тобто задана максимальна користувачем) температува  
  unsigned int m_fan_percent_mode2; //процент нагрузки вентилятора(обороти)
  float m_fan_temperature; //температура вимкнення вентилятора
  float m_hysteresis; //гістерезіс на вентилятор

  //шнек
  uint32_t m_auger_timer; //тімер на шнек
  unsigned int m_auger_stop_time1;
  unsigned int m_auger_work_time1;
  unsigned int m_auger_stop_time2; //час простою
  unsigned int m_auger_work_time2; //час роботи шнека
  
  //продувка
  unsigned int m_fan_stop_time; //час простою
  unsigned int m_fan_work_time; //час роботи
  uint32_t m_purge_timer; //таймер для продувки
  
  //підтримка
  uint32_t m_auger_support_timer; //тімер на шнек
  unsigned int m_auger_support_stop_time; //час простою
  unsigned int m_auger_support_work_time; //час роботи шнека
  
  unsigned int m_mode; //режим роботи 0 - вимкнений, 1 - ручний, 2 - автоматичний
  float m_temperature; //температура котла (з термометра) 


  void pumpWork();
  void augerWork(unsigned int stopTime, unsigned int workTime); //робота шнека
  void workMode1(); //ручний режим
  void workMode2(); //автоматичний режим
  void purge(); //продувка
  void exitTheMode(); //вихід з режиму
  void pause(); // зупинка --
  

  
public:
  DenSolidfuelBoiler(byte pump_pin, byte fan_pin, byte auger_pin);
   
  bool isOverheatedTemperature() const;
  void work();
  
  void setProcessMode(const unsigned int mode);
  void setTemperature(float temp); //виставлення температури з термостата
  void setTemperature(float temp, int mode); //виставлення температура для режиму 1(ручного) або 2(автоматичного)
  void setPumpTemperature(float temp); //температура початку роботи насоса
  void setFanPercentMode1(int percent); //виставлення напруги на вентилятор
  void setFanPercentMode2(int percent); //виставлення напруги на вентилятор
  void setFanStopTime(uint32_t sec);
  void setFanWorkTime(uint32_t sec);
  void setFanTemperature(float temp); 
  void setHysteresis(int sec); 
  void setAugerStopTimeMode1(int sec); 
  void setAugerWorkTimeMode1(int sec); 
  void setAugerStopTimeMode2(int sec); 
  void setAugerWorkTimeMode2(int sec); 
  void setAugerSupportStopTime(int sec); 
  void setAugerSupportWorkTime(int sec); 
  void setAugerWorkState(bool state);
  
  int getProcessMode() const; 
  float getPumpTemperature() const; 
  int getFanPercentMode1() const; 
  int getFanPercentMode2() const; 
  float getFanTemperature() const; 
  int getHysteresis() const; 
  int getFanStopTime() const; 
  int getFanWorkTime() const; 
  int getAugerStopTimeMode1() const; 
  int getAugerWorkTimeMode1() const; 
  int getAugerStopTimeMode2() const; 
  int getAugerWorkTimeMode2() const; 
  int getAugerSupportStopTime() const; 
  int getAugerSupportWorkTime() const; 
  bool getAugerWorkState() const;
  bool isPumpWork() const; 
  bool isFanWork() const; 
  bool isAugerWork() const; 
  void setDataByStructure(const Data &data); //завантаження налаштування з структури 
  Data getDataStructure() const; // 
};





DenSolidfuelBoiler::DenSolidfuelBoiler(byte pump_pin, byte fan_pin, byte auger_pin)
    :m_pump_pin(pump_pin), m_fan_pin(fan_pin), m_auger_pin(auger_pin)
  {
     
    m_mode = 0;
    is_pump_work = false;
    is_fan_work = false;
    is_auger_work = false;
    m_start_work = false;
    m_fan_stop_time = 0; 
    m_fan_work_time = 0;
    m_purge_timer = 0;
    m_auger_timer = 0;
    m_auger_support_timer = 0;
    //
    m_hysteresis = 5;
    m_fan_temperature = 70;
    m_fan_percent_mode1 = 50;
    m_fan_percent_mode2 = 100;
    m_fan_stop_time = 2000;
    m_fan_work_time = 1000;
    m_auger_stop_time1 = 3000;
    m_auger_work_time1 = 3000;
    m_auger_stop_time2 = 3000;
    m_auger_work_time2 = 3000;
    m_auger_support_stop_time = 2000;
    m_auger_support_work_time = 500;
    m_auger_state = 1;
  //
    
    m_pump_temperature = PUMP_MIN_TEMPERATURE; //ТИМЧАСОВО, ПІЗНІШЕ ЗАГРУЗКА З ПАМ'ЯТІ
    
    pinMode(m_pump_pin, OUTPUT);
    pinMode(m_fan_pin, OUTPUT);
//    m_fan = new PWMrelay(m_fan_pin);
//    m_fan->setPeriod(120);
    pinMode(m_auger_pin, OUTPUT);

//    m_fan->setLevel(LOW);
  }

void DenSolidfuelBoiler::setAugerWorkState(bool state)
{
   m_auger_state = state;
}

bool DenSolidfuelBoiler::getAugerWorkState() const
{
  return m_auger_state;
}
void DenSolidfuelBoiler::augerWork(unsigned int stopTime, unsigned int workTime)
{

  if(m_auger_state)
  {
    if(workTime == 0)
    {
      is_auger_work = false;
      digitalWrite(m_auger_pin, LOW);
    }
    else
    {
      if(is_auger_work && millis() - m_auger_timer > workTime)
      {
        //m_auger_timer += workTime;
        m_auger_timer = millis();
        is_auger_work = false;
        digitalWrite(m_auger_pin, LOW);
      }   
      if(!is_auger_work && millis() - m_auger_timer > stopTime)
      {
        //m_auger_timer += stopTime;
        m_auger_timer = millis();
        is_auger_work = true;
        digitalWrite(m_auger_pin, HIGH);
      }
    }
      
  }
  else
   digitalWrite(m_auger_pin, LOW);
  
}

void DenSolidfuelBoiler::purge()
{
  m_fan_pwm = (((255.0-MINIMAL_FAN_PWM)/100)*m_fan_percent_mode2) + MINIMAL_FAN_PWM;
  if(m_fan_work_time==0)
    is_fan_work = false;
  else
  {
    if(is_fan_work && millis() - m_purge_timer > m_fan_work_time)
    {
      m_purge_timer = millis();
      is_fan_work = false;
    }   
    if(!is_fan_work && millis() - m_purge_timer > m_fan_stop_time)
    {
      m_purge_timer = millis();
      is_fan_work = true;
    }
  }
  
    
  if(is_fan_work)
  {
//    m_fan->setPWM((int)m_fan_pwm);
    analogWrite(m_fan_pin, (int)m_fan_pwm);
  }
  else
//    m_fan->setPWM(0);
    analogWrite(m_fan_pin, 0);
}

void DenSolidfuelBoiler::workMode1()
{
  int m_fan_pwm = (((255.0-MINIMAL_FAN_PWM)/100)*m_fan_percent_mode1) + MINIMAL_FAN_PWM;
//  m_fan->setPWM(m_fan_pwm);
  analogWrite(m_fan_pin, (int)m_fan_pwm);
  is_fan_work = true;

  augerWork(m_auger_stop_time1, m_auger_work_time1);

  
  if(m_temperature > WORK_MODE1_TEMPERATURE)
    m_mode = 2;
}

void DenSolidfuelBoiler::workMode2()
{
  
  if(m_temperature < MIN_MODE2_TEMPERATURE)
  {
    if(!m_start_work)
      exitTheMode(); //можливо краще буде просто зупинити вентилятор!!!
    else
    {
      analogWrite(m_fan_pin, 0);
      is_fan_work = false;
      augerWork(m_auger_stop_time2, m_auger_work_time2);

    }
  }
  else
  {
    if(m_temperature > MIN_MODE2_TEMPERATURE + 3)
      m_start_work = false;
    if(m_temperature >= MIN_MODE2_TEMPERATURE && m_temperature < (m_fan_temperature - m_hysteresis)) //звичайна робота
    {
      m_fan_pwm = (((255.0-MINIMAL_FAN_PWM)/100)*m_fan_percent_mode2) + MINIMAL_FAN_PWM;

      Serial.print(m_fan_pwm);//----
      Serial.print(" ");
//      m_fan->setPWM((int)m_fan_pwm);
      analogWrite(m_fan_pin, (int)m_fan_pwm);
      is_fan_work = true;

      augerWork(m_auger_stop_time2, m_auger_work_time2);
      
    }
    if(m_temperature >= (m_fan_temperature - m_hysteresis) && m_temperature < m_fan_temperature) //робота з гістерезісом (плавне зменшення оборотів)
    {
      float step {(m_hysteresis / 100)*HYSTERESIS_PERIOD};
      float hysteresis_temperature { m_fan_temperature - m_hysteresis };
      for(int i{0}; i<HYSTERESIS_PERIOD; ++i)
        if(m_temperature > (hysteresis_temperature + (step*i)) && m_temperature <= (hysteresis_temperature + (step*(i+1)))) 
        {
          unsigned int percent = 100 - (HYSTERESIS_PERIOD*i);
          float timeWork = (m_auger_work_time2/100.0)*percent;
          
          m_fan_pwm = ((((255.0-MINIMAL_FAN_PWM)/100)*m_fan_percent_mode2)/100)*percent + MINIMAL_FAN_PWM; //виставлення шиму відносно гістерезису
          Serial.print(m_fan_pwm);
          Serial.print(" ");
//          m_fan->setPWM((int)m_fan_pwm);
          analogWrite(m_fan_pin, (int)m_fan_pwm);
          is_fan_work = true;

          augerWork(m_auger_stop_time2, (int)timeWork);
       
        }
  
      
    }
    
    if(m_temperature >= m_fan_temperature && m_temperature < CRITICAL_TEMPERATURE)
    {
      purge(); //продувка
      augerWork(m_auger_support_stop_time, m_auger_support_work_time); //підтримка
    }

    if(m_temperature >= CRITICAL_TEMPERATURE)
    {
      pause();
    }
    
    
    
  }
  
  
}

void DenSolidfuelBoiler::pumpWork()
{
  if(m_temperature >= m_pump_temperature)
  { 
    digitalWrite(m_pump_pin, HIGH); 
    is_pump_work = true;
  }
  else 
  { 
    digitalWrite(m_pump_pin, LOW); 
    is_pump_work = false;
  }
  
  
}

void DenSolidfuelBoiler::setTemperature(float temp)
{
  m_temperature = temp;
}

void DenSolidfuelBoiler::work()
{
  if(m_temperature >= (m_fan_temperature+1) || m_temperature >= (MAX_TEMPERATURE+1))
    m_overheated_temperature = true;
  else
    m_overheated_temperature = false;

  pumpWork(); //робота насоса
  if(m_mode == 0 || m_mode > 2) //зупинка(пауза)
  {
    exitTheMode();
  }
  else{
    switch(m_mode)
    {
      case 1: //ручний режим
        workMode1();
        break;
      case 2: //автоматичний режим
        workMode2();
        break;
    }
//    m_fan->tick();

  }
  
}

void DenSolidfuelBoiler::setPumpTemperature(float temp)
{
  if(temp >= PUMP_MIN_TEMPERATURE && temp <= PUMP_MAX_TEMPERATURE)
    m_pump_temperature = temp;
}

void DenSolidfuelBoiler::setProcessMode(const unsigned int mode)
{
  m_mode = mode;
  if(mode == 2)
    m_start_work = true;
}

void DenSolidfuelBoiler::exitTheMode()
{
  m_mode = 0;
  pause();
}

void DenSolidfuelBoiler::pause()
{
  analogWrite(m_fan_pin, 0);
  digitalWrite(m_auger_pin, LOW);
  is_fan_work = false;
  is_auger_work = false;
}


void DenSolidfuelBoiler::setFanTemperature(float temp)
{
  m_fan_temperature = temp;
}

void DenSolidfuelBoiler::setFanPercentMode1(int percent)
{
  m_fan_percent_mode1 = percent;
}

void DenSolidfuelBoiler::setFanPercentMode2(int percent)
{
  m_fan_percent_mode2 = percent;
}

void DenSolidfuelBoiler::setHysteresis(int sec)
{
  m_hysteresis = sec;
}

void DenSolidfuelBoiler::setAugerStopTimeMode1(int sec)
{
  m_auger_stop_time1 = sec*1000;
}

void DenSolidfuelBoiler::setAugerWorkTimeMode1(int sec)
{
  m_auger_work_time1 = sec*1000;
}

void DenSolidfuelBoiler::setAugerStopTimeMode2(int sec)
{
  m_auger_stop_time2 = sec*1000;
}

void DenSolidfuelBoiler::setAugerWorkTimeMode2(int sec)
{
  m_auger_work_time2 = sec*1000;
}

void DenSolidfuelBoiler::setAugerSupportStopTime(int sec)
{
  m_auger_support_stop_time = sec*1000;
}

void DenSolidfuelBoiler::setAugerSupportWorkTime(int sec)
{
  m_auger_support_work_time = sec*1000;
}

void DenSolidfuelBoiler::setFanStopTime(uint32_t sec)
{
  m_fan_stop_time = sec * 1000;
}

void DenSolidfuelBoiler::setFanWorkTime(uint32_t sec)
{
  m_fan_work_time = sec * 1000;
}

int DenSolidfuelBoiler::getProcessMode() const
{
  return m_mode;
}

float DenSolidfuelBoiler::getPumpTemperature() const
{
  return m_pump_temperature;
}
int DenSolidfuelBoiler::getFanPercentMode1() const
{
  return m_fan_percent_mode1;
}

int DenSolidfuelBoiler::getFanPercentMode2() const
{
  return m_fan_percent_mode2;
}

float DenSolidfuelBoiler::getFanTemperature() const
{
  return m_fan_temperature;
}

int DenSolidfuelBoiler::getHysteresis() const
{
  return (int)m_hysteresis;
}
int DenSolidfuelBoiler::getFanStopTime() const
{
  return m_fan_stop_time/1000;
}

int DenSolidfuelBoiler::getFanWorkTime() const
{
  return m_fan_work_time/1000;
}

int DenSolidfuelBoiler::getAugerStopTimeMode1() const
{
  return m_auger_stop_time1/1000;
}

int DenSolidfuelBoiler::getAugerWorkTimeMode1() const
{
  return m_auger_work_time1/1000;
}

int DenSolidfuelBoiler::getAugerStopTimeMode2() const
{
  return m_auger_stop_time2/1000;
}

int DenSolidfuelBoiler::getAugerWorkTimeMode2() const
{
  return m_auger_work_time2/1000;
}
int DenSolidfuelBoiler::getAugerSupportStopTime() const
{
  return m_auger_support_stop_time/1000;
}

int DenSolidfuelBoiler::getAugerSupportWorkTime() const
{
  return m_auger_support_work_time/1000;
}

bool DenSolidfuelBoiler::isPumpWork() const
{
  return is_pump_work;
}
bool DenSolidfuelBoiler::isFanWork() const
{
  return is_fan_work;
}
bool DenSolidfuelBoiler::isAugerWork() const
{
  return is_auger_work;
}

void DenSolidfuelBoiler::setDataByStructure(const Data &data)
{
  m_pump_temperature = data.pump_temperature;
  m_fan_percent_mode1 = data.fan_percent_mode1;
  m_fan_percent_mode2 = data.fan_percent_mode2;
  m_fan_temperature = data.fan_temperature;
  m_hysteresis = data.hysteresis;
  m_fan_stop_time = data.fan_stop_time;
  m_fan_work_time = data.fan_work_time;
  m_auger_state = data.auger_state;
  m_auger_stop_time1 = data.auger_stop_time1;
  m_auger_work_time1 = data.auger_work_time1;
  m_auger_stop_time2 = data.auger_stop_time2;
  m_auger_work_time2 = data.auger_work_time2;
  m_auger_support_stop_time = data.auger_support_stop_time;
  m_auger_support_work_time = data.auger_support_work_time;
}

Data DenSolidfuelBoiler::getDataStructure() const
{
  Data data;
  data.pump_temperature = m_pump_temperature;
  data.fan_percent_mode1 = m_fan_percent_mode1;
  data.fan_percent_mode2 = m_fan_percent_mode2;
  data.fan_temperature = m_fan_temperature;
  data.hysteresis = m_hysteresis;
  data.fan_stop_time = m_fan_stop_time;
  data.fan_work_time = m_fan_work_time;
  data.auger_state = m_auger_state;
  data.auger_stop_time1 = m_auger_stop_time1;
  data.auger_work_time1 = m_auger_work_time1;
  data.auger_stop_time2 = m_auger_stop_time2;
  data.auger_work_time2 = m_auger_work_time2;
  data.auger_support_stop_time = m_auger_support_stop_time;
  data.auger_support_work_time = m_auger_support_work_time;

  return data;
}

#endif
