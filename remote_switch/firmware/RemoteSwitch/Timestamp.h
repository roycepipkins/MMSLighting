#ifndef TIMESTAMP
#define TIMESTAMP
#include <inttypes.h>

class Timestamp
{
public:
  Timestamp();
  Timestamp(uint32_t time_in);
  uint32_t GetTime() {return time_stamp;}
  uint32_t Elapsed();
  void Update();
private:
  uint32_t time_stamp;
};
#endif
