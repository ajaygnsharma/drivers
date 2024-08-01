/*
    This module implements a linux block device driver for multimedia cards.
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

/******************************************************************************
 * Includes and defines
 */

/* #include <linux/config.h> */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/genhd.h>
#include <linux/major.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/blkdev.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/hdreg.h>
/* #include <asm/arch/hardware.h> */
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
//#include "spi.h"
#include "mmc_hw.h"
#include "lpc24xx.h"

#if 0
#define DPRINT(args...) printk(args)
#else
#define DPRINT(args...)
#endif
#define xDPRINT(args...)

#define MMC_MAJOR 242
#define MMC_SHIFT 6		/* gives us 63 (2^6-1) partitions */
#define MAJOR_NR MMC_MAJOR		
#define DEVICE_NAME "mmc"

#define MMC_MAXUNITS 1
#define MMC_BLOCKSIZE 1024

extern irqreturn_t MCI_IRQHandler(int irq, void * dev_id, struct pt_regs * regs);
extern void DMAHandler (void);

/******************************************************************************
 * Local function prototypes 
 */
static void mmc_request(request_queue_t *q);
static int mmc_ioctl(struct inode *inode, 
		     struct file *file, 
		     unsigned int cmd, 
		     unsigned long arg);
static int mmc_open(struct inode *inode, struct file *file);
static int mmc_media_change(struct gendisk *gd);
static int mmc_revalidate(struct gendisk *gd);
static int mmc_hw_get_parameters(void);

/******************************************************************************
 * Local variables
 */
static spinlock_t lock;
static struct request_queue *mmc_queue;

static struct block_device_operations mmc_bdops =
{
  .ioctl = mmc_ioctl,
  .open = mmc_open,
  .revalidate_disk = mmc_revalidate,
  .media_changed = mmc_media_change,
  .owner = THIS_MODULE,
};

static struct gendisk *mmc_gendisk;

/* these are filled out by a call to mmc_hw_get_parameters */
static int hw_sect_size;	/* in bytes */
static int hw_size;		/* in kbytes */
static int hw_nr_sects;

/* This flag is used to indicate if media has changed. Since
   we do not know when the media has changed it will be set true
   immediately before the module init function returns. This means
   that the flag will be false during add_disk, which is exactly
   what we want. */
static int media_changed = 0;

/******************************************************************************
 * module parameters
 */

int mci_activeHigh = 0; 
module_param(mci_activeHigh, int, S_IRUGO);

/******************************************************************************
 * Local function definitions
 */

/*-----------------------------------------------------------------------------
 * Read hardware paramterers (sector size, size, number of sectors)
 */
static int mmc_hw_get_parameters(void)
{
  unsigned char csddata[16];
  unsigned int sizemult;
  unsigned int size;

  mmc_read_csd(csddata);
  hw_sect_size = 1<<(csddata[5] & 0x0f);
  size = ((csddata[6]&0x03)<<10)+(csddata[7]<<2)+(csddata[8]&0xc0);
  sizemult = ((csddata[10] & 0x80)>>7)+((csddata[9] & 0x03)<<1);
  hw_nr_sects = (size+1)*(1<<(sizemult+2));
  hw_size = hw_nr_sects*hw_sect_size/1024;

  return 0;
}

/*-----------------------------------------------------------------------------
 * Open
 */
static int mmc_open(struct inode *inode, struct file *file)
{
  DPRINT("mmc_open\n");
  check_disk_change(inode->i_bdev);
  return 0;
}

/*-----------------------------------------------------------------------------
 * Media change
 */
static int mmc_media_change(struct gendisk *gd)
{
  DPRINT("mmc_media_change\n");
  return media_changed;
}

/*-----------------------------------------------------------------------------
 * Revalidate
 */
static int mmc_revalidate(struct gendisk *gd)
{
//  int ret;

  DPRINT("mmc_revalidate\n");

  if (!media_changed)
    return 0;

//  if ((ret = mmc_hw_init())) {
//    DPRINT("mmc_hw_init returned %d\n", ret);
//    return -EFAULT;
//  }
  mmc_hw_get_parameters();

  DPRINT("mmc_revalidate: hw_sect_size=%d, hw_size(kb)=%d, hw_nr_sects=%d\n",
	 hw_sect_size, hw_size, hw_nr_sects);

  blk_queue_hardsect_size(mmc_queue, hw_sect_size);

  set_capacity(gd, hw_size*2);	/* hw_size is in kbytes and we want to
				   express the size in 512 bytes
				   sectors */

  return 0;
}

/*-----------------------------------------------------------------------------
 * Ioctl
 */
static int mmc_ioctl(struct inode *inode, 
		     struct file *file, 
		     unsigned int cmd, 
		     unsigned long arg)
{
//  int err;
  long size = 0;
  struct hd_geometry geo;
// int dev;
//  int part1;
//  int npart;
  
  DPRINT("mmc_ioctl cmd=0x%x arg=0x%lx\n", cmd, arg);

  switch(cmd) {
#if 0    
  case BLKGETSIZE:
    /* Return the device size, expressed in sectors */
    if (!arg) return -EINVAL; /* NULL pointer: not valid */
    err = ! access_ok (VERIFY_WRITE, arg, sizeof(long));
    if (err) return -EFAULT;
    size = mmc_sizes[MINOR(inode->i_rdev)]*1024
      / mmc_hardsects[MINOR(inode->i_rdev)];
    DPRINT("mmc_ioctl: BLKGETSIZE size=%ld\n", size);
    if (copy_to_user((long *) arg, &size, sizeof (long)))
      return -EFAULT;
    return 0;

  case BLKRRPART: /* reread partition table */
    part1 = (dev << MMC_SHIFT) + 1;
    npart = (1 << MMC_SHIFT) - 1;
    memset(mmc_sizes + part1,
	   0,
	   npart * sizeof(int));
    memset(mmc_partitions + part1, 
	   0, 
	   npart * sizeof(struct hd_struct));
    mmc_partitions[dev << MMC_SHIFT].nr_sects = hw_nr_sects;

    register_disk(&mmc_gendisk, 
		  inode->i_rdev, 
		  1 << MMC_SHIFT, 
		  &mmc_bdops,
		  hw_nr_sects);

    return 0;
#endif    
  case HDIO_GETGEO:
    /*
     * Get geometry: since we are a virtual device, we have to make
     * up something plausible. So we claim 16 sectors, four heads,
     * and calculate the corresponding number of cylinders. We set
     * the start of data at sector four.
     */
    size = hw_size * 1024 / 512;
    geo.cylinders = (size & ~0x3f) >> 6;
    geo.heads = 8;
    geo.sectors = 32;
    geo.start = 40;
    DPRINT("mmc_ioctl: HDIO_GETGEO cyl=%ld\n", (long)(geo.cylinders));
    if (copy_to_user((void *) arg, &geo, sizeof(geo)))
      return -EFAULT;
    return 0;
    
  }
  
  return -ENOTTY; /* unknown command */
}

/*-----------------------------------------------------------------------------
 * Request handler
 */
static void mmc_request(request_queue_t *q)
{
  struct request *req;
  int i;

  xDPRINT("mmc_request\n");

  while ((req = elv_next_request(q)) != NULL) {
    if (! blk_fs_request(req)) {
      printk("mmc_request: skip non-fs request\n");
      end_request(req, 0);
      continue;
    }

    DPRINT("mmc_request: %s, sector=%lu\n", rq_data_dir(req)!=0? "write" :
	   "read", (unsigned long)(req->sector));

    for (i=0; i < req->current_nr_sectors; i++) {

      if (rq_data_dir(req)) {	/* write */
        mmc_write_sector((req->sector)*512/hw_sect_size+i, req->buffer+(i*512));
      } else {			/* read */
        mmc_read_sector((req->sector)*512/hw_sect_size+i, req->buffer+(i*512));
      }
    }

    end_request(req, 1);
  }
}

/*---------------------------------------------------------------------------- 
 * Module initialization routine
 */
static int __init mmc_mod_init(void)
{
  int ret;

  DPRINT("mmc_mod_init\n");

  if ((ret = mmc_hw_init())) {
    DPRINT("mmc_hw_init returned %d\n", ret);
    return -EAGAIN;
  }
#if MCI_DMA_ENABLED
	/* on DMA channel 0, source is memory, destination is MCI FIFO. */
	/* On DMA channel 1, source is MCI FIFO, destination is memory. */
    DMA_Init();
#endif
/*
  if (request_irq(24, // TODO 
          MCI_IRQHandler, 
          0, 
          "mci", 
          0) != 0)
  {
    printk("ea-mmc: Could not allocate IRQ (IRQ_EXT24)\n");
    return -EFAULT;
  }

  if (request_irq(25, // TODO 
          DMAHandler, 
          0, 
          "gpdma", 
          0) != 0)
  {
    printk("ea-mmc: Could not allocate IRQ (IRQ_EXT25)\n");
    return -EFAULT;
  }
*/
  mmc_hw_get_parameters();


  DPRINT("mmc_mod_init: hw_sect_size=%d, hw_size(kb)=%d, hw_nr_sects=%d\n",
	 hw_sect_size, hw_size, hw_nr_sects);

  ret = register_blkdev(MMC_MAJOR, DEVICE_NAME);
  if (ret < 0) {
    printk("ea-mmc: can't get major %d\n", MMC_MAJOR);
    return ret;
  }

  spin_lock_init(&lock);

  mmc_gendisk = alloc_disk(1 << MMC_SHIFT);
  if (!mmc_gendisk) {
    printk("ea-mmc: alloc_disk failed\n");
    return -EFAULT;
  }
  mmc_queue = blk_init_queue(mmc_request, &lock);
  blk_queue_hardsect_size(mmc_queue, hw_sect_size);

  mmc_gendisk->major = MMC_MAJOR;
  mmc_gendisk->first_minor = 0;
  mmc_gendisk->fops = &mmc_bdops;
  mmc_gendisk->queue = mmc_queue;
  mmc_gendisk->flags = GENHD_FL_REMOVABLE;
  strncpy(mmc_gendisk->disk_name, DEVICE_NAME, 32);
  set_capacity(mmc_gendisk, hw_size*2);	/* hw_size is in kbytes and we want
					   to express the size in 512 bytes 
					   sectors */
  
  media_changed = 0;

  add_disk(mmc_gendisk);

  media_changed = 1;

  DPRINT("mmc_mod_init done!\n");

  return 0;
}

/*---------------------------------------------------------------------------- 
 * Module uninitialization routine
 */
static void __exit mmc_mod_exit(void)
{
  int ret;

  DPRINT("mmc_mod_exit\n");

  del_gendisk(mmc_gendisk);

  blk_cleanup_queue(mmc_queue);

  ret = unregister_blkdev(MMC_MAJOR, DEVICE_NAME);
  if (ret < 0) {
    printk("mmc_mod_exit: unregister_blkdev failed\n");
  }

  free_irq(24, 0);
}

/***************************************************************************** 
 * Module info 
 */
MODULE_LICENSE("GPL");

module_init(mmc_mod_init);
module_exit(mmc_mod_exit);





