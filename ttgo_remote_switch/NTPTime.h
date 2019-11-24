#include <Arduino.h>
#include "Timestamp.h"
#include <time.h>
#include <string>

class NTPTime
{
    public:
        void begin(const char* ntp_server, const char* timezone_desc, const int sync_period_hrs = 4);
    
        bool sync_time();
        time_t current_time_t();
        void current_local_time(tm& local_time);
        std::string current_local_hhmm();
        std::string current_local_hhmmss();
    private:
        int sync_hrs;
        Timestamp sync_offset;
        time_t last_sync;
};
