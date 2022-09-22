#ifndef MENU
#define MENU

//const char item11[] PROGMEM = "Ручний режим";
//const char item12[] PROGMEM = "Автоматичний режим";
//const char item13[] PROGMEM = "Налаштування";

const char* menu_items[] 
{
  "Зупинка",
  "Ручний режим",
  "Автоматичний режим",
  "Налаштування"
};

//const char item21[] PROGMEM = "Насос";
//const char item22[] PROGMEM = "Вентилятор";
//const char item23[] PROGMEM = "Шнек";
//const char item24[] PROGMEM = "Димосос";

const char* menu_items_preferences[] 
{
  "Насос",
  "Вентилятор",
  "Шнек",
  "Димосос"
};

//const char item31[] PROGMEM = "Стартова темп.";

const char* menu_pump[]
{
  "Стартова темп."
};

//const char item41[] PROGMEM = "Потужність (P)";
//const char item42[] PROGMEM = "Потужність (A)";
//const char item43[] PROGMEM = "Робоча темп. (A)";
//const char item44[] PROGMEM = "Гістерезіс (A)";
//const char item45[] PROGMEM = "Час роботи (П)";
//const char item46[] PROGMEM = "Час простою (П)";

const char* menu_fan[]
{
  "Потужнiсть (P)",
  "Потужнiсть (A)",
  "Робоча темп. (A)",
  "Гiстерезiс (A)",
  "Час роботи (П)",
  "Час простою (П)"
};

//const char item51[] PROGMEM = "Стан";
//const char item52[] PROGMEM = "Час роботи (P)";
//const char item53[] PROGMEM = "Час простою (P)";
//const char item54[] PROGMEM = "Час роботи (A)";
//const char item55[] PROGMEM = "Час простою (A)";
//const char item56[] PROGMEM = "Час роботи (П)";
//const char item57[] PROGMEM = "Час простою (П)";

const char* menu_auger[] 
{
  "Стан",
  "Час роботи (P)",
  "Час простою (P)",
  "Час роботи (A)",
  "Час простою (A)",
  "Час роботи (П)",
  "Час простою (П)"
};

#endif
