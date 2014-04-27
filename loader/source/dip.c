/*

Nintendont (Loader) - Playing Gamecubes in Wii mode on a Wii U

Copyright (C) 2013  crediar

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/
#include "dip.h"
#include "ogc/cache.h"
#include <string.h>

int DVDLowRead( void *ptr, u32 len, u32 offset )
{
	DI_STATUS	= 0x2A | 0x14;			// clear IRQs
	DI_CMD_0	= 0xA8000000;
	DI_CMD_1	= ((u32)offset)>>2;
	DI_CMD_2	= len;
	DI_DMA_ADR	= (u32)ptr;
	DI_DMA_LEN	= len;
	
	
//Start cmd!
	DI_CONTROL = 3;
	while( DI_CONTROL == 3 );
	while( DI_SCONTROL & 1 );

	while(1)
	{
		if( DI_SSTATUS & 0x04 )		//Error
		{
			DI_SSTATUS = 0;
			return 0;
		}
		if( DI_SSTATUS & 0x10 )		//Transfer done
		{
			DCInvalidateRange( (void*)ptr, len );

			DI_SSTATUS = 0;
			return 1;
		}
	}
}
