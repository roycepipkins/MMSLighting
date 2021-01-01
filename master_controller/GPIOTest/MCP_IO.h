#include "ArdGPIO.h"
#include <MCP23S17.h>

class SPIClass;

class MCP_IO : public ArdGPIO
{
public:
	//TODO lets auto create these MCP objects when a pin is written. That should do.
	MCP_IO(SPIClass& spiObject, const int slaveSelectPin);
	virtual void digitalWrite( uint32_t dwPin, uint32_t dwVal );
	virtual int digitalRead( uint32_t ulPin );
	virtual void pullupMode(uint8_t, uint8_t);
	virtual void pinMode( uint32_t dwPin, uint32_t dwMode );
  virtual void inputInvert(uint8_t pin, uint8_t invert);
protected:
	MCP* mcps[8];
	SPIClass& spi;
	const int ss;

	bool checkPin(const int dwPin);

};
