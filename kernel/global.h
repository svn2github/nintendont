#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#define DEBUG		1
#define false		0
#define true		1
//#define CHEATMENU	1
#define EXIPATCH	1
#define CHEATS		1
//#define HID		1
//#define CARDDEBUG 1
#define AUDIOSTREAM 1
#define PATCHALL	1

//#define DEBUG_ES	1
//#define DEBUG_HID	1
//#define DEBUG_DI	1
//#define DEBUG_JVSIO 1
//#define DEBUG_GCAM 1
//#define DEBUG_CARD 1
//#define DEBUG_SD	1
//#define DEBUG_EXI	1
//#define DEBUG_SRAM 1
//#define DEBUG_FST	1
#define DEBUG_PATCH	1

#define REGION_ID_USA	0x45
#define REGION_ID_JAP	0x4A
#define REGION_ID_EUR	0x50

#define UINT_MAX ((unsigned int)0xffffffff)
#define MEM2_BSS __attribute__ ((section (".bss.mem2")))

#define ALIGN_FORWARD(x,align) \
	((typeof(x))((((u32)(x)) + (align) - 1) & (~(align-1))))

#define ALIGN_BACKWARD(x,align) \
	((typeof(x))(((u32)(x)) & (~(align-1))))

#define LINESIZE 0x20
#define CACHESIZE 0x4000

enum AHBDEV {
	AHB_STARLET = 0, //or MEM2 or some controller or bus or ??
	AHB_PPC = 1, //ppc or something else???
	AHB_NAND = 3,
	AHB_AES = 4,
	AHB_SHA1 = 5,
	AHB_EHCI = 6,
	AHB_SDHC = 9,
};

#define	SHARED_PTR	((void *)0x13600000)
#define	SHARED_SIZE	(0x18000)

void fatal(const char *format, ...);

void udelay(int us);

typedef unsigned char u8;
typedef unsigned char uint8_t;
typedef unsigned short u16;
typedef unsigned short uint16_t;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef int bool;
typedef unsigned int sec_t;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;

typedef volatile unsigned char vu8;
typedef volatile unsigned short vu16;
typedef volatile unsigned int vu32;
typedef volatile unsigned long long vu64;

typedef volatile signed char vs8;
typedef volatile signed short vs16;
typedef volatile signed int vs32;
typedef volatile signed long long vs64;

typedef s32 size_t;

typedef u32 u_int32_t;

typedef s32(*ipccallback)(s32 result,void *usrdata);

#define NULL ((void *)0)

#define ALIGNED(x) __attribute__((aligned(x)))

#define STACK_ALIGN(type, name, cnt, alignment)         \
	u8 _al__##name[((sizeof(type)*(cnt)) + (alignment) + \
	(((sizeof(type)*(cnt))%(alignment)) > 0 ? ((alignment) - \
	((sizeof(type)*(cnt))%(alignment))) : 0))]; \
	type *name = (type*)(((u32)(_al__##name)) + ((alignment) - (( \
	(u32)(_al__##name))&((alignment)-1))))

//the builtins in ARM mode are very efficient
#define bswap16 __builtin_bswap16
#define bswap32 __builtin_bswap32
#define bswap64 __builtin_bswap64

#define		HW_REG_BASE			0xd800000
#define		HW_PPCIRQFLAG		(HW_REG_BASE + 0x030)
#define		HW_PPCIRQMASK		(HW_REG_BASE + 0x034)
#define		HW_ARMIRQFLAG		(HW_REG_BASE + 0x038)
#define		HW_ARMIRQMASK		(HW_REG_BASE + 0x03C)
#define		HW_IPC_PPCCTRL		(HW_REG_BASE + 0x004)
#define		HW_IPC_ARMCTRL		(HW_REG_BASE + 0x00C)
#define		HW_IPC_PPCMSG		(HW_REG_BASE + 0x000)
#define		HW_IPC_ARMMSG		(HW_REG_BASE + 0x008)
#define		HW_TIMER			(HW_REG_BASE + 0x010) //increments around every 526.7 nanoseconds
#define		HW_PPCSPEED			(HW_REG_BASE + 0x018)
#define		HW_VERSION			(HW_REG_BASE + 0x214)

#define		MEM_REG_BASE		0xd8b4000
#define		MEM_PROT			(MEM_REG_BASE+0x20a)
#define		MEM_PROT_START		(MEM_REG_BASE+0x20c)
#define		MEM_PROT_END		(MEM_REG_BASE+0x20e)
#define		MEM_FLUSHREQ		(MEM_REG_BASE+0x228)
#define		MEM_FLUSHACK		(MEM_REG_BASE+0x22a)

// TODO: move to hollywood.h once we figure out WTF
#define		HW_100	(HW_REG_BASE + 0x100)
#define		HW_104	(HW_REG_BASE + 0x104)
#define		HW_108	(HW_REG_BASE + 0x108)
#define		HW_10c	(HW_REG_BASE + 0x10c)
#define		HW_110	(HW_REG_BASE + 0x110)
#define		HW_114	(HW_REG_BASE + 0x114)
#define		HW_118	(HW_REG_BASE + 0x118)
#define		HW_11c	(HW_REG_BASE + 0x11c)
#define		HW_120	(HW_REG_BASE + 0x120)
#define		HW_124	(HW_REG_BASE + 0x124)
#define		HW_130	(HW_REG_BASE + 0x130)
#define		HW_134	(HW_REG_BASE + 0x134)
#define		HW_138	(HW_REG_BASE + 0x138)
#define		HW_188	(HW_REG_BASE + 0x188)
#define		HW_18C	(HW_REG_BASE + 0x18c)

#define		ARAM_DIR_MRAM_TO_ARAM       0x00
#define		ARAM_DIR_ARAM_TO_MRAM       0x01

#define HW_BASE			0x0d800000
#define HW_GPIO_ENABLE	(HW_BASE+0xDC)
#define HW_GPIO_OUT		(HW_BASE+0xE0)
#define HW_GPIO_DIR		(HW_BASE+0xE4)
#define HW_GPIO_IN		(HW_BASE+0xE8)
#define HW_GPIO_OWNER	(HW_BASE+0xFC)

//used by exi channel 2 which we dont need
#define EXI2CSR			(HW_BASE+0x6828) //bit 0-13 are usable
#define EXI2MAR			(HW_BASE+0x682C) //bit 4-22 are usable
#define EXI2LENGTH		(HW_BASE+0x6830) //bit 4-22 are usable
#define EXI2CR			(HW_BASE+0x6834) //bit 1-5 are usable
#define EXI2DATA		(HW_BASE+0x6838) //all bits are usable

#define GPIO_POWER		(1<<0)
#define GPIO_SHUTDOWN	(1<<1)
#define GPIO_SLOT_LED	(1<<5)
#define GPIO_SENSOR_BAR (1<<8)
#define P2C(x)			((x)&0x7FFFFFFF)

enum Gameregion
{
	REGION_JAPAN = 0,
	REGION_USA,
	REGION_EXPORT,
};

enum
{
	TRI_NONE = 0,
	TRI_GP1,
	TRI_GP2,
	TRI_AX,
	TRI_VS4
} TRIGames;

typedef struct
{
	u32 data;
	u32 len;
} vector;

typedef struct PADStatus
{
    u16 button;                 // 00 Or-ed PAD_BUTTON_* bits
    s8  stickX;                 // 02 -128 <= stickX       <= 127
    s8  stickY;                 // 03 -128 <= stickY       <= 127

    s8  substickX;              // 04 -128 <= substickX    <= 127
    s8  substickY;              // 05 -128 <= substickY    <= 127
    u8  triggerLeft;            // 06   0 <= triggerLeft  <= 255
    u8  triggerRight;           // 07   0 <= triggerRight <= 255

    u8  analogA;                // 08   0 <= analogA      <= 255
    u8  analogB;                // 09   0 <= analogB      <= 255
    s8  err;                    // 10 one of PAD_ERR_* number
	s8  padding;                // 11 unused
} PADStatus;

#define PAD_BUTTON_LEFT         0x0001
#define PAD_BUTTON_RIGHT        0x0002
#define PAD_BUTTON_DOWN         0x0004
#define PAD_BUTTON_UP           0x0008
#define PAD_TRIGGER_Z           0x0010
#define PAD_TRIGGER_R           0x0020
#define PAD_TRIGGER_L           0x0040
#define PAD_USE_ORIGIN          0x0080
#define PAD_BUTTON_A            0x0100
#define PAD_BUTTON_B            0x0200
#define PAD_BUTTON_X            0x0400
#define PAD_BUTTON_Y            0x0800
#define PAD_BUTTON_MENU         0x1000
#define PAD_BUTTON_START        0x1000

static inline u16 read16(u32 addr)
{
	u32 data;
	__asm__ volatile ("ldrh\t%0, [%1]" : "=l" (data) : "l" (addr));
	return data;
}

static inline void write16(u32 addr, u16 data)
{
	__asm__ volatile ("strh\t%0, [%1]" : : "l" (data), "l" (addr));
}

static inline u32 read32(u32 addr)
{
	u32 data;
	__asm__ volatile ("ldr\t%0, [%1]" : "=l" (data) : "l" (addr));
	return data;
}

static inline void write32(u32 addr, u32 data)
{
	__asm__ volatile ("str\t%0, [%1]" : : "l" (data), "l" (addr));
}

static inline u32 set32(u32 addr, u32 set)
{
	u32 data;
	__asm__ volatile (
		"ldr\t%0, [%1]\n"
		"\torr\t%0, %2\n"
		"\tstr\t%0, [%1]"
		: "=&l" (data)
		: "l" (addr), "l" (set)
	);
	return data;
}

static inline u32 mask32(u32 addr, u32 clear, u32 set)
{
	u32 data;
	__asm__ volatile (
		"ldr\t%0, [%1]\n"
		"\tbic\t%0, %3\n"
		"\torr\t%0, %2\n"
		"\tstr\t%0, [%1]"
		: "=&l" (data)
		: "l" (addr), "l" (set), "l" (clear)
	);
	return data;
}
static inline u32 clear32(u32 addr, u32 clear)
{
	u32 data;
	__asm__ volatile (
		"ldr\t%0, [%1]\n"
		"\tbic\t%0, %2\n"
		"\tstr\t%0, [%1]"
		: "=&l" (data)
		: "l" (addr), "l" (clear)
	);
	return data;
}

static inline u32 TimerDiffTicks(u32 time)
{
	u32 curtime = read32(HW_TIMER);
	if(time > curtime) return UINT_MAX; //wrapped, return UINT_MAX to reset
	return curtime - time;
}

static inline u32 TimerDiffSeconds(u32 time)
{
	u32 curtime = read32(HW_TIMER);
	if(time > curtime) return UINT_MAX; //wrapped, return UINT_MAX to reset
	//pretty accurate, it reports the first second is over about 3.2ms early and
	//with a full 37.7 minutes difference its off by only about 8.6 seconds
	return (((curtime - time) >> 13)*71)>>14;
}

#define IsWiiU ( (*(u32*)0x0d8005A0 >> 16 ) == 0xCAFE )

#endif
