#pragma once

#include <Windows.h>
#include <stdio.h>

#define GET_MASK(NumBits) ( ~(UINT_MAX << NumBits) )
#define GET_BITFIELD_MASK(Start, NumBits) ( GET_MASK(NumBits) << Start )
#define GET_BITS(Bits, Start, Length) ( ( Bits >> Start) & GET_MASK(Length) )

//Registers
#define SFCX_CONFIG						0x00
#define SFCX_STATUS 					0x04
#define SFCX_COMMAND					0x08
#define SFCX_ADDRESS					0x0C
#define SFCX_DATA						0x10
#define SFCX_LOGICAL 					0x14
#define SFCX_PHYSICAL					0x18
#define SFCX_DPHYSADDR					0x1C
#define SFCX_MPHYSADDR					0x20

//Commands for Command Register
#define PAGE_BUF_TO_REG					0x00 			//Read page buffer to data register
#define REG_TO_PAGE_BUF					0x01 			//Write data register to page buffer
#define LOG_PAGE_TO_BUF					0x02 			//Read logical page into page buffer
#define PHY_PAGE_TO_BUF					0x03 			//Read physical page into page buffer
#define WRITE_PAGE_TO_PHY				0x04 			//Write page buffer to physical page
#define BLOCK_ERASE						0x05 			//Block Erase
#define DMA_LOG_TO_RAM					0x06 			//DMA logical flash to main memory
#define DMA_PHY_TO_RAM					0x07 			//DMA physical flash to main memory
#define DMA_RAM_TO_PHY					0x08 			//DMA main memory to physical flash
#define UNLOCK_CMD_0					0x55 			//Unlock command 0
#define UNLOCK_CMD_1					0xAA 			//Unlock command 1

//Config Register bitmasks
#define CONFIG_SW_RST					GET_BITFIELD_MASK( 0, 1 )
#define CONFIG_ECC_DIS					GET_BITFIELD_MASK( 1, 1 )
#define CONFIG_INT_EN					GET_BITFIELD_MASK( 2, 1 )
#define CONFIG_WP_EN					GET_BITFIELD_MASK( 3, 1 )
#define CONFIG_FLSH_SIZE				GET_BITFIELD_MASK( 4, 5 )
#define CONFIG_DMA_LEN					GET_BITFIELD_MASK( 6, 4 )
#define CONFIG_BYPASS					GET_BITFIELD_MASK( 10, 1 )
#define CONFIG_ULT_DLY					GET_BITFIELD_MASK( 11, 6 )
#define CONFIG_CSR_DLY					GET_BITFIELD_MASK( 17, 8 )
#define CONFIG_DIS_EXT_ER				GET_BITFIELD_MASK( 25, 1 )
#define CONFIG_DBG_MUX_SEL				GET_BITFIELD_MASK( 26, 5 )

#define CONFIG_SW_RST_GET(Config)		GET_BITS( Config, 0, 1 )
#define CONFIG_ECC_DIS_GET(Config)		GET_BITS( Config, 1, 1 )
#define CONFIG_INT_EN_GET(Config)		GET_BITS( Config, 2, 1 )
#define CONFIG_WP_EN_GET(Config)		GET_BITS( Config, 3, 1 )
#define CONFIG_FLSH_SIZE_GET(Config)	GET_BITS( Config, 4, 2 )
#define CONFIG_DMA_LEN_GET(Config)		GET_BITS( Config, 6, 4 )
#define CONFIG_BYPASS_GET(Config)		GET_BITS( Config, 10, 1 )
#define CONFIG_ULT_DLY_GET(Config)		GET_BITS( Config, 11, 6 )
#define CONFIG_CSR_DLY_GET(Config)		GET_BITS( Config, 17, 8 )
#define CONFIG_DIS_EXT_ER_GET(Config)	GET_BITS( Config, 25, 1 )
#define CONFIG_DBG_MUX_SEL_GET(Config)	GET_BITS( Config, 26, 5 )

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


// Was going to use this but do we want portability?
//struct SFCxConfigRegister
//{
//	DWORD SW_RST		: 1; // [0 - 1]
//	DWORD ECC_DIS		: 1; // [1 - 2]
//	DWORD INT_EN		: 1; // [2 - 3]
//	DWORD WP_EN			: 1; // [3 - 4]
//	DWORD FLSH_SIZE		: 2; // [4 - 6]
//	DWORD DMA_LEN		: 4; // [6 - 10]
//	DWORD BYPASS		: 1; // [10 - 11]
//	DWORD ULT_DLY		: 6; // [11 - 17]
//	DWORD CSR_DLY		: 8; // [17 - 25]
//	DWORD DIS_EXT_ER	: 1; // [25 - 26]
//	DWORD DBG_MUX_SEL	: 5; // [26 - 31]
//};

//struct SFCxStatusRegister
//{
//	DWORD BUSY		: 1; // [0 - 1]
//	DWORD WR_ER		: 1; // [1 - 2]
//	DWORD ECC_ER	: 3; // [2 - 5]
//	DWORD RNP_ER	: 1; // [5 - 6]
//	DWORD BB_ER		: 1; // [6 - 7]
//	DWORD ADDR_ER	: 1; // [7 - 8]
//	DWORD INT_CP	: 1; // [9 - 10]
//	DWORD PIN_BY_N	: 1; // [10 - 11]
//	DWORD PIN_WP_N	: 1; // [11 - 12]
//	DWORD ILL_LOG	: 1; // [12 - 13]
//};


struct SFCxConfig
{
	BOOL Valid;

	DWORD PagesPerBlock;
	DWORD NumberOfBlocks;
	DWORD NumberOfReservedBlocks;
	DWORD NumberOfPages;

	DWORD MetaSize;

	DWORD PhysicalPageSize;
	DWORD LogicalPageSize;

	DWORD PhysicalBlockSize;
	DWORD LogicalBlockSize;

	DWORD Capacity;
};

class SFCx
{
public:

	SFCxConfig Config;

	DWORD StatusCache;
	DWORD ConfigCache;

	SFCx( ) : StatusCache( 0 ), ConfigCache( 0 )
	{

	}

	virtual const char* GetName( ) = 0;

	virtual	bool EnterFlashMode( ) = 0;

	virtual	bool ExitFlashMode( ) = 0;

	virtual DWORD ReadRegister( BYTE Register ) = 0;

	virtual void WriteRegister( BYTE Register, DWORD Value ) = 0;

	DWORD GetConfig( );

	DWORD GetStatus( );

	bool ParseConfig( );

	void ToggleWriteProtection( bool Enabled );

	void DoCommand( BYTE Command, bool PollBusy = false, bool ErrorCheck = false );

	void ECCEncodePage( BYTE* PageData );

	void EraseBlock( DWORD BlockIndex );

	DWORD ReadPage( DWORD LogicalAddress, BYTE* Buffer, bool Physical );

	DWORD ReadPage( DWORD BlockIndex, DWORD PageIndex, BYTE* Buffer, bool Physical );

	DWORD WritePage( WORD LogicalAddress, BYTE* Buffer, bool Physical );

	DWORD WritePage( DWORD BlockIndex, DWORD PageIndex, BYTE* Buffer, bool Physical );
};