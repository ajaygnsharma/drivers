/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2005 Embedded Artists AB
 *
 * Description:
 *    Framework for ARM7 processor
 *
 *****************************************************************************/


#ifndef _framework_h_
#define _framework_h_

#include "config.h"

/******************************************************************************
 * Defines
 *****************************************************************************/

/* Addresses of the instruction that is executed after an exception
   (when vectors are in RAM) */
#define pInst_RESET    (*(unsigned int *)(SRAM_SADDR + 0x00))
#define pInst_UNDEF    (*(unsigned int *)(SRAM_SADDR + 0x04))
#define pInst_SWI      (*(unsigned int *)(SRAM_SADDR + 0x08))
#define pInst_PABORT   (*(unsigned int *)(SRAM_SADDR + 0x0c))
#define pInst_DABORT   (*(unsigned int *)(SRAM_SADDR + 0x10))
#define pInst_IRQ      (*(unsigned int *)(SRAM_SADDR + 0x18))
#define pInst_FIQ      (*(unsigned int *)(SRAM_SADDR + 0x1c))

/* Pointers to the jump addresses that are used after an exception
   (valid both for vectors in FLASH and RAM) */
#define pISR_RESET     (*(unsigned int *)(SRAM_SADDR + 0x20))
#define pISR_UNDEF     (*(unsigned int *)(SRAM_SADDR + 0x24))
#define pISR_SWI       (*(unsigned int *)(SRAM_SADDR + 0x28))
#define pISR_PABORT    (*(unsigned int *)(SRAM_SADDR + 0x2c))
#define pISR_DABORT    (*(unsigned int *)(SRAM_SADDR + 0x30))
#define pISR_IRQ       (*(unsigned int *)(SRAM_SADDR + 0x34))
#define pISR_FIQ       (*(unsigned int *)(SRAM_SADDR + 0x38))

/******************************************************************************
 * External variables
 *****************************************************************************/
//Pointers that define the available heap
extern unsigned char *pHeapStart;
extern unsigned char *pHeapEnd;

/******************************************************************************
 * Public functions
 *****************************************************************************/
void lowLevelInit(void);
void eaInit(void);

#endif
