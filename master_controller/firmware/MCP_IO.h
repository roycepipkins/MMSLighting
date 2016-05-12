#include "ArdGPIO.h"
#include <MCP23S17.h>

class MCP_IO : public ArdGPIO
{
public:
	MCP_IO( MCP* mcp_0,
			MCP* mcp_1 = 0,
			MCP* mcp_2 = 0,
			MCP* mcp_3 = 0,
			MCP* mcp_4 = 0,
			MCP* mcp_5 = 0,
			MCP* mcp_6 = 0,
			MCP* mcp_7 = 0
		);
	virtual void digitalWrite( uint32_t dwPin, uint32_t dwVal );
	virtual int digitalRead( uint32_t ulPin );
	virtual void pullupMode(uint8_t, uint8_t);
	virtual void pinMode( uint32_t dwPin, uint32_t dwMode );
  virtual void inputInvert(uint8_t pin, uint8_t invert);
protected:
	MCP* mcps[8];
};
