#ifndef ARDGPIO
#define ARDGPIO
#include <inttypes.h>
class ArdGPIO
{
public:
	virtual void digitalWrite( uint32_t dwPin, uint32_t dwVal ) {}
	virtual int digitalRead( uint32_t ulPin ) {}
	virtual void pullupMode(uint8_t, uint8_t) {}
	virtual void pinMode( uint32_t dwPin, uint32_t dwMode ) {}
	virtual void analogWrite( uint32_t ulPin, uint32_t ulValue ) {}
	virtual uint32_t analogRead( uint32_t ulPin ) {}
	virtual void analogReadResolution(int res) {}
	virtual void analogWriteResolution(int res) {}
	virtual void analogOutputInit( void ) {}
};
#endif
