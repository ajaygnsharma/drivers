/*
 * TI TMP121/TMP123 SPI temperature Sensor.
 *
 * Copyright (c) 2008 Manuel Lauss <mano at roarinelk.homelinux.net>
 *
 * All new formula! Contains 99% code from lm70.c which is
 *   Copyright (C) 2006 Kaiwan N Billimoria <kaiwan at designergraphix.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/sysfs.h>
#include <linux/hwmon.h>
#include <linux/mutex.h>
#include <linux/spi/spi.h>

#define DRVNAME		"mcp3002"

struct mcp3002 {
	struct device *hwmon_dev;
	struct mutex lock;
};

/* sysfs hook function */
static ssize_t mcp3002_sense_val(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct spi_device *spi = to_spi_device(dev);
	int status, val;
	u8 rxbuf[2];
	u8 txbuf[1] = {0x64};
	s16 raw=0;
	struct mcp3002 *p_mcp3002 = dev_get_drvdata(&spi->dev);

	if (mutex_lock_interruptible(&p_mcp3002->lock))
		return -ERESTARTSYS;

	/*
	 * spi_read() requires a DMA-safe buffer; so we use
	 * spi_write_then_read(), transmitting 1 byte(3 bits).
	 */
	status = spi_write_then_read(spi, &txbuf[0], 1, &rxbuf[0], 2);
	if (status < 0) {
		printk(KERN_WARNING
		"spi_write_then_read failed with status %d\n", status);
		goto out;
	}
	dev_dbg(dev, "rxbuf[1] : 0x%x rxbuf[0] : 0x%x\n", rxbuf[1], rxbuf[0]);

	raw = (rxbuf[0] << 8) + rxbuf[1];
	dev_dbg(dev, "raw=0x%x\n", raw);

	/*
	 * The "raw" temperature read into rxbuf[] is a 16-bit signed 2's
	 * complement value. Only the MSB 13 bits (1 sign  12 temperature
	 * bits) are meaningful; the LSB 3 bits are to be discarded.
	 * Each bit represents 0.0625 degrees Celsius.
	 */
	val = raw; //(raw * 625) / 80;
	status = sprintf(buf, "%d\n", val);
out:
	mutex_unlock(&p_mcp3002->lock);
	return status;
}

static DEVICE_ATTR(mcp3002_input, S_IRUGO, mcp3002_sense_val, NULL);

static ssize_t mcp3002_show_name(struct device *dev, struct device_attribute
			      *devattr, char *buf)
{
	return sprintf(buf, "mcp3002\n");
}

static DEVICE_ATTR(name, S_IRUGO, mcp3002_show_name, NULL);

/*----------------------------------------------------------------------*/

static int __devinit mcp3002_probe(struct spi_device *spi)
{
	struct mcp3002 *p_mcp3002;
	int status;

	p_mcp3002 = kzalloc(sizeof *p_mcp3002, GFP_KERNEL);
	if (!p_mcp3002)
		return -ENOMEM;

	mutex_init(&p_mcp3002->lock);

	/* sysfs hook */
	p_mcp3002->hwmon_dev = hwmon_device_register(&spi->dev);
	if (IS_ERR(p_mcp3002->hwmon_dev)) {
		dev_dbg(&spi->dev, "hwmon_device_register failed.\n");
		status = PTR_ERR(p_mcp3002->hwmon_dev);
		goto out_dev_reg_failed;
	}
	dev_set_drvdata(&spi->dev, p_mcp3002);

	if ((status = device_create_file(&spi->dev, &dev_attr_mcp3002_input))
	 || (status = device_create_file(&spi->dev, &dev_attr_name))) {
		dev_dbg(&spi->dev, "device_create_file failure.\n");
		goto out_dev_create_file_failed;
	}

	return 0;

out_dev_create_file_failed:
	device_remove_file(&spi->dev, &dev_attr_mcp3002_input);
	hwmon_device_unregister(p_mcp3002->hwmon_dev);
out_dev_reg_failed:
	dev_set_drvdata(&spi->dev, NULL);
	kfree(p_mcp3002);
	return status;
}

static int __devexit mcp3002_remove(struct spi_device *spi)
{
	struct mcp3002 *p_mcp3002 = dev_get_drvdata(&spi->dev);

	device_remove_file(&spi->dev, &dev_attr_mcp3002_input);
	device_remove_file(&spi->dev, &dev_attr_name);
	hwmon_device_unregister(p_mcp3002->hwmon_dev);
	dev_set_drvdata(&spi->dev, NULL);
	kfree(p_mcp3002);

	return 0;
}

static struct spi_driver mcp3002_driver = {
	.driver = {
		.name	= "mcp3002",
		.owner	= THIS_MODULE,
	},
	.probe	= mcp3002_probe,
	.remove	= __devexit_p(mcp3002_remove),
};

static int __init init_mcp3002(void)
{
	return spi_register_driver(&mcp3002_driver);
}

static void __exit cleanup_mcp3002(void)
{
	spi_unregister_driver(&mcp3002_driver);
}

module_init(init_mcp3002);
module_exit(cleanup_mcp3002);

MODULE_AUTHOR("Ajay Sharma");
MODULE_DESCRIPTION("Microchip MCP3002 SPI Analog to Digital Converter hwmon driver");
MODULE_LICENSE("GPL");
