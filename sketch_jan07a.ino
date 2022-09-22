#include <avr/eeprom.h>
//#include "button.h"
#include "U8glib.h"
//#include "rus5x7.h"
//#include "rus4x6.h"
#include "rus10x20.h"
//#include "cyrillic6x9.h"
//#include "rusMax20.h"
#include "digit32.h"
#include "my5x7rus.h"
//#include "my10x14rus.h"
//#include "joystik.h"
#include "microDS18B20.h"
#include "processclass.h"
#include "bitmaps.h"
#include "GyverButton.h"
#include "menu.h"



U8GLIB_ST7920_128X64_4X lcd(13, 11, 10); //дисплей
DenSolidfuelBoiler boiler(4, 3, 5); //алгоритм роботи котла
//Joystik joy(0, 1); //джойстик
//Button select_btn(A3); //кнопка вибору 

MicroDS18B20 <A0> temperature_sensor; //сенсор температури

// controller
GButton btn_left(A1);
GButton btn_top(A2);
GButton btn_bottom(A3);
GButton btn_right(A4);
GButton btn_select(A5);

const char **menu[] { menu_items, menu_items_preferences, menu_pump, menu_fan, menu_auger }; //таблиця для елементів меню
const int number_of_element[] { 4, 4, 1, 6, 7, 0 }; //кількість елементів кожного підменю
int edit_elements[7]; //таблиця для завантаження даних
int maximal_value[7];
int minimal_value[7];
unsigned int ipos, menu_level;
float temperature;

bool bitmap_flag;
bool save_flag; //сигнал для збереження
bool error_flag; //виникнення помилки
uint32_t save_timer;
bool selected_element {false};
int edit_element; //елемент для редагування (тимчасовий)
int bitmap_step;
uint32_t bitmap_timer;
uint32_t temp_timer;
Data EEMEM structAddr;

void loadData()
{

  Data data;
  eeprom_read_block((void*)&data, (const void*)&structAddr, sizeof(data));
  boiler.setDataByStructure(data);
}

void saveData()
{
  Serial.println("------------save------------");

  Data data = boiler.getDataStructure();
  eeprom_update_block((void*)&data, (const void*)&structAddr, sizeof(data));

}

void loadTable(int mLevel)
{
  switch (mLevel)
  {
    case 3:
      edit_elements[0] = (int)boiler.getPumpTemperature();
      minimal_value[0] = PUMP_MIN_TEMPERATURE;
      maximal_value[0] = PUMP_MAX_TEMPERATURE;
      break;
    case 4:
      edit_elements[0] = boiler.getFanPercentMode1();
      minimal_value[0] = 0;
      maximal_value[0] = 100;

      edit_elements[1] = boiler.getFanPercentMode2();
      minimal_value[1] = 0;
      maximal_value[1] = 100;

      edit_elements[2] = (int)boiler.getFanTemperature();
      minimal_value[2] = MIN_MODE2_TEMPERATURE;
      maximal_value[2] = MAX_TEMPERATURE;

      edit_elements[3] = boiler.getHysteresis();
      minimal_value[3] = 0;
      maximal_value[3] = 10;

      edit_elements[4] = boiler.getFanWorkTime();
      minimal_value[4] = 0;
      maximal_value[4] = 100;

      edit_elements[5] = boiler.getFanStopTime();
      minimal_value[5] = 0;
      maximal_value[5] = 100;
      break;
    case 5:
      edit_elements[0] = boiler.getAugerWorkState();
      minimal_value[0] = 0;
      maximal_value[0] = 1;

      edit_elements[1] = boiler.getAugerWorkTimeMode1();
      minimal_value[1] = 0;
      maximal_value[1] = 100;

      edit_elements[2] = boiler.getAugerStopTimeMode1();
      minimal_value[2] = 0;
      maximal_value[2] = 100;

      edit_elements[3] = boiler.getAugerWorkTimeMode2();
      minimal_value[3] = 0;
      maximal_value[3] = 100;

      edit_elements[4] = boiler.getAugerStopTimeMode2();
      minimal_value[4] = 0;
      maximal_value[4] = 100;

      edit_elements[5] = boiler.getAugerSupportWorkTime();
      minimal_value[5] = 0;
      maximal_value[5] = 100;

      edit_elements[6] = boiler.getAugerSupportStopTime();
      minimal_value[6] = 0;
      maximal_value[6] = 100;
      break;
  }
}


void errorPage() //коли відсутній датчик температури
{
  if (millis() - bitmap_timer > 700)
  {
    bitmap_timer = millis();
    bitmap_flag = !bitmap_flag;
  }
  
  if(bitmap_flag)
  {
    lcd.setFont(rus10x20);
    lcd.drawStr( 29, 24, F("ПОМИЛКА"));
  }
  
  lcd.setFont(my5x7rus);
  lcd.drawStr( 2, 40, F("Вiдсутнiй датчик"));
  lcd.drawStr( 2, 50, F("температури!"));
}

void homePage() //домашня сторінка
{
  static char workTime[4];
  static char stopTime[4];

  // збереження даних
  if (save_flag == true && millis() - save_timer > 10000)
  {
    saveData();
    save_flag = false;
  }

  // таймер для анімації іконок
  if (millis() - bitmap_timer > 700)
  {
    bitmap_timer = millis();
    bitmap_flag = !bitmap_flag;
    if (bitmap_step >= 3)
      bitmap_step = 0;
    else
      bitmap_step++;
  }

  // вивід фактичної і заданої температури
  lcd.setFont(digits32);
  if ((int)temperature < 10)
    lcd.setPrintPos(87, 34); //зміщення для одноцифрового числа
  else
    lcd.setPrintPos(70, 34); //звичай розміщення
  lcd.print((int)temperature); //вивід температури
  lcd.setFont(my5x7rus);
  lcd.drawStr( 108, 34, F("/"));
  lcd.setPrintPos(113, 34);
  lcd.print((int)boiler.getFanTemperature()); //вивід заданої користувачем межі
  lcd.drawStr(126, 28, "."); //щось типу градуса
  lcd.drawBitmapP( 107, 2, 1, 6, degree); //також градус
  
  switch (boiler.getProcessMode())
  {
    case 0:
      lcd.setFont(rus10x20);
      if (bitmap_flag)
        lcd.drawStr( 2, 58, F("Пауза"));
      break;
    case 1:
      lcd.setFont(my5x7rus);
      lcd.drawStr( 2, 58, F("Ручний режим"));
      lcd.setPrintPos(12, 8);
      lcd.print((int)boiler.getFanPercentMode1()); //вивід потужності вентилятора
      if (boiler.getAugerWorkState() > 0) //вивід налаштувань шнека
      {
        
        itoa(boiler.getAugerWorkTimeMode1(), workTime, 10);
        itoa(boiler.getAugerStopTimeMode1(), stopTime, 10);
        lcd.drawStr(12 , 28, workTime);
        lcd.drawStr(12 + strlen(workTime) * 5, 28, "/");
        lcd.drawStr(12 + strlen(workTime) * 5 + 5, 28, stopTime);
      }
      break;
    case 2:
      lcd.setFont(my5x7rus);
      if (temperature <= boiler.getFanTemperature())
      {
        lcd.setPrintPos(12, 8);
        lcd.print((int)boiler.getFanPercentMode2());
        if (boiler.getAugerWorkState() > 0)
        {
          
          itoa(boiler.getAugerWorkTimeMode2(), workTime, 10);
          itoa(boiler.getAugerStopTimeMode2(), stopTime, 10);
          lcd.drawStr(12 , 28, workTime);
          lcd.drawStr(12 + strlen(workTime) * 5, 28, "/");
          lcd.drawStr(12 + strlen(workTime) * 5 + 5, 28, stopTime);
        }
      }
      else
      {
        
        itoa(boiler.getFanWorkTime(), workTime, 10);
        itoa(boiler.getFanStopTime(), stopTime, 10);
        lcd.drawStr(12, 8, workTime);
        lcd.drawStr(12 + strlen(workTime) * 5, 8, "/");
        lcd.drawStr(12 + strlen(workTime) * 5 + 5, 8, stopTime);
        if (boiler.getAugerWorkState() > 0)
        {
          itoa(boiler.getAugerSupportWorkTime(), workTime, 10);
          itoa(boiler.getAugerSupportStopTime(), stopTime, 10);
          lcd.drawStr(12, 28, workTime);
          lcd.drawStr(12 + strlen(workTime) * 5, 28, F("/"));
          lcd.drawStr(12 + strlen(workTime) * 5 + 5, 28, stopTime);
        }
      }
      lcd.drawStr( 2, 58, "Автоматичний режим");
      break;
  }
  //налаштування насоса
  lcd.setFont(my5x7rus);
  lcd.setPrintPos(12, 18);
  lcd.print((int)boiler.getPumpTemperature()); //вивід температури для старту роботи насоса
  lcd.drawStr(24, 12, ".");
  //  u8g.setFont(my10x14rus);
  

  // відображення іконок елементів системи
  //вентилятор
  
  if (boiler.isFanWork())
    lcd.drawBitmapP( 2, 2, 1, 8, fan[bitmap_step%2]);
  else
    lcd.drawBitmapP( 2, 2, 1, 8, fan[0]);

  //насос
  if (boiler.isPumpWork())
    lcd.drawBitmapP(2, 12, 1, 8, pump[bitmap_step]);
  else
    lcd.drawBitmapP(2, 12, 1, 8, pump[0]);
    
  
  
  //шнек
  if (boiler.getAugerWorkState() > 0)
  {
    if (boiler.isAugerWork())
      lcd.drawBitmapP(2, 22, 1, 8, auger[bitmap_step]);
    else
      lcd.drawBitmapP(2, 22, 1, 8, auger[0]);
  }
  

}


void menuPage() //меню
{
  lcd.setFont(my5x7rus);
  for (int i {0}; i < number_of_element[menu_level - 1] && i < 6; ++i)
  {
    int number { ipos < 6 ? i : ((ipos - 1) - (5 - i)) };
    lcd.drawStr( 10, 10 * (i + 1), menu[menu_level - 1][number]);
    if (menu_level >= 3)
    {
      lcd.setPrintPos(100, 10 * (i + 1));
      if (number == ipos - 1 && selected_element)
        lcd.print(edit_element);
      else
        lcd.print(edit_elements[number]);
    }
  }
  if (!selected_element)
  {
    if (ipos <= 6)
      lcd.drawStr( 0, 10 * (ipos), ">");
    else
      lcd.drawStr( 0, 10 * 6, ">");
  }
  else
  {
    //draw edit bitmap
    if (ipos <= 6)
      //      lcd.drawSrt( 0, 10*(ipos), ">");
      lcd.drawBitmapP( 116, (10 * ipos) - 7, 1, 8, edit);
    else
      //      lcd.drawSrt( 0, 10*6, ">");
      lcd.drawBitmapP( 116, (10 * 6) - 7, 1, 8, edit);
  }

}

void draw(void) {
  lcd.firstPage();
  do {
    if(error_flag)
      errorPage();
    else
    {
      if (menu_level == 0)
        homePage();
      else
        menuPage();
  
      if (btn_select.isClick())
      {
        switch (menu_level)
        {
          case 0:
            menu_level = 1;
  
            break;
          case 1: //меню
            switch (ipos)
            {
              case 1:
                boiler.setProcessMode(0);
                menu_level = 0;
                ipos = 1;
                break;
              case 2:
                boiler.setProcessMode(1);
                menu_level = 0;
                ipos = 1;
                break;
              case 3:
                boiler.setProcessMode(2);
                menu_level = 0;
                ipos = 1;
                break;
              case 4:
                menu_level = 2;
                ipos = 1;
                break;
            }
  
            break;
          case 2: //налаштування
            switch (ipos)
            {
              case 1:
                menu_level = 3;
                loadTable(3);
                break;
              case 2:
                menu_level = 4;
                loadTable(4);
                break;
              case 3:
                menu_level = 5;
                loadTable(5);
                break;
            }
            ipos = 1;
            selected_element = false;
            break;
          case 3: // налаштування насоса
            if (selected_element)
              switch (ipos)
              {
                case 1:
                  boiler.setPumpTemperature(edit_element);
                  break;
              }
            if (selected_element)
            {
              edit_element = 0;
              loadTable(menu_level); save_flag = true; //збереження даних 
			  selected_element = false;
            }
            else
            {
              selected_element = true;
              edit_element = edit_elements[ipos - 1];
            }
            break;
          case 4: // налаштування вентилятора
            if (selected_element)
              switch (ipos)
              {
                case 1:
                  boiler.setFanPercentMode1(edit_element);
                  break;
                case 2:
                  boiler.setFanPercentMode2(edit_element);
                  break;
                case 3:
                  boiler.setFanTemperature(edit_element);
                  break;
                case 4:
                  boiler.setHysteresis(edit_element);
                  break;
                case 5:
                  boiler.setFanWorkTime(edit_element);
                  break;
                case 6:
                  boiler.setFanStopTime(edit_element);
                  break;
              }
            if (selected_element)
            {
              edit_element = 0;
              loadTable(menu_level);
              save_flag = true;
              selected_element = false;
  
            }
            else
            {
              selected_element = true;
              edit_element = edit_elements[ipos - 1];
            }
            break;
          case 5: //налаштування шнека
            if (selected_element)
              switch (ipos)
              {
                case 1:
                  boiler.setAugerWorkState(edit_element);
                  break;
                case 2:
                  boiler.setAugerWorkTimeMode1(edit_element);
                  break;
                case 3:
                  boiler.setAugerStopTimeMode1(edit_element);
                  break;
                case 4:
                  boiler.setAugerWorkTimeMode2(edit_element);
                  break;
                case 5:
                  boiler.setAugerStopTimeMode2(edit_element);
                  break;
                case 6:
                  boiler.setAugerSupportWorkTime(edit_element);
                  break;
                case 7:
                  boiler.setAugerSupportStopTime(edit_element);
                  break;
              }
            if (selected_element)
            {
              edit_element = 0;
              loadTable(menu_level);
              save_flag = true;
              selected_element = false;
            }
            else
            {
              selected_element = true;
              edit_element = edit_elements[ipos - 1];
            }
            break;
        }
        if (menu_level < 3)
          ipos = 1;
  
      }
  
      if (menu_level > 0)
      {
        if (selected_element)
        {
          if (btn_top.isHold())
            if (edit_element < maximal_value[ipos - 1])
              edit_element++;
  
          if (btn_bottom.isHold())
            if (edit_element > minimal_value[ipos - 1])
              edit_element--;
  
          if (btn_left.isClick())
            selected_element = false;
  
        }
        else
        {
          if (btn_top.isClick())
          {
            if (ipos == 1)
              ipos = number_of_element[menu_level - 1];
            else
              ipos --;
          }
  
          if (btn_bottom.isClick())
          {
            if (ipos == number_of_element[menu_level - 1])
              ipos = 1;
            else
              ipos ++;
          }
  
  
          if (btn_left.isClick())
          {
            if (menu_level == 1)
            {
              bitmap_timer = millis();
              if(save_flag)
                save_timer = millis();
            }
            if (menu_level > 2)
              menu_level = 2;
            else
              menu_level--;
  
            ipos = 1;
          }
        }
      }
    }
    
  } while ( lcd.nextPage() );
}

void setup()
{
  Serial.begin(9600);

  TCCR2B = 0b00000111;  // x1024 швидкість таймера 2 для зміни частоти шим-сигналу до 60Гц
  TCCR2A = 0b00000011;  // fast pwm
  if ( lcd.getMode() == U8G_MODE_R3G3B2 ) {
    lcd.setColorIndex(255);     // white
  }
  else if ( lcd.getMode() == U8G_MODE_GRAY2BIT ) {
    lcd.setColorIndex(2);         // max intensity
  }
  else if ( lcd.getMode() == U8G_MODE_BW ) {
    lcd.setColorIndex(1);         // pixel on
  }
  else if ( lcd.getMode() == U8G_MODE_HICOLOR ) {
    lcd.setHiColorByRGB(255, 255, 255);
  }
  //  pinMode(2, OUTPUT);
  Serial.begin(9600);
  loadData();
  boiler.setProcessMode(0);
  save_flag = false;
  error_flag = false;
  //relay.setLevel(HIGH);   // можно поменять уровень реле (HIGH/LOW)

  //relay.setPeriod(100);  // можно поменять период, миллисекунды
  //  relay.setPWM(10);
}



void loop()
{
  btn_left.tick();
  btn_top.tick();
  btn_bottom.tick();
  btn_right.tick();
  btn_select.tick();

//  if(joy.is_left())
//  {
//    boiler.setProcessMode(1);
//  }
//  if(joy.is_right())
//  {
//    boiler.setProcessMode(2);
//  }
//  if(joy.is_top())
//  {
//    boiler.setProcessMode(0);
//  }
 temperature = analogRead(4) / 10.0;

//  temperature_sensor.requestTemp();
//
  // if(millis() - temp_timer > 1000)
  // {
  //   temp_timer = millis();
  //   if(temperature_sensor.readTemp())
  //   {
  //     error_flag = false;
      // temperature = temperature_sensor.getTemp();
      boiler.setTemperature(temperature);
  //   }
  //   else
  //   {
  //     error_flag = true;
  //     boiler.setProcessMode(0);
  //   }
  // }
  boiler.work();
    draw();
  Serial.println(temperature);

//  Serial.println(analogRead(3));

}
