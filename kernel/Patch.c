﻿/*

Nintendont (Kernel) - Playing Gamecubes in Wii mode on a Wii U

Copyright (C) 2013  crediar
Copyright (C) 2014  FIX94

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
#include "Patch.h"
#include "string.h"
#include "ff.h"
#include "dol.h"
#include "elf.h"
#include "PatchCodes.h"
#include "PatchTimers.h"
#include "Config.h"
#include "global.h"
#include "patches.c"
#include "DI.h"
#include "SI.h"

//#define DEBUG_DSP  // Very slow!! Replace with raw dumps?

#define GAME_ID		(read32(0))
#define TITLE_ID	(GAME_ID >> 8)

#define PATCH_OFFSET_START (0x2F00 - (sizeof(u32) * 5))
#define PATCH_OFFSET_ENTRY PATCH_OFFSET_START - FakeEntryLoad_size
u32 POffset = PATCH_OFFSET_ENTRY;
vu32 Region = 0;
extern vu32 TRIGame;
extern u32 SystemRegion;

extern int dbgprintf( const char *fmt, ...);

unsigned char OSReportDM[] =
{
	0x7C, 0x08, 0x02, 0xA6, 0x90, 0x01, 0x00, 0x04, 0x90, 0xE1, 0x00, 0x08, 0x3C, 0xE0, 0xC0, 0x00, 
	0x90, 0x67, 0x18, 0x60, 0x90, 0x87, 0x18, 0x64, 0x90, 0xA7, 0x18, 0x68, 0x90, 0xC7, 0x18, 0x6C, 
	0x90, 0xE7, 0x18, 0x70, 0x91, 0x07, 0x18, 0x74, 0x80, 0x07, 0x18, 0x60, 0x7C, 0x00, 0x18, 0x00, 
	0x41, 0x82, 0xFF, 0xF8, 0x80, 0xE1, 0x00, 0x08, 0x80, 0x01, 0x00, 0x04, 0x7C, 0x08, 0x03, 0xA6, 
	0x4E, 0x80, 0x00, 0x20, 
} ;

const unsigned char DSPHashes[][0x14] =
{
	{
		0xC9, 0x7D, 0x1E, 0xD0, 0x71, 0x90, 0x47, 0x3F, 0x6A, 0x66, 0x42, 0xB2, 0x7E, 0x4A, 0xDB, 0xCD, 0xB6, 0xF8, 0x8E, 0xC3,			//	0 Dolphin=0x86840740=Zelda WW
	},
	{
		0x21, 0xD0, 0xC0, 0xEE, 0x25, 0x3D, 0x8C, 0x9E, 0x02, 0x58, 0x66, 0x7F, 0x3C, 0x1B, 0x11, 0xBC, 0x90, 0x1F, 0x33, 0xE2,			//	1 Dolphin=0x56d36052=Super Mario Sunshine
	},
	{
		0xBA, 0xDC, 0x60, 0x15, 0x33, 0x33, 0x28, 0xED, 0xB1, 0x0E, 0x72, 0xF2, 0x5B, 0x5A, 0xFB, 0xF3, 0xEF, 0x90, 0x30, 0x90,			//	2 Dolphin=0x4e8a8b21=Sonic Mega Collection
	},
	{
		0xBC, 0x17, 0x36, 0x81, 0x7A, 0x14, 0xB2, 0x1C, 0xCB, 0xF7, 0x3A, 0xD6, 0x8F, 0xDA, 0x57, 0xF8, 0x43, 0x78, 0x1A, 0xAE,			//	3 Dolphin=0xe2136399=Mario Party 5
	},
	{
		0xD9, 0x39, 0x63, 0xE3, 0x91, 0xD1, 0xA8, 0x5E, 0x4D, 0x5F, 0xD9, 0xC2, 0x9A, 0xF9, 0x3A, 0xA9, 0xAF, 0x8D, 0x4E, 0xF2,			//	4 Dolphin=0x07f88145=Zelda:OOT
	},
	{
		0xD8, 0x12, 0xAC, 0x09, 0xDC, 0x24, 0x50, 0x6B, 0x0D, 0x73, 0x3B, 0xF5, 0x39, 0x45, 0x1A, 0x23, 0x85, 0xF3, 0xC8, 0x79,			//	5 Dolphin=0x2fcdf1ec=Mario Kart DD
	},	
	{
		0x37, 0x44, 0xC3, 0x82, 0xC8, 0x98, 0x42, 0xD4, 0x9D, 0x68, 0x83, 0x1C, 0x2B, 0x06, 0x7E, 0xC7, 0xE8, 0x64, 0x32, 0x44,			//	6 Dolphin=0x3daf59b9=Star Fox Assault
	},
	{
		0xDA, 0x39, 0xA3, 0xEE, 0x5E, 0x6B, 0x4B, 0x0D, 0x32, 0x55, 0xBF, 0xEF, 0x95, 0x60, 0x18, 0x90, 0xAF, 0xD8, 0x07, 0x09,			//	7 0-Length DSP - Error
	},	 
	{
		0x8E, 0x5C, 0xCA, 0xEA, 0xA9, 0x84, 0x87, 0x02, 0xFB, 0x5C, 0x19, 0xD4, 0x18, 0x6E, 0xA7, 0x7B, 0xE5, 0xB8, 0x71, 0x78,			//	8 Dolphin=0x6CA33A6D=Donkey Kong Jungle Beat
	},	 
	{
		0xBC, 0x1E, 0x0A, 0x75, 0x09, 0xA6, 0x3E, 0x7C, 0xE6, 0x30, 0x44, 0xBE, 0xCC, 0x8D, 0x67, 0x1F, 0xA7, 0xC7, 0x44, 0xE5,			//	9 Dolphin=0x3ad3b7ac=Paper Mario The Thousand Year Door
	},	 
	{
		0x14, 0x93, 0x40, 0x30, 0x0D, 0x93, 0x24, 0xE3, 0xD3, 0xFE, 0x86, 0xA5, 0x68, 0x2F, 0x4C, 0x13, 0x38, 0x61, 0x31, 0x6C,			//	10 Dolphin=0x4be6a5cb=AC
	},	
	{
		0x09, 0xF1, 0x6B, 0x48, 0x57, 0x15, 0xEB, 0x3F, 0x67, 0x3E, 0x19, 0xEF, 0x7A, 0xCF, 0xE3, 0x60, 0x7D, 0x2E, 0x4F, 0x02,			//	11 Dolphin=0x42f64ac4=Luigi
	},
	{
		0x80, 0x01, 0x60, 0xDF, 0x89, 0x01, 0x9E, 0xE3, 0xE8, 0xF7, 0x47, 0x2C, 0xE0, 0x1F, 0xF6, 0x80, 0xE9, 0x85, 0xB0, 0x24,			//	12 Dolphin=0x267fd05a=Pikmin PAL
	},
	{
		0xB4, 0xCB, 0xC0, 0x0F, 0x51, 0x2C, 0xFE, 0xE5, 0xA4, 0xBA, 0x2A, 0x59, 0x60, 0x8A, 0xEB, 0x8C, 0x86, 0xC4, 0x61, 0x45,			//	13 Dolphin=0x6ba3b3ea=IPL NTSC 1.2
	},
	{
		0xA5, 0x13, 0x45, 0x90, 0x18, 0x30, 0x00, 0xB1, 0x34, 0x44, 0xAE, 0xDB, 0x61, 0xC5, 0x12, 0x0A, 0x72, 0x66, 0x07, 0xAA,			//	14 Dolphin=0x24b22038=IPL NTSC 1.0
	},
	{
		0x9F, 0x3C, 0x9F, 0x9E, 0x05, 0xC7, 0xD5, 0x0B, 0x38, 0x49, 0x2F, 0x2C, 0x68, 0x75, 0x30, 0xFD, 0xE8, 0x6F, 0x9B, 0xCA,			//	15 Dolphin=0x3389a79e=Metroid Prime Trilogy Wii (Needed?)
	},
};

const unsigned char DSPPattern[][0x10] =
{
	{
		0x02, 0x9F, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x02, 0xFF, 0x00, 0x00, 0x02, 0xFF, 0x00, 0x00,		//	0 Hash 12, 1, 0, 5, 8
	},
	{
		0x02, 0x9F, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x02, 0xFF, 0x00, 0x00, 0x02, 0xFF, 0x00, 0x00,		//	1 Hash 14, 13, 11, 10
	},
	{
		0x00, 0x00, 0x00, 0x00, 0x02, 0x9F, 0x0C, 0x10, 0x02, 0x9F, 0x0C, 0x1F, 0x02, 0x9F, 0x0C, 0x3B,		//	2 Hash 2
	},
	{
		0x00, 0x00, 0x00, 0x00, 0x02, 0x9F, 0x0E, 0x76, 0x02, 0x9F, 0x0E, 0x85, 0x02, 0x9F, 0x0E, 0xA1,		//	3 Hash 3
	},
	{
		0x00, 0x00, 0x00, 0x00, 0x02, 0x9F, 0x0E, 0xB3, 0x02, 0x9F, 0x0E, 0xC2, 0x02, 0x9F, 0x0E, 0xDE,		//	4 Hash 4
	},
	{
		0x00, 0x00, 0x00, 0x00, 0x02, 0x9F, 0x0E, 0x71, 0x02, 0x9F, 0x0E, 0x80, 0x02, 0x9F, 0x0E, 0x9C,		//	5 Hash 9
	},
	{
		0x00, 0x00, 0x00, 0x00, 0x02, 0x9F, 0x0E, 0x88, 0x02, 0x9F, 0x0E, 0x97, 0x02, 0x9F, 0x0E, 0xB3,		//	6 Hash 6
	},
	{
		0x00, 0x00, 0x00, 0x00, 0x02, 0x9F, 0x0D, 0xB0, 0x02, 0x9F, 0x0D, 0xBF, 0x02, 0x9F, 0x0D, 0xDB,		//	7 Hash 15
	},
};

typedef struct DspMatch
{
	u32 Length;
	u32 Pattern;
	u32 SHA1;
} DspMatch;

const DspMatch DspMatches[] =
{
	// Order Patterns together by increasing Length
	// Length, Pattern,   SHA1
	{ 0x00001A60,    0,     12 },
	{ 0x00001CE0,    0,      1 },
	{ 0x00001D20,    0,      0 },
	{ 0x00001D20,    0,      5 },
	{ 0x00001F00,    0,      8 },
	{ 0x00001280,    1,     14 },
	{ 0x00001760,    1,     13 },
	{ 0x000017E0,    1,     11 },
	{ 0x00001A00,    1,     10 },
	{ 0x000019E0,    2,      2 },
	{ 0x00001EC0,    3,      3 },
	{ 0x00001F20,    4,      4 },
	{ 0x00001EC0,    5,      9 },
	{ 0x00001F00,    6,      6 },
	{ 0x00001D40,    7,     15 },
};

#define AX_DSP_NO_DUP3 (0xFFFF)
void PatchAX_Dsp(u32 ptr, u32 Dup1, u32 Dup2, u32 Dup3, u32 Dup2Offset)
{
	static const u32 MoveLength = 0x22;
	static const u32 CopyLength = 0x12;
	static const u32 CallLength = 0x2 + 0x2; // call (2) ; jmp (2)
	static const u32 New1Length = 0x1 * 3 + 0x2 + 0x7; // 3 * orc (1) ; jmp (2) ; patch (7)
	static const u32 New2Length = 0x1; // ret
	u32 SourceAddr = Dup1 + MoveLength;
	u32 DestAddr = Dup2 + CallLength + CopyLength + New2Length;
	u32 Dup2PAddr = DestAddr; // Dup2+0x17 (0xB Patch fits before +0x22)
	u32 Tmp;

	DestAddr--;
	W16((u32)ptr + (DestAddr)* 2, 0x02DF);  // ret
	while (SourceAddr >= Dup1 + 1)  // ensure include Dup1 below
	{
		SourceAddr -= 2;
		Tmp = R32((u32)ptr + (SourceAddr)* 2); // original instructions
		if ((Tmp & 0xFFFFFFFF) == 0x195E2ED1) // lrri        $AC0.M srs         @SampleFormat, $AC0.M
		{
			DestAddr -= 7;
			W32((u32)ptr + (DestAddr + 0x0) * 2, 0x03400003); // andi        $AC1.M, #0x0003
			W32((u32)ptr + (DestAddr + 0x2) * 2, 0x8100009E); // clr         $ACC0 -
			W32((u32)ptr + (DestAddr + 0x4) * 2, 0x200002CA); // lri         $AC0.M, 0x2000 lsrn
			W16((u32)ptr + (DestAddr + 0x6) * 2, 0x1FFE);     // mrr     $AC1.M, $AC0.M
			Tmp = Tmp | 0x00010100; // lrri        $AC1.M srs         @SampleFormat, $AC1.M
		}
		DestAddr -= 2;
		W32((u32)ptr + (DestAddr)* 2, Tmp); // unchanged
		if ((Tmp & 0x0000FFF1) == 0x00002ED0) // srs @ACxAH, $AC0.M
		{
			DestAddr -= 1;
			W32((u32)ptr + (DestAddr)* 2, (Tmp & 0xFFFF0000) | 0x3E00); //  orc AC0.M AC1.M
		}
		if (DestAddr == Dup2 + CallLength) // CopyLength must be even
		{
			DestAddr = Dup1 + CallLength + MoveLength - CopyLength + New1Length;
			DestAddr -= 2;
			W32((u32)ptr + (DestAddr)* 2, 0x029F0000 | (Dup2 + CallLength)); // jmp Dup2+4
		}
	}
	W32((u32)ptr + (Dup1 + 0x00) * 2, 0x02BF0000 | (Dup1 + CallLength)); // call Dup1+4
	W32((u32)ptr + (Dup1 + 0x02) * 2, 0x029F0000 | (Dup1 + MoveLength)); // jmp Dup1+0x22
	W32((u32)ptr + (Dup2 + 0x00) * 2, 0x02BF0000 | (Dup1 + CallLength)); // call Dup1+4
	W32((u32)ptr + (Dup2 + 0x02) * 2, 0x029F0000 | (Dup2 + MoveLength)); // jmp Dup2+0x22
	Tmp = R32((u32)ptr + (Dup1 + 0x98) * 2); // original instructions
	W32((u32)ptr + (Dup1 + 0x98) * 2, 0x02BF0000 | (Dup2PAddr)); // call Dup2PAddr
	W32((u32)ptr + (Dup2 + Dup2Offset) * 2, 0x02BF0000 | (Dup2PAddr)); // call Dup2PAddr

	W16((u32)ptr + (Dup2PAddr + 0x00) * 2, Tmp >> 16); //  original instructions (Start of Dup2PAddr [0xB long])
	W32((u32)ptr + (Dup2PAddr + 0x01) * 2, 0x27D10340); // lrs         $AC1.M, @SampleFormat -
	W32((u32)ptr + (Dup2PAddr + 0x03) * 2, 0x00038100); // andi        $AC1.M, #0x0003 clr         $ACC0
	W32((u32)ptr + (Dup2PAddr + 0x05) * 2, 0x009E1FFF); // lri         $AC0.M, #0x1FFF
	W16((u32)ptr + (Dup2PAddr + 0x07) * 2, 0x02CA);     // lsrn
	W16((u32)ptr + (Dup2PAddr + 0x08) * 2, Tmp & 0xFFFF); //  original instructions
	W32((u32)ptr + (Dup2PAddr + 0x09) * 2, 0x3D0002DF); // andc        $AC1.M, $AC0.M ret

	if (Dup3 != AX_DSP_NO_DUP3)
	{
		W32((u32)ptr + (Dup3 + 0x00) * 2, 0x02BF0000 | (Dup1 + CallLength)); // call Dup1+4
		W32((u32)ptr + (Dup3 + 0x02) * 2, 0x029F0000 | (Dup3 + MoveLength)); // jmp Dup3+0x22

		W32((u32)ptr + (Dup3 + 0x04) * 2, 0x27D10340); // lrs         $AC1.M, @SampleFormat -
		W32((u32)ptr + (Dup3 + 0x06) * 2, 0x00038100); // andi        $AC1.M, #0x0003 clr         $ACC0
		W32((u32)ptr + (Dup3 + 0x08) * 2, 0x009E1FFF); // lri         $AC0.M, #0x1FFF
		W16((u32)ptr + (Dup3 + 0x0A) * 2, 0x02CA);     // lsrn
		Tmp = R32((u32)ptr + (Dup3 + 0x5D) * 2); // original instructions
		W16((u32)ptr + (Dup3 + 0x0B) * 2, Tmp >> 16); //  original instructions
		W16((u32)ptr + (Dup3 + 0x0C) * 2, 0x3D00); // andc        $AC1.M, $AC0.M
		W16((u32)ptr + (Dup3 + 0x0D) * 2, Tmp & 0xFFFF); //  original instructions
		Tmp = R32((u32)ptr + (Dup3 + 0x5F) * 2); // original instructions
		W32((u32)ptr + (Dup3 + 0x0E) * 2, Tmp); //  original instructions includes ret

		W32((u32)ptr + (Dup3 + 0x5D) * 2, 0x029F0000 | (Dup3 + CallLength)); // jmp Dup3+0x4
	}
	return;
}

void PatchZelda_Dsp(u32 DspAddress, u32 PatchAddr, u32 OrigAddress, bool Split, bool KeepOrig)
{
	u32 Tmp = R32(DspAddress + (OrigAddress + 0) * 2); // original instructions at OrigAddress
	if (Split)
	{
		W32(DspAddress + (PatchAddr + 0) * 2, (Tmp & 0xFFFF0000) | 0x00000260); // split original instructions at OrigAddress
		W32(DspAddress + (PatchAddr + 2) * 2, 0x10000000 | (Tmp & 0x0000FFFF)); // ori $AC0.M, 0x1000 in between
	}
	else
	{
		W32(DspAddress + (PatchAddr + 0) * 2, 0x02601000); // ori $AC0.M, 0x1000
		W32(DspAddress + (PatchAddr + 2) * 2, Tmp);        // original instructions at OrigAddress
	}
	if (KeepOrig)
	{
		Tmp = R32(DspAddress + (PatchAddr + 4) * 2);       // original instructions at end of patch
		Tmp = (Tmp & 0x0000FFFF) | 0x02DF0000;             // ret/
	}
	else
		Tmp = 0x02DF02DF;                                  // ret/ret
	W32(DspAddress + (PatchAddr + 4) * 2, Tmp);
	W32(DspAddress + (OrigAddress + 0) * 2, 0x02BF0000 | PatchAddr);  // call PatchAddr
}
void PatchZeldaInPlace_Dsp(u32 DspAddress, u32 OrigAddress)
{
	W32(DspAddress + (OrigAddress + 0) * 2, 0x009AFFFF); // lri $AX0.H, #0xffff
	W32(DspAddress + (OrigAddress + 2) * 2, 0x2AD62AD7); // srs @ACEAH, $AX0.H/srs @ACEAL, $AX0.H
	W32(DspAddress + (OrigAddress + 4) * 2, 0x02601000); // ori $AC0.M, 0x1000
}

void DoDSPPatch( char *ptr, u32 Version )
{
	switch (Version)
	{
		case 0:		// Zelda:WW
		{
			// 5F8 - part of halt routine
			PatchZelda_Dsp( (u32)ptr, 0x05F8, 0x05B1, true, false );
			PatchZeldaInPlace_Dsp( (u32)ptr, 0x0566 );
		} break;
		case 1:		// Mario Sunshine
		{
			// E66 - unused
			PatchZelda_Dsp( (u32)ptr, 0xE66, 0x531, false, false );
			PatchZelda_Dsp( (u32)ptr, 0xE66, 0x57C, false, false );  // same orig instructions
		} break;
		case 2:		// SSBM
		{
			PatchAX_Dsp( (u32)ptr, 0x5A8, 0x65D, 0x707, 0x8F );
		} break;
		case 3:		// Mario Party 5
		{
			PatchAX_Dsp( (u32)ptr, 0x6A3, 0x758, 0x802, 0x8F );
		} break;
		case 4:		// Beyond Good and Evil
		{
			PatchAX_Dsp( (u32)ptr, 0x6E0, 0x795, 0x83F, 0x8F );
		} break;
		case 5:		// Mario Kart Double Dash
		{		
			// 5F8 - part of halt routine
			PatchZelda_Dsp( (u32)ptr, 0x05F8, 0x05B1, true, false );
			PatchZeldaInPlace_Dsp( (u32)ptr, 0x0566 );
		} break;
		case 6:		// Star Fox Assault
		{
			PatchAX_Dsp( (u32)ptr, 0x69E, 0x753, 0x814, 0xA4 );
		} break;
		case 7:		// 0 Length DSP
		{
			dbgprintf( "Error! 0 Length DSP\r\n" );
		} break;
		case 8:		// Donkey Kong Jungle Beat
		{
			// 6E5 - part of halt routine
			PatchZelda_Dsp( (u32)ptr, 0x06E5, 0x069E, true, false );
			PatchZeldaInPlace_Dsp( (u32)ptr, 0x0653 );
		} break; 
		case 9:		// Paper Mario The Thousand Year Door
		{
			PatchAX_Dsp( (u32)ptr, 0x69E, 0x753, 0x7FD, 0x8F );
		} break;
		case 10:	// Animal Crossing
		{
			// CF4 - unused
			PatchZelda_Dsp( (u32)ptr, 0x0CF4, 0x00C0, false, false );
			PatchZelda_Dsp( (u32)ptr, 0x0CF4, 0x010B, false, false );  // same orig instructions
		} break;
		case 11:	// Luigi
		{
			// BE8 - unused
			PatchZelda_Dsp( (u32)ptr, 0x0BE8, 0x00BE, false, false );
			PatchZelda_Dsp( (u32)ptr, 0x0BE8, 0x0109, false, false );  // same orig instructions
		} break;
		case 12:	// Pikmin PAL
		{
			// D2B - unused
			PatchZelda_Dsp( (u32)ptr, 0x0D2B, 0x04A8, false, true );
			PatchZelda_Dsp( (u32)ptr, 0x0D2B, 0x04F3, false, true );  // same orig instructions
		} break;
		case 13:	// IPL NTSC v1.2
		{
			// BA8 - unused
			PatchZelda_Dsp( (u32)ptr, 0x0BA8, 0x00BE, false, false );
			PatchZelda_Dsp( (u32)ptr, 0x0BA8, 0x0109, false, false );  // same orig instructions
		} break;
		case 14:	// IPL NTSC v1.0
		{
			// 938 - unused
			PatchZelda_Dsp( (u32)ptr, 0x0938, 0x00BE, false, false );
			PatchZelda_Dsp( (u32)ptr, 0x0938, 0x0109, false, false );  // same orig instructions
		} break;
		case 15:	// Metroid Prime Trilogy Wii (needed?)
		{
			PatchAX_Dsp( (u32)ptr, 0x69E, 0x753, AX_DSP_NO_DUP3, 0xA4 );
		} break;
		default:
		{
		} break;
	}
}

static bool write32A( u32 Offset, u32 Value, u32 CurrentValue, u32 ShowAssert )
{
	if( read32(Offset) != CurrentValue )
	{
		#ifdef DEBUG_PATCH
		//if( ShowAssert)
			//dbgprintf("AssertFailed: Offset:%08X is:%08X should:%08X\r\n", Offset, read32(Offset), CurrentValue );
		#endif
		return false;
	}

	write32( Offset, Value );
	return true;
}
void PatchB( u32 dst, u32 src )
{
	u32 newval = (dst - src);
	newval&= 0x03FFFFFC;
	newval|= 0x48000000;
	*(vu32*)src = newval;
}
void PatchBL( u32 dst, u32 src )
{
	u32 newval = (dst - src);
	newval&= 0x03FFFFFC;
	newval|= 0x48000001;
	*(vu32*)src = newval;
}
/*
	This offset gets randomly overwritten, this workaround fixes that problem.
*/
void Patch31A0( void )
{
	u32 PatchOffset = PATCH_OFFSET_START;

	u32 i;
	for (i = 0; i < (4 * 0x04); i+=0x04)
	{
		u32 Orig = *(vu32*)(0x319C + i);
		if ((Orig & 0xFC000002) == 0x40000000)
		{
			u32 NewAddr = (((s16)(Orig & 0xFFFC)) + 0x319C - PatchOffset);
			Orig = (Orig & 0xFFFF0003) | NewAddr;
#ifdef DEBUG_PATCH
			dbgprintf("Patch:[Patch31A0] applied (0x%08X), 0x%08X=0x%08X\r\n", 0x319C + i, PatchOffset + i, Orig);
#endif
		}
		else if ((Orig & 0xFC000002) == 0x48000000)
		{
			u32 BaseAddr = (Orig & 0x3FFFFFC);
			if(BaseAddr & 0x2000000) BaseAddr |= 0xFC000000;
			u32 NewAddr = (((s32)BaseAddr) + 0x319C - PatchOffset) & 0x3FFFFFC;
			Orig = (Orig & 0xFC000003) | NewAddr;
#ifdef DEBUG_PATCH
			dbgprintf("Patch:[Patch31A0] applied (0x%08X), 0x%08X=0x%08X\r\n", 0x319C + i, PatchOffset + i, Orig);
#endif
		}
		*(vu32*)(PatchOffset + i) = Orig;
	}

	PatchB( PatchOffset, 0x319C );
	PatchB( 0x31AC, PatchOffset+0x10 );
}

u32 PatchCopy(const u8 *PatchPtr, const u32 PatchSize)
{
	POffset -= PatchSize;
	memcpy( (void*)POffset, PatchPtr, PatchSize );
	return POffset;
}

void PatchFuncInterface( char *dst, u32 Length )
{
	u32 SIPatched = 0;
	u32 EXIPatched = 0;
	u32 AIPatched = 0;

	int i;

	u32 LISReg=-1;
	u32 LISOff=-1;

	for( i=0; i < Length; i+=4 )
	{
		u32 op = read32( (u32)dst + i );

		if( (op & 0xFC1FFFFF) == 0x3C00CC00 )	// lis rX, 0xCC00
		{
			LISReg = (op & 0x3E00000) >> 21;
			LISOff = (u32)dst + i;
		}

		if( (op & 0xFC000000) == 0x38000000 )	// li rX, x
		{
			u32 src = (op >> 16) & 0x1F;
			u32 dst = (op >> 21) & 0x1F;
			if ((src != LISReg) && (dst == LISReg))
			{
				LISReg = -1;
				LISOff = (u32)dst + i;
			}
		}

		if( (op & 0xFC00FF00) == 0x38006C00 ) // addi rX, rY, 0x6C00 (ai)
		{
			u32 src = (op >> 16) & 0x1F;
			if( src == LISReg )
			{
				write32( (u32)LISOff, (LISReg<<21) | 0x3C00CD80 );	// Patch to: lis rX, 0xCD80
				//dbgprintf("AI:[%08X] %08X: lis r%u, 0xCD80\r\n", (u32)LISOff, read32( (u32)LISOff), LISReg );
				AIPatched++;
				LISReg = -1;
			}
			u32 dst = (op >> 21) & 0x1F;
			if( dst == LISReg )
				LISReg = -1;
		}
		else if( (op & 0xFC00FF00) == 0x38006400 ) // addi rX, rY, 0x6400 (si)
		{
			u32 src = (op >> 16) & 0x1F;
			if( src == LISReg )
			{
				if (ConfigGetConfig(NIN_CFG_NATIVE_SI))
				{
					write32((u32)LISOff, (LISReg << 21) | 0x3C00CD80);	// Patch to: lis rX, 0xCD80
					//dbgprintf("SI:[%08X] %08X: lis r%u, 0xCD80\r\n", (u32)LISOff, read32( (u32)LISOff), LISReg );
				}
				else
				{
					write32((u32)LISOff, (LISReg << 21) | 0x3C00D302);	// Patch to: lis rX, 0xD302
					//dbgprintf("SI:[%08X] %08X: lis r%u, 0xD302\r\n", (u32)LISOff, read32( (u32)LISOff), LISReg );
				}
				SIPatched++;
				LISReg = -1;
			}
			u32 dst = (op >> 21) & 0x1F;
			if( dst == LISReg )
				LISReg = -1;
		} else if( (op & 0xFC00FF00) == 0x38006800 ) // addi rX, rY, 0x6800 (exi)
		{
			u32 src = (op >> 16) & 0x1F;
			if( src == LISReg )
			{
				if( ConfigGetConfig(NIN_CFG_MEMCARDEMU) == false )
				{
					write32( (u32)LISOff, (LISReg<<21) | 0x3C00CD80 );	// Patch to: lis rX, 0xCD80
					//dbgprintf("EXI:[%08X] %08X: lis r%u, 0xCD80\r\n", (u32)LISOff, read32( (u32)LISOff), LISReg );
					EXIPatched++;
					LISReg = -1;
				}
			}
			u32 dst = (op >> 21) & 0x1F;
			if( dst == LISReg )
				LISReg = -1;
		}

		if(    ( (op & 0xF8000000 ) == 0x80000000 )   // lwz and lwzu
		    || ( (op & 0xF8000000 ) == 0x90000000 ) ) // stw and stwu
		{
			u32 src = (op >> 16) & 0x1F;
			u32 dst = (op >> 21) & 0x1F;
			u32 val = op & 0xFFFF;
			
			if( src == LISReg )
			{
				if( (val & 0xFF00) == 0x6C00 ) // case with 0x6CXY(rZ) (ai)
				{
					write32( (u32)LISOff, (LISReg<<21) | 0x3C00CD80 );	// Patch to: lis rX, 0xCD80
					//dbgprintf("AI:[%08X] %08X: lis r%u, 0xCD80\r\n", (u32)LISOff, read32( (u32)LISOff), LISReg );
					AIPatched++;
					LISReg = -1;
				}
				else if((val & 0xFF00) == 0x6400) // case with 0x64XY(rZ) (si)
				{
					if (ConfigGetConfig(NIN_CFG_NATIVE_SI))
					{
						write32((u32)LISOff, (LISReg << 21) | 0x3C00CD80);	// Patch to: lis rX, 0xCD80
						//dbgprintf("SI:[%08X] %08X: lis r%u, 0xCD80\r\n", (u32)LISOff, read32( (u32)LISOff), LISReg );
					}
					else
					{
						write32((u32)LISOff, (LISReg << 21) | 0x3C00D302);	// Patch to: lis rX, 0xD302
						//dbgprintf("SI:[%08X] %08X: lis r%u, 0xD302\r\n", (u32)LISOff, read32( (u32)LISOff), LISReg );
					}
					SIPatched++;
					LISReg = -1;
				}
				else if((val & 0xFF00) == 0x6800) // case with 0x68XY(rZ) (exi)
				{
					if( ConfigGetConfig(NIN_CFG_MEMCARDEMU) == false )
					{
						write32( (u32)LISOff, (LISReg<<21) | 0x3C00CD80 );	// Patch to: lis rX, 0xCD80
						//dbgprintf("EXI:[%08X] %08X: lis r%u, 0xCD80\r\n", (u32)LISOff, read32( (u32)LISOff), LISReg );
						EXIPatched++;
					}
					LISReg = -1;
				}
			}

			if( dst == LISReg )
				LISReg = -1;
		}

		if( op == 0x4E800020 )	// blr
		{
			LISReg=-1;
		}
	}
#ifdef DEBUG_PATCH
	dbgprintf("Patch:[SI] applied %u times\r\n", SIPatched);
	if( ConfigGetConfig(NIN_CFG_MEMCARDEMU) == false )
		dbgprintf("Patch:[EXI] applied %u times\r\n", EXIPatched);
	dbgprintf("Patch:[AI] applied %u times\r\n", AIPatched);
	#endif
}
s32 piReg = -1;
void PatchProcessorInterface( char *ptr )
{
	/* Pokemon XD - PI_FIFO_WP Direct */
	if(*(u32*)(ptr) == 0x80033014 && (*(u32*)(ptr+12) & 0xFC00FFFF) == 0x540037FE) //extrwi rZ, rY, 1,5
	{
		// Extract WRAPPED bit
		u32 op = read32((u32)(ptr+12));
		W16((u32)(ptr+12+2), 0x1FFE);
		#ifdef DEBUG_PATCH
		u32 src = (op >> 21) & 0x1F; //rY
		u32 dst = (op >> 16) & 0x1F; //rZ
		dbgprintf( "Patch:[PI_FIFO_WP PKM] extrwi r%i,r%i,1,2 (0x%08X)\r\n", dst, src, ptr+12 );
		#endif
		return;
	}
	/* Dolphin SDK - GX __piReg */
	if(piReg == -1)
		return;
	if((*(u32*)(ptr) & 0xFC0FFFFF) == (0x800D0000 | piReg)) // lwz rX, __piReg(r13)
	{
		u32 dst = (read32((u32)ptr) >> 21) & 0x1F; //rX
		u32 piRegBuf = (u32)ptr - 4;
		u32 lPtr, sPtr;
		s32 lReg = -1, sReg = -1;
		for(lPtr = 0; lPtr < 0x20; lPtr += 4)
		{
			u32 op = read32(piRegBuf+lPtr);
			if(lReg == -1)
			{
				if((op & 0xFC00FFFF) == 0x80000014) //lwz rY, 0x14(rX)
				{
					u32 src = (op >> 16) & 0x1F;
					if(src == dst)
						lReg = (op >> 21) & 0x1F; //rY
				}
			}
			else
			{
				if((op & 0xFC00FFFF) == 0x54000188) //rlwinm rZ, rY, 0,6,4
				{
					u32 lSrc = (op >> 21) & 0x1F; //rY
					if(lSrc == lReg)
					{
						// Keep WRAPPED bit
						W16(piRegBuf+lPtr+2, 0x00C2);
						#ifdef DEBUG_PATCH
						u32 lDst = (op >> 16) & 0x1F; //rZ
						dbgprintf( "Patch:[PI_FIFO_WP] rlwinm r%i,r%i,0,3,1 (0x%08X)\r\n", lDst, lSrc, piRegBuf+lPtr );
						#endif
						break;
					}
				}
				else if((op & 0xFC00FFFF) == 0x540037FE) //extrwi rZ, rY, 1,5
				{
					u32 lSrc = (op >> 21) & 0x1F; //rY
					if(lSrc == lReg)
					{
						// Extract WRAPPED bit
						W16(piRegBuf+lPtr+2, 0x1FFE);
						#ifdef DEBUG_PATCH
						u32 lDst = (op >> 16) & 0x1F; //rZ
						dbgprintf( "Patch:[PI_FIFO_WP] extrwi r%i,r%i,1,2 (0x%08X)\r\n", lDst, lSrc, piRegBuf+lPtr );
						#endif
						break;
					}
				}
			}
			if(sReg == -1)
			{
				if((op & 0xFC00FFFF) == 0x54000188) //rlwinm rY, rZ, 0,6,4
				{
					sPtr = lPtr;
					sReg = (op >> 16) & 0x1F; //rY
				}
			}
			else
			{
				if((op & 0xFC00FFFF) == 0x90000014) //stw rY, 0x14(rX)
				{
					u32 sDst = (op >> 16) & 0x1F; //rX
					u32 sSrc = (op >> 21) & 0x1F; //rY
					if(sSrc == sReg && dst == sDst)
					{
						// Keep WRAPPED bit
						W16(piRegBuf+sPtr+2, 0x00C2);
						#ifdef DEBUG_PATCH
						sSrc = (read32(piRegBuf+sPtr) >> 21) & 0x1F;
						dbgprintf( "Patch:[PI_FIFO_WP] rlwinm r%i,r%i,0,3,1 (0x%08X)\r\n", sReg, sSrc, piRegBuf+sPtr );
						#endif
						break;
					}
				}
			}
		}
	}
}

void PatchDiscInterface( char *dst )
{
	int i;

	u32 LISReg=-1;
	u32 LISOff=-1;

	for( i=0; ; i+=4 )
	{
		u32 op = read32( (u32)dst + i );

		if( (op & 0xFC1FFFFF) == 0x3C00CC00 )	// lis rX, 0xCC00
		{
			LISReg = (op & 0x3E00000) >> 21;
			LISOff = (u32)dst + i;
		}

		if( (op & 0xFC000000) == 0x38000000 )	// li rX, x
		{
			u32 src = (op >> 16) & 0x1F;
			u32 dst = (op >> 21) & 0x1F;
			if ((src != LISReg) && (dst == LISReg))
			{
				LISReg = -1;
				LISOff = (u32)dst + i;
			}
		}

		if( (op & 0xFC00FF00) == 0x38006000 ) // addi rX, rY, 0x6000 (di)
		{
			u32 src = (op >> 16) & 0x1F;
			if( src == LISReg )
			{
				write32( (u32)LISOff, (LISReg<<21) | 0x3C00CD80 );	// Patch to: lis rX, 0xCD80
				//dbgprintf("DI:[%08X] %08X: lis r%u, 0xCD80\r\n", (u32)LISOff, read32( (u32)LISOff), LISReg );
				LISReg = -1;
			}
			u32 dst = (op >> 21) & 0x1F;
			if( dst == LISReg )
				LISReg = -1;
		}

		if(    ( (op & 0xF8000000 ) == 0x80000000 )   // lwz and lwzu
		    || ( (op & 0xF8000000 ) == 0x90000000 ) ) // stw and stwu
		{
			u32 src = (op >> 16) & 0x1F;
			u32 dst = (op >> 21) & 0x1F;
			u32 val = op & 0xFFFF;

			if( src == LISReg )
			{
				if( (val & 0xFF00) == 0x6000 ) // case with 0x60XY(rZ) (di)
				{
					write32( (u32)LISOff, (LISReg<<21) | 0x3C00CD80 );	// Patch to: lis rX, 0xCD80
					//dbgprintf("DI:[%08X] %08X: lis r%u, 0xCD80\r\n", (u32)LISOff, read32( (u32)LISOff), LISReg );
					LISReg = -1;
				}
			}
			if( dst == LISReg )
				LISReg = -1;
		}
		if( op == 0x4E800020 )	// blr
			break;
	}
}
void PatchFunc( char *ptr )
{
	u32 i	= 0;
	u32 reg=-1;

	while(1)
	{
		u32 op = read32( (u32)ptr + i );

		if( op == 0x4E800020 )	// blr
			break;

		if( (op & 0xFC00FFFF) == 0x3C00CC00 )	// lis rX, 0xCC00
		{
			reg = (op & 0x3E00000) >> 21;
			
			write32( (u32)ptr + i, (reg<<21) | 0x3C00C000 );	// Patch to: lis rX, 0xC000
			//#ifdef DEBUG_PATCH
			//dbgprintf("[%08X] %08X: lis r%u, 0xC000\r\n", (u32)ptr+i, read32( (u32)ptr+i), reg );
			//#endif
		}

		if( (op & 0xFC00FFFF) == 0x3C00A800 )	// lis rX, 0xA800
		{
			write32( (u32)ptr + i, (op & 0x3E00000) | 0x3C00A700 );		// Patch to: lis rX, 0xA700
			//#ifdef DEBUG_PATCH
			//dbgprintf("[%08X] %08X: lis rX, 0xA700\r\n", (u32)ptr+i, read32( (u32)ptr+i) );
			//#endif
		}

		// Triforce
		if( (op & 0xFC00FFFF) == 0x380000A8 )	// li rX, 0xA8
		{
			write32( (u32)ptr + i, (op & 0x3E00000) | 0x380000A7 );		// Patch to: li rX, 0xA7
			//#ifdef DEBUG
			//dbgprintf("[%08X] %08X: li rX, 0xA7\r\n", (u32)ptr+i, read32( (u32)ptr+i) );
			//#endif
		}

		if( (op & 0xFC00FFFF) == 0x38006000 )	// addi rX, rY, 0x6000
		{
			u32 src = (op >> 16) & 0x1F;
			u32 dst = (op >> 21) & 0x1F;

			if( src == reg )
			{
				write32( (u32)ptr + i, (dst<<21) | (src<<16) | 0x38002F00 );	// Patch to: addi rX, rY, 0x2F00
				//#ifdef DEBUG_PATCH
				//dbgprintf("[%08X] %08X: addi r%u, r%u, 0x2F00\r\n", (u32)ptr+i, read32( (u32)ptr+i), dst, src );
				//#endif
			}
		}

		if( (op & 0xFC000000 ) == 0x80000000 )
		{
			u32 src = (op >> 16) & 0x1F;
			u32 dst = (op >> 21) & 0x1F;
			u32 val = op & 0xFFFF;
			
			if( src == reg )
			{
				if( (val & 0xFF00) == 0x6000 )	// case with 0x60XY(rZ)
				{
					write32( (u32)ptr + i,  (dst<<21) | (src<<16) | 0x2F00 | (val&0xFF) | 0x80000000 );	// Patch to: lwz rX, 0x2FXY(rZ)
					//#ifdef DEBUG_PATCH
					//dbgprintf("[%08X] %08X: lwz r%u, 0x%04X(r%u)\r\n", (u32)ptr+i, read32( (u32)ptr+i), dst, 0x2F00 | (val&0xFF), src );
					//#endif
				}
			}
		}
		else if( (op & 0xFC000000 ) == 0x90000000 )
		{
			u32 src = (op >> 16) & 0x1F;
			u32 dst = (op >> 21) & 0x1F;
			u32 val = op & 0xFFFF;
			
			if( src == reg )
			{
				if( (val & 0xFF00) == 0x6000 )	// case with 0x60XY(rZ)
				{
					write32( (u32)ptr + i,  (dst<<21) | (src<<16) | 0x2F00 | (val&0xFF) | 0x90000000 );	// Patch to: stw rX, 0x2FXY(rZ)
					//#ifdef DEBUG_PATCH
					//dbgprintf("[%08X] %08X: stw r%u, 0x%04X(r%u)\r\n", (u32)ptr+i, read32( (u32)ptr+i), dst, 0x2F00 | (val&0xFF), src );
					//#endif
				}
			}
		}
		i += 4;
	}
}

void MPattern( u8 *Data, u32 Length, FuncPattern *FunctionPattern )
{
	u32 i;

	memset32( FunctionPattern, 0, sizeof(FuncPattern) );

	for( i = 0; i < Length; i+=4 )
	{
		u32 word = *(u32*)(Data + i);

		if( (word & 0xFC000003) == 0x48000001 )
			FunctionPattern->FCalls++;

		if( (word & 0xFC000003) == 0x48000000 )
			FunctionPattern->Branch++;
		if( (word & 0xFFFF0000) == 0x40800000 )
			FunctionPattern->Branch++;
		if( (word & 0xFFFF0000) == 0x41800000 )
			FunctionPattern->Branch++;
		if( (word & 0xFFFF0000) == 0x40810000 )
			FunctionPattern->Branch++;
		if( (word & 0xFFFF0000) == 0x41820000 )
			FunctionPattern->Branch++;

		if( (word & 0xFC000000) == 0x80000000 )
			FunctionPattern->Loads++;
		if( (word & 0xFF000000) == 0x38000000 )
			FunctionPattern->Loads++;
		if( (word & 0xFF000000) == 0x3C000000 )
			FunctionPattern->Loads++;

		if( (word & 0xFC000000) == 0x90000000 )
			FunctionPattern->Stores++;
		if( (word & 0xFC000000) == 0x94000000 )
			FunctionPattern->Stores++;

		if( (word & 0xFF000000) == 0x7C000000 )
			FunctionPattern->Moves++;

		if( word == 0x4E800020 )
			break;
	}

	FunctionPattern->Length = i;
}

int CPattern( FuncPattern *FPatA, FuncPattern *FPatB  )
{
	return ( memcmp( FPatA, FPatB, sizeof(u32) * 6 ) == 0 );
}

void SRAM_Checksum( unsigned short *buf, unsigned short *c1, unsigned short *c2) 
{
	u32 i;
	*c1 = 0; *c2 = 0;
	for (i = 0;i<4;++i)
	{
		*c1 += buf[0x06 + i];
		*c2 += (buf[0x06 + i] ^ 0xFFFF);
	}
	//dbgprintf("New Checksum: %04X %04X\r\n", *c1, *c2 );
}

char *getVidStr(u32 in)
{
	switch(in)
	{
		case VI_NTSC:
			return "NTSC";
		case VI_PAL:
			return "PAL50";
		case VI_EUR60:
			return "PAL60";
		case VI_MPAL:
			return "PAL-M";
		case VI_480P:
			return "480p";
		default:
			return "Unknown Video Mode";
	}
}

#ifdef DEBUG_PATCH
static inline void printvidpatch(u32 ori_v, u32 new_v, u32 addr)
{
	dbgprintf("Patch:Replaced %s with %s (0x%08X)\r\n", getVidStr(ori_v), getVidStr(new_v), addr );
}
static inline void printpatchfound(const char *name, const char *type, u32 offset)
{
	if(type) dbgprintf("Patch:[%s %s] applied (0x%08X)\r\n", name, type, offset);
	else dbgprintf("Patch:[%s] applied (0x%08X)\r\n", name, offset);
}
#else
#define printvidpatch(...)
#define printpatchfound(...)
#endif

#define PATCH_STATE_NONE  0
#define PATCH_STATE_LOAD  1
#define PATCH_STATE_PATCH 2
#define PATCH_STATE_DONE  4

u32 PatchState = PATCH_STATE_NONE;
u32 PSOHack    = 0;
u32 ELFLoading = 0;
u32 DOLSize    = 0;
u32 DOLMinOff  = 0;
u32 DOLMaxOff  = 0;
vu32 TRIGame   = 0;
vu32 GameEntry = 0;

void DoPatches( char *Buffer, u32 Length, u32 DiscOffset )
{
	int i, j, k;
	u32 read;
	u32 value = 0;

	// PSO 1&2
	if( (TITLE_ID) == 0x47504F )
	{
		switch( DiscOffset )
		{
			case 0x56B8E7E0:	// AppSwitcher	[EUR]
			case 0x56C49600:	// [USA] v1.1
			case 0x56C4C980:	// [USA] v1.0
			{
				PatchState	= PATCH_STATE_PATCH;
				DOLSize		= Length;
				#ifdef DEBUG_PATCH
				dbgprintf("Patch:PSO 1&2 loading AppSwitcher:0x%p %u\r\n", Buffer, Length );
				#endif
			} break;
			case 0x5668FE20:	// psov3.dol [EUR]
			case 0x56750660:	// [USA] v1.1
			case 0x56753EC0:	// [USA] v1.0
			{
				#ifdef DEBUG_PATCH
				dbgprintf("Patch:PSO 1&2 loading psov3.dol:0x%p %u\r\n", Buffer, Length );
				#endif

				PSOHack = 1;
			} break;
		}
	}

	if( (PatchState & 0x3) == PATCH_STATE_NONE )
	{
		if( Length == 0x100 || PSOHack )
		{
			if( read32( (u32)Buffer ) == 0x100 )
			{
				ELFLoading = 0;
				//quickly calc the size
				DOLSize = sizeof(dolhdr);
				dolhdr *dol = (dolhdr*)Buffer;

				for( i=0; i < 7; ++i )
					DOLSize += dol->sizeText[i];
				for( i=0; i < 11; ++i )
					DOLSize += dol->sizeData[i];

				DOLMinOff=0x81800000;
				DOLMaxOff=0;

				for( i=0; i < 7; ++i )
				{
					if( dol->addressText[i] == 0 )
						continue;

					if( DOLMinOff > dol->addressText[i])
						DOLMinOff = dol->addressText[i];

					if( DOLMaxOff < dol->addressText[i] + dol->sizeText[i] )
						DOLMaxOff = dol->addressText[i] + dol->sizeText[i];
				}

				for( i=0; i < 11; ++i )
				{
					if( dol->addressData[i] == 0 )
						continue;

					if( DOLMinOff > dol->addressData[i])
						DOLMinOff = dol->addressData[i];

					if( DOLMaxOff < dol->addressData[i] + dol->sizeData[i] )
						DOLMaxOff = dol->addressData[i] + dol->sizeData[i];
				}

				DOLMinOff -= 0x80000000;
				DOLMaxOff -= 0x80000000;

				if( PSOHack )
				{
					DOLMinOff = (u32)Buffer;
					DOLMaxOff = (u32)Buffer + DOLSize;
				}
#ifdef DEBUG_DI
				dbgprintf("DIP:DOL Size:%d MinOff:0x%08X MaxOff:0x%08X\r\n", DOLSize, DOLMinOff, DOLMaxOff );
#endif
				/* Hack Position */
				GameEntry = dol->entrypoint;
				dol->entrypoint = PATCH_OFFSET_ENTRY + 0x80000000;
				PatchState |= PATCH_STATE_LOAD;
			}
			PSOHack = 0;
		}
		else if( read32( (u32)Buffer ) == 0x7F454C46 && ((Elf32_Ehdr*)Buffer)->e_phnum )
		{
			ELFLoading = 1;
#ifdef DEBUG_DI
			dbgprintf("DIP:Game is loading an ELF 0x%08x (%u)\r\n", DiscOffset, Length );
#endif
			DOLSize = 0;

			Elf32_Ehdr *ehdr = (Elf32_Ehdr*)Buffer;
#ifdef DEBUG_DI
			dbgprintf("DIP:ELF Start:0x%08X Programheader Entries:%u\r\n", ehdr->e_entry, ehdr->e_phnum );
#endif
			DOLMinOff = 0x81800000;
			DOLMaxOff = 0;
			for( i=0; i < ehdr->e_phnum; ++i )
			{
				Elf32_Phdr *phdr = (Elf32_Phdr*)(Buffer + ehdr->e_phoff + (i * sizeof(Elf32_Phdr)));
#ifdef DEBUG_DI
				dbgprintf("DIP:ELF Programheader:0x%08X\r\n", phdr );
#endif
				if( DOLMinOff > phdr->p_vaddr)
					DOLMinOff = phdr->p_vaddr;

				if( DOLMaxOff < phdr->p_vaddr + phdr->p_filesz )
					DOLMaxOff = phdr->p_vaddr + phdr->p_filesz;

				DOLSize += (phdr->p_filesz+31) & (~31);	// align by 32byte
			}
			DOLMinOff -= 0x80000000;
			DOLMaxOff -= 0x80000000;
#ifdef DEBUG_DI
			dbgprintf("DIP:ELF Size:%d MinOff:0x%08X MaxOff:0x%08X\r\n", DOLSize, DOLMinOff, DOLMaxOff );
#endif
			/* Hack Position */
			GameEntry = ehdr->e_entry;
			ehdr->e_entry = PATCH_OFFSET_ENTRY + 0x80000000;
			PatchState |= PATCH_STATE_LOAD;
		}
	}

	if( PatchState == PATCH_STATE_DONE && (u32)Buffer == 0x01300000 && *(u8*)Buffer == 0x48 )
	{	/* Make sure to force patch that area */
		PatchState = PATCH_STATE_PATCH;
		DOLSize = Length;
		DOLMinOff = (u32)Buffer;
		DOLMaxOff = DOLMinOff + Length;
	}

	if (!(PatchState & PATCH_STATE_PATCH))
		return;
	piReg = -1;

	sync_after_write(Buffer, Length);

	Buffer = (char*)DOLMinOff;
	Length = DOLMaxOff - DOLMinOff;

#ifdef DEBUG_PATCH
	dbgprintf("Patch:Offset:0x%08X EOffset:0x%08X Length:%08X\r\n", Buffer, DOLMaxOff, Length );
	dbgprintf("Patch:Game ID = %x\r\n", read32(0));
#endif

	sync_before_read(Buffer, Length);

	/* Have gc patches loaded at the same place for reloading games */
	POffset = PATCH_OFFSET_ENTRY;

	u32 DCInvalidateRangeAddr = PatchCopy(DCInvalidateRange, DCInvalidateRange_size);

	u32 FakeInterruptAddr = PatchCopy(FakeInterrupt, FakeInterrupt_size);
	u32 FakeInterrupt_DBGAddr = PatchCopy(FakeInterrupt_DBG, FakeInterrupt_DBG_size);
	u32 TCIntrruptHandlerAddr = PatchCopy(TCIntrruptHandler, TCIntrruptHandler_size);
	u32 SIIntrruptHandlerAddr = PatchCopy(SIIntrruptHandler, SIIntrruptHandler_size);

	u32 SIInitStoreAddr = PatchCopy(SIInitStore, SIInitStore_size);
	u32 FakeRSWLoadAddr = PatchCopy(FakeRSWLoad, FakeRSWLoad_size);
	u32 FakeRSWStoreAddr = PatchCopy(FakeRSWStore, FakeRSWStore_size);

	u32 AIInitDMAAddr = PatchCopy(AIInitDMA, AIInitDMA_size);
	u32 __DSPHandlerAddr = PatchCopy(__DSPHandler, __DSPHandler_size);

	// HACK: PokemonXD and Pokemon Colosseum low memory clear patch
	if(( TITLE_ID == 0x475858 ) || ( TITLE_ID == 0x474336 ))
	{
		// patch out initial memset(0x1800, 0, 0x1800)
		if( (read32(0) & 0xFF) == 0x4A )	// JAP
			write32( 0x560C, 0x60000000 );
		else								// EUR/USA
			write32( 0x5614, 0x60000000 );

		// patch memset to jump to test function
		write32(0x00005498, 0x4BFFABF0);

		// patch in test < 0x3000 function
		write32(0x00000088, 0x3D008000);
		write32(0x0000008C, 0x61083000);
		write32(0x00000090, 0x7C044000);
		write32(0x00000094, 0x4180542C);
		write32(0x00000098, 0x90E40004);
		write32(0x0000009C, 0x48005400);

		// skips __start init of debugger mem
		write32(0x00003194, 0x48000028);
	}

	if (read32(0x02856EC) == 0x386000A8)
	{
		dbgprintf("TRI:Mario kart GP 2\r\n");
		TRIGame = 1;
		SystemRegion = REGION_JAPAN;

		//Skip device test
		write32( 0x002E340, 0x60000000 );
		write32( 0x002E34C, 0x60000000 );

		//Disable wheel
		write32( 0x007909C, 0x98650022 );

		//Disable CARD
		write32( 0x0073BF4, 0x98650023 );
		write32( 0x0073C10, 0x98650023 );

		//Disable cam
		write32( 0x0073BD8, 0x98650025 );

		//VS wait patch 
		write32( 0x0084FC4, 0x4800000C );
		write32( 0x0085000, 0x60000000 );

		//Game vol
		write32( 0x00790E8, 0x39000009 );
		//Attract vol
		write32( 0x00790F4, 0x38C0000C );

		//Disable Commentary (sets volume to 0 )
		write32( 0x001B6510, 0x38800000 );

		//Patches the analog input count
		write32( 0x000392F4, 0x38000003 );

		PatchB( PatchCopy(PADReadB, PADReadB_size), 0x0038EF4 );
		memcpy( (void*)0x028A128, PADReadSteer, PADReadSteer_size );

		//memcpy( (void*)0x002CE3C, OSReportDM, sizeof(OSReportDM) );
		//memcpy( (void*)0x002CE8C, OSReportDM, sizeof(OSReportDM) );
		//write32( 0x002CEF8, 0x60000000 );
	}

	if( read32( 0x0210C08 ) == 0x386000A8 )	// Virtua Striker 4 Ver 2006 (EXPORT)
	{
		dbgprintf("TRI:Virtua Striker 4 Ver 2006 (EXPORT)\n");
		TRIGame = 2;
		SystemRegion = REGION_USA;

		memcpy( (void*)0x0208314, PADReadSteerVSSimple, sizeof(PADReadSteerVSSimple) );	
		memcpy( (void*)0x0208968, PADReadVSSimple, sizeof(PADReadVSSimple) );

		//memcpy( (void*)0x001C2B80, OSReportDM, sizeof(OSReportDM) );
	}

	if( read32(0x01851C4) == 0x386000A8 )	// FZero AX
	{
		dbgprintf("TRI:FZero AX\n");
		TRIGame = 3;
		SystemRegion = REGION_JAPAN;

		//Patch EXI ID check
		write32( 0x01AD60C, 0x4800001C );

		//Reset loop
		write32( 0x01B5410, 0x60000000 );
		//DBGRead fix
		write32( 0x01BEF38, 0x60000000 );

		//Disable CARD
		write32( 0x017B2BC, 0x38C00000 );

		//Motor init patch
		write32( 0x0175710, 0x60000000 );
		write32( 0x0175714, 0x60000000 );
		write32( 0x01756AC, 0x60000000 );

		//patching system waiting stuff
		write32( 0x0180DB8, 0x48000054 );
		write32( 0x0180E1C, 0x48000054 );

		//Network waiting
		write32( 0x0180FD8, 0x4800004C );

		//Goto Test menu
		//write32( 0x00DF3D0, 0x60000000 );

		//English 
		write32( 0x000DF698, 0x38000000 );

		//GXProg patch
		memcpy( (void*)0x0302AC0, (void*)0x0302A84, 0x3C );
		memcpy( (void*)0x021D3EC, (void*)0x0302A84, 0x3C );

		//Unkreport
		PatchB( 0x01882C0, 0x0191B54 );
		PatchB( 0x01882C0, 0x01C53CC );
		PatchB( 0x01882C0, 0x01CC684 );

		memcpy( (void*)0x001B3D10, PADReadSteerF, PADReadSteerF_size );
		memcpy( (void*)0x001B4340, PADReadF, PADReadF_size );

		//memcpy( (void*)0x01CAACC, patch_fwrite_GC, sizeof(patch_fwrite_GC) );
		//memcpy( (void*)0x01882C0, OSReportDM, sizeof(OSReportDM) );
	}
	if( read32(0x023D240) == 0x386000A8 )	// Mario kart GP (TRI)
	{
		dbgprintf("TRI:Mario kart GP\n");
		TRIGame = 4;
		SystemRegion = REGION_JAPAN;

		//Reset skip
		write32( 0x024F95C, 0x60000000 );

		//Unlimited CARD uses
		write32( 0x01F5C44, 0x60000000 );

		//Disable cam
		write32( 0x00790A0, 0x98650025 );

		//Disable CARD
		write32( 0x00790B4, 0x98650023 );
		write32( 0x00790CC, 0x98650023 );

		//Disable wheel/handle
		write32( 0x007909C, 0x98650022 );

		//VS wait
		write32( 0x00BE10C, 0x4800002C );

		//cam loop
		write32( 0x009F1E0, 0x60000000 );

		//Skip device test
		write32( 0x0031BF0, 0x60000000 );
		write32( 0x0031BFC, 0x60000000 );

		//GXProg patch
		memcpy( (void*)0x036369C, (void*)0x040EB88, 0x3C );

		PatchB( PatchCopy(PADReadB, PADReadB_size), 0x003C6F0 );
		memcpy( (void*)0x024E4B0, PADReadSteer, PADReadSteer_size );

		//some report check skip
		//write32( 0x00307CC, 0x60000000 );

		//memcpy( (void*)0x00330BC, OSReportDM, sizeof(OSReportDM) );
		//memcpy( (void*)0x0030710, OSReportDM, sizeof(OSReportDM) );
		//memcpy( (void*)0x0030760, OSReportDM, sizeof(OSReportDM) );
		//memcpy( (void*)0x020CBCC, OSReportDM, sizeof(OSReportDM) );	// UNkReport
		//memcpy( (void*)0x02281B8, OSReportDM, sizeof(OSReportDM) );	// UNkReport4
	}

	PatchFuncInterface( Buffer, Length );
	sync_after_write( Buffer, Length );
	sync_before_read( Buffer, Length );

#ifdef PATCHALL
	u32 PatchCount = 1|64|128;
#else
	u32 PatchCount = 1|2|16|64|128|2048;
#endif

#ifdef CHEATS
	if( IsWiiU )
	{
		if( ConfigGetConfig(NIN_CFG_CHEATS) )
			PatchCount &= ~64;

	} else {

		if( ConfigGetConfig(NIN_CFG_DEBUGGER|NIN_CFG_CHEATS) )
			PatchCount &= ~64;
	}
#endif
	if( ConfigGetConfig(NIN_CFG_FORCE_PROG) || (ConfigGetVideoMode() & NIN_VID_FORCE) )
		PatchCount &= ~128;

	u8 *SHA1i = (u8*)malloca( 0x60, 0x40 );
	u8 *hash  = (u8*)malloca( 0x14, 0x40 );
	u32 gxReg1 = 0, gxReg2 = 0, gxReg1DBG = 0, gxReg2DBG = 0;
	for( i=0; i < Length; i+=4 )
	{
		if( (PatchCount & 2048) == 0  )	// __OSDispatchInterrupt
		{
			if( read32((u32)Buffer + i + 0 ) == 0x3C60CC00 &&
				read32((u32)Buffer + i + 4 ) == 0x83E33000  
				)
			{
				printpatchfound("__OSDispatchInterrupt",NULL, (u32)Buffer + i);
				PatchBL( FakeInterruptAddr, (u32)Buffer + i + 4 );
				if(ConfigGetConfig(NIN_CFG_MEMCARDEMU) == true)
				{
					// EXI Device 0 Control Register
					write32A( (u32)Buffer+i+0x114, 0x3C60C000, 0x3C60CC00, 1 );
					write32A( (u32)Buffer+i+0x118, 0x80830010, 0x80836800, 1 );
				}
				// EXI Device 2 Control Register (Trifroce)
				write32A( (u32)Buffer+i+0x188, 0x3C60C000, 0x3C60CC00, 1 );
				write32A( (u32)Buffer+i+0x18C, 0x38630018, 0x38636800, 1 );
				write32A( (u32)Buffer+i+0x190, 0x80830000, 0x80830028, 1 );

				PatchCount |= 2048;
			} else if( read32( (u32)Buffer + i + 0 ) == 0x3C60CC00 &&
				read32( (u32)Buffer + i + 4 ) == 0x83A33000 && read32( (u32)Buffer + i + 8 ) == 0x57BD041C)
			{
				printpatchfound("__OSDispatchInterrupt","DBG", (u32)Buffer + i + 4);
				PatchBL( FakeInterrupt_DBGAddr, (u32)Buffer + i + 4 );
				if(ConfigGetConfig(NIN_CFG_MEMCARDEMU) == true)
				{
					// EXI Device 0 Control Register
					write32A( (u32)Buffer+i+0x138, 0x3C60C000, 0x3C60CC00, 1 );
					write32A( (u32)Buffer+i+0x13C, 0x83C30010, 0x83C36800, 1 );
				}
				PatchCount |= 2048;
			}
		}

		if( (PatchCount & 1) == 0 )	// 803463EC 8034639C
		if( (read32( (u32)Buffer + i )		& 0xFC00FFFF)	== 0x5400077A &&
			(read32( (u32)Buffer + i + 4 )	& 0xFC00FFFF)	== 0x28000000 &&
			 read32( (u32)Buffer + i + 8 )								== 0x41820008 &&
			(read32( (u32)Buffer + i +12 )	& 0xFC00FFFF)	== 0x64002000
			)  
		{
			#ifdef DEBUG_PATCH
			dbgprintf("Patch:[__OSDispatchInterrupt]: 0x%08X (0x%08X)\r\n", (u32)Buffer + i + 0, (u32)Buffer + i + 0x1A8 );
			#endif
			write32( (u32)Buffer + i + 0x1A8,	(read32( (u32)Buffer + i + 0x1A8 )	& 0xFFFF0000) | 0x0463 );
			
			PatchCount |= 1;
		}

		if( (PatchCount & 2) == 0 )	// SetInterruptMask
		if( read32( (u32)Buffer + i )		== 0x5480056A &&
			read32( (u32)Buffer + i + 4 )	== 0x28000000 &&
			read32( (u32)Buffer + i + 8 )	== 0x40820008 &&
			(read32( (u32)Buffer + i +12 )&0xFC00FFFF) == 0x60000004
			) 
		{
			printpatchfound("SetInterruptMask",NULL, (u32)Buffer + i + 12);

			write32( (u32)Buffer + i + 12, (read32( (u32)Buffer + i + 12 ) & 0xFFFF0000) | 0x4004 );

			PatchCount |= 2;
		}

		if( (PatchCount & 4) == 0 )
		if( (read32( (u32)Buffer + i + 0 ) & 0xFFFF) == 0x6000 &&
			(read32( (u32)Buffer + i + 4 ) & 0xFFFF) == 0x002A &&
			(read32( (u32)Buffer + i + 8 ) & 0xFFFF) == 0x0054 
			) 
		{
			u32 Offset = (u32)Buffer + i - 8;
			u32 HwOffset = Offset;
			if ((read32(Offset + 4) & 0xFFFF) == 0xCC00)	// Loader
				HwOffset = Offset + 4;
			#ifdef DEBUG_PATCH
			dbgprintf("Patch:[__DVDInterruptHandler]: 0x%08X (0x%08X)\r\n", Offset, HwOffset );
			#endif
			PatchDiscInterface( (char*)Offset );
			PatchBL(DCInvalidateRangeAddr, HwOffset);
			PatchCount |= 4;
		}
		
		if( (PatchCount & 8) == 0 )
		{
			if( (read32( (u32)Buffer + i + 0 ) & 0xFFFF) == 0xCC00 &&			// Game
				(read32( (u32)Buffer + i + 4 ) & 0xFFFF) == 0x6000 &&
				(read32( (u32)Buffer + i +12 ) & 0xFFFF) == 0x001C 
				) 
			{
				u32 Offset = (u32)Buffer + i;
				printpatchfound("cbForStateBusy",NULL, Offset);
				value = *(vu32*)(Offset+8);
				value&= 0x03E00000;
				value|= 0x38000000;
				*(vu32*)(Offset+8) = value;

				PatchFunc( Buffer + i + 12 );

				PatchCount |= 8;

			} else if(	(read32( (u32)Buffer + i + 0 ) & 0xFFFF) == 0xCC00 && // Loader
						(read32( (u32)Buffer + i + 4 ) & 0xFFFF) == 0x6018 &&
						(read32( (u32)Buffer + i +12 ) & 0xFFFF) == 0x001C 
				)
			{
				u32 Offset = (u32)Buffer + i;
				printpatchfound("cbForStateBusy","DBG", Offset);
				value = *(vu32*)(Offset+4);
				value&= 0x03E00000;
				value|= 0x38000000;
				*(vu32*)(Offset+4) = value;

				PatchFunc( Buffer + i + 8 );

				PatchCount |= 8;
			}
		}

		if( (PatchCount & 16) == 0 )
		{
			if( read32( (u32)Buffer + i + 0 ) == 0x3C608000 )
			{
				if( ((read32( (u32)Buffer + i + 4 ) & 0xFC1FFFFF ) == 0x800300CC) && ((read32( (u32)Buffer + i + 8 ) >> 24) == 0x54 ) )
				{
					printpatchfound("VIConfigure",NULL, (u32)Buffer + i);
					write32( (u32)Buffer + i + 4, 0x5400F0BE | ((read32( (u32)Buffer + i + 4 ) & 0x3E00000) >> 5	) );
					PatchCount |= 16;
				}
			}
		}

		if( (PatchCount & 32) == 0 )	// GXInit __piReg
		{
			/* Release SDK */
			if( read32( (u32)Buffer + i ) == 0x3C80CC00 )
				gxReg1 = (u32)Buffer + i;
			if( gxReg1 && read32( (u32)Buffer + i ) == 0x38A43000 )
				gxReg2 = (u32)Buffer + i;
			if( gxReg2 && R16( (u32)Buffer + i ) == 0x90AD )
			{
				piReg = R16( (u32)Buffer + i + 2 );
				#ifdef DEBUG_PATCH
				dbgprintf("Patch:[GXInit] stw r5,-0x%X(r13) (0x%08X)\r\n", 0x10000 - piReg, (u32)Buffer + i + 2);
				#endif
				PatchCount |= 32;
			}
			/* Debug SDK */
			if( read32( (u32)Buffer + i ) == 0x3C600C00 )
				gxReg1DBG = (u32)Buffer + i;
			if( gxReg1DBG && read32( (u32)Buffer + i ) == 0x38633000 )
				gxReg2DBG = (u32)Buffer + i;
			if( gxReg2DBG && R16( (u32)Buffer + i ) == 0x906D )
			{
				piReg = R16( (u32)Buffer + i + 2 );
				#ifdef DEBUG_PATCH
				dbgprintf("Patch:[GXInit DBG] stw r5,-0x%X(r13) (0x%08X)\r\n", 0x10000 - piReg, (u32)Buffer + i + 2);
				#endif
				PatchCount |= 32;
			}
		}
#ifdef CHEATS
		if( (PatchCount & 64) == 0 )
		{
			//OSSleepThread(Pattern 1)
			if( read32((u32)Buffer + i + 0 ) == 0x3C808000 &&
				( read32((u32)Buffer + i + 4 ) == 0x3C808000 || read32((u32)Buffer + i + 4 ) == 0x808400E4 ) &&
				( read32((u32)Buffer + i + 8 ) == 0x38000004 || read32((u32)Buffer + i + 8 ) == 0x808400E4 )
			)
			{
				int j = 12;

				while( *(vu32*)(Buffer+i+j) != 0x4E800020 )
					j+=4;

				u32 DebuggerHook = (u32)Buffer + i + j;
				printpatchfound("Hook:OSSleepThread",NULL, DebuggerHook);

				u32 DBGSize;

				FIL fs;
				if( f_open( &fs, "/sneek/kenobiwii.bin", FA_OPEN_EXISTING|FA_READ ) != FR_OK )
				{
					#ifdef DEBUG_PATCH
					dbgprintf( "Patch:Could not open:\"%s\", this file is required for debugging!\r\n", "/sneek/kenobiwii.bin" );
					#endif
				}
				else
				{
					if( fs.fsize != 0 )
					{
						DBGSize = fs.fsize;

						//Read file to memory
						s32 ret = f_read( &fs, (void*)0x1800, fs.fsize, &read );
						if( ret != FR_OK )
						{
							#ifdef DEBUG_PATCH
							dbgprintf( "Patch:Could not read:\"%s\":%d\r\n", "/sneek/kenobiwii.bin", ret );
							#endif
							f_close( &fs );
						}
						else
						{
							f_close( &fs );

							if( IsWiiU )
							{
								*(vu32*)(P2C(*(vu32*)0x1808)) = 0;
							}
							else
							{
								if( ConfigGetConfig(NIN_CFG_DEBUGWAIT) )
									*(vu32*)(P2C(*(vu32*)0x1808)) = 1;
								else
									*(vu32*)(P2C(*(vu32*)0x1808)) = 0;
							}

							memcpy( (void *)0x1800, (void*)0, 6 );

							u32 newval = 0x18A8 - DebuggerHook;
								newval&= 0x03FFFFFC;
								newval|= 0x48000000;

							*(vu32*)(DebuggerHook) = newval;

							if( ConfigGetConfig( NIN_CFG_CHEATS ) )
							{
								char *path = (char*)malloc( 128 );

								if( ConfigGetConfig(NIN_CFG_CHEAT_PATH) )
								{
									_sprintf( path, "%s", ConfigGetCheatPath() );
								} else {
									_sprintf( path, "/games/%.6s/%.6s.gct", (char*)0x1800, (char*)0x1800 );
								}

								FIL CodeFD;
								u32 read;

								if( f_open( &CodeFD, path, FA_OPEN_EXISTING|FA_READ ) == FR_OK )
								{
									if( CodeFD.fsize >= 0x2E60 - (0x1800+DBGSize-8) )
									{
										#ifdef DEBUG_PATCH
										dbgprintf("Patch:Cheatfile is too large, it must not be large than %d bytes!\r\n", 0x2E60 - (0x1800+DBGSize-8));
										#endif
									} else {
										if( f_read( &CodeFD, (void*)(0x1800+DBGSize-8), CodeFD.fsize, &read ) == FR_OK )
										{
											#ifdef DEBUG_PATCH
											dbgprintf("Patch:Copied cheat file to memory\r\n");
											#endif
											write32( 0x1804, 1 );
										} else {
											#ifdef DEBUG_PATCH
											dbgprintf("Patch:Failed to read cheat file:\"%s\"\r\n", path );
											#endif
										}
									}

									f_close( &CodeFD );

								} else {
									#ifdef DEBUG_PATCH
									dbgprintf("Patch:Failed to open/find cheat file:\"%s\"\r\n", path );
									#endif
								}

								free(path);
							}
						}
					}
				}

				PatchCount |= 64;
			}
		}
#endif
		if( (PatchCount & 128) == 0  )
		{
			/* might be a bit overkill but should detect most video mode in memory */
			while((read32((u32)Buffer+i) & 0xFFFFFF00) == 0 && (read32((u32)Buffer+i+4) & 0xFF000000) == 0x02000000
				&& (read32((u32)Buffer+i+12) & 0xFF00FF00) == 0x00000200 && read32((u32)Buffer+i+24) == 0x00000606
				&& read32((u32)Buffer+i+32) == 0x06060606 && read32((u32)Buffer+i+44) == 0x06060606)
			{
				if( ConfigGetConfig(NIN_CFG_FORCE_PROG) )
				{
					switch(read32((u32)Buffer+i))
					{
						case 0x00: //NTSC
							printvidpatch(VI_NTSC, VI_480P, (u32)Buffer+i);
							write32( (u32)Buffer+i, 0x02 );
							write32( (u32)Buffer+i+0x14, 0); //mode sf
							memcpy( Buffer+i+0x30, GXProgAt30, sizeof(GXProgAt30) );
							break;
						case 0x04: //PAL50
							printvidpatch(VI_PAL, VI_480P, (u32)Buffer+i);
							write32( (u32)Buffer+i, 0x02 );
							//write32( (u32)Buffer+i, 0x06 );
							write32( (u32)Buffer+i+0x14, 0); //mode sf
							memcpy( Buffer+i+0x30, GXProgAt30, sizeof(GXProgAt30) );
							//memcpy(Buffer+i, GXNtsc480Prog, sizeof(GXNtsc480Prog));
							break;
						case 0x08: //MPAL
						case 0x14: //PAL60
							printvidpatch(VI_EUR60, VI_480P, (u32)Buffer+i);
							write32( (u32)Buffer+i, 0x02 );
							//write32( (u32)Buffer+i, 0x16 );
							write32( (u32)Buffer+i+0x14, 0); //mode sf
							memcpy( Buffer+i+0x30, GXProgAt30, sizeof(GXProgAt30) );
							break;
						default:
							break;
					}
				}
				else
				{
					u8 NinForceMode = ConfigGetVideoMode() & NIN_VID_FORCE_MASK;
					switch(read32((u32)Buffer+i))
					{
						case 0x00: //NTSC
							if(NinForceMode == NIN_VID_FORCE_NTSC)
								break;
							if(NinForceMode == NIN_VID_FORCE_MPAL)
							{
								printvidpatch(VI_NTSC, VI_MPAL, (u32)Buffer+i);
								write32( (u32)Buffer+i, 0x08 );
							}
							else //PAL60
							{
								printvidpatch(VI_NTSC, VI_EUR60, (u32)Buffer+i);
								write32( (u32)Buffer+i, 0x14 );
							}
							write32( (u32)Buffer+i+0x14, 1); //mode df
							memcpy( Buffer+i+0x30, GXIntDfAt30, sizeof(GXIntDfAt30) );
							break;
						case 0x08: //MPAL
							if(NinForceMode == NIN_VID_FORCE_MPAL)
								break;
							if(NinForceMode == NIN_VID_FORCE_NTSC)
							{
								printvidpatch(VI_MPAL, VI_NTSC, (u32)Buffer+i);
								write32( (u32)Buffer+i, 0x00 );
							}
							else //PAL60
							{
								printvidpatch(VI_MPAL, VI_EUR60, (u32)Buffer+i);
								write32( (u32)Buffer+i, 0x14 );
							}
							write32( (u32)Buffer+i+0x14, 1); //mode df
							memcpy( Buffer+i+0x30, GXIntDfAt30, sizeof(GXIntDfAt30) );
						case 0x04: //PAL50
							if(NinForceMode == NIN_VID_FORCE_PAL50 || NinForceMode == NIN_VID_FORCE_PAL60)
								break;
							if(NinForceMode == NIN_VID_FORCE_MPAL)
							{
								printvidpatch(VI_PAL, VI_MPAL, (u32)Buffer+i);
								write32( (u32)Buffer+i, 0x08 );
							}
							else //NTSC
							{
								printvidpatch(VI_PAL, VI_NTSC, (u32)Buffer+i);
								write32( (u32)Buffer+i, 0x00 );
							}
							memcpy( Buffer+i+0x04, GXIntDfAt04, sizeof(GXIntDfAt04) ); //terrible workaround I know
							write32( (u32)Buffer+i+0x14, 1); //mode df
							memcpy( Buffer+i+0x30, GXIntDfAt30, sizeof(GXIntDfAt30) );
							break;
						case 0x14: //PAL60
							if(NinForceMode == NIN_VID_FORCE_PAL50 || NinForceMode == NIN_VID_FORCE_PAL60)
								break;
							if(NinForceMode == NIN_VID_FORCE_MPAL)
							{
								printvidpatch(VI_EUR60, VI_MPAL, (u32)Buffer+i);
								write32( (u32)Buffer+i, 0x08 );
							}
							else //NTSC
							{
								printvidpatch(VI_EUR60, VI_NTSC, (u32)Buffer+i);
								write32( (u32)Buffer+i, 0x00 );
							}
							write32( (u32)Buffer+i+0x14, 1); //mode df
							memcpy( Buffer+i+0x30, GXIntDfAt30, sizeof(GXIntDfAt30) );
							break;
						default:
							break;
					}
				}
				i+=0x3C;
			}
		}

		if( (PatchCount & 256) == 0 )	//DVDLowStopMotor
		{
			if( read32( (u32)Buffer + i ) == 0x3C00E300 )
			{
				printpatchfound("DVDLowStopMotor",NULL, (u32)Buffer + i);
				PatchFunc( Buffer + i - 12 );
				PatchCount |= 256;
			}
		}

		if( (PatchCount & 512) == 0 )	//DVDLowReadDiskID
		{
			if( (read32( (u32)Buffer + i ) & 0xFC00FFFF ) == 0x3C00A800 && (read32( (u32)Buffer + i + 4 ) & 0xFC00FFFF ) == 0x38000040 )
			{
				printpatchfound("DVDLowReadDiskID",NULL, (u32)Buffer + i);
				PatchFunc( Buffer + i );
				PatchCount |= 512;
			}
		}

		if( (PatchCount & 1024) == 0  )	// DSP patch
		{
			u32 l,Known=-1;

			u32 UseLast = 0;
			bool PatternMatch = false;
			// for each pattern/length combo
			for( l=0; l < sizeof(DspMatches) / sizeof(DspMatch); ++l )
			{
				if (UseLast == 0)
					PatternMatch = ( memcmp( (void*)(Buffer+i), DSPPattern[DspMatches[l].Pattern], 0x10 ) == 0 );
				if (PatternMatch)
				{
					#ifdef DEBUG_PATCH
					dbgprintf("Patch:Matching [DSPPattern] (0x%08X) v%u\r\n", (u32)Buffer + i, l );
					#endif

					if (DspMatches[l].Length-UseLast > 0)
						memcpy( (void*)0x12E80000+UseLast, (unsigned char*)Buffer+i+UseLast, DspMatches[l].Length-UseLast );
					
					if (DspMatches[l].Length != UseLast)
					{
						sha1( SHA1i, NULL, 0, 0, NULL );
						sha1( SHA1i, (void*)0x12E80000, DspMatches[l].Length, 2, hash );
					}

					if( memcmp( DSPHashes[DspMatches[l].SHA1], hash, 0x14 ) == 0 )
					{
						Known = DspMatches[l].SHA1;
#ifdef DEBUG_DSP
						dbgprintf("DSP before Patch\r\n");
						hexdump((void*)(Buffer + i), DspMatches[l].Length);
#endif
						DoDSPPatch(Buffer + i, Known);
#ifdef DEBUG_DSP
						dbgprintf("DSP after Patch\r\n");
						hexdump((void*)(Buffer + i), DspMatches[l].Length);
#endif

						#ifdef DEBUG_PATCH
						dbgprintf("Patch:[DSPROM] DSPv%u\r\n", Known );
						#endif
						PatchCount |= 1024;
						break;
					}
				}
				if ( (l < sizeof(DspMatches) / sizeof(DspMatch)) && (DspMatches[l].Pattern == DspMatches[l+1].Pattern) )
					UseLast = DspMatches[l].Length;
				else
					UseLast = 0;
			}
		}

		if( PatchCount == (1|2|4|8|16|32|64|128|256|512|1024|2048) )
			break;
	}
	free(hash);
	free(SHA1i);
	#ifdef DEBUG_PATCH
	dbgprintf("PatchCount:%08X\r\n", PatchCount );
	if( (PatchCount & 1024) == 0  )
		dbgprintf("Patch:Unknown DSP ROM\r\n");
	#endif

	if( ((PatchCount & (1|2|4|8|2048)) != (1|2|4|8|2048)) )
	{
		#ifdef DEBUG_PATCH
		dbgprintf("Patch:Could not apply all required patches!\r\n");
		#endif
		Shutdown();
	}
	sync_after_write( Buffer, Length );

	u32 patitr;
	for(patitr = 0; patitr < PCODE_MAX; ++patitr)
	{
		FuncPattern *CurPatterns = AllFPatterns[patitr].pat;
		u32 CurPatternsLen = AllFPatterns[patitr].patlen;
		for( j=0; j < CurPatternsLen; ++j )
			CurPatterns[j].Found = 0;
	}
	u32 PADInitOffset = 0, SIInitOffset = 0;
	sync_before_read( Buffer, Length );
	i = 0;
	while(i < Length)
	{
		PatchTimers((u32)Buffer + i);
		PatchProcessorInterface(Buffer + i);

		if( *(u32*)(Buffer + i) != 0x4E800020 )
		{
			i+=4;
			continue;
		}
		i+=4;

		FuncPattern fp;
		MPattern( (u8*)(Buffer+i), Length, &fp );
		//if ((((u32)Buffer + i) & 0x7FFFFFFF) == 0x00000000) //(FuncPrint)
		//	dbgprintf("FuncPattern: 0x%X, %d, %d, %d, %d, %d\r\n", fp.Length, fp.Loads, fp.Stores, fp.FCalls, fp.Branch, fp.Moves);
		for(patitr = 0; patitr < PCODE_MAX; ++patitr)
		{
			if(AllFPatterns[patitr].patmode == PCODE_TRI && TRIGame == 0)
				continue;
			if(AllFPatterns[patitr].patmode == PCODE_EXI && (TRIGame == 3 || ConfigGetConfig(NIN_CFG_MEMCARDEMU) == false))
				continue;
			if ((ConfigGetConfig(NIN_CFG_NATIVE_SI)) && (AllFPatterns[patitr].patmode == PCODE_SI))
				continue;
			FuncPattern *CurPatterns = AllFPatterns[patitr].pat;
			u32 CurPatternsLen = AllFPatterns[patitr].patlen;
			bool patfound = false;
			for( j=0; j < CurPatternsLen; ++j )
			{
				if( CurPatterns[j].Found ) //Skip already found patches
					continue;

				if( CPattern( &fp, &(CurPatterns[j]) ) )
				{
					u32 FOffset = (u32)Buffer + i;

					CurPatterns[j].Found = FOffset;
					//#ifdef DEBUG_PATCH
					//dbgprintf("Patch:[%s] found (0x%08X)\r\n", CurPatterns[j].Name, FOffset );
					//#endif

					switch( CurPatterns[j].PatchLength )
					{
						case FCODE_PatchFunc:	// DVDLowRead
						{
							printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset);
							PatchFunc( (char*)FOffset );
						} break;
						case FCODE_Return:	// __AI_set_stream_sample_rate
						{
							printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset);
							write32( FOffset, 0x4E800020 );
						} break;
						case FCODE_AIResetStreamCount:	// Audiostreaming hack
						{
							switch( TITLE_ID )
							{
								case 0x474544:	// Eternal Darkness
								break;
								default:
								{
									printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset);
									write32( FOffset + 0xB4, 0x60000000 );
									write32( FOffset + 0xC0, 0x60000000 );
								} break;
							}
						} break;
						case FCODE_GXInitTlutObj:	// GXInitTlutObj
						{
							if( read32( FOffset+0x11C ) == 0x5400023E )
							{
								printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + 0x11C);
								write32( FOffset+0x11C, 0x5400033E );
							}
							else if( read32( FOffset+0x34 ) == 0x5400023E )
							{
								printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + 0x34);
								write32( FOffset+0x34, 0x5400033E );
							}
							else if( read32( FOffset+0x28 ) == 0x5004C00E )
							{
								printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + 0x28);
								write32( FOffset+0x28, 0x5004C016 );
							}
							else
								CurPatterns[j].Found = 0; // False hit
						} break;
						case FCODE___ARChecksize_A:
						{
							printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + 0x57C);
							write32(FOffset + 0x57C, 0x48000378); // b +0x0378, No Expansion
						} break;
						case FCODE___ARChecksize_B:
						{
							printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + 0x370);
							write32(FOffset + 0x370, 0x48001464); // b +0x1464, No Expansion
						} break;
						case FCODE___ARChecksize_DBG_B:
						{
							printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + 0x15C);
							write32(FOffset + 0x15C, 0x48000188); // b +0x0188, No Expansion
						} break;
						case FCODE___ARChecksize_C:
						{
							printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + 0x484);
							write32(FOffset + 0x484, 0x60000000); // nop, Memory Present
							write32(FOffset + 0x768, 0x60000000); // nop, 16MB Internal
							write32(FOffset + 0xB88, 0x48000324); // b +0x0324, No Expansion
						} break;
						case FCODE___ARChecksize_DBG_A:
						{
							printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + 0x160);
							write32(FOffset + 0x160, 0x4800001C); // b +0x001C, Memory Present
							write32(FOffset + 0x2BC, 0x60000000); // nop, 16MB Internal
							write32(FOffset + 0x394, 0x4800017C); // b +0x017C, No Expansion
						} break;
	// Widescreen hack by Extrems
						case FCODE_C_MTXPerspective:	//	C_MTXPerspective
						{
							if( !ConfigGetConfig(NIN_CFG_FORCE_WIDE) )
								break;

							printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset);
							*(volatile float *)0x50 = 0.5625f;

							memcpy((void*)(FOffset+ 28),(void*)(FOffset+ 36),44);
							memcpy((void*)(FOffset+188),(void*)(FOffset+192),16);

							*(unsigned int*)(FOffset+52) = 0x48000001 | ((*(unsigned int*)(FOffset+52) & 0x3FFFFFC) + 8);
							*(unsigned int*)(FOffset+72) = 0x3C600000 | (0x80000050 >> 16);		// lis		3, 0x8180
							*(unsigned int*)(FOffset+76) = 0xC0230000 | (0x80000050 & 0xFFFF);	// lfs		1, -0x1C (3)
							*(unsigned int*)(FOffset+80) = 0xEC240072;							// fmuls	1, 4, 1
						} break;
						case FCODE_C_MTXLightPerspective:	//	C_MTXLightPerspective
						{
							if( !ConfigGetConfig(NIN_CFG_FORCE_WIDE) )
								break;

							printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset);
							*(volatile float *)0x50 = 0.5625f;

							*(u32*)(FOffset+36) = *(u32*)(FOffset+32);

							memcpy((void*)(FOffset+ 28),(void*)(FOffset+ 36),60);
							memcpy((void*)(FOffset+184),(void*)(FOffset+188),16);

							*(u32*)(FOffset+68) += 8;
							*(u32*)(FOffset+88) = 0x3C600000 | (0x80000050 >> 16); 		// lis		3, 0x8180
							*(u32*)(FOffset+92) = 0xC0230000 | (0x80000050 & 0xFFFF);	// lfs		1, -0x90 (3)
							*(u32*)(FOffset+96) = 0xEC240072;							// fmuls	1, 4, 1
						} break;
						case FCODE_J3DUClipper_clip:	//	clip
						{
							if( !ConfigGetConfig(NIN_CFG_FORCE_WIDE) )
								break;

							printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset);
							write32( FOffset,   0x38600000 );
							write32( FOffset+4, 0x4E800020 );
						} break;
						case FCODE___OSReadROM:	//	__OSReadROM
						{
							printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset);
							memcpy( (void*)FOffset, __OSReadROM, sizeof(__OSReadROM) );
						} break;
						case FCODE___OSInitAudioSystem_A:
						{
							printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + 0x4C);
							write32( FOffset+0x4C, 0x48000060 ); // b +0x0060, Skip Checks
						} break;
						case FCODE___OSInitAudioSystem_B:
						{
							printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + 0x60);
							write32( FOffset+0x60, 0x48000060 ); // b +0x0060, Skip Checks
						} break;
						case FCODE___GXSetVAT:	// Patch for __GXSetVAT, fixes the dungeon map freeze in Wind Waker
						{
							switch( TITLE_ID )
							{
								case 0x505A4C:	// The Legend of Zelda: Collector's Edition
									if( !(DOLSize == 3847012 || DOLSize == 3803812) )			// only patch the main.dol of the Zelda:ww game 
										break;
								case 0x475A4C:	// The Legend of Zelda: The Wind Waker
								{
									write32(FOffset, (read32(FOffset) & 0xff00ffff) | 0x00220000);
									memcpy( (void *)(FOffset + 4), __GXSetVAT_patch, sizeof(__GXSetVAT_patch) );
									printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset);
								} break;
								default:
								break;
							}
						} break;
						case FCODE_EXIIntrruptHandler:
						{
							u32 PatchOffset = 0x4;
							while ((read32(FOffset + PatchOffset) != 0x90010004) && (PatchOffset < 0x20)) // MaxSearch=0x20
								PatchOffset += 4;
							if (read32(FOffset + PatchOffset) != 0x90010004) 	// stw r0, 0x0004(sp)
							{
								CurPatterns[j].Found = 0; // False hit
								break;
							}
							PatchBL( TCIntrruptHandlerAddr, (FOffset + PatchOffset) );
							printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + PatchOffset);
						} break;
						case FCODE_EXIDMA:	// EXIDMA
						{
							u32 off		= 0;
							s32 reg		=-1;
							u32 valueB	= 0;

							while(1)
							{
								off += 4;

								u32 op = read32( (u32)FOffset + off );

								if( reg == -1 )
								{
									if( (op & 0xFC1F0000) == 0x3C000000 )	// lis rX, 0xCC00
									{
										reg = (op & 0x3E00000) >> 21;
										if( reg == 3 )
										{
											value = read32( FOffset + off ) & 0x0000FFFF;
											#ifdef DEBUG_PATCH
											//dbgprintf("lis:%08X value:%04X\r\n", FOffset+off, value );
											#endif
										}
									}
								}
								if( reg != -1 )
								{
									if( (op & 0xFC000000) == 0x38000000 )	// addi rX, rY, z
									{
										u32 src = (op >> 16) & 0x1F;
										if( src == reg )
										{
											u32 dst = (op >> 21) & 0x1F;
											if(dst == 0)
											{
												valueB = read32( FOffset + off ) & 0x0000FFFF;
												#ifdef DEBUG_PATCH
												//dbgprintf("addi:%08X value:%04X\r\n", FOffset+off, valueB);
												#endif
												break;
											}
											else //false hit
											{
												value = 0;
												reg = -1;
											}
										}
									}
								}
								if( op == 0x4E800020 )	// blr
									break;
							}
							printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset);
							memcpy( (void*)(FOffset), EXIDMA, sizeof(EXIDMA) );
							/* Insert CB value into patched EXIDMA */
							W16(FOffset + 2, value);
							W16(FOffset + 6, valueB + 4);
						} break;
						case FCODE_EXIUnlock:
						{
							u32 k;
							u32 found = 0;
							for(k = 0; k < CurPatterns[j].Length; k+=4)
							{
								if(read32(FOffset+k) == 0x54000734)
								{
									printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + k);
									memcpy((void*)FOffset, EXILock, sizeof(EXILock));
									found = 1;
									break;
								}
							}
							if(found == 0)
								CurPatterns[j].Found = 0; // False hit
						} break;
						case FCODE___CARDStat_A:	// CARD timeout
						{
							write32( FOffset+0x124, 0x60000000 );
							write32( FOffset+0x18C, 0x60000000 );
							printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset);
						} break;
						case FCODE___CARDStat_B:	// CARD timeout
						{
							write32( FOffset+0x118, 0x60000000 );
							write32( FOffset+0x180, 0x60000000 );
							printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset);
						} break;
						case FCODE___CARDStat_C:	// CARD timeout
						{
							write32( FOffset+0x124, 0x60000000 );
							write32( FOffset+0x194, 0x60000000 );
							write32( FOffset+0x1FC, 0x60000000 );
							printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset);
						} break;
						case FCODE_ARInit:
						{
							/* Use GC Bus Clock Speed for Refresh Value */
							if(read32(FOffset + 0x5C) == 0x3C608000)
							{
								write32(FOffset + 0x5C, 0x3C6009A8);
								write32(FOffset + 0x64, 0x3803EC80);
								printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + 0x5C);
							}
							else if(read32(FOffset + 0x78) == 0x3C608000)
							{
								write32(FOffset + 0x78, 0x3C6009A8);
								write32(FOffset + 0x7C, 0x3803EC80);
								printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + 0x78);
							}
							else if(read32(FOffset + 0x24) == 0x38604000 || read32(FOffset + 0x2C) == 0x38604000)
							{
								#ifdef DEBUG_PATCH
								dbgprintf("Patch:[ARInit] skipped (0x%08X)\r\n", FOffset);
								#endif
							}
							else
								CurPatterns[j].Found = 0;
						} break;
						case FCODE_ARStartDMA:	//	ARStartDMA
						{
							if( (TITLE_ID) == 0x474234 ||	// Burnout 2
								(TITLE_ID) == 0x47564A ||	// Viewtiful Joe
								(TITLE_ID) == 0x474146 ||	// Animal Crossing
								(TITLE_ID) == 0x475852 )	// Mega Man X Command Mission
							{
								u32 PatchOffset = 0x20;
								while ((read32(FOffset + PatchOffset) != 0x3CC0CC00) && (PatchOffset < 0x40)) // MaxSearch=0x40
									PatchOffset += 4;
								if (read32(FOffset + PatchOffset) != 0x3CC0CC00)	// lis	r6,	0xCC00
								{
									CurPatterns[j].Found = 0; // False hit
									break;
								}
								PatchBL( PatchCopy(ARStartDMA_Hook, sizeof(ARStartDMA_Hook)), (FOffset + PatchOffset) );
								printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + PatchOffset);
								break;
							}
							else if( (TITLE_ID) == 0x47384D ||	// Paper Mario
									 (TITLE_ID) == 0x475951 ||	// Mario Superstar Baseball
									 (TITLE_ID) == 0x474154 ||	// ATV Quad Power Racing 2
									 (TITLE_ID) == 0x47504E ||	// P.N.03
									 (TITLE_ID) == 0x474D4F ||	// Micro Machines
									 (TITLE_ID) == 0x473442 ||	// Resident Evil 4
									 (TITLE_ID) == 0x473258 )	// Sonic Gems Collection
							{
								memcpy( (void*)FOffset, ARStartDMA_PM, sizeof(ARStartDMA_PM) );
							}
							else if((TITLE_ID) == 0x474832) // NFS: HP2
							{
								memcpy( (void*)FOffset, ARStartDMA_NFS, sizeof(ARStartDMA_NFS) );
							}
							else
							{
								memcpy( (void*)FOffset, ARStartDMA, sizeof(ARStartDMA) );
								if( (TITLE_ID) != 0x474156 )	// Avatar Last Airbender
								{
									u32 PatchOffset = 0;
									for (PatchOffset = 0; PatchOffset < sizeof(ARStartDMA); PatchOffset += 4)
									{
										if (*(u32*)(ARStartDMA + PatchOffset) == 0x90C35028)	// 	stw		%r6,	AR_DMA_CNT@l(%r3)
										{
											write32(FOffset + PatchOffset, 0x90E35028);			// 	stw		%r7,	AR_DMA_CNT@l(%r3)
											break;
										}
									}
								}
							}
							printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset);
						} break;
						case FCODE_GCAMIdentify:
						{
							if(read32(FOffset + 0x38) == 0x9003601C)
							{
								printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset);
								PatchFunc( (char*)FOffset );
							}
							else
								CurPatterns[j].Found = 0; // False hit
						} break;
						case FCODE_GCAMSendCommand:
						{
							printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset);
							if( TRIGame == 1 )
							{
								PatchB( PatchCopy(GCAMSendCommand2, sizeof(GCAMSendCommand2)), FOffset );
								break;
							}
							if( TRIGame == 2 )
							{
								PatchB( PatchCopy(GCAMSendCommandVSExp, sizeof(GCAMSendCommandVSExp)), FOffset );
								break;
							}
							if( TRIGame == 3 )
							{
								PatchB( PatchCopy(GCAMSendCommandF, sizeof(GCAMSendCommandF)), FOffset );
								break;
							}
							if( TRIGame == 4 )
							{
								PatchB( PatchCopy(GCAMSendCommand, sizeof(GCAMSendCommand)), FOffset );
								break;
							}
						} break;
						case FCODE___fwrite:	// patch_fwrite_Log
						case FCODE___fwrite_D:	// patch_fwrite_LogB
						{
							if( ConfigGetConfig( NIN_CFG_DEBUGGER ) || !ConfigGetConfig(NIN_CFG_OSREPORT) )
							{
								#ifdef DEBUG_PATCH
								dbgprintf("Patch:[patch_fwrite_Log] skipped (0x%08X)\r\n", FOffset);
								#endif
								break;
							}

							if( IsWiiU )
							{
								if( CurPatterns[j].Patch == patch_fwrite_GC ) // patch_fwrite_Log works fine
								{
									#ifdef DEBUG_PATCH
									dbgprintf("Patch:[patch_fwrite_GC] skipped (0x%08X)\r\n", FOffset);
									#endif
									break;
								}
							}
							printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset);
							memcpy((void*)FOffset, patch_fwrite_Log, sizeof(patch_fwrite_Log));
							if (CurPatterns[j].PatchLength == FCODE___fwrite_D)
							{
								write32(FOffset, 0x7C852379); // mr.     %r5,%r4
								write32(FOffset + sizeof(patch_fwrite_Log) - 0x8, 0x38600000); // li      %r3,0
							}
							break;
						}
						case FCODE__SITransfer:	//	_SITransfer
						{
							//e.g. Mario Strikers
							if (write32A(FOffset + 0x60, 0x7CE70078, 0x7CE70038, 0)) // clear errors - andc r7,r7,r0
								printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + 0x60);

							//e.g. Billy Hatcher
							if (write32A(FOffset + 0xF8, 0x7F390078, 0x7F390038, 0)) // clear errors - andc r7,r7,r0
								printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + 0xF8);

							//e.g. Mario Strikers
							if (write32A(FOffset + 0x148, 0x5400007E, 0x50803E30, 0)) // clear tc - rlwinm r0,r0,0,1,31
								printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + 0x148);

							//e.g. Luigi's Mansion
							if (write32A(FOffset + 0x140, 0x5400007E, 0x50A03E30, 0)) // clear tc - rlwinm r0,r0,0,1,31
								printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + 0x140);

							//e.g. Billy Hatcher
							if (write32A(FOffset + 0x168, 0x5400007E, 0x50603E30, 0)) // clear tc - rlwinm r0,r0,0,1,31
								printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + 0x168);

							//e.g. Batman Vengeance
							if (write32A(FOffset + 0x164, 0x5400007E, 0x50603E30, 0)) // clear tc - rlwinm r0,r0,0,1,31
								printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + 0x164);
						} break;
						case FCODE_CompleteTransfer:	//	CompleteTransfer
						{
							//e.g. Mario Strikers
							if (write32A(FOffset + 0x38, 0x5400007C, 0x5400003C, 0)) // clear  tc - rlwinm r0,r0,0,1,30
								printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + 0x38);

							//e.g. Billy Hatcher (func before CompleteTransfer does this)
							if (write32A(FOffset - 0x18, 0x57FF007C, 0x57FF003C, 0)) // clear  tc - rlwinm r31,r31,0,1,30
								printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset - 0x18);

							u32 k = 0;
							for(k = 0x10; k < 0x20; k+=4)
							{
								if (write32A(FOffset + k, 0x3C000000, 0x3C008000, 0)) // clear  tc - lis r0,0
								{
									printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + k);
									break;
								}
							}
						} break;
						case FCODE_SIInit:	//	SIInit
						{
							u32 k;
							u32 found = 0;
							for(k = 0x40; k < CurPatterns[j].Length; k += 4)
							{
								if (write32A(FOffset + k, 0x3C000000, 0x3C008000, 0)) // clear tc - lis r0,0
								{
									found = 1;
									SIInitOffset = FOffset + CurPatterns[j].Length;
									printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + k);
									/* Found SIInit, also sets SIInterruptHandler */
									u32 SIBase = 0;
									for(k += 4; k < CurPatterns[j].Length; k += 4)
									{
										if((read32(FOffset+k) & 0xFC00F000) == 0x3C008000)
											SIBase = (R16(FOffset+k+2) << 16);
										if(SIBase && (read32(FOffset+k) & 0xFFF00000) == 0x38800000)
										{
											SIBase = P2C(SIBase + (s16)R16(FOffset+k+2));
											u32 PatchOffset = 0x4;
											while ((read32(SIBase + PatchOffset) != 0x90010004) && (PatchOffset < 0x20)) // MaxSearch=0x20
												PatchOffset += 4;
											if (read32(SIBase + PatchOffset) != 0x90010004) 	// stw r0, 0x0004(sp)
												break;
											PatchBL( SIIntrruptHandlerAddr, (SIBase + PatchOffset) );
											printpatchfound("SIInterruptHandler", NULL, SIBase + PatchOffset);
											if (write32A(SIBase + 0xB4, 0x7F7B0078, 0x7F7B0038, 0)) // clear  tc - andc r27,r27,r0
												printpatchfound("SIInterruptHandler", NULL, SIBase + 0xB4);
											if (write32A(SIBase + 0x134, 0x7CA50078, 0x7CA50038, 0)) // clear  tc - andc r5,r5,r0
												printpatchfound("SIInterruptHandler", NULL, SIBase + 0x134);
											break;
										}
									}
									break;
								}
							}
							if(found == 0)
								CurPatterns[j].Found = 0; // False hit
						} break;
						case FCODE_SIPollingInterrupt:	//	SIEnablePollingInterrupt
						{
							//e.g. SSBM
							if (write32A(FOffset + 0x68, 0x60000000, 0x54A50146, 0)) // leave rdstint alone - nop
								printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + 0x68);
							if (write32A(FOffset + 0x6C, 0x60000000, 0x54A5007C, 0)) // leave tcinit tcstart alone - nop
								printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + 0x6C);

							//e.g. Billy Hatcher
							if (write32A(FOffset + 0x78, 0x60000000, 0x57FF0146, 0)) // leave rdstint alone - nop
								printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + 0x78);
							if (write32A(FOffset + 0x7C, 0x60000000, 0x57FF007C, 0)) // leave tcinit tcstart alone - nop
								printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + 0x7C);
						} break;
						case FCODE_PADRead:
						{
							memcpy((void*)FOffset, PADRead, PADRead_size);
							printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset);
							/* Search for PADInit over PADRead */
							u32 k;
							u32 lastEnd = 4;
							for(k = 8; k < 0x400; k += 4)
							{
								if(read32(FOffset - k) == 0x4E800020)
									lastEnd = k;
								else if((read32(FOffset - k) & 0xFC00FFFF) == 0x3C00F000)
								{
									PADInitOffset = FOffset - lastEnd;
									break;
								}
							}
						} break;
						case FCODE_PADIsBarrel:
						{
							if(memcmp((void*)(FOffset), PADIsBarrelOri, sizeof(PADIsBarrelOri)) != 0)
							{
								CurPatterns[j].Found = 0; // False hit
								break;
							}
							memcpy((void*)(FOffset), PADIsBarrel, sizeof(PADIsBarrel));
							printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset);
						} break;
						case FCODE___OSResetHandler:
						{
							if(read32(FOffset + 0x84) == 0x540003DF)
							{
								PatchBL( FakeRSWLoadAddr, (FOffset + 0x84) );
								PatchBL( FakeRSWLoadAddr, (FOffset + 0x94) );
								PatchBL( FakeRSWStoreAddr, (FOffset + 0xD4) );
								printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + 0x84);
							}
							else
								CurPatterns[j].Found = 0;
						} break;
						case FCODE_OSGetResetState:
						{
							if(read32(FOffset + 0x28) == 0x540003DF) /* Patch C */
							{
								PatchBL( FakeRSWLoadAddr, (FOffset + 0x28) );
								printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + 0x28);
							}
							else if(read32(FOffset + 0x2C) == 0x540003DF) /* Patch A, B */
							{
								PatchBL( FakeRSWLoadAddr, (FOffset + 0x2C) );
								printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset + 0x2C);
							}
							else
								CurPatterns[j].Found = 0;
						} break;
						case FCODE_AIInitDMA:
						{
							if(read32(FOffset + 0x20) == 0x3C80CC00)
							{
								PatchBL( AIInitDMAAddr, (FOffset + 0x20) );
								printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset);
							}
							else
								CurPatterns[j].Found = 0;
						} break;
						case FCODE___DSPHandler:
						{
							if(read32(FOffset + 0xF8) == 0x2C000000)
							{
								PatchBL( __DSPHandlerAddr, (FOffset + 0xF8) );
								printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset);
							}
							else
								CurPatterns[j].Found = 0;
						} break;
						default:
						{
							if( CurPatterns[j].Patch == (u8*)ARQPostRequest )
							{
								if (   (TITLE_ID) != 0x474D53  // Super Mario Sunshine
									&& (TITLE_ID) != 0x474C4D  // Luigis Mansion
									&& (TITLE_ID) != 0x475049  // Pikmin
									&& (TITLE_ID) != 0x474146  // Animal Crossing
									&& (TITLE_ID) != 0x474C56) // Chronicles of Narnia
								{
									#ifdef DEBUG_PATCH
									dbgprintf("Patch:[ARQPostRequest] skipped (0x%08X)\r\n", FOffset);
									#endif
									break;
								}
							}
							if( CurPatterns[j].Patch == SIGetType )
							{
								if ((TITLE_ID) == 0x475853) // Sonic Adventure DX
								{
									#ifdef DEBUG_PATCH
									dbgprintf("Patch:[SIGetType] skipped (0x%08X)\r\n", FOffset);
									#endif
									break;
								}
							}
							if( CurPatterns[j].Patch == DVDGetDriveStatus )
							{
								if( (TITLE_ID) != 0x474754 &&	// Chibi-Robo!
									(TITLE_ID) != 0x475041 )	// Pok駑on Channel
									break;
							}
							if( (CurPatterns[j].Length >> 16) == (FCODES  >> 16) )
							{
								#ifdef DEBUG_PATCH
								dbgprintf("Patch:Unhandled dead case:%08X\r\n", CurPatterns[j].Length );
								#endif
							}
							else
							{
								printpatchfound(CurPatterns[j].Name, CurPatterns[j].Type, FOffset);
								memcpy( (void*)(FOffset), CurPatterns[j].Patch, CurPatterns[j].PatchLength );
							}
						} break;
					}
					// If this is a patch group set all others of this group as found aswell
					if( CurPatterns[j].Group && CurPatterns[j].Found )
					{
						for( k=0; k < CurPatternsLen; ++k )
						{
							if( CurPatterns[k].Group == CurPatterns[j].Group )
							{
								if( !CurPatterns[k].Found )		// Don't overwrite the offset!
									CurPatterns[k].Found = -1;	// Usually this holds the offset, to determinate it from a REALLY found pattern we set it -1 which still counts a logical TRUE
								#ifdef DEBUG_PATCH
								//dbgprintf("Setting [%s] to found!\r\n", CurPatterns[k].Name );
								#endif
							}
						}
					}
					// We dont need to compare this to more patterns
					i += (CurPatterns[j].Length - 4);
					patfound = true;
					break;
				}
			}
			if(patfound) break;
		}
	}

	/* Check for PADInit, if not found use SIInit */
	if(PADInitOffset != 0)
	{
		PatchB(SIInitStoreAddr, PADInitOffset);
		printpatchfound("PADInit", NULL, PADInitOffset);
	}
	else if(SIInitOffset != 0)
	{
		PatchB(SIInitStoreAddr, SIInitOffset);
		printpatchfound("SIInit", NULL, SIInitOffset);
	}

	#ifdef DEBUG_PATCH
	for(patitr = 0; patitr < PCODE_MAX; ++patitr)
	{
		if(AllFPatterns[patitr].patmode == PCODE_TRI && TRIGame == 0)
			continue;
		if(AllFPatterns[patitr].patmode == PCODE_EXI && (TRIGame == 3 || ConfigGetConfig(NIN_CFG_MEMCARDEMU) == false))
			continue;
		if ((ConfigGetConfig(NIN_CFG_NATIVE_SI)) && (AllFPatterns[patitr].patmode == PCODE_SI))
			continue;
		FuncPattern *CurPatterns = AllFPatterns[patitr].pat;
		u32 CurPatternsLen = AllFPatterns[patitr].patlen;
		for( j=0; j < CurPatternsLen; ++j )
		{
			if(!CurPatterns[j].Found)
			{
				dbgprintf("Patch:[%s] not found\r\n", CurPatterns[j].Name );
				if(CurPatterns[j].Group)
					for( k = j; j < CurPatternsLen && CurPatterns[j].Group == CurPatterns[k].Group; ++j ) ;
			}
		}
	}
	#endif
	PatchState = PATCH_STATE_DONE;

	/*if(GAME_ID == 0x47365145) //Megaman Collection
	{
		memcpy((void*)0x5A110, OSReportDM, sizeof(OSReportDM));
		sync_after_write((void*)0x5A110, sizeof(OSReportDM));

		memcpy((void*)0x820FC, OSReportDM, sizeof(OSReportDM));
		sync_after_write((void*)0x820FC, sizeof(OSReportDM));

		#ifdef DEBUG_PATCH
		dbgprintf("Patch:Patched Megaman Collection\r\n");
		#endif
	}
	if(GAME_ID == 0x47585845) // Pokemon XD
	{
		memcpy((void*)0x2A65CC, OSReportDM, sizeof(OSReportDM));
		sync_after_write((void*)0x2A65CC, sizeof(OSReportDM));
	}*/
	if( (GAME_ID & 0xFFFFFF00) == 0x47425600 )	// Batman Vengeance
	{
		// Skip Usage of EXI Channel 2
		if(write32A(0x0018B5DC, 0x60000000, 0x4801D6E1, 0))
		{
			#ifdef DEBUG_PATCH
			dbgprintf("Patch:Patched Batman Vengeance NTSC-U\r\n");
			#endif
		}
		else if(write32A(0x0015092C, 0x60000000, 0x4801D599, 0))
		{
			#ifdef DEBUG_PATCH
			dbgprintf("Patch:Patched Batman Vengeance PAL\r\n");
			#endif
		}
	}
	else if( (GAME_ID & 0xFFFFFF00) == 0x47433600 )	// Pokemon Colosseum
	{
		// Memory Card inserted hack
		if( ConfigGetConfig(NIN_CFG_MEMCARDEMU) == true )
		{
			if(write32A(0x000B0D88, 0x38600001, 0x4801C0B1, 0))
			{
				#ifdef DEBUG_PATCH
				dbgprintf("Patch:Patched Pokemon Colosseum NTSC-J\r\n");
				#endif
			}
			else if(write32A(0x000B30DC, 0x38600001, 0x4801C2ED, 0))
			{
				#ifdef DEBUG_PATCH
				dbgprintf("Patch:Patched Pokemon Colosseum NTSC-U\r\n");
				#endif
			}
			else if(write32A(0x000B66DC, 0x38600001, 0x4801C2ED, 0))
			{
				#ifdef DEBUG_PATCH
				dbgprintf("Patch:Patched Pokemon Colosseum PAL\r\n");
				#endif
			}
		}
	}
	else if( (GAME_ID & 0xFFFFFF00) == 0x475A4C00 )	// GZL=Wind Waker
	{
		//Anti FrameDrop Panic
		/* NTSC-U Final */
		if(read32(0x00221A28) == 0x40820034 && read32(0x00256424) == 0x41820068)
		{
			//	write32( 0x03945B0, 0x8039408C );	// Test menu
			write32(0x00221A28, 0x48000034);
			write32(0x00256424, 0x48000068);
			#ifdef DEBUG_PATCH
			dbgprintf("Patch:Patched WW NTSC-U\r\n");
			#endif
		}
		/* NTSC-U Demo */
		if(read32(0x0021D33C) == 0x40820034 && read32(0x00251EF8) == 0x41820068)
		{
			write32(0x0021D33C, 0x48000034);
			write32(0x00251EF8, 0x48000068);
			#ifdef DEBUG_PATCH
			dbgprintf("Patch:Patched WW NTSC-U Demo\r\n");
			#endif
		}
		/* PAL Final */
		if(read32(0x001F1FE0) == 0x40820034 && read32(0x0025B5C4) == 0x41820068)
		{
			write32(0x001F1FE0, 0x48000034);
			write32(0x0025B5C4, 0x48000068);
			#ifdef DEBUG_PATCH
			dbgprintf("Patch:Patched WW PAL\r\n");
			#endif
		}
		/* NTSC-J Final */
		if(read32(0x0021EDD4) == 0x40820034 && read32(0x00253BCC) == 0x41820068)
		{
			write32(0x0021EDD4, 0x48000034);
			write32(0x00253BCC, 0x48000068);
			#ifdef DEBUG_PATCH
			dbgprintf("Patch:Patched WW NTSC-J\r\n");
			#endif
		}
		/*if( ConfigGetConfig( NIN_CFG_OSREPORT ) )
		{
			*(vu32*)(0x00006858) = 0x60000000;		//OSReport disabled patch
			PatchB( 0x00006950, 0x003191BC );
		}*/
	}
	PatchStaticTimers();

	sync_after_write( Buffer, Length );
}

void PatchInit()
{
	memcpy((void*)PATCH_OFFSET_ENTRY, FakeEntryLoad, FakeEntryLoad_size);
	sync_after_write((void*)PATCH_OFFSET_ENTRY, FakeEntryLoad_size);
}

void PatchGame()
{
	write32(0x13002740, 0); //Clear SI Inited
	sync_after_write((void*)0x13002740, 0x20);
	if(GameEntry < 0x31A0)
		Patch31A0();
	PatchState = PATCH_STATE_PATCH;
	u32 FullLength = (DOLMaxOff - DOLMinOff + 31) & (~31);
	DoPatches( (void*)DOLMinOff, FullLength, 0 );

	write32( DIP_CMD_1, FullLength >> 5 );
	write32( DIP_CMD_2, DOLMinOff );
	dbgprintf("Jumping to 0x%08X\n", GameEntry);
	write32( DIP_IMM, GameEntry );
}
