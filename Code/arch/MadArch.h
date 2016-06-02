#ifndef __MAD_ARCH_H__
#define __MAD_ARCH_H__

#include "stm32f10x.h"

#define KEIL_MDK
#define MAD_OS_DEBUG

#ifdef MAD_OS_DEBUG
#define MadStatic
#else
#define MadStatic static
#endif
#define MadConst  const

typedef void* 			MadVptr;
typedef unsigned char   MadU8;
typedef unsigned short  MadU16;
typedef unsigned int    MadU32;
typedef char            MadS8;
typedef short           MadS16;
typedef int             MadS32;
typedef MadU32          MadUint;
typedef MadS32          MadInt;
typedef MadU8           MadBool;

typedef MadU32          MadCpsr_t;
typedef MadU32          MadStk_t;
typedef MadU8           MadFlag_t;
typedef MadU32          MadTim_t;

#define MAD_UINT_MAX    (0xFFFFFFFF)
#define MAD_U16_MAX     (0xFFFF)

#define ARM_SYSTICK_CLK        (9000000)
#define SYSTICKS_PER_SEC       (1000)
#define SCB_ISCR               (*(MadU32 *)(0x004 + 0x0D00 + 0xE000E000))
#define PendSV_Mask            (0x00000001<<28)
#define madSched()             do{ SCB_ISCR = PendSV_Mask; __nop();__nop();__nop(); }while(0)
#define madEnterCritical(cpsr) do{ cpsr = __get_BASEPRI(); __set_BASEPRI(0x10); }while(0)
#define madExitCritical(cpsr)  do{ __set_BASEPRI(cpsr); }while(0)
#define madUnRdyMap(res, src)  do{ res = src; __asm{ rbit res, res; clz res, res; } }while(0)

#endif
