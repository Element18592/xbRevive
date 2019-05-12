#include "SFCx.h"

DWORD SFCx::GetConfig()
{
	return this->ReadRegister( SFCX_CONFIG );
}

DWORD SFCx::GetStatus( )
{
	return this->ReadRegister( SFCX_STATUS );
}

bool SFCx::ParseConfig( )
{
	auto ConfigReg = this->GetConfig( );
	auto Config = &this->Config;

	Config->MetaSize = 0x10;
	Config->LogicalPageSize = 0x200;
	Config->PhysicalPageSize = Config->LogicalPageSize + Config->MetaSize;

	/*Do Some Shit*/
	Config->LogicalBlockSize = 0x20000;
	Config->PagesPerBlock = Config->LogicalBlockSize / Config->LogicalPageSize;

	Config->PhysicalBlockSize = Config->PhysicalPageSize * Config->PagesPerBlock;

	Config->NumberOfBlocks = 0x1000;

	Config->NumberOfPages = Config->PagesPerBlock * Config->NumberOfBlocks;

	return true;
}

void SFCx::DoCommand(BYTE Command, bool PollBusy, bool ErrorCheck)
{
	if ( Command <= 0xF || Command == UNLOCK_CMD_0 || Command == UNLOCK_CMD_1 )
	{
		this->WriteRegister( SFCX_COMMAND, Command );

		if ( Command == LOG_PAGE_TO_BUF || Command == PHY_PAGE_TO_BUF || Command == WRITE_PAGE_TO_PHY || Command == BLOCK_ERASE )
		{
			DWORD Status = 0;

			do {
				Status = this->ReadRegister( SFCX_STATUS );
			} while ( Status & STATUS_BUSY );
		}
	}
	else
	{
		printf( "Invalid Command ID: %02X\n", Command );
	}
}

void SFCx::ECCEncodePage(BYTE* PageData)
{
	DWORD v1 = 0;

	for (int i = 0; i < 0x1066; ++i)
	{
		DWORD v2 = v1 ^ (((1 << i % 8) & PageData[i / 8]) == 0);
		if (v2 & 1)
			v2 ^= 0x6954559u;
		v1 = v2 >> 1;
	}

	for (int i = 0x1066; i < 0x1080; i++)
	{
		if (v1 & 1)
			PageData[i / 8] &= ~(1 << i % 8);
		else
			PageData[i / 8] |= 1 << i % 8;
		v1 >>= 1;
		++i;
	}
}

void SFCx::EraseBlock( DWORD BlockIndex )
{
	DWORD Config = this->ReadRegister( SFCX_CONFIG );

	this->WriteRegister( SFCX_CONFIG, Config | CONFIG_WP_EN );

	this->WriteRegister( SFCX_ADDRESS, BlockIndex * this->Config.LogicalBlockSize );

	this->DoCommand( UNLOCK_CMD_1 );
	this->DoCommand( UNLOCK_CMD_0 );

	this->DoCommand( BLOCK_ERASE );
}

DWORD SFCx::ReadPage( DWORD LogicalAddress, BYTE* Buffer, bool Physical )
{
	this->WriteRegister( SFCX_ADDRESS, LogicalAddress );

	this->DoCommand( Physical ? PHY_PAGE_TO_BUF : LOG_PAGE_TO_BUF );

	this->WriteRegister( SFCX_ADDRESS, 0 );

	auto PageSize = Physical ? this->Config.PhysicalPageSize : this->Config.LogicalPageSize;

	for ( int i = 0; i < PageSize; i += 4 )
	{
		this->DoCommand( PAGE_BUF_TO_REG ); // Move page buffer to data register
		*(DWORD*)( &Buffer[ i ] ) = this->ReadRegister( SFCX_DATA );  // get contents of dataregister
	}

	return 0;
}

DWORD SFCx::ReadPage( DWORD BlockIndex, DWORD PageIndex, BYTE* Buffer, bool Physical )
{
	DWORD BlockAddress = BlockIndex * this->Config.LogicalBlockSize;
	DWORD PageOffset = PageIndex * this->Config.LogicalPageSize;

	return SFCx::ReadPage( BlockAddress + PageOffset, Buffer, Physical );
}

DWORD SFCx::WritePage( WORD LogicalAddress, BYTE* Buffer, bool Physical )
{
	this->WriteRegister( SFCX_ADDRESS, 0 );

	auto PageSize = Physical ? this->Config.PhysicalPageSize : this->Config.LogicalPageSize;

	for ( int i = 0; i < PageSize; i += 4 )
	{
		this->WriteRegister( SFCX_DATA, *(DWORD*)( &Buffer[ i ] ) );  // set contents of dataregister
		this->DoCommand( REG_TO_PAGE_BUF ); // Move data register to page buffer
	}

	this->WriteRegister( SFCX_ADDRESS, LogicalAddress );

	this->DoCommand( UNLOCK_CMD_0 );

	this->DoCommand( UNLOCK_CMD_1 );

	this->DoCommand( WRITE_PAGE_TO_PHY );

	return 0;
}

DWORD SFCx::WritePage( DWORD BlockIndex, DWORD PageIndex, BYTE* Buffer, bool Physical )
{
	DWORD BlockAddress = BlockIndex * this->Config.LogicalBlockSize;
	DWORD PageOffset = PageIndex * this->Config.LogicalPageSize;

	return SFCx::WritePage( BlockAddress + PageOffset, Buffer, Physical );
}