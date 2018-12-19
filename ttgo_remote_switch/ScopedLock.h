#include <freertos/freertos.h>
#include <freertos/semphr.h>
#include <inttypes.h>

class ScopedLock
{
public:
    ScopedLock(SemaphoreHandle_t semaphore);
    virtual ~ScopedLock();
protected:
    void lock();
    bool tryLock(uint32_t try_time);
    void unlock();

    SemaphoreHandle_t sema4;
};