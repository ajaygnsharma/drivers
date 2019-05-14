/*
    i2c-lpc2xxx.c - Driver for the NXP LPC2xxx i2c controller

    Copyright (C) 2008 Embedded Artists AB.

    Based on i2c-mv64xxx.c

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <asm/hardware.h>
#include <asm/arch/board.h>
#include <asm/io.h>

#define DRIVER "lpc2xxx-i2c"

/* Register defines */


#define LPC2XXX_I2CON_AA   (1<<2) // 0x04
#define LPC2XXX_I2CON_SI   (1<<3) // 0x08
#define LPC2XXX_I2CON_STO  (1<<4) // 0x10
#define LPC2XXX_I2CON_STA  (1<<5) // 0x20
#define LPC2XXX_I2CON_I2EN (1<<6) // 0x40

/* Ctlr status values */
#define	LPC2XXX_I2C_STATUS_BUS_ERR			0x00
#define	LPC2XXX_I2C_STATUS_MAST_START			0x08
#define	LPC2XXX_I2C_STATUS_MAST_REPEAT_START		0x10
#define	LPC2XXX_I2C_STATUS_MAST_WR_ADDR_ACK		0x18
#define	LPC2XXX_I2C_STATUS_MAST_WR_ADDR_NO_ACK		0x20
#define	LPC2XXX_I2C_STATUS_MAST_WR_ACK			0x28
#define	LPC2XXX_I2C_STATUS_MAST_WR_NO_ACK		0x30
#define	LPC2XXX_I2C_STATUS_MAST_LOST_ARB		0x38
#define	LPC2XXX_I2C_STATUS_MAST_RD_ADDR_ACK		0x40
#define	LPC2XXX_I2C_STATUS_MAST_RD_ADDR_NO_ACK		0x48
#define	LPC2XXX_I2C_STATUS_MAST_RD_DATA_ACK		0x50
#define	LPC2XXX_I2C_STATUS_MAST_RD_DATA_NO_ACK		0x58
#define	LPC2XXX_I2C_STATUS_MAST_WR_ADDR_2_ACK		0xd0
#define	LPC2XXX_I2C_STATUS_MAST_WR_ADDR_2_NO_ACK	0xd8
#define	LPC2XXX_I2C_STATUS_MAST_RD_ADDR_2_ACK		0xe0
#define	LPC2XXX_I2C_STATUS_MAST_RD_ADDR_2_NO_ACK	0xe8
#define	LPC2XXX_I2C_STATUS_NO_STATUS			0xf8

/* Driver states */
enum {
	LPC2XXX_I2C_STATE_INVALID,
	LPC2XXX_I2C_STATE_IDLE,
	LPC2XXX_I2C_STATE_WAITING_FOR_START_COND,
	LPC2XXX_I2C_STATE_WAITING_FOR_ADDR_1_ACK,
	LPC2XXX_I2C_STATE_WAITING_FOR_ADDR_2_ACK,
	LPC2XXX_I2C_STATE_WAITING_FOR_SLAVE_ACK,
	LPC2XXX_I2C_STATE_WAITING_FOR_SLAVE_DATA,
};

/* Driver actions */
enum {
	LPC2XXX_I2C_ACTION_INVALID,
	LPC2XXX_I2C_ACTION_CONTINUE,
	LPC2XXX_I2C_ACTION_SEND_START,
	LPC2XXX_I2C_ACTION_SEND_ADDR_1,
	LPC2XXX_I2C_ACTION_SEND_ADDR_2,
	LPC2XXX_I2C_ACTION_SEND_DATA,
	LPC2XXX_I2C_ACTION_RCV_DATA,
	LPC2XXX_I2C_ACTION_RCV_DATA_STOP,
	LPC2XXX_I2C_ACTION_SEND_STOP,
};


struct lpc2xxx_i2c_regs {
	volatile unsigned long I2CONSET;
	volatile unsigned long I2STAT;
	volatile unsigned long I2DAT;
	volatile unsigned long I2ADR;
	volatile unsigned long I2SCLH;
	volatile unsigned long I2SCLL;
	volatile unsigned long I2CONCLR;
};

struct lpc2xxx_i2c_data {
	int			irq;
	u32			state;
	u32			action;
	u32			aborting;
	u32			set_bits;
	u32			clr_bits;
	void __iomem		*reg_base;
	struct lpc2xxx_i2c_regs* regs;
	u32			reg_base_p;
	u32			reg_base_size;
	u32			addr1;
	u32			addr2;
	u32			bytes_left;
	u32			byte_posn;
	u32			block;
	int			rc;
	u32			freq;
	u32			fpclk;
	wait_queue_head_t	waitq;
	spinlock_t		lock;
	struct i2c_msg		*msg;
	struct i2c_adapter	adapter;
};

/*
 *****************************************************************************
 *
 *	Finite State Machine & Interrupt Routines
 *
 *****************************************************************************
 */
static void
lpc2xxx_i2c_fsm(struct lpc2xxx_i2c_data *drv_data, u32 status)
{


	/*
	 * If state is idle, then this is likely the remnants of an old
	 * operation that driver has given up on or the user has killed.
	 * If so, issue the stop condition and go to idle.
	 */
	if (drv_data->state == LPC2XXX_I2C_STATE_IDLE) {
		drv_data->action = LPC2XXX_I2C_ACTION_SEND_STOP;
		return;
	}

	/* The status from the ctlr [mostly] tells us what to do next */
	switch (status) {
	/* Start condition interrupt */
	case LPC2XXX_I2C_STATUS_MAST_START: /* 0x08 */
	case LPC2XXX_I2C_STATUS_MAST_REPEAT_START: /* 0x10 */
		drv_data->action = LPC2XXX_I2C_ACTION_SEND_ADDR_1;
		drv_data->state = LPC2XXX_I2C_STATE_WAITING_FOR_ADDR_1_ACK;

		break;

	/* Performing a write */
	case LPC2XXX_I2C_STATUS_MAST_WR_ADDR_ACK: /* 0x18 */
		if (drv_data->msg->flags & I2C_M_TEN) {
			drv_data->action = LPC2XXX_I2C_ACTION_SEND_ADDR_2;
			drv_data->state =
				LPC2XXX_I2C_STATE_WAITING_FOR_ADDR_2_ACK;
			break;
		}
		/* FALLTHRU */
	case LPC2XXX_I2C_STATUS_MAST_WR_ADDR_2_ACK: /* 0xd0 */
	case LPC2XXX_I2C_STATUS_MAST_WR_ACK: /* 0x28 */
		if ((drv_data->bytes_left == 0)
				|| (drv_data->aborting
					&& (drv_data->byte_posn != 0))) {
			drv_data->action = LPC2XXX_I2C_ACTION_SEND_STOP;
			drv_data->state = LPC2XXX_I2C_STATE_IDLE;

			drv_data->set_bits &= ~LPC2XXX_I2CON_AA;
		} else {
			drv_data->action = LPC2XXX_I2C_ACTION_SEND_DATA;
			drv_data->state =
				LPC2XXX_I2C_STATE_WAITING_FOR_SLAVE_ACK;
			drv_data->bytes_left--;
		}
		break;

	/* Performing a read */
	case LPC2XXX_I2C_STATUS_MAST_RD_ADDR_ACK: /* 40 */

		if (drv_data->msg->flags & I2C_M_TEN) {
			drv_data->action = LPC2XXX_I2C_ACTION_SEND_ADDR_2;
			drv_data->state =
				LPC2XXX_I2C_STATE_WAITING_FOR_ADDR_2_ACK;
			break;
		}
		/* FALLTHRU */
	case LPC2XXX_I2C_STATUS_MAST_RD_ADDR_2_ACK: /* 0xe0 */
		if (drv_data->bytes_left == 0) {
			drv_data->action = LPC2XXX_I2C_ACTION_SEND_STOP;
			drv_data->state = LPC2XXX_I2C_STATE_IDLE;
			break;
		}
		/* FALLTHRU */
	case LPC2XXX_I2C_STATUS_MAST_RD_DATA_ACK: /* 0x50 */
		if (status != LPC2XXX_I2C_STATUS_MAST_RD_DATA_ACK)
			drv_data->action = LPC2XXX_I2C_ACTION_CONTINUE;
		else {
			drv_data->action = LPC2XXX_I2C_ACTION_RCV_DATA;
			drv_data->bytes_left--;
		}
		drv_data->state = LPC2XXX_I2C_STATE_WAITING_FOR_SLAVE_DATA;

		if ((drv_data->bytes_left <= 1) || drv_data->aborting) {
			drv_data->set_bits &= ~LPC2XXX_I2CON_AA;
			drv_data->clr_bits |= LPC2XXX_I2CON_AA;
		}

		break;

	case LPC2XXX_I2C_STATUS_MAST_RD_DATA_NO_ACK: /* 0x58 */
		drv_data->action = LPC2XXX_I2C_ACTION_RCV_DATA_STOP;
		drv_data->state = LPC2XXX_I2C_STATE_IDLE;
		break;

	case LPC2XXX_I2C_STATUS_MAST_WR_ADDR_NO_ACK: /* 0x20 */
	case LPC2XXX_I2C_STATUS_MAST_WR_NO_ACK: /* 30 */
	case LPC2XXX_I2C_STATUS_MAST_RD_ADDR_NO_ACK: /* 48 */
		/* Doesn't seem to be a device at other end */
		drv_data->action = LPC2XXX_I2C_ACTION_SEND_STOP;
		drv_data->state = LPC2XXX_I2C_STATE_IDLE;
		drv_data->rc = -ENODEV;
		break;

	default:
		dev_err(&drv_data->adapter.dev,
			"LPC2xxx_i2c_fsm: Ctlr Error -- state: 0x%x, "
			"status: 0x%x, addr: 0x%x, flags: 0x%x\n",
			 drv_data->state, status, drv_data->msg->addr,
			 drv_data->msg->flags);
		drv_data->action = LPC2XXX_I2C_ACTION_SEND_STOP;
		drv_data->state = LPC2XXX_I2C_STATE_IDLE;
		drv_data->rc = -EIO;
	}
}

static void
lpc2xxx_i2c_do_action(struct lpc2xxx_i2c_data *drv_data)
{

	switch(drv_data->action) {
	case LPC2XXX_I2C_ACTION_CONTINUE:

		drv_data->regs->I2CONCLR = drv_data->clr_bits;
		drv_data->regs->I2CONSET = drv_data->set_bits;

		break;

	case LPC2XXX_I2C_ACTION_SEND_START:

		drv_data->regs->I2CONCLR = drv_data->clr_bits;
		drv_data->regs->I2CONSET = (drv_data->set_bits|LPC2XXX_I2CON_STA);		

		break;

	case LPC2XXX_I2C_ACTION_SEND_ADDR_1:

		drv_data->regs->I2DAT = drv_data->addr1;
		drv_data->regs->I2CONCLR = drv_data->clr_bits;
		break;

	case LPC2XXX_I2C_ACTION_SEND_ADDR_2:
		break;

	case LPC2XXX_I2C_ACTION_SEND_DATA:
		drv_data->regs->I2DAT = drv_data->msg->buf[drv_data->byte_posn++];
		drv_data->regs->I2CONCLR = drv_data->clr_bits;
		
		break;

	case LPC2XXX_I2C_ACTION_RCV_DATA:
		
		drv_data->msg->buf[drv_data->byte_posn++] = drv_data->regs->I2DAT;

		drv_data->regs->I2CONCLR = drv_data->clr_bits;
		drv_data->regs->I2CONSET = drv_data->set_bits;
				
		break;

	case LPC2XXX_I2C_ACTION_RCV_DATA_STOP:

		drv_data->msg->buf[drv_data->byte_posn++] = drv_data->regs->I2DAT;

		drv_data->set_bits &= ~LPC2XXX_I2CON_I2EN;
		drv_data->clr_bits |= LPC2XXX_I2CON_I2EN;

		drv_data->regs->I2CONSET = (drv_data->set_bits|LPC2XXX_I2CON_STO);
		drv_data->regs->I2CONCLR = drv_data->set_bits;

		drv_data->block = 0;
		wake_up_interruptible(&drv_data->waitq);
		break;

	case LPC2XXX_I2C_ACTION_INVALID:
	default:
		dev_err(&drv_data->adapter.dev,
			"lpc2xxx_i2c_do_action: Invalid action: %d\n",
			drv_data->action);
		drv_data->rc = -EIO;
		/* FALLTHRU */
	case LPC2XXX_I2C_ACTION_SEND_STOP:
		
		drv_data->set_bits &= ~LPC2XXX_I2CON_I2EN;

		drv_data->regs->I2CONCLR = drv_data->clr_bits;
		drv_data->regs->I2CONSET = (drv_data->set_bits|LPC2XXX_I2CON_STO);

		drv_data->block = 0;
		wake_up_interruptible(&drv_data->waitq);
		break;
	}
}

static int
lpc2xxx_i2c_intr(int irq, void *dev_id)
{
	struct lpc2xxx_i2c_data	*drv_data = dev_id;
	unsigned long	flags;
	u32		status;
	int		rc = IRQ_NONE;

	spin_lock_irqsave(&drv_data->lock, flags);
	while (drv_data->regs->I2CONSET & LPC2XXX_I2CON_SI) {
		status = drv_data->regs->I2STAT;

		lpc2xxx_i2c_fsm(drv_data, status);
		lpc2xxx_i2c_do_action(drv_data);
		rc = IRQ_HANDLED;
	}
	spin_unlock_irqrestore(&drv_data->lock, flags);

	return rc;
}

/*
 *****************************************************************************
 *
 *	I2C Msg Execution Routines
 *
 *****************************************************************************
 */
static void
lpc2xxx_i2c_prepare_for_io(struct lpc2xxx_i2c_data *drv_data,
	struct i2c_msg *msg)
{
	u32	dir = 0;

	drv_data->msg = msg;
	drv_data->byte_posn = 0;
	drv_data->bytes_left = msg->len;
	drv_data->aborting = 0;
	drv_data->rc = 0;

	drv_data->set_bits = (LPC2XXX_I2CON_AA|LPC2XXX_I2CON_I2EN);
	drv_data->clr_bits = (LPC2XXX_I2CON_STA|LPC2XXX_I2CON_SI);

	if (msg->flags & I2C_M_RD) {
		dir = 1;
	} 

	if (msg->flags & I2C_M_REV_DIR_ADDR)
		dir ^= 1;

	if (msg->flags & I2C_M_TEN) {
		drv_data->addr1 = 0xf0 | (((u32)msg->addr & 0x300) >> 7) | dir;
		drv_data->addr2 = (u32)msg->addr & 0xff;
	} else {
		drv_data->addr1 = ((u32)msg->addr & 0x7f) << 1 | dir;
		drv_data->addr2 = 0;
	}
}

static void
lpc2xxx_i2c_wait_for_completion(struct lpc2xxx_i2c_data *drv_data)
{
	long		time_left;
	unsigned long	flags;
	char		abort = 0;

	time_left = wait_event_interruptible_timeout(drv_data->waitq,
		!drv_data->block, msecs_to_jiffies(drv_data->adapter.timeout));

	spin_lock_irqsave(&drv_data->lock, flags);
	if (!time_left) { /* Timed out */
		drv_data->rc = -ETIMEDOUT;
		abort = 1;
	} else if (time_left < 0) { /* Interrupted/Error */
		drv_data->rc = time_left; /* errno value */
		abort = 1;
	}

	if (abort && drv_data->block) {
		drv_data->aborting = 1;
		spin_unlock_irqrestore(&drv_data->lock, flags);

		time_left = wait_event_timeout(drv_data->waitq,
			!drv_data->block,
			msecs_to_jiffies(drv_data->adapter.timeout));

		if ((time_left <= 0) && drv_data->block) {
			drv_data->state = LPC2XXX_I2C_STATE_IDLE;
			dev_err(&drv_data->adapter.dev,
				"lpc2xxx: I2C bus locked, block: %d, "
				"time_left: %d\n", drv_data->block,
				(int)time_left);
		}
	} else
		spin_unlock_irqrestore(&drv_data->lock, flags);
}

static int
lpc2xxx_i2c_execute_msg(struct lpc2xxx_i2c_data *drv_data, struct i2c_msg *msg)
{
	unsigned long	flags;

	spin_lock_irqsave(&drv_data->lock, flags);
	lpc2xxx_i2c_prepare_for_io(drv_data, msg);

	if (unlikely(msg->flags & I2C_M_NOSTART)) { /* Skip start/addr phases */
		if (drv_data->msg->flags & I2C_M_RD) {
			/* No action to do, wait for slave to send a byte */
			drv_data->action = LPC2XXX_I2C_ACTION_CONTINUE;
			drv_data->state =
				LPC2XXX_I2C_STATE_WAITING_FOR_SLAVE_DATA;
		} else {
			drv_data->action = LPC2XXX_I2C_ACTION_SEND_DATA;
			drv_data->state =
				LPC2XXX_I2C_STATE_WAITING_FOR_SLAVE_ACK;
			drv_data->bytes_left--;
		}
	} else {
		drv_data->action = LPC2XXX_I2C_ACTION_SEND_START;
		drv_data->state = LPC2XXX_I2C_STATE_WAITING_FOR_START_COND;
	}

	drv_data->block = 1;
	lpc2xxx_i2c_do_action(drv_data);
	spin_unlock_irqrestore(&drv_data->lock, flags);

	lpc2xxx_i2c_wait_for_completion(drv_data);
	return drv_data->rc;
}

/*
 *****************************************************************************
 *
 *	I2C Core Support Routines (Interface to higher level I2C code)
 *
 *****************************************************************************
 */
static u32
lpc2xxx_i2c_func(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

static int
lpc2xxx_i2c_xfer(struct i2c_adapter *adap, struct i2c_msg msgs[], int num)
{
	struct lpc2xxx_i2c_data *drv_data = i2c_get_adapdata(adap);
	int	i, rc;

	for (i=0; i<num; i++)
		if ((rc = lpc2xxx_i2c_execute_msg(drv_data, &msgs[i])) < 0)
			return rc;

	return num;
}

static const struct i2c_algorithm lpc2xxx_i2c_algo = {
	.master_xfer   = lpc2xxx_i2c_xfer,
	.functionality = lpc2xxx_i2c_func,
};

/*
 *****************************************************************************
 *
 *	Driver Interface & Early Init Routines
 *
 *****************************************************************************
 */
static void __devinit
lpc2xxx_i2c_hw_init(struct lpc2xxx_i2c_data *drv_data)
{
	int divider = 0;

	divider = drv_data->fpclk / drv_data->freq;

	drv_data->regs->I2SCLL = divider / 2;
	drv_data->regs->I2SCLH = divider / 2;
	

	drv_data->state = LPC2XXX_I2C_STATE_IDLE;
}

static int __devinit
lpc2xxx_i2c_map_regs(struct platform_device *pd,
	struct lpc2xxx_i2c_data *drv_data)
{
	struct resource	*r;

	if ((r = platform_get_resource(pd, IORESOURCE_MEM, 0)) &&
		request_mem_region(r->start, (r->end - r->start + 1),
			drv_data->adapter.name)) {
		drv_data->reg_base_size = r->end - r->start + 1;
		drv_data->reg_base = ioremap(r->start,
			drv_data->reg_base_size);
		drv_data->reg_base_p = r->start;
		
	} else
		return -ENOMEM;

	return 0;
}

static void __devexit
lpc2xxx_i2c_unmap_regs(struct lpc2xxx_i2c_data *drv_data)
{
	if (drv_data->reg_base) {
		iounmap(drv_data->reg_base);
		release_mem_region(drv_data->reg_base_p,
			drv_data->reg_base_size);
	}

	drv_data->reg_base = NULL;
	drv_data->reg_base_p = 0;
}

static int __devinit
lpc2xxx_i2c_probe(struct platform_device *pd)
{
	struct lpc2xxx_i2c_data	*drv_data;
	struct lpc2xxx_i2c_pdata	*pdata = pd->dev.platform_data;
	int	rc;

	if ((pd->id != 0) || !pdata) {
		return -ENODEV;
	}

	drv_data = kzalloc(sizeof(struct lpc2xxx_i2c_data), GFP_KERNEL);
	if (!drv_data)
		return -ENOMEM;

	if (lpc2xxx_i2c_map_regs(pd, drv_data)) {
		rc = -ENODEV;
		goto exit_kfree;
	}

	strlcpy(drv_data->adapter.name, DRIVER " adapter",
		I2C_NAME_SIZE);

	init_waitqueue_head(&drv_data->waitq);
	spin_lock_init(&drv_data->lock);

	drv_data->freq  = pdata->freq;
	drv_data->fpclk = pdata->fpclk;

	drv_data->irq = platform_get_irq(pd, 0);
	if (drv_data->irq < 0) {
		rc = -ENXIO;
		goto exit_unmap_regs;
	}
	drv_data->regs = (struct lpc2xxx_i2c_regs *)drv_data->reg_base;
	drv_data->adapter.dev.parent = &pd->dev;
	drv_data->adapter.id = I2C_HW_A_LPC2XXX;
	drv_data->adapter.algo = &lpc2xxx_i2c_algo;
	drv_data->adapter.owner = THIS_MODULE;
	drv_data->adapter.class = I2C_CLASS_HWMON;
	drv_data->adapter.timeout = pdata->timeout;
	drv_data->adapter.retries = pdata->retries;
	platform_set_drvdata(pd, drv_data);
	i2c_set_adapdata(&drv_data->adapter, drv_data);

	lpc2xxx_i2c_hw_init(drv_data);

	if (request_irq(drv_data->irq, lpc2xxx_i2c_intr, 0,
			DRIVER, drv_data)) {
		dev_err(&drv_data->adapter.dev,
			"lpc2xxx: Can't register intr handler irq: %d\n",
			drv_data->irq);
		rc = -EINVAL;
		goto exit_unmap_regs;
	} else if ((rc = i2c_add_adapter(&drv_data->adapter)) != 0) {
		dev_err(&drv_data->adapter.dev,
			"lpc2xxx: Can't add i2c adapter, rc: %d\n", -rc);
		goto exit_free_irq;
	}

	return 0;

	exit_free_irq:
		free_irq(drv_data->irq, drv_data);
	exit_unmap_regs:
		lpc2xxx_i2c_unmap_regs(drv_data);
	exit_kfree:
		kfree(drv_data);
	return rc;
}

static int __devexit
lpc2xxx_i2c_remove(struct platform_device *dev)
{
	struct lpc2xxx_i2c_data *drv_data = platform_get_drvdata(dev);
	int	rc;

	rc = i2c_del_adapter(&drv_data->adapter);
	free_irq(drv_data->irq, drv_data);
	lpc2xxx_i2c_unmap_regs(drv_data);
	
	kfree(drv_data);

	return rc;
}

static struct platform_driver lpc2xxx_i2c_driver = {
	.probe		= lpc2xxx_i2c_probe,
	.remove		= lpc2xxx_i2c_remove,
//	.suspend	= lpc22xx_i2c_suspend,
//	.resume		= lpc22xx_i2c_resume,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= DRIVER,
	},
};

static int __init
lpc2xxx_i2c_init(void)
{
	return platform_driver_register(&lpc2xxx_i2c_driver);
}

static void __exit
lpc2xxx_i2c_exit(void)
{
	platform_driver_unregister(&lpc2xxx_i2c_driver);
}

module_init(lpc2xxx_i2c_init);
module_exit(lpc2xxx_i2c_exit);

MODULE_AUTHOR("Embedded Artists AB <mailto:support@embeddedartists.com>");
MODULE_DESCRIPTION("I2C driver for NXP LPC2xxx microcontrollers");
MODULE_LICENSE("GPL");
