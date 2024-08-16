/*
    This module implements a driver for LPC2468 PWC peripheral.
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

#define PWM_MAJOR 236
#define NUM_PWM_DEVICES 12


// string representation of the supported commands
#define CMD_OPEN_STR       "open"       // open:port
#define CMD_CLOSE_STR      "close"      // close:
#define CMD_SETDUTY_STR    "set duty"   // set duty:duty
#define CMD_SETFREQ_STR    "set freq"   // set freq:freq
#define CMD_HELP_STR       "help"       // help:


// integer representation of the supported commands
#define CMD_INVALID    -1
#define CMD_OPEN       0
#define CMD_CLOSE      1
#define CMD_SETDUTY    2
#define CMD_SETFREQ    3
#define CMD_HELP       4

#define m_reg_read(reg) (*(volatile unsigned long *)(reg))
#define m_reg_write(reg, data) ((*(volatile unsigned long *)(reg)) = (volatile unsigned long)(data))
#define m_reg_bfs(reg, data) (*(volatile unsigned long *)(reg)) |= (data)
#define m_reg_bfc(reg, data) (*(volatile unsigned long *)(reg)) &= ~(data)


typedef struct
{
  int openCnt;
  int pwm;
  unsigned long mr_reg;
  int num;
  int pinsel;
} ea_pwm_output_t;

typedef struct
{
  int openCnt;
  int pin;
} ea_pwm_t;

static int pwm_open(struct inode* inode, 
                    struct file* file);
static int pwm_close(struct inode* inode, 
                     struct file* file);

static ssize_t pwm_write(struct file *p_file, 
	                const char *p_buf, 
		        size_t count, 
                        loff_t *p_pos);



/******************************************************************************
 * Local variables
 *****************************************************************************/

static struct file_operations pwm_fops = {
  .owner   = THIS_MODULE,
  .write   = pwm_write,
  .open    = pwm_open,
  .release = pwm_close,
};

static struct cdev* pwm_cdev = NULL;

DECLARE_MUTEX(pwm_lock);

static char* commands[] = 
{
  CMD_OPEN_STR,
  CMD_CLOSE_STR,
  CMD_SETDUTY_STR,
  CMD_SETFREQ_STR,
  CMD_HELP_STR,
};

static char* commands_help[] = 
{
  "open:pin index",
  "close:",
  "set duty:duty",
  "set freq:freq",
  "help:",
};

static ea_pwm_t pwms[] = 
{
  {0}, // pwm0
  {0}  // pwm1
};

static ea_pwm_output_t pwm_outputs[] =
{
  {0, 0, PWM0MR1, 1, 0}, //pwm0.1 
  {0, 0, PWM0MR2, 2, 0}, //pwm0.2
  {0, 0, PWM0MR3, 3, 0}, //pwm0.3
  {0, 0, PWM0MR4, 4, 0}, //pwm0.4
  {0, 0, PWM0MR5, 5, 0}, //pwm0.5
  {0, 0, PWM0MR6, 6, 0}, //pwm0.6
  {0, 1, PWM1MR1, 1, 0}, //pwm1.1 
  {0, 1, PWM1MR2, 2, 0}, //pwm1.2
  {0, 1, PWM1MR3, 3, 0}, //pwm1.3
  {0, 1, PWM1MR4, 4, 0}, //pwm1.4
  {0, 1, PWM1MR5, 5, 0}, //pwm1.5
  {0, 1, PWM1MR6, 6, 0}, //pwm1.6
};

static unsigned long pinsel_pwm0[] = {PINSEL2, PINSEL7};
static unsigned long pinsel_pwm1[] = {PINSEL3, PINSEL4, PINSEL7};

static int pinsel_pwm0_bits[2][6] =
{
  {3<<4, 3<<6, 3<<10, 3<<12, 3<<14, 3<<22}, // PINSEL2
  {1<<1, 1<<3, 1<<5, 1<<7, 1<<9, 1<<11},    // PINSEL7
};

static int pinsel_pwm0_clearbits[2][6] =
{
  {3<<4, 3<<6, 3<<10, 3<<12, 3<<14, 3<<22}, // PINSEL2
  {3<<0, 3<<2, 3<<4, 3<<6, 3<<8, 3<<10},    // PINSEL7
};

static int pinsel_pwm1_bits[3][6] =
{
  {1<<5, 1<<9, 1<<11, 1<<15, 1<<17, 1<<21}, // PINSEL3
  {1<<0, 1<<2, 1<<4, 1<<6, 1<<8, 1<<10},    // PINSEL4
  {3<<16, 3<<18, 3<<20, 3<<22, 3<<24, 3<<26}, // PINSEL7
};

static int pinsel_pwm1_clearbits[3][6] =
{
  {3<<4, 3<<8, 3<<10, 3<<14, 3<<16, 3<<20}, // PINSEL3
  {3<<0, 3<<2, 3<<4, 3<<6, 3<<8, 3<<10},    // PINSEL4
  {3<<16, 3<<18, 3<<20, 3<<22, 3<<24, 3<<26}, // PINSEL7
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

static int strToInt(const char* pStr, int len, int base)
{
  //                      0,1,2,3,4,5,6,7,8,9,:,;,<,=,>,?,@,A ,B ,C ,D ,E ,F
  static const int v[] = {0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,0,10,11,12,13,14,15}; 
  int i   = 0;
  int val = 0;
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

static int getCommand(const char __user* buf, size_t count, char** ppArg)
{
  int i = 0;
  char* pEnd = NULL;

  pEnd = strchr(buf, ':');
  if(pEnd == NULL)
    return -1;

  *ppArg = pEnd+1;

  for(i = 0; i < sizeof(commands)/sizeof(char*); i++)
  {
    if(strnicmp(commands[i], buf, pEnd-buf) == 0)
    {
      return i;
    }    
  }

  return CMD_INVALID;
}

static void pwmOpen(int num, const char* arg)
{
  int len    = 0;
  int pinsel = 0;
  int pwmNum = pwm_outputs[num].pwm;

  DPRINT("pwmOpen: num=%d, pwmNum=%d, open1=%d, open2=%d\n",
    num, pwmNum, pwms[pwmNum].openCnt, pwm_outputs[num].openCnt);

  len = strlen(arg);
  if(strchr(arg, '\n') != NULL)
    len--;

  if(len > 0)
  {
    pinsel = strToInt(arg, len, 10);
  }

  if(pwms[pwmNum].openCnt == 0)
  {
    if(pwmNum == 0)
    {
      if(pinsel > sizeof(pinsel_pwm0) / sizeof(unsigned long))
        return;

      m_reg_bfs(pinsel_pwm0[pinsel], 
        pinsel_pwm0_bits[pinsel][pwm_outputs[num].num-1]);
    }
    else
    {
      if(pinsel > sizeof(pinsel_pwm1) / sizeof(unsigned long))
        return;

      m_reg_bfs(pinsel_pwm1[pinsel], 
        pinsel_pwm1_bits[pinsel][pwm_outputs[num].num-1]);

    }
  }

  if(pwm_outputs[num].openCnt == 0)
  {
    if(pwmNum == 0)
    {
      // bit num in PCR req is 8 + output number
      m_reg_bfs(PWM0PCR, (1 << (pwm_outputs[num].num+8)));
    }
    else
    {
      // bit num in PCR req is 8 + output number
      m_reg_bfs(PWM1PCR, (1 << (pwm_outputs[num].num+8)));
    }
    
  }

  pwms[pwmNum].openCnt++;
  pwm_outputs[num].openCnt++;
}

static void pwmClose(int num)
{
  int pinsel = 0;
  int pwmNum = pwm_outputs[num].pwm;

  DPRINT("pwmClose: num=%d, pwmNum=%d, open1=%d, open2=%d\n",
    num, pwmNum, pwms[pwmNum].openCnt, pwm_outputs[num].openCnt);

  pwms[pwmNum].openCnt--;
  pwm_outputs[num].openCnt--;  

  pinsel = pwm_outputs[num].pinsel;

  if(pwms[pwmNum].openCnt == 0)
  {
    if(pwmNum == 0)
    {
      m_reg_bfc(pinsel_pwm0[pinsel], 
        pinsel_pwm0_clearbits[pinsel][pwm_outputs[num].num-1]);
    }
    else
    {
      m_reg_bfs(pinsel_pwm1[pinsel], 
        pinsel_pwm1_clearbits[pinsel][pwm_outputs[num].num-1]);

    }
  }

  if(pwm_outputs[num].openCnt == 0)
  {
    if(pwmNum == 0)
    {
      // bit num in PCR req is 8 + output number
      m_reg_bfc(PWM0PCR, (1 << (pwm_outputs[num].num+8)));
    }
    else
    {
      // bit num in PCR req is 8 + output number
      m_reg_bfc(PWM1PCR, (1 << (pwm_outputs[num].num+8)));
    }
    
  }


}

static void pwmDuty(int num, const char* arg)
{
  int pwmNum = pwm_outputs[num].pwm;
  int duty = 0;
  int dec  = 0;
  char* pDec = NULL;
  int len = 0;

  if(pwm_outputs[num].openCnt == 0)
  {  
    printk("  warning: PWM output hasn't been opened\n");
    return;
  }

  len = strlen(arg);
  if(strchr(arg, '\n') != NULL)
    len--;

  // get decimal value
  pDec = strchr(arg, '.');
  if(pDec != NULL)
  {
    dec = strToInt(pDec+1, len-(pDec-arg)-1, 10);
    while(dec >= 10)
      dec /=10;

    len = pDec-arg;
  }

  duty = strToInt(arg, len, 10) * 10;

  if(duty > 1000)
    duty = 1000;
  
  // add the decimal
  duty += dec; 

  
  DPRINT("pwmDuty: num=%d, duty=%d, freq=%x arg=%s\n", 
    num, duty, m_reg_read(PWM1MR0), arg);

  

  if(pwmNum == 0)
  {
    m_reg_write(pwm_outputs[num].mr_reg, 
      ( m_reg_read(PWM0MR0) * duty / 1000 ));
    m_reg_write(PWM0LER, (1 << pwm_outputs[num].num));    
  }
  else
  {
    m_reg_write(pwm_outputs[num].mr_reg, 
      ( m_reg_read(PWM1MR0) * duty / 1000 ));
    m_reg_write(PWM1LER, (1 << pwm_outputs[num].num));    
  }

}

static void pwmFreq(int num, const char* arg)
{
  int pwmNum = pwm_outputs[num].pwm;
  int freq = 0;
  int len = 0;

  if(pwm_outputs[num].openCnt == 0)
  {  
    printk("  warning: PWM output hasn't been opened\n");
    return;
  }

  len = strlen(arg);
  if(strchr(arg, '\n') != NULL)
    len--;

  freq = strToInt(arg, len, 10) * 10;

  if(freq > 500000)
    freq = 500000;
  if(freq < 1)
    freq = 1;
  
  DPRINT("pwmFreq: num=%d, freq=%d arg=%s\n", 
    num, freq, arg);



  if(pwmNum == 0)
  {
    m_reg_write(PWM0MR0, (57600000/freq));
    m_reg_write(PWM0LER, 1);    
  }
  else
  {
    m_reg_write(PWM1MR0, (57600000/freq));
    m_reg_write(PWM1LER, 1);    
  }

}

static void help(void)
{
  int i = 0;

  printk("Available commands:\n");
    
  for(i = 0; i < sizeof(commands_help)/sizeof(char*); i++)
  {
    printk("  %s\n", commands_help[i]);    
  }

  printk("\npin index\n");
  printk("  for pwm channel 0, i.e. /dev/pwm0x:\n");
  printk("    0 - pin 1, PINSEL2\n");
  printk("    1 - pin 3, PINSEL7\n");
  printk("  for pwm channel 1, i.e. /dev/pwm1x:\n");
  printk("    0 - pin 1, PINSEL3\n");
  printk("    1 - pin 2, PINSEL4\n");
  printk("    2 - pin 3, PINSEL7\n");

  printk("\nduty - duty cycle in %%, i.e., value between 1 and 100\n");
  printk("freq - frequency in Hz\n");

}


static int pwm_open(struct inode* inode, 
                    struct file* file)
{

  int addr = 0;
 
  addr = MINOR(inode->i_rdev);

  DPRINT("pwm_open %d\n", addr);

  file->private_data = (void *) addr;

  return 0;
}

static int pwm_close(struct inode* inode, 
                     struct file* file)
{
  return 0;
}


static ssize_t pwm_write(struct file *filp, 
	                const char *bufp, 
		        size_t count, 
		        loff_t *p_pos)
{
  unsigned int num = 0;
  int cmd = 0;
  char* arg = NULL;
        
  num = (unsigned int)filp->private_data;

  DPRINT("pwm_write %d, count=%d\n", addr, count);

  if(num >= NUM_PWM_DEVICES)
  {
    return -ENODEV;
  }

  if (down_interruptible(&pwm_lock))
  {
    return -ERESTARTSYS;
  }

    
  cmd = getCommand(bufp, count, &arg);

  switch(cmd)
  {
  case CMD_OPEN:
    pwmOpen(num, arg);
    break;
  case CMD_CLOSE:
    pwmClose(num);
    break;
  case CMD_SETDUTY:
    pwmDuty(num, arg);
    break;
  case CMD_SETFREQ:
    pwmFreq(num, arg);
    break;
  case CMD_HELP:
    help();
    break;
  default:
    printk("pwm_write: unknown command %s\n", bufp); 
  }

  up(&pwm_lock);

  return count;
}


static void __exit pwm_mod_exit(void)
{
  dev_t dev;

  DPRINT("pwm_mod_exit\n");

  dev = MKDEV(PWM_MAJOR, 0);

  cdev_del(pwm_cdev);

  unregister_chrdev_region(dev, NUM_PWM_DEVICES);
}

static int __init pwm_mod_init(void){
  int ret;
  dev_t dev;

  DPRINT("pwm_mod_init\n");

  dev = MKDEV(PWM_MAJOR, 0);

  ret = register_chrdev_region(dev, NUM_PWM_DEVICES, "pwm");

  if(ret)
  {
    printk("pwm: failed to register char device\n");
    return ret;
  }

  pwm_cdev = cdev_alloc();

  cdev_init(pwm_cdev, &pwm_fops);
  pwm_cdev->owner = THIS_MODULE;
  pwm_cdev->ops   = &pwm_fops;

  ret = cdev_add(pwm_cdev, dev, NUM_PWM_DEVICES);

  if (ret)
    printk("pwm: Error adding\n");

  m_reg_write(PWM0PR,  0x00);   // no prescaling
  m_reg_write(PWM0MCR, 0x02);   // reset counter if MR0 match
  m_reg_write(PWM0MR0, 0x3000); // frequency
  m_reg_write(PWM0MR1, 0x0000);
  m_reg_write(PWM0MR2, 0x0000);
  m_reg_write(PWM0MR3, 0x0000);
  m_reg_write(PWM0MR4, 0x0000);
  m_reg_write(PWM0MR5, 0x0000);
  m_reg_write(PWM0MR6, 0x0000);
  m_reg_write(PWM0LER, 0x7F); // latch
  m_reg_write(PWM0TCR, 0x09); // enable counter and PWM

  m_reg_write(PWM1PR,  0x00);   // no prescaling
  m_reg_write(PWM1MCR, 0x02);   // reset counter if MR0 match
  m_reg_write(PWM1MR0, 0x3000); // frequency
  m_reg_write(PWM1MR1, 0x0000);
  m_reg_write(PWM1MR2, 0x0000);
  m_reg_write(PWM1MR3, 0x0000);
  m_reg_write(PWM1MR4, 0x0000);
  m_reg_write(PWM1MR5, 0x0500); // make sure the LCD is lit
  m_reg_write(PWM1MR6, 0x0000);
  m_reg_write(PWM1LER, 0x7F); // latch
  m_reg_write(PWM1TCR, 0x09); // enable counter and PWM
  

  return ret;
}

MODULE_LICENSE("GPL");

module_init(pwm_mod_init);
module_exit(pwm_mod_exit);



