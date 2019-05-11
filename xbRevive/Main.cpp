#include "..\XSidecar\XSidecar\XSidecar.h"

#include <Windows.h>
#include <stdio.h>

typedef unsigned long long QWORD;

//Registers
#define SFCX_CONFIG				0x00
#define SFCX_STATUS 			0x04
#define SFCX_COMMAND			0x08
#define SFCX_ADDRESS			0x0C
#define SFCX_DATA				0x10
#define SFCX_LOGICAL 			0x14
#define SFCX_PHYSICAL			0x18
#define SFCX_DPHYSADDR			0x1C
#define SFCX_MPHYSADDR			0x20

//Commands for Command Register
#define PAGE_BUF_TO_REG			0x00 			//Read page buffer to data register
#define REG_TO_PAGE_BUF			0x01 			//Write data register to page buffer
#define LOG_PAGE_TO_BUF			0x02 			//Read logical page into page buffer
#define PHY_PAGE_TO_BUF			0x03 			//Read physical page into page buffer
#define WRITE_PAGE_TO_PHY		0x04 			//Write page buffer to physical page
#define BLOCK_ERASE				0x05 			//Block Erase
#define DMA_LOG_TO_RAM			0x06 			//DMA logical flash to main memory
#define DMA_PHY_TO_RAM			0x07 			//DMA physical flash to main memory
#define DMA_RAM_TO_PHY			0x08 			//DMA main memory to physical flash
#define UNLOCK_CMD_0			0x55 			//Unlock command 0
#define UNLOCK_CMD_1			0xAA 			//Unlock command 1

//Config Register bitmasks
#define CONFIG_DBG_MUX_SEL  	0x7C000000u		//Debug MUX Select
#define CONFIG_DIS_EXT_ER   	0x2000000u		//Disable External Error (Pre Jasper?)
#define CONFIG_CSR_DLY      	0x1FE0000u		//Chip Select to Timing Delay
#define CONFIG_ULT_DLY      	0x1F800u		//Unlocking Timing Delay
#define CONFIG_BYPASS       	0x400u			//Debug Bypass
#define CONFIG_DMA_LEN      	0x3C0u			//DMA Length in Pages
#define CONFIG_FLSH_SIZE    	0x30u			//Flash Size (Pre Jasper)
#define CONFIG_WP_EN        	0x8u			//Write Protect Enable
#define CONFIG_INT_EN       	0x4u			//Interrupt Enable
#define CONFIG_ECC_DIS      	0x2u			//ECC Decode Disable
#define CONFIG_SW_RST       	0x1u			//Software reset

//Status Register bitmasks
#define STATUS_ILL_LOG      	0x800u			//Illegal Logical Access
#define STATUS_PIN_WP_N     	0x400u			//NAND Not Write Protected
#define STATUS_PIN_BY_N     	0x200u			//NAND Not Busy
#define STATUS_INT_CP       	0x100u			//Interrupt
#define STATUS_ADDR_ER      	0x80u			//Address Alignment Error
#define STATUS_BB_ER        	0x40u			//Bad Block Error
#define STATUS_RNP_ER       	0x20u			//Logical Replacement not found
#define STATUS_ECC_ER       	0x1Cu			//ECC Error, 3 bits, need to determine each
#define STATUS_WR_ER        	0x2u			//Write or Erase Error
#define STATUS_BUSY         	0x1u			//Busy
#define STATUS_ERROR			(STATUS_ILL_LOG|STATUS_ADDR_ER|STATUS_BB_ER|STATUS_RNP_ER|STATUS_ECC_ER|STATUS_WR_ER)

//Jasper 16 MB NAND WITH CONFIG 0x00023010 

//Jasper 256 MB NAND WITH CONFIG 0x008A3020 

//Jasper 512 MB NAND WITH CONFIG 0xAA3020 
//0x21000 PHYSICAL BLOCK SIZE
//0x210 PHYSICAL PAGE SIZE
//PHYSICAL = PAGE DATA + META DATA (ECC)

//Jasper 512 MB NAND WITH CONFIG 0xAA3020
//0x20000 LOGICAL BLOCK SIZE
//0x200 LOGICAL PAGE SIZE
//LOGICAL = PAGE DATA WITHOUT META DATA (NO ECC)

#define JASPER_PHYSICAL_BLOCK_SIZE 0x21000
#define JASPER_PHYSICAL_PAGE_SIZE 0x210
#define JASPER_PHYSICAL_PAGES_PER_BLOCK (JASPER_PHYSICAL_BLOCK_SIZE / JASPER_PHYSICAL_PAGE_SIZE)

#define JASPER_LOGICAL_BLOCK_SIZE 0x20000
#define JASPER_LOGICAL_PAGE_SIZE 0x200
#define JASPER_LOGICAL_PAGES_PER_BLOCK (JASPER_LOGICAL_BLOCK_SIZE / JASPER_LOGICAL_PAGE_SIZE)

BOOL _inline SPIBegin( HANDLE Sidecar )
{
	if ( XSidecarEmulatorSpiBegin( Sidecar ) == 0 )
	{
		printf( "XSidecarEmulatorSpiBegin: Failed\n" );
		return FALSE;
	}

	return TRUE;
}

BOOL _inline SPIEnd( HANDLE Sidecar )
{
	if ( XSidecarEmulatorSpiEnd( Sidecar ) == 0 )
	{
		printf( "XSidecarEmulatorSpiEnd: Failed\n" );
		return FALSE;
	}

	return TRUE;
}

DWORD ReadSPIReg( HANDLE Sidecar, BYTE Register )
{
	BYTE ReadPacket[ 2 ] = {
		( Register << 2 ) | 1,
		0xC1,
	};

	if ( XSidecarEmulatorSpiWrite( Sidecar, ReadPacket, sizeof( ReadPacket ) ) == 0 )
	{
		printf( "XSidecarEmulatorSpiWrite: Failed\n" );
		return 0;
	}

	DWORD Buffer = 0;
	DWORD BytesRead = 0;

	if ( XSidecarEmulatorSpiRead( Sidecar, (PBYTE)&Buffer, 4, &BytesRead ) == 0 )
	{
		printf( "XSidecarEmulatorSpiRead: Failed\n" );

		return 0;
	}

	return Buffer;
}

BOOL WriteSPIReg( HANDLE Sidecar, BYTE Register, DWORD Data )
{
	BYTE WritePacket[ 6 ] = {
		( Register << 2 ) | 2,
		7,
		0,
		0,
		0,
		0
	};

	*(DWORD*)&WritePacket[ 2 ] = ( Data );

	if ( XSidecarEmulatorSpiWrite( Sidecar, WritePacket, sizeof( WritePacket ) ) == 0 )
	{
		printf( "XSidecarEmulatorSpiWrite: Failed\n" );
		return 0;
	}

	return TRUE;
}

DWORD _inline GetStatus( HANDLE Sidecar )
{
	return ReadSPIReg( Sidecar, SFCX_STATUS );
}

DWORD _inline GetConfig( HANDLE Sidecar )
{
	return ReadSPIReg( Sidecar, SFCX_CONFIG );
}

void WaitWhileBusy( HANDLE Sidecar )
{

}

void SPIDoCommand( HANDLE Sidecar, BYTE Command )
{
	WriteSPIReg( Sidecar, SFCX_COMMAND, Command );

	WaitWhileBusy( Sidecar );
}

DWORD _inline GetBlockSize( BOOL Logical )
{
	return Logical ? JASPER_LOGICAL_BLOCK_SIZE : JASPER_PHYSICAL_BLOCK_SIZE;
}

DWORD _inline GetPageSize( BOOL Logical )
{
	return Logical ? JASPER_LOGICAL_PAGE_SIZE : JASPER_PHYSICAL_PAGE_SIZE;
}

DWORD _inline GetPagesPerBlock( BOOL Logical )
{
	return Logical ? JASPER_LOGICAL_PAGES_PER_BLOCK : JASPER_PHYSICAL_PAGES_PER_BLOCK;
}

DWORD _inline IndexToAddress( DWORD BlockIndex, DWORD PageIndex, BOOL Logical )
{
	return ( BlockIndex * JASPER_LOGICAL_BLOCK_SIZE ) + ( PageIndex * JASPER_LOGICAL_PAGE_SIZE );
}

DWORD ReadPage( HANDLE Sidecar, DWORD Address, BYTE* Buffer, BOOL Logical )
{
	DWORD PageSize = GetPageSize( Logical );

	WriteSPIReg( Sidecar, SFCX_ADDRESS, Address );

	SPIDoCommand( Sidecar, Logical ? LOG_PAGE_TO_BUF : PHY_PAGE_TO_BUF );

	WriteSPIReg( Sidecar, SFCX_ADDRESS, 0 );

	for ( int i = 0; i < PageSize; i += 4 )
	{
		SPIDoCommand( Sidecar, PAGE_BUF_TO_REG ); // Move page buffer to data register
		*(DWORD*)( &Buffer[ i ] ) = ReadSPIReg( Sidecar, SFCX_DATA );  // get contents of dataregister
	}

	return 0;
}

DWORD ReadPage( HANDLE Sidecar, DWORD BlockIndex, DWORD PageIndex, BYTE* Buffer, BOOL Logical )
{
	if ( PageIndex >= GetPagesPerBlock( Logical ) )
	{
		printf( "[ERROR] Invalid Page ID\n" );
	}

	DWORD Address = IndexToAddress( BlockIndex, PageIndex, Logical );

	//printf( "Reading Page: %i/%i Address ( %08X )\n", PageIndex, GetPagesPerBlock( Logical ), Address );

	return ReadPage( Sidecar, Address, Buffer, Logical );
}

DWORD ReadBlock( HANDLE Sidecar, DWORD BlockID, BYTE* Buffer, BOOL Logical )
{
	for ( int i = 0; i < GetPagesPerBlock( Logical ); ++i )
	{
		DWORD Offset = GetPageSize( Logical ) * i;

		DWORD Status = ReadPage( Sidecar, BlockID, i, &Buffer[ Offset ], Logical );
	}

	return 0;
}

DWORD WritePage( HANDLE Sidecar, DWORD Address, BYTE* Buffer, BOOL Logical )
{
	DWORD PageSize = GetPageSize( Logical );

	if ( Address % PageSize != 0 )
	{
		printf( "[ERROR] Misaligned Address\n" );
		return 0;
	}

	WriteSPIReg( Sidecar, SFCX_ADDRESS, 0 );

	for ( int i = 0; i < PageSize; i += 4 )
	{
		WriteSPIReg( Sidecar, SFCX_DATA, *(DWORD*)( &Buffer[ i ] ) );  // set contents of dataregister
		SPIDoCommand( Sidecar, REG_TO_PAGE_BUF ); // Move data register to page buffer
	}

	SPIDoCommand( Sidecar, UNLOCK_CMD_0 );

	SPIDoCommand( Sidecar, UNLOCK_CMD_1 );

	SPIDoCommand( Sidecar, WRITE_PAGE_TO_PHY );

	return GetStatus( Sidecar );
}

DWORD WritePage( HANDLE Sidecar, DWORD BlockIndex, DWORD PageIndex, BYTE* Buffer, BOOL Logical )
{
	if ( PageIndex >= GetPagesPerBlock( Logical ) )
	{
		printf( "[ERROR] Invalid Page ID\n" );
	}

	DWORD Address = IndexToAddress( BlockIndex, PageIndex, Logical );

	return WritePage( Sidecar, Address, Buffer, Logical );
}

DWORD EraseBlock( HANDLE Sidecar, DWORD BlockID )
{
	WriteSPIReg( Sidecar, SFCX_ADDRESS, IndexToAddress( BlockID, 0, FALSE ) );

	SPIDoCommand( Sidecar, UNLOCK_CMD_1 );
	SPIDoCommand( Sidecar, UNLOCK_CMD_0 );

	WriteSPIReg( Sidecar, SFCX_COMMAND, BLOCK_ERASE );

	return GetStatus( Sidecar );
}

DWORD WriteBlock( HANDLE Sidecar, DWORD BlockID, BYTE* Buffer, BOOL Logical )
{
	if ( Logical )
		return STATUS_ERROR; // Not done yet

	EraseBlock( Sidecar, BlockID );

	for ( int i = 0; i < GetPagesPerBlock( Logical ); ++i )
	{
		DWORD Offset = IndexToAddress( 0, i, Logical );

		DWORD Status = WritePage( Sidecar, BlockID, i, &Buffer[ Offset ], Logical );
	}

	return GetStatus( Sidecar );
}

void ReadNAND( HANDLE Sidecar )
{
	FILE* DumpFile;

	if ( fopen_s( &DumpFile, "Nand.bin", "wb+" ) == 0 )
	{
		unsigned int config = ReadSPIReg( Sidecar, SFCX_CONFIG );

		WriteSPIReg( Sidecar, SFCX_CONFIG, config & ~( CONFIG_INT_EN | CONFIG_WP_EN | CONFIG_DMA_LEN ) );

		DWORD NumBlocks = 10;

		printf( "Reading blocks:  Logical      Physical\n" );
		printf( "                 ==========   ==========\n" );

		for ( int i = 0; i < NumBlocks; i++ )
		{
			DWORD Logical = IndexToAddress( i, 0, TRUE );
			DWORD Physical = IndexToAddress( i, 0, FALSE );
			DWORD CurrentBlockNum = i + 1;

			double PercentDone = ( (double)CurrentBlockNum / (double)NumBlocks ) * 100.0;

			printf( "                 0x%08X - 0x%08X          (%3.2f%%)\n", Logical, Physical, PercentDone );

			BYTE Block[ JASPER_PHYSICAL_BLOCK_SIZE ];

			ReadBlock( Sidecar, i, Block, FALSE );

			fwrite( Block, 1, JASPER_PHYSICAL_BLOCK_SIZE, DumpFile );
		}

		printf( "Finished\n" );

		fclose( DumpFile );
	}
	else
	{
		printf( "Failed To Open File\n" );
	}
}

void FlashNand( HANDLE Sidecar )
{
	FILE* FlashFile;

	if ( fopen_s( &FlashFile, "NandToFlash.bin", "wb+" ) == 0 )
	{
		fseek( FlashFile, 0, SEEK_END ); // seek to end of file

		SIZE_T Size = ftell( FlashFile ); // get current file pointer

		fseek( FlashFile, 0, SEEK_SET ); // seek back to beginning of file

		WriteSPIReg( Sidecar, SFCX_CONFIG, ReadSPIReg( Sidecar, SFCX_CONFIG ) | CONFIG_WP_EN );

		if ( Size % JASPER_PHYSICAL_BLOCK_SIZE != 0 )
		{
			printf( "File Misaligned to blocks" );
			return;
		}

		if ( Size % JASPER_PHYSICAL_PAGE_SIZE != 0 )
		{
			printf( "File Misaligned to pages" );
			return;
		}

		DWORD NumBlocks = Size / JASPER_PHYSICAL_BLOCK_SIZE;

		printf( "Writing blocks:  Logical      Physical\n" );
		printf( "                 ==========   ==========\n" );

		for ( int i = 0; i < NumBlocks; i++ )
		{
			DWORD Logical = IndexToAddress( i, 0, TRUE );
			DWORD Physical = IndexToAddress( i, 0, FALSE );
			DWORD CurrentBlockNum = i + 1;

			double PercentDone = ( (double)CurrentBlockNum / (double)NumBlocks ) * 100.0;

			printf( "                 0x%08X - 0x%08X          (%3.2f%%)\n", Logical, Physical, PercentDone );

			BYTE BlockBuffer[ JASPER_PHYSICAL_BLOCK_SIZE ];

			fread( BlockBuffer, 1, JASPER_PHYSICAL_BLOCK_SIZE, FlashFile );

			WriteBlock( Sidecar, i, BlockBuffer, FALSE );
		}

		printf( "Flashing Done\n" );
	}
	else
	{
		printf( "Couldnt Open NandToFlash.bin\n" );
	}
}

int main( int argc, const char* argv[] )
{
	printf(
		"SabMoDsoft (R) Xbox 360 Flash Tool (2.0.3206.0)\n"
		"Copyright (C) SabMoDsoft Corporation 1998-2019. All rights reserved.\n"
		"\n"
	);

	if ( LoadXSidecar( ) )
	{
		auto List = XSidecarOpenEmulatorList( );
		auto ListCount = XSidecarGetListItemCount( List );

		printf( "List Count: %i\n", ListCount );

		if ( ListCount == 0 )
		{
			printf( "No sidecar detected\n" );
			return 0;
		}

		WCHAR ConsoleNameBuffer[ 0x200 ];

		auto Sidecar = XSidecarOpenListItem( List, 0 );

		XSidecarGetName( Sidecar, ConsoleNameBuffer, 0x200, 0 );

		printf( "Connected To %ws\n", ConsoleNameBuffer );

		BOOL Power = FALSE;

		XSidecarEmulatorGetPowerState( Sidecar, &Power );

		if ( Power == TRUE )
		{
			printf( "Powering down %ws\n", ConsoleNameBuffer );
			XSidecarEmulatorSetPowerState( Sidecar, FALSE );
			Sleep( 3500 );
		}

		printf( "Entering Flash Mode...\n" );

		//SPIEnd( Sidecar );

		if ( SPIBegin( Sidecar ) )
		{
			printf( "Entered Flash Mode\n" );

			ReadNAND( Sidecar );

			printf( "Leaving Flash Mode...\n" );

			if ( SPIEnd( Sidecar ) )
			{
				printf( "Left Flash Mode...\n" );
			}
			else
			{
				printf( "Could Not Leave Flash Mode\n" );
			}
		}
		else
		{
			printf( "Failed To Enter Flashing Mode\n" );
		}

		XSidecarCloseList( List );
	}

	UnloadXSidecar( );

	getchar( );

	return 0;
}