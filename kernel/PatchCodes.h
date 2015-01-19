#ifndef __PATCHCODES_H__
#define __PATCHCODES_H__

#include "global.h"

#include "asm/EXIImm.h"
#include "asm/EXISelect.h"
#include "asm/EXILock.h"
#include "asm/EXIDMA.h"
#include "asm/EXIProbe.h"
#include "asm/EXIGetID.h"
#include "asm/__CARDReadStatus.h"
#include "asm/__CARDClearStatus.h"
#include "asm/__CARDEraseSector.h"

#include "asm/ARQPostRequest.h"
#include "asm/ARInit.h"
#include "asm/ARGetBaseAddress.h"
#include "asm/ARStartDMA.h"
#include "asm/ARStartDMA_PM.h"
#include "asm/ARStartDMA_NFS.h"
#include "asm/ARStartDMA_Hook.h"
#include "asm/__ARHandler.h"
#include "asm/SITransfer.h"
#include "asm/SIGetType.h"
#include "asm/__OSReadROM.h"
#include "asm/FakeInterrupt.h"
#include "asm/FakeInterrupt_DBG.h"
#include "asm/__DVDInterruptHandler.h"
#include "asm/TCIntrruptHandler.h"
#include "asm/SIIntrruptHandler.h"
#include "asm/SIInitStore.h"
#include "asm/PADRead.h"
#include "asm/PADControlAllMotors.h"
#include "asm/PADControlMotor.h"
#include "asm/PADIsBarrel.h"
#include "asm/DVDInquiryAsync.h"
#include "asm/DVDSeekAbsAsyncPrio.h"
#include "asm/GCAMSendCommand.h"
#include "asm/PADReadB.h"
#include "asm/PADReadSteer.h"
#include "asm/PADReadF.h"
#include "asm/PADReadSteerF.h"
#include "asm/PADReadVS.h"
#include "asm/PADReadSteerVS.h"
#include "asm/patch_fwrite_Log.h"
#include "asm/patch_fwrite_GC.h"
#include "asm/FakeRSWLoad.h"
#include "asm/FakeRSWStore.h"
#include "asm/FakeEntryLoad.h"
#include "asm/SwitcherPrs.h"
#include "asm/OSReportDM.h"
#include "asm/AIInitDMA.h"
#include "asm/__DSPHandler.h"
#include "asm/DVDLowAudioStream.h"
#include "asm/DVDLowRequestAudioStatus.h"
#include "asm/DVDLowAudioBufferConfig.h"

#include "asm/DatelTimer.h"

unsigned char SRAM[64] =
{
    0x42, 0x8B,
	0xBD, 0x71,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x17, 0xCA, 0x2A, 0x85, 
    0x00,
	0x00,
	0x00,
	0x3C,
	0x9B, 0x58, 0x5A, 0xB5,		0xB6,0xC7, 0x92, 0xB7,		0x55,0x49, 0xC6, 0x0B, 
    0x4A, 0x09, 0x00, 0x45,		0x0D,0x00, 0xB2, 0x1D,		0x41,0x03, 0x88, 0x1D,
	0x49, 0x41, 0x50, 0x04,
	0x00, 0x00,
	0x00, 0x00,
	0x00, 0x00,
	0x00, 0x00,
	0xFE,
	0x00,
	0xC8, 0xC8,
	0x01, 0x05,
	0x57, 0x28, 
} ;

//this is the data from my wii disk drive
const u8 DiskDriveInfo[32] = {
	0x00,0x00,0x00,0x02,
	0x20,0x08,0x07,0x14, // 7/14/2008
	0x41,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00
};

const u32 __GXSetVAT_patch[31] = {
	/*0x8122ce00,*/ 0x39400000, 0x896904f2, 0x7d284b78,
	0x556007ff, 0x41820050, 0x38e00008, 0x3cc0cc01,
	0x98e68000, 0x61400070, 0x61440080, 0x61430090,
	0x98068000, 0x38000000, 0x80a8001c, 0x90a68000,
	0x98e68000, 0x98868000, 0x8088003c, 0x90868000,
	0x98e68000, 0x98668000, 0x8068005c, 0x90668000,
	0x98068000, 0x556bf87f, 0x394a0001, 0x39080004,
	0x4082ffa0, 0x38000000, 0x980904f2, 0x4e800020
};

const u8 GXIntDfAt04[] = {
	0x02, 0x80, 0x01, 0xE0, 0x01, 0xE0, 0x00, 0x28, 0x00, 0x00, 0x02, 0x80, 0x01, 0xE0, 0x00, 0x00,
};

const u8 GXDeflickerOn[] = {
	0x08, 0x08, 0x0A, 0x0C, 0x0A, 0x08, 0x08
};

const u8 GXDeflickerOff[] = {
	0x00, 0x00, 0x15, 0x16, 0x15, 0x00, 0x00
};

const u32 DVDGetDriveStatus[] = {
        0x38600000,     //  li		r3, 0
        0x4E800020      //  blr
};

// Audio streaming replacement functions copied from Swiss r92
const u32 DVDLowAudioStatusNULL[17] = {
        // execute function(1); passed in on r4
        0x9421FFC0,     //  stwu        sp, -0x0040 (sp)
        0x7C0802A6,     //  mflr        r0
        0x90010000,     //  stw         r0, 0 (sp)
        0x7C8903A6,     //  mtctr       r4
        0x3C80C000,     //  lis         r4, 0xC000
        0x2E830000,     //  cmpwi       cr5, r3, 0
        0x4196000C,     //  beq-        cr5, +0xC ?
        0x38600001,     //  li          r3, 1
        0x48000008,     //  b           +0x8 ?
        0x38600000,     //  li          r3, 0
        0x90642F20,     //  stw         r3, 0x2F20 (r4)
        0x38600001,     //  li          r3, 1
        0x4E800421,     //  bctrl
        0x80010000,     //  lwz         r0, 0 (sp)
        0x7C0803A6,     //  mtlr        r0
        0x38210040,     //  addi        sp, sp, 64
        0x4E800020      //  blr
};

const u32 DVDLowAudioConfigNULL[10] = {
        // execute callback(1); passed in on r5 without actually touching the drive!
        0x9421FFC0,     //  stwu        sp, -0x0040 (sp)
        0x7C0802A6,     //  mflr        r0
        0x90010000,     //  stw         r0, 0 (sp)
        0x7CA903A6,     //  mtctr       r5
        0x38600001,     //  li          r3, 1
        0x4E800421,     //  bctrl
        0x80010000,     //  lwz         r0, 0 (sp)
        0x7C0803A6,     //  mtlr        r0
        0x38210040,     //  addi        sp, sp, 64
        0x4E800020      //  blr
};

const u32 DVDLowReadAudioNULL[10] = {
        // execute callback(1); passed in on r6 without actually touching the drive!
        0x9421FFC0,     //  stwu        sp, -0x0040 (sp)
        0x7C0802A6,     //  mflr        r0
        0x90010000,     //  stw         r0, 0 (sp)
        0x7CC903A6,     //  mtctr       r6
        0x38600001,     //  li          r3, 1
        0x4E800421,     //  bctr;
        0x80010000,     //  lwz         r0, 0 (sp)
        0x7C0803A6,     //  mtlr        r0
        0x38210040,     //  addi        sp, sp, 64
        0x4E800020      //  blr
};

//function header is good enough to verify
const u32 PADIsBarrelOri[] = {
		0x2C030000,		// cmpwi	r3,0
		0x4180000C,		// blt		0x10
		0x2C030004,		// cmpwi	r3,4
		0x4180000C,		// blt		0x18
		0x38600000,		// li		r3,0
		0x4E800020,		// blr
		0x3C008000		// lis		r0,0x8000
};

#endif
