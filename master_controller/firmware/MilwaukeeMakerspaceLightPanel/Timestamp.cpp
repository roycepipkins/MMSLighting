#include "Timestamp.h"
#include <Arduino.h>

Timestamp::Timestamp():
time_stamp(millis())
{

}

Timestamp::Timestamp(uint32_t time_in):
time_stamp(time_in)
{

}

uint32_t Timestamp::Elapsed()
{
  uint32_t c_time = millis();
  if (c_time >= time_stamp)
  {
    return (c_time - time_stamp);
  }
  else
  {
    return (~((uint32_t)(0)) - time_stamp + c_time);
  }
}

void Timestamp::Update()
{
  time_stamp = millis();
}
