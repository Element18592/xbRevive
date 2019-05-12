#pragma once

#include "SFCx.h"
#include "..\XSidecar\XSidecar\XSidecar.h"

class SPISidecar : public SFCx
{
public:

	HANDLE Sidecar;

	SPISidecar( HANDLE Sidecar ) : SFCx( )
	{
		this->Sidecar = Sidecar;
	}

	virtual const char* GetName()
	{
		return "SPI Sidecar";
	}

	virtual bool EnterFlashMode()
	{
		if ( XSidecarEmulatorSpiBegin( this->Sidecar ) == 0 )
		{
			printf( "XSidecarEmulatorSpiBegin: Failed\n" );
			return false;
		}

		return true;
	}

	virtual bool ExitFlashMode()
	{
		if ( XSidecarEmulatorSpiEnd( this->Sidecar ) == 0 )
		{
			printf( "XSidecarEmulatorSpiEnd: Failed\n" );
			return false;
		}

		return true;
	}

	virtual DWORD ReadRegister(BYTE Register)
	{
		BYTE ReadPacket[ 2 ] = {
			( Register << 2 ) | 1,
			0xC1,
		};

		if ( XSidecarEmulatorSpiWrite( this->Sidecar, ReadPacket, sizeof( ReadPacket ) ) == 0 )
		{
			printf( "XSidecarEmulatorSpiWrite: Failed\n" );
			return 0;
		}

		DWORD Buffer = 0;
		DWORD BytesRead = 0;

		if ( XSidecarEmulatorSpiRead( this->Sidecar, (PBYTE)&Buffer, 4, &BytesRead ) == 0 )
		{
			printf( "XSidecarEmulatorSpiRead: Failed\n" );

			return 0;
		}

		return Buffer;
	}

	virtual void WriteRegister(BYTE Register, DWORD Value)
	{
		BYTE WritePacket[ 6 ] = {
				( Register << 2 ) | 2,
				7,
				0,
				0,
				0,
				0
		};

		*(DWORD*)& WritePacket[ 2 ] = ( Value );

		if ( XSidecarEmulatorSpiWrite( this->Sidecar, WritePacket, sizeof( WritePacket ) ) == 0 )
		{
			printf( "XSidecarEmulatorSpiWrite: Failed\n" );
		}
	}
};