#include <Windows.h>
#include <stdio.h>
#include <chrono>

#include "SPISidecar.h"

void PrintUsage( )
{
	printf( "Reads and writes Xbox 360 flash to revive consoles\n\nUsage: xbRevive [-Dump | -Flash ]\n" );
}

HANDLE GetSidecar( )
{
	auto List = XSidecarOpenEmulatorList( );

	auto ListCount = XSidecarGetListItemCount( List );

	WCHAR ConsoleNameBuffer[ 0x200 ];

	int SidecarIndex = 0;

	if ( ListCount == 1 )
	{
		printf( "Connecting To %ws\n", ConsoleNameBuffer );
		SidecarIndex = 0;
	}
	else if ( ListCount > 1 )
	{
		for ( int i = 0; i < ListCount; ++i )
		{
			memcpy( ConsoleNameBuffer, 0, sizeof( ConsoleNameBuffer ) );
			auto Sidecar = XSidecarOpenListItem( List, 0 );
			XSidecarGetName( Sidecar, ConsoleNameBuffer, 0x200, 0 );
			printf( "%i - %ws\n", i, ConsoleNameBuffer );
		}

		char SidecarSelection[ 0x200 ];

		fgets( SidecarSelection, sizeof( SidecarSelection ), stdin );

		if ( sscanf_s( SidecarSelection, "%d", &SidecarIndex ) == 1 )
		{
			if ( SidecarIndex >= ListCount )
			{
				printf( "Invalid Sidecar\n" );

				return 0;
			}
		}
		else
		{
			printf( "Invalid Sidecar\n" );
			return 0;
		}
	}
	else
	{
		printf( "No Consoles Detected\n" );

		return 0;
	}

	auto Sidecar = XSidecarOpenListItem( List, SidecarIndex );

	return Sidecar;
}


void DumpFlash( SFCx* Nand )
{
	FILE* DumpFile;

	if ( fopen_s( &DumpFile, "Nand.bin", "wb+" ) == 0 )
	{
		printf( "[%s] Entering Flashing Mode\n", Nand->GetName( ) );

		Nand->EnterFlashMode( );

		Nand->ParseConfig( );

		Nand->Config.NumberOfBlocks = 512;

		BYTE* BlockBuffer = (BYTE*)malloc( Nand->Config.PhysicalBlockSize );

		if ( !BlockBuffer )
		{
			printf( "Failed To Allocate Block\n" );
			return;
		}

		DWORD NumberOfPages = Nand->Config.NumberOfBlocks * Nand->Config.PagesPerBlock;

		printf( "Page Size (Logical): %i\n", Nand->Config.LogicalPageSize );
		printf( "Block Size (Logical): %i\n", Nand->Config.LogicalBlockSize );
		printf( "Dumping: %i Blocks\n", Nand->Config.NumberOfBlocks );
		printf( "Dumping Physical Pages (PAGE + META)\n" );

		auto StartNand = std::chrono::steady_clock::now( );

		for ( int i = 0; i < Nand->Config.NumberOfBlocks; ++i )
		{
			for ( int j = 0; j < Nand->Config.PagesPerBlock; ++j )
			{
				auto StartPage = std::chrono::steady_clock::now( );

				Nand->ReadPage( i, j, &BlockBuffer[ j * Nand->Config.PhysicalPageSize ], true );

				auto EndPage = std::chrono::steady_clock::now( );

				auto ElapsedPage = std::chrono::duration_cast<std::chrono::milliseconds>( EndPage - StartPage );

				DWORD CompletedPages = ( i * Nand->Config.PagesPerBlock ) + j + 1;

				float Percentage = ( (double)CompletedPages / (double)NumberOfPages ) * 100.0;

				printf( "\rReading Nand  %0.03f%% (%i/%i Pages) (%i/%i Blocks) Page Took: %lli ms", Percentage, CompletedPages, NumberOfPages, i + 1, Nand->Config.NumberOfBlocks, ElapsedPage.count( ) );
			}

			fwrite( BlockBuffer, 1, Nand->Config.PhysicalBlockSize, DumpFile );
		}

		auto EndNand = std::chrono::steady_clock::now( );

		auto ElapsedNand = std::chrono::duration_cast<std::chrono::milliseconds>( EndNand - StartNand );

		printf( "\nRead Nand Took: %lli ms\n", ElapsedNand.count( ) );

		printf( "[%s] Exiting Flashing Mode\n", Nand->GetName( ) );

		Nand->ExitFlashMode( );

		free( BlockBuffer );

		fclose( DumpFile );
	}
	else
	{
		printf( "Couldnt Open Dump File\n" );
	}
}

void FlashNand( SFCx* Nand )
{
	FILE* FlashFile;

	if ( fopen_s( &FlashFile, "NandToFlash.bin", "wb+" ) == 0 )
	{
		printf( "[%s] Entering Flashing Mode\n", Nand->GetName( ) );

		Nand->EnterFlashMode( );

		Nand->ParseConfig( );

		fseek( FlashFile, 0, SEEK_END ); // seek to end of file

		SIZE_T Size = ftell( FlashFile ); // get current file pointer

		fseek( FlashFile, 0, SEEK_SET ); // seek back to beginning of file

		if ( Size % Nand->Config.PhysicalBlockSize != 0 )
		{
			printf( "File Misaligned to blocks" );
			return;
		}

		if ( Size % Nand->Config.PhysicalPageSize != 0 )
		{
			printf( "File Misaligned to pages" );
			return;
		}

		Nand->Config.NumberOfBlocks = Size / Nand->Config.PhysicalBlockSize;

		BYTE* BlockBuffer = (BYTE*)malloc( Nand->Config.PhysicalBlockSize );

		if ( !BlockBuffer )
		{
			printf( "Failed To Allocate Block\n" );
			return;
		}

		DWORD NumberOfPages = Nand->Config.NumberOfBlocks * Nand->Config.PagesPerBlock;

		printf( "Page Size (Logical): %i\n", Nand->Config.LogicalPageSize );
		printf( "Block Size (Logical): %i\n", Nand->Config.LogicalBlockSize );
		printf( "Flashing: %i Blocks\n", Nand->Config.NumberOfBlocks );
		printf( "Flashing Physical Pages (PAGE + META)\n" );

		auto StartNand = std::chrono::steady_clock::now( );

		for ( int i = 0; i < Nand->Config.NumberOfBlocks; ++i )
		{
			fread( BlockBuffer, 1, Nand->Config.PhysicalBlockSize, FlashFile );

			for ( int j = 0; j < Nand->Config.PagesPerBlock; ++j )
			{
				auto StartPage = std::chrono::steady_clock::now( );

				Nand->WritePage( i, j, &BlockBuffer[ j * Nand->Config.PhysicalPageSize ], true );

				auto EndPage = std::chrono::steady_clock::now( );

				auto ElapsedPage = std::chrono::duration_cast<std::chrono::milliseconds>( EndPage - StartPage );

				DWORD CompletedPages = ( i * Nand->Config.PagesPerBlock ) + j + 1;

				float Percentage = ( (double)CompletedPages / (double)NumberOfPages ) * 100.0;

				printf( "\rFlashing Nand  %0.03f%% (%i/%i Pages) (%i/%i Blocks) Page Took: %lli ms", Percentage, CompletedPages, NumberOfPages, i + 1, Nand->Config.NumberOfBlocks, ElapsedPage.count( ) );
			}

		}

		auto EndNand = std::chrono::steady_clock::now( );

		auto ElapsedNand = std::chrono::duration_cast<std::chrono::milliseconds>( EndNand - StartNand );

		printf( "\nFlash Nand Took: %lli ms\n", ElapsedNand.count( ) );

		printf( "[%s] Exiting Flashing Mode\n", Nand->GetName( ) );

		Nand->ExitFlashMode( );

		free( BlockBuffer );

		fclose( FlashFile );
	}
	else
	{
		printf( "Couldnt Open Flash File\n" );
	}
}

bool ProcessArgs( int argc, const char* argv[ ] )
{
	if ( argc == 1 )
		return false;

	for ( int i = 1; i < argc; ++i )
	{
		if ( strcmp( argv[ i ], "Dump" ) == 0 )
		{
			printf( "Dumping Lmao\n" );

			auto Sidecar = GetSidecar( );

			if ( Sidecar )
			{
				WCHAR ConsoleNameBuffer[ 0x200 ];

				XSidecarGetName( Sidecar, ConsoleNameBuffer, 0x200, 0 );

				printf( "Connected To: %ws\n", ConsoleNameBuffer );

				SPISidecar Nand = SPISidecar( Sidecar ); // woud be what ever interface they asked for

				DumpFlash( &Nand );
			}
			else
			{
				printf( "Failed To Connect To Sidecar\n" );
			}

			break;
		}

		if ( strcmp( argv[ i ], "Flash" ) == 0 )
		{
			printf( "Flashing\n" );

			auto Sidecar = GetSidecar( );

			if ( Sidecar )
			{
				WCHAR ConsoleNameBuffer[ 0x200 ];

				XSidecarGetName( Sidecar, ConsoleNameBuffer, 0x200, 0 );

				printf( "Connected To: %ws\n", ConsoleNameBuffer );

				SPISidecar Nand = SPISidecar( Sidecar ); // woud be what ever interface they asked for

				FlashNand( &Nand );
			}
			else
			{
				printf( "Failed To Connect To Sidecar\n" );
			}

			break;
		}
	}

	return true;
}

int main( int argc, const char* argv[ ] )
{
	printf(
		"SabMoDsoft (R) Xbox 360 Flash Tool (2.0.3206.0)\n"
		"Copyright (C) SabMoDsoft Corporation 1998-2019. All rights reserved.\n"
		"\n"
	);

	SetConsoleTitleA( "xbRevive" );

	if ( LoadXSidecar( ) )
	{
		const char* TestArgs[ ] = {
			argv[0],
			"Dump"
		};

		if ( !ProcessArgs( ARRAYSIZE( TestArgs ), TestArgs ) )
		{
			PrintUsage( );
		}

		UnloadXSidecar( );
	}

	return 0;
}