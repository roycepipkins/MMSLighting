#include "MCP_IO.h"
#include <Arduino.h>

//TODO accept the slave select pin
MCP_IO::MCP_IO(SPIClass& spiObject, const int slaveSelectPin):
spi(spiObject),
ss(slaveSelectPin)
{
	
}

bool MCP_IO::checkPin(const int dwPin)
{
	int mcp_idx = dwPin >> 4;

	if (mcp_idx >= 8) return false;

	if (!mcps[mcp_idx])
	{
		mcps[mcp_idx] = new MCP(spi, mcp_idx, ss);
    if (mcps[mcp_idx] != nullptr) mcps[mcp_idx]->begin();
	}

	return mcps[mcp_idx] != nullptr;
}



void MCP_IO::digitalWrite( uint32_t dwPin, uint32_t dwVal )
{
	if (checkPin(dwPin))
	{
		mcps[dwPin >> 4]->digitalWrite((dwPin&15) + 1, dwVal);  
	}
}

int MCP_IO::digitalRead( uint32_t ulPin )
{
	if (checkPin(ulPin))
	{
		return mcps[ulPin >> 4]->digitalRead((ulPin&15) + 1);
	}

  return 0;
}

void MCP_IO::pullupMode(uint8_t pin, uint8_t mode)
{
	if (checkPin(pin))
	{
		mcps[pin >> 4]->pullupMode((pin&15) + 1, mode);
	}
}

void MCP_IO::pinMode( uint32_t dwPin, uint32_t dwMode )
{
	if (checkPin(dwPin))
	{
		mcps[dwPin >> 4]->pinMode((dwPin&15) + 1, dwMode);
	}
}

void MCP_IO::inputInvert(uint8_t pin, uint8_t invert)
{
  if (checkPin(pin))
  {
    mcps[pin >> 4]->inputInvert((pin&15) + 1, invert);
  }
}
