#ifndef JOYSTIKH
#define JOYSTIKH
#define DEB_TIME 400
#define HOLD_TIME 500
class Joystik
{
private:
  const byte m_pin_x;
  const byte m_pin_y;
  uint32_t m_tmr;
  bool m_flag;
  uint32_t m_hold_timer;
  int m_counter;
public:
  Joystik(byte pin_x, byte pin_y);
  bool is_left();
  bool is_right();
  bool is_top(); 
  bool is_topHold();
  bool is_bottom();
  bool is_bottomHold();
};

Joystik::Joystik(byte pin_x, byte pin_y)
  :m_pin_x(pin_x), m_pin_y(pin_y)
{
  m_flag = false;
}
bool Joystik::is_left()
{
  int state {analogRead(m_pin_x)};
  Serial.println("millis:");
  Serial.println(millis());
  Serial.println("Timer:");
  Serial.println(m_tmr);
  if((state >= 0 && state <=5) && millis() - m_tmr > DEB_TIME)
  {
    m_tmr = millis();
    return true;
  }
  return false;
}

bool Joystik::is_right()
{
  Serial.println("millis:");
  Serial.println(millis());
  Serial.println("Timer:");
  Serial.println(m_tmr);
  int state {analogRead(m_pin_x)};
  if(state >= 1000 && millis() - m_tmr > DEB_TIME)
  {
    m_tmr = millis();
    return true;
  }
  return false;
}

bool Joystik::is_top()
{
  Serial.println("millis:");
  Serial.println(millis());
  Serial.println("Timer:");
  Serial.println(m_tmr);
  int state {analogRead(m_pin_y)};
  if((state >= 0 && state <=5) && millis() - m_tmr > DEB_TIME)
  {
    m_tmr = millis();
    m_counter++;
    if(m_counter == 4)
      m_flag = true;
    else
      m_flag = false;
    return true;
  }
  return false;
}

bool Joystik::is_topHold()
{
  int state {analogRead(m_pin_y)};
  int tm = DEB_TIME;
  
  if(millis() - m_hold_timer > 500)
  {
    m_counter = 0;
  } 
  if(m_counter >= 10)
  {
    
    m_counter = 10;
    tm = 1;
  }
  
  if((state >= 0 && state <=5) && millis() - m_tmr > tm)
  {
    m_tmr = millis();
    m_hold_timer = millis();
    m_counter++;
    return true;
  }
  return false;
}
bool Joystik::is_bottom()
{
  Serial.println("millis:");
  Serial.println(millis());
  Serial.println("Timer:");
  Serial.println(m_tmr);
  int state {analogRead(m_pin_y)};
  if(state >= 1000 && millis() - m_tmr > DEB_TIME)
  {
    m_tmr = millis();
    return true;
  }
  return false;
}

bool Joystik::is_bottomHold()
{
  int state {analogRead(m_pin_y)};
  int tm = DEB_TIME;
  
  if(millis() - m_hold_timer > 500)
  {
    m_counter = 0;
  } 
  if(m_counter >= 10)
  {
    
    m_counter = 10;
    tm = 1;
  }
  
  if(state >= 1000 && millis() - m_tmr > tm)
  {
    m_hold_timer = millis();
    m_tmr = millis();
    m_counter++;
    return true;
  }
  return false;
}
#endif
