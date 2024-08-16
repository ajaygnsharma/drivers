/*
    This module implements a driver for LPC2468 Special Function Register.
    Copyright (C) 2007  Embedded Artists AB (www.embeddedartists.com)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* #include <linux/config.h> */
#include <linux/types.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <asm/semaphore.h>
/* #include <asm/arch/hardware.h> */
#include "lpc24xx.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/


#if 0
  #define DPRINT(args...) printk(args)
#else
  #define DPRINT(args...)
#endif
#define xDPRINT(args...)

#define SFR_MAJOR 235
#define NUM_SFR_DEVICES 1


#define m_reg_read(reg) (*(volatile unsigned long *)(reg))
#define m_reg_write(reg, data) ((*(volatile unsigned long *)(reg)) = (volatile unsigned long)(data))
#define m_reg_bfs(reg, data) (*(volatile unsigned long *)(reg)) |= (data)
#define m_reg_bfc(reg, data) (*(volatile unsigned long *)(reg)) &= ~(data)

typedef struct
{
  char* regName;
  unsigned long regAddr;
} ea_reg_t;

static int sfr_open(struct inode* inode, 
                    struct file* file);
static int sfr_close(struct inode* inode, 
                     struct file* file);

static ssize_t sfr_write(struct file *p_file, 
	                const char *p_buf, 
		        size_t count, 
                        loff_t *p_pos);



/******************************************************************************
 * Local variables
 *****************************************************************************/

static struct file_operations sfr_fops = {
  .owner   = THIS_MODULE,
  .write   = sfr_write,
  .open    = sfr_open,
  .release = sfr_close,
};

static struct cdev* sfr_cdev = NULL;


static ea_reg_t registers[] =
{
/* Pin Connect Block */
  {"PINSEL0", PINSEL0},
  {"PINSEL1", PINSEL1},
  {"PINSEL2", PINSEL2},
  {"PINSEL3", PINSEL3},
  {"PINSEL4", PINSEL4},
  {"PINSEL5", PINSEL5},
  {"PINSEL6", PINSEL6},
  {"PINSEL7", PINSEL7},
  {"PINSEL8", PINSEL8},
  {"PINSEL9", PINSEL9},
  {"PINSEL10", PINSEL10},

  {"PINMODE0", PINMODE0},
  {"PINMODE1", PINMODE1},
  {"PINMODE2", PINMODE2},
  {"PINMODE3", PINMODE3},
  {"PINMODE4", PINMODE4},
  {"PINMODE5", PINMODE5},
  {"PINMODE6", PINMODE6},
  {"PINMODE7", PINMODE7},
  {"PINMODE8", PINMODE8},
  {"PINMODE9", PINMODE9},

/* General Purpose Input/Output (GPIO) */
  {"IOPIN0", IOPIN0},
  {"IOSET0", IOSET0},
  {"IODIR0", IODIR0},
  {"IOCLR0", IOCLR0},
  {"IOPIN1", IOPIN1},
  {"IOSET1", IOSET1},
  {"IODIR1", IODIR1},
  {"IOCLR1", IOCLR1},

/* GPIO Interrupt Registers */
  {"IO0_INT_EN_R",   IO0_INT_EN_R},
  {"IO0_INT_EN_F",   IO0_INT_EN_F},
  {"IO0_INT_STAT_R", IO0_INT_STAT_R},
  {"IO0_INT_STAT_F", IO0_INT_STAT_F},
  {"IO0_INT_CLR",    IO0_INT_CLR},
  {"IO2_INT_EN_R",   IO2_INT_EN_R},
  {"IO2_INT_EN_F",   IO2_INT_EN_F},
  {"IO2_INT_STAT_R", IO2_INT_STAT_R},
  {"IO2_INT_STAT_F", IO2_INT_STAT_F},
  {"IO2_INT_CLR",    IO2_INT_CLR},
  {"IO_INT_STAT",    IO_INT_STAT},

  {"PARTCFG", PARTCFG},

/* Fast I/O setup */
  {"FIO0DIR",  FIO0DIR},
  {"FIO0MASK", FIO0MASK},
  {"FIO0PIN",  FIO0PIN},
  {"FIO0SET",  FIO0SET},
  {"FIO0CLR",  FIO0CLR},

  {"FIO1DIR",  FIO1DIR},
  {"FIO1MASK", FIO1MASK},
  {"FIO1PIN",  FIO1PIN},
  {"FIO1SET",  FIO1SET},
  {"FIO1CLR",  FIO1CLR},

  {"FIO2DIR",  FIO2DIR},
  {"FIO2MASK", FIO2MASK},
  {"FIO2PIN",  FIO2PIN},
  {"FIO2SET",  FIO2SET},
  {"FIO2CLR",  FIO2CLR},

  {"FIO3DIR",  FIO3DIR},
  {"FIO3MASK", FIO3MASK},
  {"FIO3PIN",  FIO3PIN},
  {"FIO3SET",  FIO3SET},
  {"FIO3CLR",  FIO3CLR},

  {"FIO4DIR",  FIO4DIR},
  {"FIO4MASK", FIO4MASK},
  {"FIO4PIN",  FIO4PIN},
  {"FIO4SET",  FIO4SET},
  {"FIO4CLR",  FIO4CLR},

/* FIOs can be accessed through WORD, HALF-WORD or BYTE. */
  {"FIO0DIR0", FIO0DIR0},
  {"FIO1DIR0", FIO1DIR0},
  {"FIO2DIR0", FIO2DIR0},
  {"FIO3DIR0", FIO3DIR0},
  {"FIO4DIR0", FIO4DIR0},

  {"FIO0DIR1", FIO0DIR1},
  {"FIO1DIR1", FIO1DIR1},
  {"FIO2DIR1", FIO2DIR1},
  {"FIO3DIR1", FIO3DIR1},
  {"FIO4DIR1", FIO4DIR1},

  {"FIO0DIR2", FIO0DIR2},
  {"FIO1DIR2", FIO1DIR2},
  {"FIO2DIR2", FIO2DIR2},
  {"FIO3DIR2", FIO3DIR2},
  {"FIO4DIR2", FIO4DIR2},

  {"FIO0DIR3", FIO0DIR3},
  {"FIO1DIR3", FIO1DIR3},
  {"FIO2DIR3", FIO2DIR3},
  {"FIO3DIR3", FIO3DIR3},
  {"FIO4DIR3", FIO4DIR3},

  {"FIO0DIRL", FIO0DIRL},
  {"FIO1DIRL", FIO1DIRL},
  {"FIO2DIRL", FIO2DIRL},
  {"FIO3DIRL", FIO3DIRL},
  {"FIO4DIRL", FIO4DIRL},

  {"FIO0DIRU", FIO0DIRU},
  {"FIO1DIRU", FIO1DIRU},
  {"FIO2DIRU", FIO2DIRU},
  {"FIO3DIRU", FIO3DIRU},
  {"FIO4DIRU", FIO4DIRU},

  {"FIO0MASK0", FIO0MASK0},
  {"FIO1MASK0", FIO1MASK0},
  {"FIO2MASK0", FIO2MASK0},
  {"FIO3MASK0", FIO3MASK0},
  {"FIO4MASK0", FIO4MASK0},

  {"FIO0MASK1", FIO0MASK1},
  {"FIO1MASK1", FIO1MASK1},
  {"FIO2MASK1", FIO2MASK1},
  {"FIO3MASK1", FIO3MASK1},
  {"FIO4MASK1", FIO4MASK1},

  {"FIO0MASK2", FIO0MASK2},
  {"FIO1MASK2", FIO1MASK2},
  {"FIO2MASK2", FIO2MASK2},
  {"FIO3MASK2", FIO3MASK2},
  {"FIO4MASK2", FIO4MASK2},

  {"FIO0MASK3", FIO0MASK3},
  {"FIO1MASK3", FIO1MASK3},
  {"FIO2MASK3", FIO2MASK3},
  {"FIO3MASK3", FIO3MASK3},
  {"FIO4MASK3", FIO4MASK3},

  {"FIO0MASKL", (FIO_BASE_ADDR + 0x10)}, 
  {"FIO1MASKL", (FIO_BASE_ADDR + 0x30)}, 
  {"FIO2MASKL", (FIO_BASE_ADDR + 0x50)}, 
  {"FIO3MASKL", (FIO_BASE_ADDR + 0x70)}, 
  {"FIO4MASKL", (FIO_BASE_ADDR + 0x90)}, 

  {"FIO0MASKU", (FIO_BASE_ADDR + 0x12)}, 
  {"FIO1MASKU", (FIO_BASE_ADDR + 0x32)}, 
  {"FIO2MASKU", (FIO_BASE_ADDR + 0x52)}, 
  {"FIO3MASKU", (FIO_BASE_ADDR + 0x72)}, 
  {"FIO4MASKU", (FIO_BASE_ADDR + 0x92)}, 

  {"FIO0PIN0", (FIO_BASE_ADDR + 0x14)}, 
  {"FIO1PIN0", (FIO_BASE_ADDR + 0x34)}, 
  {"FIO2PIN0", (FIO_BASE_ADDR + 0x54)}, 
  {"FIO3PIN0", (FIO_BASE_ADDR + 0x74)}, 
  {"FIO4PIN0", (FIO_BASE_ADDR + 0x94)}, 

  {"FIO0PIN1", (FIO_BASE_ADDR + 0x15)}, 
  {"FIO1PIN1", (FIO_BASE_ADDR + 0x25)}, 
  {"FIO2PIN1", (FIO_BASE_ADDR + 0x55)}, 
  {"FIO3PIN1", (FIO_BASE_ADDR + 0x75)}, 
  {"FIO4PIN1", (FIO_BASE_ADDR + 0x95)}, 

  {"FIO0PIN2", (FIO_BASE_ADDR + 0x16)}, 
  {"FIO1PIN2", (FIO_BASE_ADDR + 0x36)}, 
  {"FIO2PIN2", (FIO_BASE_ADDR + 0x56)}, 
  {"FIO3PIN2", (FIO_BASE_ADDR + 0x76)}, 
  {"FIO4PIN2", (FIO_BASE_ADDR + 0x96)}, 

  {"FIO0PIN3", (FIO_BASE_ADDR + 0x17)}, 
  {"FIO1PIN3", (FIO_BASE_ADDR + 0x37)}, 
  {"FIO2PIN3", (FIO_BASE_ADDR + 0x57)}, 
  {"FIO3PIN3", (FIO_BASE_ADDR + 0x77)}, 
  {"FIO4PIN3", (FIO_BASE_ADDR + 0x97)}, 

  {"FIO0PINL", (FIO_BASE_ADDR + 0x14)}, 
  {"FIO1PINL", (FIO_BASE_ADDR + 0x34)}, 
  {"FIO2PINL", (FIO_BASE_ADDR + 0x54)}, 
  {"FIO3PINL", (FIO_BASE_ADDR + 0x74)}, 
  {"FIO4PINL", (FIO_BASE_ADDR + 0x94)}, 

  {"FIO0PINU", (FIO_BASE_ADDR + 0x16)}, 
  {"FIO1PINU", (FIO_BASE_ADDR + 0x36)}, 
  {"FIO2PINU", (FIO_BASE_ADDR + 0x56)}, 
  {"FIO3PINU", (FIO_BASE_ADDR + 0x76)}, 
  {"FIO4PINU", (FIO_BASE_ADDR + 0x96)}, 

  {"FIO0SET0", (FIO_BASE_ADDR + 0x18)}, 
  {"FIO1SET0", (FIO_BASE_ADDR + 0x38)}, 
  {"FIO2SET0", (FIO_BASE_ADDR + 0x58)}, 
  {"FIO3SET0", (FIO_BASE_ADDR + 0x78)}, 
  {"FIO4SET0", (FIO_BASE_ADDR + 0x98)}, 

  {"FIO0SET1", (FIO_BASE_ADDR + 0x19)}, 
  {"FIO1SET1", (FIO_BASE_ADDR + 0x29)}, 
  {"FIO2SET1", (FIO_BASE_ADDR + 0x59)}, 
  {"FIO3SET1", (FIO_BASE_ADDR + 0x79)}, 
  {"FIO4SET1", (FIO_BASE_ADDR + 0x99)}, 

  {"FIO0SET2", (FIO_BASE_ADDR + 0x1A)}, 
  {"FIO1SET2", (FIO_BASE_ADDR + 0x3A)}, 
  {"FIO2SET2", (FIO_BASE_ADDR + 0x5A)}, 
  {"FIO3SET2", (FIO_BASE_ADDR + 0x7A)}, 
  {"FIO4SET2", (FIO_BASE_ADDR + 0x9A)}, 

  {"FIO0SET3", (FIO_BASE_ADDR + 0x1B)}, 
  {"FIO1SET3", (FIO_BASE_ADDR + 0x3B)}, 
  {"FIO2SET3", (FIO_BASE_ADDR + 0x5B)}, 
  {"FIO3SET3", (FIO_BASE_ADDR + 0x7B)}, 
  {"FIO4SET3", (FIO_BASE_ADDR + 0x9B)}, 

  {"FIO0SETL", (FIO_BASE_ADDR + 0x18)}, 
  {"FIO1SETL", (FIO_BASE_ADDR + 0x38)}, 
  {"FIO2SETL", (FIO_BASE_ADDR + 0x58)}, 
  {"FIO3SETL", (FIO_BASE_ADDR + 0x78)}, 
  {"FIO4SETL", (FIO_BASE_ADDR + 0x98)}, 

  {"FIO0SETU", (FIO_BASE_ADDR + 0x1A)}, 
  {"FIO1SETU", (FIO_BASE_ADDR + 0x3A)}, 
  {"FIO2SETU", (FIO_BASE_ADDR + 0x5A)}, 
  {"FIO3SETU", (FIO_BASE_ADDR + 0x7A)}, 
  {"FIO4SETU", (FIO_BASE_ADDR + 0x9A)}, 

  {"FIO0CLR0", (FIO_BASE_ADDR + 0x1C)}, 
  {"FIO1CLR0", (FIO_BASE_ADDR + 0x3C)}, 
  {"FIO2CLR0", (FIO_BASE_ADDR + 0x5C)}, 
  {"FIO3CLR0", (FIO_BASE_ADDR + 0x7C)}, 
  {"FIO4CLR0", (FIO_BASE_ADDR + 0x9C)}, 

  {"FIO0CLR1", (FIO_BASE_ADDR + 0x1D)}, 
  {"FIO1CLR1", (FIO_BASE_ADDR + 0x2D)}, 
  {"FIO2CLR1", (FIO_BASE_ADDR + 0x5D)}, 
  {"FIO3CLR1", (FIO_BASE_ADDR + 0x7D)}, 
  {"FIO4CLR1", (FIO_BASE_ADDR + 0x9D)}, 

  {"FIO0CLR2", (FIO_BASE_ADDR + 0x1E)}, 
  {"FIO1CLR2", (FIO_BASE_ADDR + 0x3E)}, 
  {"FIO2CLR2", (FIO_BASE_ADDR + 0x5E)}, 
  {"FIO3CLR2", (FIO_BASE_ADDR + 0x7E)}, 
  {"FIO4CLR2", (FIO_BASE_ADDR + 0x9E)}, 

  {"FIO0CLR3", (FIO_BASE_ADDR + 0x1F)}, 
  {"FIO1CLR3", (FIO_BASE_ADDR + 0x3F)}, 
  {"FIO2CLR3", (FIO_BASE_ADDR + 0x5F)}, 
  {"FIO3CLR3", (FIO_BASE_ADDR + 0x7F)}, 
  {"FIO4CLR3", (FIO_BASE_ADDR + 0x9F)}, 

  {"FIO0CLRL", (FIO_BASE_ADDR + 0x1C)}, 
  {"FIO1CLRL", (FIO_BASE_ADDR + 0x3C)}, 
  {"FIO2CLRL", (FIO_BASE_ADDR + 0x5C)}, 
  {"FIO3CLRL", (FIO_BASE_ADDR + 0x7C)}, 
  {"FIO4CLRL", (FIO_BASE_ADDR + 0x9C)}, 

  {"FIO0CLRU", (FIO_BASE_ADDR + 0x1E)}, 
  {"FIO1CLRU", (FIO_BASE_ADDR + 0x3E)}, 
  {"FIO2CLRU", (FIO_BASE_ADDR + 0x5E)}, 
  {"FIO3CLRU", (FIO_BASE_ADDR + 0x7E)}, 
  {"FIO4CLRU", (FIO_BASE_ADDR + 0x9E)}, 

/* Memory Accelerator Module (MAM) */
  {"MAMCR", MAMCR},
  {"MAMTIM", MAMTIM},
  {"MEMMAP", MEMMAP},

/* Phase Locked Loop (PLL) */
  {"PLLCON", PLLCON},
  {"PLLCFG", PLLCFG},
  {"PLLSTAT", PLLSTAT},
  {"PLLFEED", PLLFEED},

/* Power Control */
  {"PCON",   (SCB_BASE_ADDR + 0x0C0)},
  {"PCONP",  (SCB_BASE_ADDR + 0x0C4)},

/* Clock Divider */
  {"CCLKCFG", CCLKCFG},
  {"USBCLKCFG", USBCLKCFG},
  {"CLKSRCSEL", CLKSRCSEL},
  {"PCLKSEL0", PCLKSEL0},
  {"PCLKSEL1", PCLKSEL1},
	
/* External Interrupts */
  {"EXTINT", EXTINT},
  {"INTWAKE", INTWAKE},
  {"EXTMODE", EXTMODE},
  {"EXTPOLAR", EXTPOLAR},


/* Reset, reset source identification */
  {"RSIR", RSIR},

/* RSID, code security protection */
  {"CSPR", CSPR},

/* AHB configuration */
  {"AHBCFG1", AHBCFG1},
  {"AHBCFG2", AHBCFG2},

/* System Controls and Status */
  {"SCS", SCS},


/* External Memory Controller (EMC) */
  {"EMC_CTRL", EMC_CTRL},
  {"EMC_STAT", EMC_STAT},
  {"EMC_CONFIG", EMC_CONFIG},

/* Dynamic RAM access registers */
  {"EMC_DYN_CTRL", EMC_DYN_CTRL},
  {"EMC_DYN_RFSH", EMC_DYN_RFSH},
  {"EMC_DYN_RD_CFG", EMC_DYN_RD_CFG},
  {"EMC_DYN_RP", EMC_DYN_RP},
  {"EMC_DYN_RAS", EMC_DYN_RAS},
  {"EMC_DYN_SREX", EMC_DYN_SREX},
  {"EMC_DYN_APR", EMC_DYN_APR},
  {"EMC_DYN_DAL", EMC_DYN_DAL},
  {"EMC_DYN_WR", EMC_DYN_WR},
  {"EMC_DYN_RC", EMC_DYN_RC},
  {"EMC_DYN_RFC", EMC_DYN_RFC},
  {"EMC_DYN_XSR", EMC_DYN_XSR},
  {"EMC_DYN_RRD", EMC_DYN_RRD},
  {"EMC_DYN_MRD", EMC_DYN_MRD},

  {"EMC_DYN_CFG0", EMC_DYN_CFG0},
  {"EMC_DYN_RASCAS0", EMC_DYN_RASCAS0},
  {"EMC_DYN_CFG1", EMC_DYN_CFG1},
  {"EMC_DYN_RASCAS1", EMC_DYN_RASCAS1},
  {"EMC_DYN_CFG2", EMC_DYN_CFG2},
  {"EMC_DYN_RASCAS2", EMC_DYN_RASCAS2},
  {"EMC_DYN_CFG3", EMC_DYN_CFG3},
  {"EMC_DYN_RASCAS3", EMC_DYN_RASCAS3},

/* static RAM access registers */
  {"EMC_STA_CFG0", EMC_STA_CFG0},
  {"EMC_STA_WAITWEN0", EMC_STA_WAITWEN0},
  {"EMC_STA_WAITOEN0", EMC_STA_WAITOEN0},
  {"EMC_STA_WAITRD0", EMC_STA_WAITRD0},
  {"EMC_STA_WAITPAGE0", EMC_STA_WAITPAGE0},
  {"EMC_STA_WAITWR0", EMC_STA_WAITWR0},
  {"EMC_STA_WAITTURN0", EMC_STA_WAITTURN0},

  {"EMC_STA_CFG1", EMC_STA_CFG1},
  {"EMC_STA_WAITWEN1", EMC_STA_WAITWEN1},
  {"EMC_STA_WAITOEN1", EMC_STA_WAITOEN1},
  {"EMC_STA_WAITRD1", EMC_STA_WAITRD1},
  {"EMC_STA_WAITPAGE1", EMC_STA_WAITPAGE1},
  {"EMC_STA_WAITWR1", EMC_STA_WAITWR1},
  {"EMC_STA_WAITTURN1", EMC_STA_WAITTURN1},

  {"EMC_STA_CFG2", EMC_STA_CFG2},
  {"EMC_STA_WAITWEN2", EMC_STA_WAITWEN2},
  {"EMC_STA_WAITOEN2", EMC_STA_WAITOEN2},
  {"EMC_STA_WAITRD2", EMC_STA_WAITRD2},
  {"EMC_STA_WAITPAGE2", EMC_STA_WAITPAGE2},
  {"EMC_STA_WAITWR2", EMC_STA_WAITWR2},
  {"EMC_STA_WAITTURN2", EMC_STA_WAITTURN2},

  {"EMC_STA_CFG3", EMC_STA_CFG3},
  {"EMC_STA_WAITWEN3", EMC_STA_WAITWEN3},
  {"EMC_STA_WAITOEN3", EMC_STA_WAITOEN3},
  {"EMC_STA_WAITRD3", EMC_STA_WAITRD3},
  {"EMC_STA_WAITPAGE3", EMC_STA_WAITPAGE3},
  {"EMC_STA_WAITWR3", EMC_STA_WAITWR3},
  {"EMC_STA_WAITTURN3", EMC_STA_WAITTURN3},

  {"EMC_STA_EXT_WAIT", EMC_STA_EXT_WAIT},

/* Timer 0 */
  {"T0IR", T0IR},
  {"T0TCR", T0TCR},
  {"T0TC", T0TC},
  {"T0PR", T0PR},
  {"T0PC", T0PC},
  {"T0MCR", T0MCR},
  {"T0MR0", T0MR0},
  {"T0MR1", T0MR1},
  {"T0MR2", T0MR2},
  {"T0MR3", T0MR3},
  {"T0CCR", T0CCR},
  {"T0CR0", T0CR0},
  {"T0CR1", T0CR1},
  {"T0CR2", T0CR2},
  {"T0CR3", T0CR3},
  {"T0EMR", T0EMR},
  {"T0CTCR", T0CTCR},

/* Timer 1 */
  {"T1IR", T1IR},
  {"T1TCR", T1TCR},
  {"T1TC", T1TC},
  {"T1PR", T1PR},
  {"T1PC", T1PC},
  {"T1MCR", T1MCR},
  {"T1MR0", T1MR0},
  {"T1MR1", T1MR1},
  {"T1MR2", T1MR2},
  {"T1MR3", T1MR3},
  {"T1CCR", T1CCR},
  {"T1CR0", T1CR0},
  {"T1CR1", T1CR1},
  {"T1CR2", T1CR2},
  {"T1CR3", T1CR3},
  {"T1EMR", T1EMR},
  {"T1CTCR", T1CTCR},

/* Timer 2 */
  {"T2IR", T2IR},
  {"T2TCR", T2TCR},
  {"T2TC", T2TC},
  {"T2PR", T2PR},
  {"T2PC", T2PC},
  {"T2MCR", T2MCR},
  {"T2MR0", T2MR0},
  {"T2MR1", T2MR1},
  {"T2MR2", T2MR2},
  {"T2MR3", T2MR3},
  {"T2CCR", T2CCR},
  {"T2CR0", T2CR0},
  {"T2CR1", T2CR1},
  {"T2CR2", T2CR2},
  {"T2CR3", T2CR3},
  {"T2EMR", T2EMR},
  {"T2CTCR", T2CTCR},

/* Timer 3 */
  {"T3IR", T3IR},
  {"T3TCR", T3TCR},
  {"T3TC", T3TC},
  {"T3PR", T3PR},
  {"T3PC", T3PC},
  {"T3MCR", T3MCR},
  {"T3MR0", T3MR0},
  {"T3MR1", T3MR1},
  {"T3MR2", T3MR2},
  {"T3MR3", T3MR3},
  {"T3CCR", T3CCR},
  {"T3CR0", T3CR0},
  {"T3CR1", T3CR1},
  {"T3CR2", T3CR2},
  {"T3CR3", T3CR3},
  {"T3EMR", T3EMR},
  {"T3CTCR", T3CTCR},

/* Pulse Width Modulator (PWM) */
  {"PWM0IR", PWM0IR},
  {"PWM0TCR", PWM0TCR},
  {"PWM0TC", PWM0TC},
  {"PWM0PR", PWM0PR},
  {"PWM0PC", PWM0PC},
  {"PWM0MCR", PWM0MCR},
  {"PWM0MR0", PWM0MR0},
  {"PWM0MR1", PWM0MR1},
  {"PWM0MR2", PWM0MR2},
  {"PWM0MR3", PWM0MR3},
  {"PWM0CCR", PWM0CCR},
  {"PWM0CR0", PWM0CR0},
  {"PWM0CR1", PWM0CR1},
  {"PWM0CR2", PWM0CR2},
  {"PWM0CR3", PWM0CR3},
  {"PWM0EMR", PWM0EMR},
  {"PWM0MR4", PWM0MR4},
  {"PWM0MR5", PWM0MR5},
  {"PWM0MR6", PWM0MR6},
  {"PWM0PCR", PWM0PCR},
  {"PWM0LER", PWM0LER},
  {"PWM0CTCR", PWM0CTCR},

  {"PWM1IR", PWM1IR},
  {"PWM1TCR", PWM1TCR},
  {"PWM1TC", PWM1TC},
  {"PWM1PR", PWM1PR},
  {"PWM1PC", PWM1PC},
  {"PWM1MCR", PWM1MCR},
  {"PWM1MR0", PWM1MR0},
  {"PWM1MR1", PWM1MR1},
  {"PWM1MR2", PWM1MR2},
  {"PWM1MR3", PWM1MR3},
  {"PWM1CCR", PWM1CCR},
  {"PWM1CR0", PWM1CR0},
  {"PWM1CR1", PWM1CR1},
  {"PWM1CR2", PWM1CR2},
  {"PWM1CR3", PWM1CR3},
  {"PWM1EMR", PWM1EMR},
  {"PWM1MR4", PWM1MR4},
  {"PWM1MR5", PWM1MR5},
  {"PWM1MR6", PWM1MR6},
  {"PWM1PCR", PWM1PCR},
  {"PWM1LER", PWM1LER},
  {"PWM1CTCR", PWM1CTCR},


/* Universal Asynchronous Receiver Transmitter 0 (UART0) */
  {"U0RBR", U0RBR},
  {"U0THR", U0THR},
  {"U0DLL", U0DLL},
  {"U0DLM", U0DLM},
  {"U0IER", U0IER},
  {"U0IIR", U0IIR},
  {"U0FCR", U0FCR},
  {"U0LCR", U0LCR},
  {"U0LSR", U0LSR},
  {"U0SCR", U0SCR},
  {"U0ACR", U0ACR},
  {"U0ICR", U0ICR},
  {"U0FDR", U0FDR},
  {"U0TER", U0TER},

/* Universal Asynchronous Receiver Transmitter 1 (UART1) */
  {"U1RBR", U1RBR},
  {"U1THR", U1THR},
  {"U1DLL", U1DLL},
  {"U1DLM", U1DLM},
  {"U1IER", U1IER},
  {"U1IIR", U1IIR},
  {"U1FCR", U1FCR},
  {"U1LCR", U1LCR},
  {"U1MCR", U1MCR},
  {"U1LSR", U1LSR},
  {"U1MSR", U1MSR},
  {"U1SCR", U1SCR},
  {"U1ACR", U1ACR},
  {"U1FDR", U1FDR},
  {"U1TER", U1TER},

/* Universal Asynchronous Receiver Transmitter 2 (UART2) */
  {"U2RBR", (UART2_BASE_ADDR + 0x00)},
  {"U2THR", (UART2_BASE_ADDR + 0x00)},
  {"U2DLL", (UART2_BASE_ADDR + 0x00)},
  {"U2DLM", (UART2_BASE_ADDR + 0x04)},
  {"U2IER", (UART2_BASE_ADDR + 0x04)},
  {"U2IIR", (UART2_BASE_ADDR + 0x08)},
  {"U2FCR", (UART2_BASE_ADDR + 0x08)},
  {"U2LCR", (UART2_BASE_ADDR + 0x0C)},
  {"U2LSR", (UART2_BASE_ADDR + 0x14)},
  {"U2SCR", (UART2_BASE_ADDR + 0x1C)},
  {"U2ACR", (UART2_BASE_ADDR + 0x20)},
  {"U2ICR", (UART2_BASE_ADDR + 0x24)},
  {"U2FDR", (UART2_BASE_ADDR + 0x28)},
  {"U2TER", (UART2_BASE_ADDR + 0x30)},

/* Universal Asynchronous Receiver Transmitter 3 (UART3) */
  {"U3RBR", (UART3_BASE_ADDR + 0x00)},
  {"U3THR", (UART3_BASE_ADDR + 0x00)},
  {"U3DLL", (UART3_BASE_ADDR + 0x00)},
  {"U3DLM", (UART3_BASE_ADDR + 0x04)},
  {"U3IER", (UART3_BASE_ADDR + 0x04)},
  {"U3IIR", (UART3_BASE_ADDR + 0x08)},
  {"U3FCR", (UART3_BASE_ADDR + 0x08)},
  {"U3LCR", (UART3_BASE_ADDR + 0x0C)},
  {"U3LSR", (UART3_BASE_ADDR + 0x14)},
  {"U3SCR", (UART3_BASE_ADDR + 0x1C)},
  {"U3ACR", (UART3_BASE_ADDR + 0x20)},
  {"U3ICR", (UART3_BASE_ADDR + 0x24)},
  {"U3FDR", (UART3_BASE_ADDR + 0x28)},
  {"U3TER", (UART3_BASE_ADDR + 0x30)},

/* I2C Interface 0 */
  {"I20CONSET", },
  {"I20STAT", },
  {"I20DAT", },
  {"I20ADR", },
  {"I20SCLH", },
  {"I20SCLL", },
  {"I20CONCLR", },

/* I2C Interface 1 */
  {"I21CONSET", (I2C1_BASE_ADDR + 0x00)},
  {"I21STAT", (I2C1_BASE_ADDR + 0x04)},
  {"I21DAT", (I2C1_BASE_ADDR + 0x08)},
  {"I21ADR", (I2C1_BASE_ADDR + 0x0C)},
  {"I21SCLH", (I2C1_BASE_ADDR + 0x10)},
  {"I21SCLL", (I2C1_BASE_ADDR + 0x14)},
  {"I21CONCLR", (I2C1_BASE_ADDR + 0x18)},

/* I2C Interface 2 */
  {"I22CONSET", (I2C2_BASE_ADDR + 0x00)},
  {"I22STAT", (I2C2_BASE_ADDR + 0x04)},
  {"I22DAT", (I2C2_BASE_ADDR + 0x08)},
  {"I22ADR", (I2C2_BASE_ADDR + 0x0C)},
  {"I22SCLH", (I2C2_BASE_ADDR + 0x10)},
  {"I22SCLL", (I2C2_BASE_ADDR + 0x14)},
  {"I22CONCLR", (I2C2_BASE_ADDR + 0x18)},

/* SPI0 (Serial Peripheral Interface 0) */
  {"S0SPCR", (SPI0_BASE_ADDR + 0x00)},
  {"S0SPSR", (SPI0_BASE_ADDR + 0x04)},
  {"S0SPDR", (SPI0_BASE_ADDR + 0x08)},
  {"S0SPCCR", (SPI0_BASE_ADDR + 0x0C)},
  {"S0SPINT", (SPI0_BASE_ADDR + 0x1C)},

/* SSP0 Controller */
  {"SSP0CR0", (SSP0_BASE_ADDR + 0x00)},
  {"SSP0CR1", (SSP0_BASE_ADDR + 0x04)},
  {"SSP0DR", (SSP0_BASE_ADDR + 0x08)},
  {"SSP0SR", (SSP0_BASE_ADDR + 0x0C)},
  {"SSP0CPSR", (SSP0_BASE_ADDR + 0x10)},
  {"SSP0IMSC", (SSP0_BASE_ADDR + 0x14)},
  {"SSP0RIS", (SSP0_BASE_ADDR + 0x18)},
  {"SSP0MIS", (SSP0_BASE_ADDR + 0x1C)},
  {"SSP0ICR", (SSP0_BASE_ADDR + 0x20)},
  {"SSP0DMACR", (SSP0_BASE_ADDR + 0x24)},

/* SSP1 Controller */
  {"SSP1CR0", (SSP1_BASE_ADDR + 0x00)},
  {"SSP1CR1", (SSP1_BASE_ADDR + 0x04)},
  {"SSP1DR", (SSP1_BASE_ADDR + 0x08)},
  {"SSP1SR", (SSP1_BASE_ADDR + 0x0C)},
  {"SSP1CPSR", (SSP1_BASE_ADDR + 0x10)},
  {"SSP1IMSC", (SSP1_BASE_ADDR + 0x14)},
  {"SSP1RIS", (SSP1_BASE_ADDR + 0x18)},
  {"SSP1MIS", (SSP1_BASE_ADDR + 0x1C)},
  {"SSP1ICR", (SSP1_BASE_ADDR + 0x20)},
  {"SSP1DMACR", (SSP1_BASE_ADDR + 0x24)},

/* Real Time Clock */
  {"RTC_ILR", (RTC_BASE_ADDR + 0x00)},
  {"RTC_CTC", (RTC_BASE_ADDR + 0x04)},
  {"RTC_CCR", (RTC_BASE_ADDR + 0x08)},
  {"RTC_CIIR", (RTC_BASE_ADDR + 0x0C)},
  {"RTC_AMR", (RTC_BASE_ADDR + 0x10)},
  {"RTC_CTIME0", (RTC_BASE_ADDR + 0x14)},
  {"RTC_CTIME1", (RTC_BASE_ADDR + 0x18)},
  {"RTC_CTIME2", (RTC_BASE_ADDR + 0x1C)},
  {"RTC_SEC", (RTC_BASE_ADDR + 0x20)},
  {"RTC_MIN", (RTC_BASE_ADDR + 0x24)},
  {"RTC_HOUR", (RTC_BASE_ADDR + 0x28)},
  {"RTC_DOM", (RTC_BASE_ADDR + 0x2C)},
  {"RTC_DOW", (RTC_BASE_ADDR + 0x30)},
  {"RTC_DOY", (RTC_BASE_ADDR + 0x34)},
  {"RTC_MONTH", (RTC_BASE_ADDR + 0x38)},
  {"RTC_YEAR", (RTC_BASE_ADDR + 0x3C)},
  {"RTC_CISS", (RTC_BASE_ADDR + 0x40)},
  {"RTC_ALSEC", (RTC_BASE_ADDR + 0x60)},
  {"RTC_ALMIN", (RTC_BASE_ADDR + 0x64)},
  {"RTC_ALHOUR", (RTC_BASE_ADDR + 0x68)},
  {"RTC_ALDOM", (RTC_BASE_ADDR + 0x6C)},
  {"RTC_ALDOW", (RTC_BASE_ADDR + 0x70)},
  {"RTC_ALDOY", (RTC_BASE_ADDR + 0x74)},
  {"RTC_ALMON", (RTC_BASE_ADDR + 0x78)},
  {"RTC_ALYEAR", (RTC_BASE_ADDR + 0x7C)},
  {"RTC_PREINT", (RTC_BASE_ADDR + 0x80)},
  {"RTC_PREFRAC", (RTC_BASE_ADDR + 0x84)},

/* A/D Converter 0 (AD0) */
  {"AD0CR", AD0CR},
  {"AD0GDR", AD0GDR},
  {"AD0INTEN", AD0INTEN},
  {"AD0DR0", AD0DR0},
  {"AD0DR1", AD0DR1},
  {"AD0DR2", AD0DR2},
  {"AD0DR3", AD0DR3},
  {"AD0DR4", AD0DR4},
  {"AD0DR5", AD0DR5},
  {"AD0DR6", AD0DR6},
  {"AD0DR7", AD0DR7},
  {"AD0STAT", AD0STAT},

/* D/A Converter */
  {"DACR", DACR},

/* Watchdog */
  {"WDMOD", WDMOD},
  {"WDTC", WDTC},
  {"WDFEED", WDFEED},
  {"WDTV", WDTV},
  {"WDCLKSEL", WDCLKSEL},

/* CAN CONTROLLERS AND ACCEPTANCE FILTER */
  {"CAN_AFMR", (CAN_ACCEPT_BASE_ADDR + 0x00)},  	
  {"CAN_SFF_SA", (CAN_ACCEPT_BASE_ADDR + 0x04)},  	
  {"CAN_SFF_GRP_SA", (CAN_ACCEPT_BASE_ADDR + 0x08)},
  {"CAN_EFF_SA", (CAN_ACCEPT_BASE_ADDR + 0x0C)},
  {"CAN_EFF_GRP_SA", (CAN_ACCEPT_BASE_ADDR + 0x10)},  	
  {"CAN_EOT", (CAN_ACCEPT_BASE_ADDR + 0x14)},
  {"CAN_LUT_ERR_ADR", (CAN_ACCEPT_BASE_ADDR + 0x18)},  	
  {"CAN_LUT_ERR", (CAN_ACCEPT_BASE_ADDR + 0x1C)},

  {"CAN_TX_SR", (CAN_CENTRAL_BASE_ADDR + 0x00)},  	
  {"CAN_RX_SR", (CAN_CENTRAL_BASE_ADDR + 0x04)},  	
  {"CAN_MSR", (CAN_CENTRAL_BASE_ADDR + 0x08)},

  {"CAN1MOD", (CAN1_BASE_ADDR + 0x00)},  	
  {"CAN1CMR", (CAN1_BASE_ADDR + 0x04)},  	
  {"CAN1GSR", (CAN1_BASE_ADDR + 0x08)},  	
  {"CAN1ICR", (CAN1_BASE_ADDR + 0x0C)},  	
  {"CAN1IER", (CAN1_BASE_ADDR + 0x10)},
  {"CAN1BTR", (CAN1_BASE_ADDR + 0x14)},  	
  {"CAN1EWL", (CAN1_BASE_ADDR + 0x18)},  	
  {"CAN1SR", (CAN1_BASE_ADDR + 0x1C)},  	
  {"CAN1RFS", (CAN1_BASE_ADDR + 0x20)},  	
  {"CAN1RID", (CAN1_BASE_ADDR + 0x24)},
  {"CAN1RDA", (CAN1_BASE_ADDR + 0x28)},  	
  {"CAN1RDB", (CAN1_BASE_ADDR + 0x2C)},
  	
  {"CAN1TFI1", (CAN1_BASE_ADDR + 0x30)},  	
  {"CAN1TID1", (CAN1_BASE_ADDR + 0x34)},  	
  {"CAN1TDA1", (CAN1_BASE_ADDR + 0x38)},
  {"CAN1TDB1", (CAN1_BASE_ADDR + 0x3C)},  	
  {"CAN1TFI2", (CAN1_BASE_ADDR + 0x40)},  	
  {"CAN1TID2", (CAN1_BASE_ADDR + 0x44)},  	
  {"CAN1TDA2", (CAN1_BASE_ADDR + 0x48)},  	
  {"CAN1TDB2", (CAN1_BASE_ADDR + 0x4C)},
  {"CAN1TFI3", (CAN1_BASE_ADDR + 0x50)},  	
  {"CAN1TID3", (CAN1_BASE_ADDR + 0x54)},  	
  {"CAN1TDA3", (CAN1_BASE_ADDR + 0x58)},  	
  {"CAN1TDB3", (CAN1_BASE_ADDR + 0x5C)},

  {"CAN2MOD", (CAN2_BASE_ADDR + 0x00)},  	
  {"CAN2CMR", (CAN2_BASE_ADDR + 0x04)},  	
  {"CAN2GSR", (CAN2_BASE_ADDR + 0x08)},  	
  {"CAN2ICR", (CAN2_BASE_ADDR + 0x0C)},  	
  {"CAN2IER", (CAN2_BASE_ADDR + 0x10)},
  {"CAN2BTR", (CAN2_BASE_ADDR + 0x14)},  	
  {"CAN2EWL", (CAN2_BASE_ADDR + 0x18)},  	
  {"CAN2SR", (CAN2_BASE_ADDR + 0x1C)},  	
  {"CAN2RFS", (CAN2_BASE_ADDR + 0x20)},  	
  {"CAN2RID", (CAN2_BASE_ADDR + 0x24)},
  {"CAN2RDA", (CAN2_BASE_ADDR + 0x28)},  	
  {"CAN2RDB", (CAN2_BASE_ADDR + 0x2C)},
  	
  {"CAN2TFI1", (CAN2_BASE_ADDR + 0x30)},  	
  {"CAN2TID1", (CAN2_BASE_ADDR + 0x34)},  	
  {"CAN2TDA1", (CAN2_BASE_ADDR + 0x38)},
  {"CAN2TDB1", (CAN2_BASE_ADDR + 0x3C)},  	
  {"CAN2TFI2", (CAN2_BASE_ADDR + 0x40)},  	
  {"CAN2TID2", (CAN2_BASE_ADDR + 0x44)},  	
  {"CAN2TDA2", (CAN2_BASE_ADDR + 0x48)},  	
  {"CAN2TDB2", (CAN2_BASE_ADDR + 0x4C)},
  {"CAN2TFI3", (CAN2_BASE_ADDR + 0x50)},  	
  {"CAN2TID3", (CAN2_BASE_ADDR + 0x54)},  	
  {"CAN2TDA3", (CAN2_BASE_ADDR + 0x58)},  	
  {"CAN2TDB3", (CAN2_BASE_ADDR + 0x5C)},

/* MultiMedia Card Interface(MCI) Controller */
  {"MCI_POWER", (MCI_BASE_ADDR + 0x00)},
  {"MCI_CLOCK", (MCI_BASE_ADDR + 0x04)},
  {"MCI_ARGUMENT", (MCI_BASE_ADDR + 0x08)},
  {"MCI_COMMAND", (MCI_BASE_ADDR + 0x0C)},
  {"MCI_RESP_CMD", (MCI_BASE_ADDR + 0x10)},
  {"MCI_RESP0", (MCI_BASE_ADDR + 0x14)},
  {"MCI_RESP1", (MCI_BASE_ADDR + 0x18)},
  {"MCI_RESP2", (MCI_BASE_ADDR + 0x1C)},
  {"MCI_RESP3", (MCI_BASE_ADDR + 0x20)},
  {"MCI_DATA_TMR", (MCI_BASE_ADDR + 0x24)},
  {"MCI_DATA_LEN", (MCI_BASE_ADDR + 0x28)},
  {"MCI_DATA_CTRL", (MCI_BASE_ADDR + 0x2C)},
  {"MCI_DATA_CNT", (MCI_BASE_ADDR + 0x30)},
  {"MCI_STATUS", (MCI_BASE_ADDR + 0x34)},
  {"MCI_CLEAR", (MCI_BASE_ADDR + 0x38)},
  {"MCI_MASK0", (MCI_BASE_ADDR + 0x3C)},
  {"MCI_MASK1", (MCI_BASE_ADDR + 0x40)},
  {"MCI_FIFO_CNT", (MCI_BASE_ADDR + 0x48)},
  {"MCI_FIFO", (MCI_BASE_ADDR + 0x80)},

/* I2S Interface Controller (I2S) */
  {"I2S_DAO", (I2S_BASE_ADDR + 0x00)},
  {"I2S_DAI", (I2S_BASE_ADDR + 0x04)},
  {"I2S_TX_FIFO", (I2S_BASE_ADDR + 0x08)},
  {"I2S_RX_FIFO", (I2S_BASE_ADDR + 0x0C)},
  {"I2S_STATE", (I2S_BASE_ADDR + 0x10)},
  {"I2S_DMA1", (I2S_BASE_ADDR + 0x14)},
  {"I2S_DMA2", (I2S_BASE_ADDR + 0x18)},
  {"I2S_IRQ", (I2S_BASE_ADDR + 0x1C)},
  {"I2S_TXRATE", (I2S_BASE_ADDR + 0x20)},
  {"I2S_RXRATE", (I2S_BASE_ADDR + 0x24)},

/* General-purpose DMA Controller */
  {"GPDMA_INT_STAT", (DMA_BASE_ADDR + 0x000)},
  {"GPDMA_INT_TCSTAT", (DMA_BASE_ADDR + 0x004)},
  {"GPDMA_INT_TCCLR", (DMA_BASE_ADDR + 0x008)},
  {"GPDMA_INT_ERR_STAT", (DMA_BASE_ADDR + 0x00C)},
  {"GPDMA_INT_ERR_CLR", (DMA_BASE_ADDR + 0x010)},
  {"GPDMA_RAW_INT_TCSTAT", (DMA_BASE_ADDR + 0x014)},
  {"GPDMA_RAW_INT_ERR_STAT", (DMA_BASE_ADDR + 0x018)},
  {"GPDMA_ENABLED_CHNS", (DMA_BASE_ADDR + 0x01C)},
  {"GPDMA_SOFT_BREQ", (DMA_BASE_ADDR + 0x020)},
  {"GPDMA_SOFT_SREQ", (DMA_BASE_ADDR + 0x024)},
  {"GPDMA_SOFT_LBREQ", (DMA_BASE_ADDR + 0x028)},
  {"GPDMA_SOFT_LSREQ", (DMA_BASE_ADDR + 0x02C)},
  {"GPDMA_CONFIG", (DMA_BASE_ADDR + 0x030)},
  {"GPDMA_SYNC", (DMA_BASE_ADDR + 0x034)},

/* DMA channel 0 registers */
  {"GPDMA_CH0_SRC", (DMA_BASE_ADDR + 0x100)},
  {"GPDMA_CH0_DEST", (DMA_BASE_ADDR + 0x104)},
  {"GPDMA_CH0_LLI", (DMA_BASE_ADDR + 0x108)},
  {"GPDMA_CH0_CTRL", (DMA_BASE_ADDR + 0x10C)},
  {"GPDMA_CH0_CFG", (DMA_BASE_ADDR + 0x110)},

/* DMA channel 1 registers */
  {"GPDMA_CH1_SRC", (DMA_BASE_ADDR + 0x120)},
  {"GPDMA_CH1_DEST", (DMA_BASE_ADDR + 0x124)},
  {"GPDMA_CH1_LLI", (DMA_BASE_ADDR + 0x128)},
  {"GPDMA_CH1_CTRL", (DMA_BASE_ADDR + 0x12C)},
  {"GPDMA_CH1_CFG", (DMA_BASE_ADDR + 0x130)},

/* USB Controller */
  {"USB_INT_BASE_ADDR", USB_INT_BASE_ADDR},

  {"USB_INT_STAT", USB_INT_STAT},

/* USB Device Interrupt Registers */
  {"DEV_INT_STAT", DEV_INT_STAT},
  {"DEV_INT_EN", DEV_INT_EN},
  {"DEV_INT_CLR", DEV_INT_CLR},
  {"DEV_INT_SET", DEV_INT_SET},
  {"DEV_INT_PRIO", DEV_INT_PRIO},

/* USB Device Endpoint Interrupt Registers */
  {"EP_INT_STAT", EP_INT_STAT},
  {"EP_INT_EN", EP_INT_EN},
  {"EP_INT_CLR", EP_INT_CLR},
  {"EP_INT_SET", EP_INT_SET},
  {"EP_INT_PRIO", EP_INT_PRIO},

/* USB Device Endpoint Realization Registers */
  {"REALIZE_EP", REALIZE_EP},
  {"EP_INDEX", EP_INDEX},
  {"MAXPACKET_SIZE", MAXPACKET_SIZE},

/* USB Device Command Reagisters */
  {"CMD_CODE", CMD_CODE},
  {"CMD_DATA", CMD_DATA},

/* USB Device Data Transfer Registers */
  {"RX_DATA", RX_DATA},
  {"TX_DATA", TX_DATA},
  {"RX_PLENGTH", RX_PLENGTH},
  {"TX_PLENGTH", TX_PLENGTH},
  {"USB_CTRL", USB_CTRL},

/* USB Device DMA Registers */
  {"DMA_REQ_STAT", DMA_REQ_STAT},
  {"DMA_REQ_CLR", DMA_REQ_CLR},
  {"DMA_REQ_SET", DMA_REQ_SET},
  {"UDCA_HEAD", UDCA_HEAD},
  {"EP_DMA_STAT", EP_DMA_STAT},
  {"EP_DMA_EN", EP_DMA_EN},
  {"EP_DMA_DIS", EP_DMA_DIS},
  {"DMA_INT_STAT", DMA_INT_STAT},
  {"DMA_INT_EN", DMA_INT_EN},
  {"EOT_INT_STAT", EOT_INT_STAT},
  {"EOT_INT_CLR", EOT_INT_CLR},
  {"EOT_INT_SET", EOT_INT_SET},
  {"NDD_REQ_INT_STAT", NDD_REQ_INT_STAT},
  {"NDD_REQ_INT_CLR", NDD_REQ_INT_CLR},
  {"NDD_REQ_INT_SET", NDD_REQ_INT_SET},
  {"SYS_ERR_INT_STAT", SYS_ERR_INT_STAT},
  {"SYS_ERR_INT_CLR", SYS_ERR_INT_CLR},
  {"SYS_ERR_INT_SET", SYS_ERR_INT_SET},


/* USB Host Controller */
  {"HC_REVISION", HC_REVISION},
  {"HC_CONTROL", HC_CONTROL},
  {"HC_CMD_STAT", HC_CMD_STAT},
  {"HC_INT_STAT", HC_INT_STAT},
  {"HC_INT_EN", HC_INT_EN},
  {"HC_INT_DIS", HC_INT_DIS},
  {"HC_HCCA", HC_HCCA},
  {"HC_PERIOD_CUR_ED", HC_PERIOD_CUR_ED},
  {"HC_CTRL_HEAD_ED", HC_CTRL_HEAD_ED},
  {"HC_CTRL_CUR_ED", HC_CTRL_CUR_ED},
  {"HC_BULK_HEAD_ED", HC_BULK_HEAD_ED},
  {"HC_BULK_CUR_ED", HC_BULK_CUR_ED},
  {"HC_DONE_HEAD", HC_DONE_HEAD},
  {"HC_FM_INTERVAL", HC_FM_INTERVAL},
  {"HC_FM_REMAINING", HC_FM_REMAINING},
  {"HC_FM_NUMBER", HC_FM_NUMBER},
  {"HC_PERIOD_START", HC_PERIOD_START},
  {"HC_LS_THRHLD", HC_LS_THRHLD},
  {"HC_RH_DESCA", HC_RH_DESCA},
  {"HC_RH_DESCB", HC_RH_DESCB},
  {"HC_RH_STAT", HC_RH_STAT},
  {"HC_RH_PORT_STAT1", HC_RH_PORT_STAT1},
  {"HC_RH_PORT_STAT2", HC_RH_PORT_STAT2},

/* USB OTG Controller */
  {"OTG_INT_STAT", OTG_INT_STAT},
  {"OTG_INT_EN", OTG_INT_EN},
  {"OTG_INT_SET", OTG_INT_SET},
  {"OTG_INT_CLR", OTG_INT_CLR},
  {"OTG_STAT_CTRL", OTG_STAT_CTRL},
  {"OTG_TIMER", OTG_TIMER},

  {"OTG_I2C_RX", OTG_I2C_RX},
  {"OTG_I2C_TX", OTG_I2C_TX},
  {"OTG_I2C_STS", OTG_I2C_STS},
  {"OTG_I2C_CTL", OTG_I2C_CTL},
  {"OTG_I2C_CLKHI", OTG_I2C_CLKHI},
  {"OTG_I2C_CLKLO", OTG_I2C_CLKLO},

  {"USBOTG_CLK_BASE_ADDR", USBOTG_CLK_BASE_ADDR},
  {"OTG_CLK_CTRL", OTG_CLK_CTRL},
  {"OTG_CLK_STAT", OTG_CLK_STAT},


/* Ethernet MAC (32 bit data bus) -- all registers are RW unless indicated in parentheses */
  {"MAC_MAC1", MAC_MAC1},
  {"MAC_MAC2", MAC_MAC2},
  {"MAC_IPGT", MAC_IPGT},
  {"MAC_IPGR", MAC_IPGR},
  {"MAC_CLRT", MAC_CLRT},
  {"MAC_MAXF", MAC_MAXF},
  {"MAC_SUPP", MAC_SUPP},
  {"MAC_TEST", MAC_TEST},
  {"MAC_MCFG", MAC_MCFG},
  {"MAC_MCMD", MAC_MCMD},
  {"MAC_MADR", MAC_MADR},
  {"MAC_MWTD", MAC_MWTD},
  {"MAC_MRDD", MAC_MRDD},
  {"MAC_MIND", MAC_MIND},

  {"MAC_SA0", MAC_SA0},
  {"MAC_SA1", MAC_SA1},
  {"MAC_SA2", MAC_SA2},

  {"MAC_COMMAND", MAC_COMMAND},
  {"MAC_STATUS", MAC_STATUS},
  {"MAC_RXDESCRIPTOR", MAC_RXDESCRIPTOR},
  {"MAC_RXSTATUS", MAC_RXSTATUS},
  {"MAC_RXDESCRIPTORNUM", MAC_RXDESCRIPTORNUM},
  {"MAC_RXPRODUCEINDEX", MAC_RXPRODUCEINDEX},
  {"MAC_RXCONSUMEINDEX", MAC_RXCONSUMEINDEX},
  {"MAC_TXDESCRIPTOR", MAC_TXDESCRIPTOR},
  {"MAC_TXSTATUS", MAC_TXSTATUS},
  {"MAC_TXDESCRIPTORNUM", MAC_TXDESCRIPTORNUM},
  {"MAC_TXPRODUCEINDEX", MAC_TXPRODUCEINDEX},
  {"MAC_TXCONSUMEINDEX", MAC_TXCONSUMEINDEX},

  {"MAC_TSV0", MAC_TSV0},
  {"MAC_TSV1", MAC_TSV1},
  {"MAC_RSV", MAC_RSV},

  {"MAC_FLOWCONTROLCNT", MAC_FLOWCONTROLCNT},
  {"MAC_FLOWCONTROLSTS", MAC_FLOWCONTROLSTS},

  {"MAC_RXFILTERCTRL", MAC_RXFILTERCTRL},
  {"MAC_RXFILTERWOLSTS", MAC_RXFILTERWOLSTS},
  {"MAC_RXFILTERWOLCLR", MAC_RXFILTERWOLCLR},

  {"MAC_HASHFILTERL", MAC_HASHFILTERL},
  {"MAC_HASHFILTERH", MAC_HASHFILTERH},

  {"MAC_INTSTATUS", MAC_INTSTATUS},
  {"MAC_INTENABLE", MAC_INTENABLE},
  {"MAC_INTCLEAR", MAC_INTCLEAR},
  {"MAC_INTSET", MAC_INTSET},

  {"MAC_POWERDOWN", MAC_POWERDOWN},
  {"MAC_MODULEID", MAC_MODULEID},

};

/******************************************************************************
 * Local functions
 *****************************************************************************/

static char chToUpper(char ch)
{
  if((ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9'))
  {
    return ch;
  }
  else
  {
    return ch - ('a'-'A');
  }   
}

static unsigned long strToInt(const char* pStr, int len, int base)
{
  //                      0,1,2,3,4,5,6,7,8,9,:,;,<,=,>,?,@,A ,B ,C ,D ,E ,F
  static const int v[] = {0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,0,10,11,12,13,14,15}; 
  int i   = 0;
  unsigned long val = 0;
  int dec = 1;
  int idx = 0; 

  for(i = len; i > 0; i--)
  {
    idx = chToUpper(pStr[i-1]) - '0';    

    if(idx > sizeof(v)/sizeof(int))
    {
      printk("strToInt: illegal character %c\n", pStr[i-1]);
      continue;
    }
    
    val += (v[idx]) * dec;
    dec *= base;
  }

  return val; 
}

static int sfr_open(struct inode* inode, 
                    struct file* file)
{
  return 0;
}

static int sfr_close(struct inode* inode, 
                     struct file* file)
{
  return 0;
}

static unsigned long getRegister(char* pReg) 
{
  int i = 0;
  unsigned long reg = 0xFFFFFFFF;  

  // register address
  if(strnicmp("0x", pReg, 2) == 0)
  {
    pReg += 2;

    // always treat reg as hex value
    reg = strToInt(pReg, strlen(pReg), 16);
  }

  // register name
  else
  {
    for(i = 0; i < sizeof(registers)/sizeof(ea_reg_t); i++)
    {
      if(strcmp(registers[i].regName, pReg) == 0)
      {
        reg = registers[i].regAddr;
      }
    }
  }

  return reg;
}

static void help(void)
{
  printk("\n  Read from register:  register:?\n");
  printk("  Write to a register: register:value\n\n");
  printk("  \"register\" can be either a hexadecimal value, e.g., 0xE002C004\n");
  printk("   or an alias, e.g., PINSEL1\n");
}

static ssize_t sfr_write(struct file *filp, 
	                const char *bufp, 
		        size_t count, 
		        loff_t *p_pos)
{
  int base = 10;
  int val = 0;
  int len = 0;
  char* pReg = (char*)bufp;
  char* pVal = NULL;
  unsigned long reg = 0;

        
  DPRINT("sfr_write count=%d\n", count);

  if(strncmp("help", pReg, 4) == 0)
  {
    help();
    return count;
  }

  pVal = strchr(pReg, ':');
  if(pVal != NULL)
  {
    *pVal = '\0';
    pVal++;
  
    reg = getRegister(pReg);

    if(reg == 0xFFFFFFFF)
    {
      printk("  warning: unknown register: %s\n", pReg);
      return count;
    }

    len = strlen(pVal);
    if(strchr(pVal, '\n') != NULL)
      len--;

    if(strnicmp("0x", pVal, 2) == 0)
    {
      pVal += 2;
      len -=  2;
      base = 16; 
    }
   
    // reading a register
    if(*pVal == '?')
    {
      printk("0x%lx\n", m_reg_read(reg));
    }
    else
    {
      val = strToInt(pVal, len, base);
      m_reg_write(reg, val);
    }
  }
  else
  {
    printk("  warning: invalid argument, must be in the format 'register:value'\n");
  }  

  return count;
}


static void __exit sfr_mod_exit(void)
{
  dev_t dev;

  DPRINT("sfr_mod_exit\n");

  dev = MKDEV(SFR_MAJOR, 0);

  cdev_del(sfr_cdev);

  unregister_chrdev_region(dev, NUM_SFR_DEVICES);
}

static int __init sfr_mod_init(void){
  int ret;
  dev_t dev;

  DPRINT("sfr_mod_init\n");

  dev = MKDEV(SFR_MAJOR, 0);

  ret = register_chrdev_region(dev, NUM_SFR_DEVICES, "sfr");

  if(ret)
  {
    printk("sfr: failed to register char device\n");
    return ret;
  }

  sfr_cdev = cdev_alloc();

  cdev_init(sfr_cdev, &sfr_fops);
  sfr_cdev->owner = THIS_MODULE;
  sfr_cdev->ops   = &sfr_fops;

  ret = cdev_add(sfr_cdev, dev, NUM_SFR_DEVICES);

  if (ret)
    printk("pwm: Error adding\n");

  return ret;
}

MODULE_LICENSE("GPL");

module_init(sfr_mod_init);
module_exit(sfr_mod_exit);



