#include "ScopedLock.h"


ScopedLock::ScopedLock(SemaphoreHandle_t semaphore):
sema4(semaphore)
{
    lock();
}

ScopedLock::~ScopedLock()
{
    unlock();
}

void ScopedLock::lock()
{
    xSemaphoreTake(sema4, portMAX_DELAY);
}

bool ScopedLock::tryLock(uint32_t try_time)
{
    xSemaphoreTake(sema4, try_time / portTICK_PERIOD_MS );
}

void ScopedLock::unlock()
{
    xSemaphoreGive(sema4);
}