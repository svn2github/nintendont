/*

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
#include "global.h"
#include "Patch.h"
#include "PatchCodes.h"

enum
{
	FCODES								= 0xdead0000,
	FCODE_ARInit,
	FCODE_ARStartDMA,
	FCODE_AIResetStreamCount,
	FCODE_Return,
										//__AI_set_stream_sample_rate
	FCODE_GXInitTlutObj,
										//GXInitTlutObj_A,
										//GXInitTlutObj_B,
	FCODE__SITransfer, 
										//_SITransfer_A,
										//_SITransfer_B,
	FCODE_CompleteTransfer,
										//CompleteTransfer_A,
										//CompleteTransfer_B,
										//CompleteTransfer_C,
	FCODE_SIInit,
										//SIInit_A,
										//SIInit_B,
										//SIInit_C,
	FCODE_SIPollingInterrupt,
										//SIEnablePollingInterrupt_A,
										//SIEnablePollingInterrupt_B,
	FCODE___ARChecksize_A,
	FCODE___ARChecksize_B,
	FCODE___ARChecksize_C,
	FCODE___ARChecksize_DBG,
	FCODE___OSReadROM,
	FCODE_C_MTXPerspective,
	FCODE_C_MTXLightPerspective,
	FCODE_J3DUClipper_clip,
	//FCODE_C_MTXOrtho,
	FCODE_GCAMIdentify,
	FCODE_GCAMSendCommand,
	FCODE_PatchFunc,
										//GCAMSendCMD,	
										//GCAMRead,	
										//GCAMExecute,	
										//GCAMWrite,	
										//GCAMIdentify,
										//DVDLowRead_A,
										//DVDLowRead_B,	
										//DVDLowRead_C,	
										//DVDLowAudioStream_A,	
										//DVDLowAudioStream_B,	
										//DVDLowRequestAudioStatus,	
										//DVDLowAudioBufferConfig,
	FCODE___fwrite,
										//__fwrite_A,	
										//__fwrite_B,	
										//__fwrite_C,
	FCODE___fwrite_D,
	FCODE___GXSetVAT,
	FCODE_EXIIntrruptHandler,
										//TCIntrruptHandler_A,	
										//TCIntrruptHandler_B,	
										//TCIntrruptHandler_C,	
										//TCIntrruptHandler_D,	
										//EXIntrruptHandler_A,	
										//EXIntrruptHandler_B,	
										//EXIntrruptHandler_C,
	FCODE_SIInterruptHandler,
										//SIInterruptHandler_A,	
										//SIInterruptHandler_B,
										//SIInterruptHandler_C,
										//SIInterruptHandler_D,
	FCODE_PADIsBarrel,
	FCODE_EXIDMA,
	FCODE_EXIUnlock,
	FCODE___CARDStat_A,
	FCODE___CARDStat_B,
	FCODE___CARDStat_C,
	FCODE___OSResetHandler,
	FCODE_OSGetResetState,
	FCODE___OSInitAudioSystem_A,
	FCODE___OSInitAudioSystem_B,
	FCODE_AIInitDMA,
	FCODE___DSPHandler,
} FPatternCodes;

enum
{
	FGROUP_NONE				= 0x0,
	FGROUP_DVDInquiryAsync,
	FGROUP_ARInit,
	FGROUP_ARStartDMA,
	FGROUP_SIGetType,
	FGROUP__SITransfer,
	FGROUP_CompleteTransfer,
	FGROUP_SIInit,
	FGROUP_SIPollingInterrupt,
	FGROUP___ARChecksize,
	FGROUP_DVDLowRead,
	FGROUP_DVDLowAudioStream,
	FGROUP___fwrite,
	FGROUP_PADRead,
	FGROUP_PADControlAllMotors,
	FGROUP_PADControlMotor,
	FGROUP_TCIntrruptHandler,
	FGROUP_EXIntrruptHandler,
	FGROUP_SIInterruptHandler,
	FGROUP_EXILock,
	FGROUP_EXIUnlock,
	FGROUP_EXISelect,
	FGROUP_EXIImm,
	FGROUP_EXIDMA,
	FGROUP_EXISync,
	FGROUP_EXIDeselect,
	FGROUP___EXIProbe,
	FGROUP_CARDStat,
	FGROUP_OSGetResetState,
	FGROUP___OSInitAudioSystem,
} FPatternGroups;

FuncPattern NormalFPatterns[] =
{
	{   0xCC,   17,   10,    5,    3,    2,	DVDInquiryAsync,	DVDInquiryAsync_size,		"DVDInquiryAsync",		"A",		FGROUP_DVDInquiryAsync,		0 },
	{   0xC0,   18,    8,    4,    1,    3,	DVDInquiryAsync,	DVDInquiryAsync_size,		"DVDInquiryAsync",		"B",		FGROUP_DVDInquiryAsync,		0 },
//	{   0xB8,   15,    7,    5,    3,    2,	DVDInquiryAsync,	DVDInquiryAsync_size,		"DVDInquiryAsync",		"C",		FGROUP_DVDInquiryAsync,		0 },

	{   0xC8,   16,    9,    5,    3,    3,	DVDSeekAbsAsyncPrio,DVDSeekAbsAsyncPrio_size,	"DVDSeekAbsAsyncPrio",	NULL,		FGROUP_NONE,				0 },
	{   0xA8,   10,    4,    4,    6,    3,	DVDGetDriveStatus,	sizeof(DVDGetDriveStatus),	"DVDGetDriveStatus",	NULL,		FGROUP_NONE,				0 },
#ifndef AUDIOSTREAM
	{   0xD4,   13,    8,   11,    2,    7,	NULL,				FCODE_AIResetStreamCount,"AIResetStreamSampleCount",NULL,		FGROUP_NONE,				0 },
#endif
	{   0x44,    4,    4,    0,    0,    2,	NULL,				FCODE_GXInitTlutObj,		"GXInitTlutObj",		"A",		FGROUP_NONE,				0 },	// Don't group them, false hit prevents finding real offset
	{   0x34,    5,    4,    0,    0,    0,	NULL,				FCODE_GXInitTlutObj,		"GXInitTlutObj",		"B",		FGROUP_NONE,				0 },

	{   0xEC,    9,    6,    2,    0,    8,	NULL,				FCODE_ARStartDMA,			"ARStartDMA",			NULL,		FGROUP_ARStartDMA,			0 },
	{  0x16C,   29,    3,    5,    3,    9,	NULL,				FCODE_ARStartDMA,			"ARStartDMA",			"DBG",		FGROUP_ARStartDMA,			0 },

	{   0xB8,   17,   10,    5,    1,    2,	NULL,				FCODE_ARInit,				"ARInit",				"A",		FGROUP_ARInit,				0 },
	{   0xC0,   18,   10,    6,    1,    2,	NULL,				FCODE_ARInit,				"ARInit",				"B",		FGROUP_ARInit,				0 },
	{   0xF0,   21,   12,    5,    1,    2,	NULL,				FCODE_ARInit,				"ARInit",				"C",		FGROUP_ARInit,				0 },
	{  0x168,   31,   13,   10,    2,    3,	NULL,				FCODE_ARInit,				"ARInit",				"DBG",		FGROUP_ARInit,				0 },

	{  0x910,   87,   33,   18,    5,   63,	NULL,				FCODE___ARChecksize_A,		"__ARChecksize",		"A",		FGROUP___ARChecksize,		0 },
	{ 0x17F0,  204,   51,   27,    5,  178,	NULL,				FCODE___ARChecksize_B,		"__ARChecksize",		"B",		FGROUP___ARChecksize,		0 },
	{  0xEC8,  129,   29,   32,    9,   80,	NULL,				FCODE___ARChecksize_C,		"__ARChecksize",		"C",		FGROUP___ARChecksize,		0 },
	{  0x530,  183,    4,   58,   13,   16,	NULL,				FCODE___ARChecksize_DBG,	"__ARChecksize",		"DBG",		FGROUP___ARChecksize,		0 },

	{  0x158,   26,   22,    5,   13,    2,	ARQPostRequest,		ARQPostRequest_size,		"ARQPostRequest",		NULL,		FGROUP_NONE,				0 },
//	{  0x120,   28,    6,   10,    2,    7,	NULL,				FCODE___OSReadROM,			"__OSReadROM",						FGROUP_NONE,				0 },
#ifdef PATCHALL
	{   0xCC,    3,    3,    1,    0,    3,	NULL,				FCODE_C_MTXPerspective,		"C_MTXPerspective",		NULL,		FGROUP_NONE,				0 },
	{   0xC8,    3,    3,    1,    0,    3,	NULL,				FCODE_C_MTXLightPerspective,"C_MTXLightPerspective",NULL,		FGROUP_NONE,				0 },
	{  0x144,    9,    3,    1,   10,    6,	NULL,				FCODE_J3DUClipper_clip,		"J3DUClipper::clip()",	NULL,		FGROUP_NONE,				0 },	// These are two different functions
	{  0x2E4,   39,    8,    3,   13,    9,	NULL,				FCODE_J3DUClipper_clip,		"J3DUClipper::clip()",	NULL,		FGROUP_NONE,				0 },
//	{   0x94,    0,    0,    0,    0,    0,	NULL,				FCODE_J3DUClipper_clip,		"C_MTXOrtho",						FGROUP_NONE,				0 },
#endif
	{  0x10C,   30,   18,    5,    2,    3,	NULL,				FCODE_PatchFunc,			"DVDLowRead",			"A",		FGROUP_DVDLowRead,			0 },
	{   0xDC,   23,   18,    3,    2,    4,	NULL,				FCODE_PatchFunc,			"DVDLowRead",			"B",		FGROUP_DVDLowRead,			0 },
	{  0x104,   29,   17,    5,    2,    3,	NULL,				FCODE_PatchFunc,			"DVDLowRead",			"C",		FGROUP_DVDLowRead,			0 },
#ifdef AUDIOSTREAM
	{   0x94,   18,   10,    2,    0,    2,	DVDLowAudioStream,	DVDLowAudioStream_size,		"DVDLowAudioStream",	"A",		FGROUP_DVDLowAudioStream,	0 },
	{   0x8C,   16,   12,    1,    0,    2,	DVDLowAudioStream,	DVDLowAudioStream_size,		"DVDLowAudioStream",	"B",		FGROUP_DVDLowAudioStream,	0 },
	{   0x88,   18,    8,    2,    0,    2,	DVDLowRequestAudioStatus,DVDLowRequestAudioStatus_size,"DVDLowRequestAudioStatus",	NULL,FGROUP_NONE,			0 },
	{   0x98,   19,    8,    2,    1,    3,	DVDLowAudioBufferConfig, DVDLowAudioBufferConfig_size, "DVDLowAudioBufferConfig",	NULL,FGROUP_NONE,			0 },
#else
	{   0x94,   18,   10,    2,    0,    2,	DVDLowReadAudioNULL,sizeof(DVDLowReadAudioNULL),"DVDLowAudioStream",	"A",		FGROUP_DVDLowAudioStream,	0 },
	{   0x8C,   16,   12,    1,    0,    2,	DVDLowReadAudioNULL,sizeof(DVDLowReadAudioNULL),"DVDLowAudioStream",	"B",		FGROUP_DVDLowAudioStream,	0 },
	{   0x88,   18,    8,    2,    0,    2,	DVDLowAudioStatusNULL,	sizeof(DVDLowAudioStatusNULL),"DVDLowRequestAudioStatus",	NULL,FGROUP_NONE,			0 },
	{   0x98,   19,    8,    2,    1,    3,	DVDLowAudioConfigNULL,	sizeof(DVDLowAudioConfigNULL),"DVDLowAudioBufferConfig",	NULL,FGROUP_NONE,			0 },
#endif
	{  0x308,   40,   18,   10,   23,   17,	NULL,				FCODE___fwrite,				"__fwrite",				"A",		FGROUP___fwrite,			0 },
	{  0x338,   48,   20,   10,   24,   16,	NULL,				FCODE___fwrite,				"__fwrite",				"B",		FGROUP___fwrite,			0 },
	{  0x2D8,   41,   17,    8,   21,   13,	NULL,				FCODE___fwrite,				"__fwrite",				"C",		FGROUP___fwrite,			0 },
//	{  0x1FC,   47,    4,   14,   18,    7,	NULL,				FCODE___fwrite_D,			"__fwrite D",						FGROUP___fwrite,			0 },

	{   0x98,    8,    3,    0,    3,    5,	NULL,				FCODE___GXSetVAT,			"__GXSetVAT",			NULL,		FGROUP_NONE,				0 },
#ifdef PATCHALL
	{   0xF0,   20,   11,    3,    3,    9,	NULL,				FCODE___OSResetHandler,	"__OSResetSWInterruptHandler",NULL,		FGROUP_NONE,				0 },

	{  0x294,   39,   16,    5,   16,   46,	NULL,				FCODE_OSGetResetState,		"OSGetResetButtonState","A",		FGROUP_OSGetResetState,		0 },
	{  0x2A0,   40,   16,    5,   17,   46,	NULL,				FCODE_OSGetResetState,		"OSGetResetButtonState","B",		FGROUP_OSGetResetState,		0 },
	{  0x1F0,   34,   14,    6,   12,   28,	NULL,				FCODE_OSGetResetState,		"OSGetResetButtonState","C",		FGROUP_OSGetResetState,		0 },
#endif
	{  0x28C,   70,    8,    8,   10,    4,	NULL,				FCODE___OSInitAudioSystem_A,"__OSInitAudioSystem",	"DBG A",	FGROUP___OSInitAudioSystem,	0 },
	{  0x2B8,   77,    8,   12,   10,    4,	NULL,				FCODE___OSInitAudioSystem_B,"__OSInitAudioSystem",	"DBG B",	FGROUP___OSInitAudioSystem,	0 },
#ifdef AUDIOSTREAM
	{   0x84,    8,    4,    2,    0,    5,	NULL,				FCODE_AIInitDMA,			"AIInitDMA",			NULL,		FGROUP_NONE,				0 },
	{  0x420,  103,   23,   34,   32,    9,	NULL,				FCODE___DSPHandler,			"__DSPHandler",			NULL,		FGROUP_NONE,				0 },
#endif
};

FuncPattern TRIFPatterns[] =
{
	{   0xB4,   18,   11,    1,    0,    7,	NULL,				FCODE_PatchFunc,			"GCAMSendCMD",			NULL,		FGROUP_NONE,				0 },
	{   0xCC,   20,   16,    5,    0,    3,	NULL,				FCODE_PatchFunc,			"GCAMRead",				NULL,		FGROUP_NONE,				0 },
	{   0x9C,   18,    9,    4,    0,    2,	NULL,				FCODE_PatchFunc,			"GCAMExecute",			NULL,		FGROUP_NONE,				0 },
	{   0xB0,   19,   12,    4,    0,    2,	NULL,				FCODE_PatchFunc,			"GCAMWrite",			NULL,		FGROUP_NONE,				0 },
	{   0x98,   19,    9,    4,    0,    2,	NULL,				FCODE_GCAMIdentify,			"GCAMIdentify",			NULL,		FGROUP_NONE,				0 },
	{   0x54,   10,    2,    2,    0,    2,	NULL,				FCODE_GCAMSendCommand,		"GCAMSendCommand",		NULL,		FGROUP_NONE,				0 },

	{  0x168,   22,   10,    7,    6,   10,	SITransfer,			sizeof(SITransfer),			"SITransfer",			NULL,		FGROUP_NONE,				0 },
};

FuncPattern SIFPatterns[] =
{
	{  0x2F8,   60,   22,    2,   16,   25,	NULL,				FCODE_CompleteTransfer,		"CompleteTransfer",		"A",		FGROUP_CompleteTransfer,	0 },
	{  0x240,   40,   14,    0,   13,   11,	NULL,				FCODE_CompleteTransfer,		"CompleteTransfer",		"B",		FGROUP_CompleteTransfer,	0 },
	{  0x180,   29,    9,    3,    9,    9,	NULL,				FCODE_CompleteTransfer,		"CompleteTransfer",		"C",		FGROUP_CompleteTransfer,	0 },
	{   0xE0,   18,    4,    0,    6,    3,	NULL,				FCODE_CompleteTransfer,		"CompleteTransfer",		"DBG",		FGROUP_CompleteTransfer,	0 },

	{  0x340,   61,   10,    7,   26,   32,	NULL,				FCODE_SIInterruptHandler,	"SIInterruptHandler",	"A",		FGROUP_SIInterruptHandler,	0 },
	{  0x114,   21,    4,    4,    5,   11,	NULL,				FCODE_SIInterruptHandler,	"SIInterruptHandler",	"B",		FGROUP_SIInterruptHandler,	0 },
	{  0x2EC,   50,    7,    9,   14,   27,	NULL,				FCODE_SIInterruptHandler,	"SIInterruptHandler",	"C",		FGROUP_SIInterruptHandler,	0 },
	{  0x258,   39,    6,    7,   15,   17,	NULL,				FCODE_SIInterruptHandler,	"SIInterruptHandler",	"D",		FGROUP_SIInterruptHandler,	0 },
	{   0x8C,   13,    4,    3,    1,    3,	NULL,				FCODE_SIInterruptHandler,	"SIInterruptHandler",	"DBG",		FGROUP_SIInterruptHandler,	0 },

	{   0x94,    8,   10,    2,    4,    2,	NULL,				FCODE_SIPollingInterrupt,	"SIEnablePollingInterrupt","A",		FGROUP_SIPollingInterrupt,	0 },
	{   0xA4,    9,    5,    2,    6,    4,	NULL,				FCODE_SIPollingInterrupt,	"SIEnablePollingInterrupt","B",		FGROUP_SIPollingInterrupt,	0 },

	{   0xB0,   21,    9,    8,    0,    2,	NULL,				FCODE_SIInit,				"SIInit",				"A",		FGROUP_SIInit,				0 },
	{   0x70,   13,    8,    2,    0,    2,	NULL,				FCODE_SIInit,				"SIInit",				"B",		FGROUP_SIInit,				0 },
	{   0x90,   17,    8,    6,    0,    2,	NULL,				FCODE_SIInit,				"SIInit",				"C",		FGROUP_SIInit,				0 },
	{   0xA0,   20,    8,    7,    0,    2,	NULL,				FCODE_SIInit,				"SIInit",				"D",		FGROUP_SIInit,				0 },
	{   0xB0,   22,    9,    8,    0,    2,	NULL,				FCODE_SIInit,				"SIInit",				"E",		FGROUP_SIInit,				0 },
	{   0x9C,   19,    9,    6,    0,    2,	NULL,				FCODE_SIInit,				"SIInit",				"F",		FGROUP_SIInit,				0 },
	{   0x7C,   15,    9,    2,    0,    2,	NULL,				FCODE_SIInit,				"SIInit",				"DBG",		FGROUP_SIInit,				0 },

	{  0x208,   38,   18,    3,   13,   10,	NULL,				FCODE__SITransfer,			"__SITransfer",			"A",		FGROUP__SITransfer,			0 },
	{  0x204,   37,   18,    3,   13,   11,	NULL,				FCODE__SITransfer,			"__SITransfer",			"B",		FGROUP__SITransfer,			0 },
	{  0x208,   38,   11,    7,   13,    9,	NULL,				FCODE__SITransfer,			"__SITransfer",			"C",		FGROUP__SITransfer,			0 },
	{  0x204,   37,   11,    7,   13,    9,	NULL,				FCODE__SITransfer,			"__SITransfer",			"DBG",		FGROUP__SITransfer,			0 },

	{  0x1C0,   35,    9,    8,    7,   19,	SIGetType,			SIGetType_size,				"SIGetType",			"A",		FGROUP_SIGetType,			0 },
	{  0x1F4,   27,    9,    9,    9,   24,	SIGetType,			SIGetType_size,				"SIGetType",			"B",		FGROUP_SIGetType,			0 },

	{  0x3A8,   86,   13,   27,   17,   24,	PADRead,			PADRead_size,				"PADRead",				"A",		FGROUP_PADRead,				0 },
	{  0x2FC,   73,    8,   23,   16,   15,	PADRead,			PADRead_size,				"PADRead",				"B",		FGROUP_PADRead,				0 },
	{  0x3B0,   87,   13,   27,   17,   25,	PADRead,			PADRead_size,				"PADRead",				"C",		FGROUP_PADRead,				0 },
	{  0x334,   78,    7,   20,   17,   19,	PADRead,			PADRead_size,				"PADRead",				"D",		FGROUP_PADRead,				0 },
	{  0x2A8,   66,    4,   20,   17,   14,	PADRead,			PADRead_size,				"PADRead",				"E",		FGROUP_PADRead,				0 },

	{   0xB4,    8,    2,    5,    4,    5,	PADControlAllMotors,PADControlAllMotors_size,	"PADControlAllMotors",	"A",		FGROUP_PADControlAllMotors,	0 },
	{   0xC8,    9,    2,    5,    5,    5,	PADControlAllMotors,PADControlAllMotors_size,	"PADControlAllMotors",	"B",		FGROUP_PADControlAllMotors,	0 },

	{   0xB4,   11,    5,    5,    3,    5,	PADControlMotor,	PADControlMotor_size,		"PADControlMotor",		"A",		FGROUP_PADControlMotor,		0 },
	{   0xA0,   10,    5,    5,    2,    5,	PADControlMotor,	PADControlMotor_size,		"PADControlMotor",		"B",		FGROUP_PADControlMotor,		0 },
	{   0xB8,   14,    5,    4,    2,    7,	PADControlMotor,	PADControlMotor_size,		"PADControlMotor",		"C",		FGROUP_PADControlMotor,		0 },

	{   0x14,    1,    0,    0,    2,    0,	NULL,				FCODE_PADIsBarrel,			"PADIsBarrel",			NULL,		FGROUP_NONE,				0 },
};

FuncPattern EXIFPatterns[] =
{
	{  0x258,   36,    8,    5,   12,   32,	EXIImm,				EXIImm_size,				"EXIImm",				"A",		FGROUP_EXIImm,				0 },
	{  0x258,   27,    8,    5,   12,   17,	EXIImm,				EXIImm_size,				"EXIImm",				"B",		FGROUP_EXIImm,				0 },
	{  0x158,   24,    7,    5,    7,    9,	EXIImm,				EXIImm_size,				"EXIImm",				"PKM",		FGROUP_EXIImm,				0 },
	{  0x1E4,   38,    7,    9,   12,    9,	EXIImm,				EXIImm_size,				"EXIImm",				"DBG",		FGROUP_EXIImm,				0 },

	{   0xE8,   17,    7,    5,    2,    5,	NULL,				FCODE_EXIDMA,				"EXIDMA",				"A",		FGROUP_EXIDMA,				0 },
	{   0xE8,   17,    7,    5,    2,    4,	NULL,				FCODE_EXIDMA,				"EXIDMA",				"B",		FGROUP_EXIDMA,				0 },
	{  0x124,   28,    8,    5,    2,    8,	NULL,				FCODE_EXIDMA,				"EXIDMA",				"PKM",		FGROUP_EXIDMA,				0 },
	{  0x1C0,   42,    5,   10,    8,    6,	NULL,				FCODE_EXIDMA,				"EXIDMA",				"DBG",		FGROUP_EXIDMA,				0 },

	{  0x234,   39,    3,    3,   12,   19,	EXILock,			EXILock_size,				"EXISync",				"A",		FGROUP_EXISync,				0 },
	{  0x248,   40,    3,    4,   13,   19,	EXILock,			EXILock_size,				"EXISync",				"B",		FGROUP_EXISync,				0 },
	{  0x204,   31,    3,    3,   11,   17,	EXILock,			EXILock_size,				"EXISync",				"C",		FGROUP_EXISync,				0 },
	{  0x234,   35,    3,    3,   12,   17,	EXILock,			EXILock_size,				"EXISync",				"D",		FGROUP_EXISync,				0 },
	{  0x16C,   26,    3,    3,    9,    7,	EXILock,			EXILock_size,				"EXISync",				"PKM",		FGROUP_EXISync,				0 },
	{  0x1A8,   34,    2,    7,    9,    8,	EXILock,			EXILock_size,				"EXISync",				"DBG A",	FGROUP_EXISync,				0 },
	{  0x13C,   25,    2,    6,    7,    7,	EXILock,			EXILock_size,				"EXISync",				"DBG B",	FGROUP_EXISync,				0 },

	{  0x170,   30,    7,    5,    8,    9,	EXIProbe,			EXIProbe_size,				"__EXIProbe",			"A",		FGROUP___EXIProbe,			0 },
	{  0x170,   30,    7,    5,    8,   10,	EXIProbe,			EXIProbe_size,				"__EXIProbe",			"B",		FGROUP___EXIProbe,			0 },
	{  0x164,   30,    4,    5,    8,   10,	EXIProbe,			EXIProbe_size,				"__EXIProbe",			"C",		FGROUP___EXIProbe,			0 },
	{  0x1B0,   34,    6,    5,    8,    8,	EXIProbe,			EXIProbe_size,				"__EXIProbe",			"PKM",		FGROUP___EXIProbe,			0 },
	{  0x1B8,   38,    5,    7,   10,   10,	EXIProbe,			EXIProbe_size,				"__EXIProbe",			"DBG A",	FGROUP___EXIProbe,			0 },
	{  0x1BC,   39,    5,    7,   10,   10,	EXIProbe,			EXIProbe_size,				"__EXIProbe",			"DBG B",	FGROUP___EXIProbe,			0 },
	{  0x1AC,   38,    2,    7,   10,   10,	EXIProbe,			EXIProbe_size,				"__EXIProbe",			"DBG C",	FGROUP___EXIProbe,			0 },

	{  0x128,   18,    4,    6,   11,    8,	EXISelect,			EXISelect_size,				"EXISelect",			"A",		FGROUP_EXISelect,			0 },
	{  0x128,   18,    4,    6,   11,    7,	EXISelect,			EXISelect_size,				"EXISelect",			"B",		FGROUP_EXISelect,			0 },
	{  0x13C,   20,    4,    6,   11,    6,	EXISelect,			EXISelect_size,				"EXISelect",			"PKM",		FGROUP_EXISelect,			0 },
	{  0x1CC,   33,    3,   10,   17,    6,	EXISelect,			EXISelect_size,				"EXISelect",			"DBG",		FGROUP_EXISelect,			0 },

	{  0x10C,   20,    8,    6,   12,    4,	EXILock,			EXILock_size,				"EXIDeselect",			"A",		FGROUP_EXIDeselect,			0 },
	{  0x10C,   20,    8,    6,   12,    3,	EXILock,			EXILock_size,				"EXIDeselect",			"B",		FGROUP_EXIDeselect,			0 },
	{  0x104,   17,    3,    6,   12,    4,	EXILock,			EXILock_size,				"EXIDeselect",			"PKM",		FGROUP_EXIDeselect,			0 },
	{  0x130,   22,    3,    7,   14,    5,	EXILock,			EXILock_size,				"EXIDeselect",			"DBG A",	FGROUP_EXIDeselect,			0 },
	{  0x12C,   21,    3,    7,   14,    5,	EXILock,			EXILock_size,				"EXIDeselect",			"DBG B",	FGROUP_EXIDeselect,			0 },

	{   0x7C,   10,    3,    0,    1,    7,	NULL,				FCODE_EXIIntrruptHandler,	"EXIntrruptHandler",	"A",		FGROUP_EXIntrruptHandler,	0 },
	{   0xC4,   19,    6,    4,    1,    7,	NULL,				FCODE_EXIIntrruptHandler,	"EXIntrruptHandler",	"B",		FGROUP_EXIntrruptHandler,	0 },
	{   0xC4,   19,    6,    4,    1,    8,	NULL,				FCODE_EXIIntrruptHandler,	"EXIntrruptHandler",	"C",		FGROUP_EXIntrruptHandler,	0 },
	{   0xBC,   16,    3,    4,    1,    3,	NULL,				FCODE_EXIIntrruptHandler,	"EXIntrruptHandler",	"PKM",		FGROUP_EXIntrruptHandler,	0 },
	{   0xC4,   20,    2,    6,    3,    3,	NULL,				FCODE_EXIIntrruptHandler,	"EXIntrruptHandler",	"DBG A",	FGROUP_EXIntrruptHandler,	0 },
	{   0xC8,   21,    2,    6,    3,    3,	NULL,				FCODE_EXIIntrruptHandler,	"EXIntrruptHandler",	"DBG B",	FGROUP_EXIntrruptHandler,	0 },
	{   0xA4,   16,    3,    2,    3,    2,	NULL,				FCODE_EXIIntrruptHandler,	"EXIntrruptHandler",	"DBG C",	FGROUP_EXIntrruptHandler,	0 },

	{  0x1F0,   34,    9,    1,    8,   21,	NULL,				FCODE_EXIIntrruptHandler,	"TCIntrruptHandler",	"A",		FGROUP_TCIntrruptHandler,	0 },
	{  0x214,   41,    9,    5,    8,   22,	NULL,				FCODE_EXIIntrruptHandler,	"TCIntrruptHandler",	"B",		FGROUP_TCIntrruptHandler,	0 },
	{  0x214,   37,    9,    5,    8,   21,	NULL,				FCODE_EXIIntrruptHandler,	"TCIntrruptHandler",	"C",		FGROUP_TCIntrruptHandler,	0 },
	{  0x158,   28,    5,    5,    6,    4,	NULL,				FCODE_EXIIntrruptHandler,	"TCIntrruptHandler",	"PKM",		FGROUP_TCIntrruptHandler,	0 },
	{   0xE4,   22,    3,    8,    3,    3,	NULL,				FCODE_EXIIntrruptHandler,	"TCIntrruptHandler",	"DBG A",	FGROUP_TCIntrruptHandler,	0 },
	{   0xE8,   23,    3,    8,    3,    3,	NULL,				FCODE_EXIIntrruptHandler,	"TCIntrruptHandler",	"DBG B",	FGROUP_TCIntrruptHandler,	0 },
	{   0xC4,   18,    4,    4,    3,    4,	NULL,				FCODE_EXIIntrruptHandler,	"TCIntrruptHandler",	"DBG C",	FGROUP_TCIntrruptHandler,	0 },
//	{   0xA8,   17,    6,    1,    1,    7,	NULL,				FCODE_EXIIntrruptHandler,	"TCIntrruptHandler E",			FGROUP_TCIntrruptHandler,	0 },

	{   0xF0,   17,    7,    5,    5,    7,	EXILock,			EXILock_size,				"EXILock",				"A",		FGROUP_EXILock,				0 },
	{   0xF0,   18,    7,    5,    5,    6,	EXILock,			EXILock_size,				"EXILock",				"B",		FGROUP_EXILock,				0 },
	{   0xF8,   19,    5,    5,    6,    6,	EXILock,			EXILock_size,				"EXILock",				"PKM",		FGROUP_EXILock,				0 },
	{  0x1A4,   35,    5,    9,   13,    6,	EXILock,			EXILock_size,				"EXILock",				"DBG",		FGROUP_EXILock,				0 },

	{   0xD8,   21,    8,    5,    3,    3,	NULL,				FCODE_EXIUnlock,			"EXIUnlock",			"A",		FGROUP_EXIUnlock,			0 },
	{   0xD8,   21,    8,    5,    3,    2,	NULL,				FCODE_EXIUnlock,			"EXIUnlock",			"B",		FGROUP_EXIUnlock,			0 },
	{   0xC4,   18,    4,    5,    3,    3,	NULL,				FCODE_EXIUnlock,			"EXIUnlock",			"PKM",		FGROUP_EXIUnlock,			0 },
	{   0xF0,   23,    4,    6,    5,    4,	NULL,				FCODE_EXIUnlock,			"EXIUnlock",			"DBG A",	FGROUP_EXIUnlock,			0 },
	{   0xEC,   22,    4,    6,    5,    4,	NULL,				FCODE_EXIUnlock,			"EXIUnlock",			"DBG B",	FGROUP_EXIUnlock,			0 },

//	{  0x378,   69,   11,   26,   20,   20,	EXIGetID,			sizeof(EXIGetID),			"EXIGetID",							FGROUP_NONE,				0 },
	{   0xEC,   24,    6,    6,    3,    7,	__CARDReadStatus,	__CARDReadStatus_size,		"__CARDReadStatus",		NULL,		FGROUP_NONE,				0 },
	{   0xA8,   17,    5,    4,    3,    5,	__CARDClearStatus,	__CARDClearStatus_size,		"__CARDClearStatus",	NULL,		FGROUP_NONE,				0 },
	{  0x1B0,   32,    6,    8,   13,   14,	NULL,				FCODE___CARDStat_A,			"__CARDStat",			"A",		FGROUP_CARDStat,			0 },
	{  0x19C,   38,    8,    6,   13,   14,	NULL,				FCODE___CARDStat_B,			"__CARDStat",			"B",		FGROUP_CARDStat,			0 },
	{  0x220,   37,    6,    9,   15,   25,	NULL,				FCODE___CARDStat_C,			"__CARDStat",			"C",		FGROUP_CARDStat,			0 },
//	{  0x130,   33,    8,    6,    5,    2,	__CARDReadSegment,	sizeof(__CARDReadSegment),	"__CARDReadSegment",				FGROUP_NONE,				0 },
//	{   0x60,    7,    6,    1,    1,    3,	__CARDRead,			sizeof(__CARDRead),			"__CARDRead",						FGROUP_NONE,				0 },
//	{   0xDC,   17,    9,    4,    3,    2,	__CARDEraseSector,	sizeof(__CARDEraseSector),	"__CARDEraseSector",				FGROUP_NONE,				0 },
};

enum
{
	PCODE_NORMAL = 0,
	PCODE_TRI,
	PCODE_SI,
	PCODE_EXI,
	PCODE_MAX,
} AllPGroups;

FuncPatterns AllFPatterns[] = 
{
	{ NormalFPatterns, sizeof(NormalFPatterns) / sizeof(FuncPattern), PCODE_NORMAL },
	{ TRIFPatterns, sizeof(TRIFPatterns) / sizeof(FuncPattern), PCODE_TRI },
	{ SIFPatterns, sizeof(SIFPatterns) / sizeof(FuncPattern), PCODE_SI },
	{ EXIFPatterns, sizeof(EXIFPatterns) / sizeof(FuncPattern), PCODE_EXI },
};
