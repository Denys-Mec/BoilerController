#ifndef DATASTRUCTURE
#define DATASTRUCTURE

struct Data
{
  float pump_temperature;
  unsigned int fan_percent_mode1;
  unsigned int fan_percent_mode2;
  float fan_temperature;
  float hysteresis;
  unsigned int fan_stop_time;
  unsigned int fan_work_time;
  bool auger_state;
  unsigned int auger_stop_time1;
  unsigned int auger_work_time1;
  unsigned int auger_stop_time2;
  unsigned int auger_work_time2;
  unsigned int auger_support_stop_time;
  unsigned int auger_support_work_time;
};

#endif
