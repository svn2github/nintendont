/*
ISO.c for Nintendont (Kernel)

Copyright (C) 2014 FIX94

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
#include "Config.h"
#include "ISO.h"
#include "FST.h"
#include "DI.h"
#include "ff.h"

extern u32 Region;
extern u32 TRIGame;

u32 ISOFileOpen = 0;

#define CACHE_MAX		0x400
#define CACHE_START		(u8*)0x11000000
#define CACHE_SIZE		0x1E80000

u32 CacheInited = 0;
u32 TempCacheCount = 0;
u32 DataCacheOffset = 0;
u8 *DCCache = CACHE_START;
u32 DCacheLimit = CACHE_SIZE;
DataCache DC[CACHE_MAX];

extern u32 USBReadTimer;
static FIL GameFile;
static u32 LastOffset = UINT_MAX, LastLength = 0, readptr;

static inline void ISOReadDirect(void *Buffer, u32 Length, u32 Offset)
{
	if(ISOFileOpen == 0)
		return;

	if(LastOffset != Offset)
		f_lseek( &GameFile, Offset );

	f_read( &GameFile, Buffer, Length, &readptr );

	LastOffset = Offset + Length;
	LastLength = Length;
	//refresh read timeout
	USBReadTimer = read32(HW_TIMER);
}

bool ISOInit()
{
	s32 ret = f_open( &GameFile, ConfigGetGamePath(), FA_READ|FA_OPEN_EXISTING );
	if( ret != FR_OK )
		return false;

	/* Setup table */
	DWORD tmp = 1; //size 1 to get the real size
	GameFile.cltbl = &tmp;
	f_lseek(&GameFile, CREATE_LINKMAP);
	GameFile.cltbl = malloc(tmp * sizeof(DWORD));
	GameFile.cltbl[0] = tmp; //fatfs automatically sets real size into tmp
	f_lseek(&GameFile, CREATE_LINKMAP); //actually create it

	/* Setup direct reader */
	ISOFileOpen = 1;
	LastOffset = UINT_MAX;
	LastLength = 0;

	/* Set Low Mem */
	ISOReadDirect((void*)0x0, 0x20, 0x0);
	sync_after_write((void*)0x0, 0x20); //used by game so sync it
	/* Get Region */
	ISOReadDirect(&Region, sizeof(u32), 0x458);
	/* Reset Cache */
	CacheInited = 0;

	return true;
}

void ISOClose()
{
	if(ISOFileOpen)
	{
		f_close( &GameFile );
		free(GameFile.cltbl);
		GameFile.cltbl = NULL;
	}
	ISOFileOpen = 0;
}

void ISOSetupCache()
{
	if(ISOFileOpen == 0 || CacheInited)
		return;

	DCCache = CACHE_START;
	DCacheLimit = CACHE_SIZE;
	/* Setup Caching */
	if(TRIGame) //triforce buffer is after cache
		DCacheLimit -= 0x300000;

	u32 MemCardSize = ConfigGetMemcardSize();
	if (!ConfigGetConfig(NIN_CFG_MEMCARDEMU))
		MemCardSize = 0;
	DCCache += MemCardSize; //memcard is before cache
	DCacheLimit -= MemCardSize;

	memset32(DC, 0, sizeof(DataCache)* CACHE_MAX);

	DataCacheOffset = 0;
	TempCacheCount = 0;

	CacheInited = 1;
}

u8 *ISORead(u32* Length, u32 Offset)
{
	if(CacheInited == 0)
	{
		if (*Length > DI_READ_BUFFER_LENGTH)
			*Length = DI_READ_BUFFER_LENGTH;
		ISOReadDirect(DI_READ_BUFFER, *Length, Offset);
		return DI_READ_BUFFER;
	}
	u32 i;

	for( i = 0; i < CACHE_MAX; ++i )
	{
		if(DC[i].Size == 0) continue;
		if( Offset >= DC[i].Offset && Offset + *Length <= DC[i].Offset + DC[i].Size )
		{
			//dbgprintf("DI: Cached Read Offset:%08X Size:%08X Buffer:%p\r\n", DC[i].Offset, DC[i].Size, DC[i].Data );
			return DC[i].Data + (Offset - DC[i].Offset);
		}
	}

	if( (Offset == LastOffset) && (*Length < 0x8000) )
	{	//pre-load data, guessing
		u32 OriLength = *Length;
		while((*Length += OriLength) < 0x10000) ;
	}

	// case we ran out of positions
	if( TempCacheCount >= CACHE_MAX )
		TempCacheCount = 0;

	// case we filled up the cache
	if( (DataCacheOffset + *Length) >= DCacheLimit )
	{
		for( i = 0; i < CACHE_MAX; ++i )
			DC[i].Size = 0; //quickly delete old cache content
		DataCacheOffset = 0;
		TempCacheCount = 0;
	}

	u32 pos = TempCacheCount;
	TempCacheCount++;

	DC[pos].Data = DCCache + DataCacheOffset;
	DC[pos].Offset = Offset;
	DC[pos].Size = *Length;

	ISOReadDirect(DC[pos].Data, *Length, Offset);

	DataCacheOffset += *Length;
	return DC[pos].Data;
}
