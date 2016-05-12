#include "MCP_IO.h"

MCP_IO::MCP_IO( MCP* mcp_0,
				MCP* mcp_1,
				MCP* mcp_2,
				MCP* mcp_3,
				MCP* mcp_4,
				MCP* mcp_5,
				MCP* mcp_6,
				MCP* mcp_7
			)
{
	mcps[0] = mcp_0;
	mcps[1] = mcp_1;
	mcps[2] = mcp_2;
	mcps[3] = mcp_3;
	mcps[4] = mcp_4;
	mcps[5] = mcp_5;
	mcps[6] = mcp_6;
	mcps[7] = mcp_7;
}

void MCP_IO::digitalWrite( uint32_t dwPin, uint32_t dwVal )
{
	int mcp_idx = dwPin >> 4;
	if (mcps[mcp_idx])
	{
		mcps[mcp_idx]->digitalWrite((dwPin&15) + 1, dwVal);  
	}
}

int MCP_IO::digitalRead( uint32_t ulPin )
{
	int mcp_idx = ulPin >> 4;
	if (mcps[mcp_idx])
	{
		return mcps[mcp_idx]->digitalRead((ulPin&15) + 1);
	}

  return 0;
 
}

void MCP_IO::pullupMode(uint8_t pin, uint8_t mode)
{
	int mcp_idx = pin >> 4;
	if (mcps[mcp_idx])
	{
		mcps[mcp_idx]->pullupMode((pin&15) + 1, mode);
	}
}

void MCP_IO::pinMode( uint32_t dwPin, uint32_t dwMode )
{
	int mcp_idx = dwPin >> 4;
	if (mcps[mcp_idx])
	{
		mcps[mcp_idx]->pinMode((dwPin&15) + 1, dwMode);
	}
}

void MCP_IO::inputInvert(uint8_t pin, uint8_t invert)
{
  int mcp_idx = pin >> 4;
  if (mcps[mcp_idx])
  {
    mcps[mcp_idx]->inputInvert((pin&15) + 1, invert);
  }
}

