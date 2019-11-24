#include "NTPTime.h"
#include <sstream>
#include <iomanip>

void NTPTime::begin(const char* ntp_server, const char* timezone_desc, const int sync_period_hrs)
{
    sync_hrs = sync_period_hrs;
    configTime(0, 0, ntp_server);
    setenv("TZ", timezone_desc, 1);
    sync_time();
}

bool NTPTime::sync_time()
{
    Timestamp retry;
    while(retry.Elapsed() < 10000)
    {
      time_t now = 0;
      time(&now);
      if (now > 1451606400)
      {
          sync_offset.Update();
          last_sync = now;
          return true;
      }
    }
    
    return false;
}

time_t NTPTime::current_time_t()
{
    if (sync_offset.Elapsed() > sync_hrs * 3600 * 1000) 
        sync_time();
    return last_sync + (sync_offset.Elapsed() / 1000);
}

void NTPTime::current_local_time(tm& local_time)
{
    time_t now = current_time_t();
    localtime_r(&now, &local_time);
}

std::string NTPTime::current_local_hhmm()
{
    using namespace std;
    tm now = {0};
    current_local_time(now);
    stringstream hhmm;
    hhmm << std::setfill('0') << std::setw(2) << now.tm_hour << ":";
    hhmm << std::setfill('0') << std::setw(2) << now.tm_min;
    return hhmm.str();
}

std::string NTPTime::current_local_hhmmss()
{
    using namespace std;
    tm now = {0};
    current_local_time(now);
    stringstream hhmmss;
    hhmmss << std::setfill('0') << std::setw(2) << now.tm_hour << ":";
    hhmmss << std::setfill('0') << std::setw(2) << now.tm_min << ":";
    hhmmss << std::setfill('0') << std::setw(2) << now.tm_sec;
    
    return hhmmss.str();
}
