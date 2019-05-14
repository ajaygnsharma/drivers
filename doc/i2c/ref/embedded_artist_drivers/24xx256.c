/*
    24xx256.c - A 256 Kbit Serial Electrical Erasable PROM (EEPROM)
		with a page write capability of up to 64 bytes of
		data.

    Copyright (C) 2008 Embedded Artists AB.

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


#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>

#define BIN_ATTR_NAME "data0"

/* Addresses to scan */
#if defined(CONFIG_BOARD_LPC2478_MICROBLOX)

/*
 * The LPC2478 Micro-Blox has a PCF8563 (RTC) on address 0x51
 */
static unsigned short normal_i2c[] = { 0x50, 0x52, 0x53, 0x54,
					0x55, 0x56, 0x57, I2C_CLIENT_END };
#else
static unsigned short normal_i2c[] = { 0x50, 0x51, 0x52, 0x53, 0x54,
					0x55, 0x56, 0x57, I2C_CLIENT_END };
#endif

/* Insmod parameters */
I2C_CLIENT_INSMOD_1(mc24xx256);

/* Size of EEPROM in bytes (256Kbit -> 32K) */
#define EEPROM_SIZE 0x8000

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y)) 
#endif

#define MAX_PAGE_SIZE 64

static int mc24xx256_attach_adapter(struct i2c_adapter *adapter);
static int mc24xx256_detect(struct i2c_adapter *adapter, int address, int kind);
static int mc24xx256_detach_client(struct i2c_client *client);

/* This is the driver that will be inserted */
static struct i2c_driver mc24xx256_driver = {
	.driver = {
		.name	= "mc24xx256",
	},
	.attach_adapter	= mc24xx256_attach_adapter,
	.detach_client	= mc24xx256_detach_client,
};

/** Get EEPROM size, should probably be moved to arch/board_xxx.c instead */
static u32 getEepromSize(int addr) 
{
#if defined(CONFIG_BOARD_LPC2478_MICROBLOX)

	/* 
	 * The LPC2478 Micro-Blox has a 256kBit EEPROM on address 0x50
	 * and a 128 kBit EEPROM on address 0x53
	 */

	if (addr == 0x50)
		return 0x8000;
	else if (addr == 0x53)
		return 0x4000;
	else
		return 0;
#else
	return 0x8000;
#endif	
}

static char* getClientName(int addr)
{
#if defined(CONFIG_BOARD_LPC2478_MICROBLOX)

	if (addr == 0x50)
		return "mc24xx256";
	else if (addr == 0x53)
		return "mc24xx128";
	else
		return 0;
#else
	return "mc24xx256";
#endif
}

static s32 mc24xx256_send_address(struct i2c_client *client, u16 addr)
{
	return i2c_smbus_write_byte_data(client, (addr>>8), addr&0xff);
}

static ssize_t mc24xx256_write(struct kobject *kobj, char *buf, loff_t off, size_t count)
{
	size_t writeCount = 0;
	s32 len = 0;
	char sendBuf[MAX_PAGE_SIZE+2]; // + 2 bytes for address
	struct i2c_client *client = to_i2c_client(container_of(kobj, struct device, kobj));
	u32 size = getEepromSize(client->addr);

	if (off > size)
		return 0;
	if (off + count > size)
		count = size - off;

	/*
	 * The 24XX256 can handle page writes of up to 64 bytes in a page.
	 * The page write operations are limited to writing bytes within
	 * a physical page, regardless of the number of bytes actually
	 * being written. Physical page boundaries start at addresses 
	 * that are integer multiples of the page size.
	 *
	 * If a page write command attempts to write across a physical 
	 * page boundary, the result is that the data wraps around to 
	 * the beginning of the current page.
	 */

	sendBuf[0] = (off >> 8) & 0xff;
	sendBuf[1] = off & 0xff;

	// First send data up to the first boundary (if there is enough data)
	len = (((off >> 6) + 1) << 6) - off;
	memcpy(sendBuf+2, buf, MIN(len, count));

	len = i2c_master_send(client, sendBuf, MIN(len, count)+2);

	if (len < 0) {
		dev_dbg(&client->dev, "write: failed to send data\n");
		writeCount = len;
		goto nexit;
	}

	len -= 2;
	count -= len;
	buf   += len;
	writeCount += len;

	// now we now that each transfer will start on a boundary

	while (count > 0)
	{

		sendBuf[0] = ((off+writeCount) >> 8) & 0xff;
		sendBuf[1] = (off+writeCount) & 0xff;
		memcpy(sendBuf+2, buf, MIN(MAX_PAGE_SIZE, count));

		/*
 		 * i2c_master_send may return before the data has been written
		 * to eeprom. If a second send request is made before the 
		 * previous is finished the data will be lost. 
		 *
		 * There don't seem to be a simple way of making sure the
		 * send request is finished so we add a small delay here.
		 */

		mdelay(20);

		len = i2c_master_send(client, sendBuf, MIN(MAX_PAGE_SIZE, count)+2);

		if (len < 0) {
			dev_dbg(&client->dev, "write: failed to send data\n");
			writeCount = len;
			goto nexit;
		}

		len -= 2;
		count -= len;
		buf   += len;
		writeCount += len;
	}

nexit:
	return writeCount;
}


static ssize_t mc24xx256_read(struct kobject *kobj, char *buf, loff_t off, size_t count)
{

	struct i2c_client *client = to_i2c_client(container_of(kobj, struct device, kobj));

	s32 len = 0;
	size_t readCount = 0;
	u32 size = getEepromSize(client->addr);

	if (off > size)
		return 0;
	if (off + count > size)
		count = size - off;

	// set the address to start reading from
	if ((len = mc24xx256_send_address(client, off))) {
		dev_dbg(&client->dev, "eeprom read start has failed!\n");
		readCount = len;
		goto nexit;
	}		

	do {
		len = i2c_master_recv(client, buf, MIN(count, I2C_SMBUS_BLOCK_MAX));

		if (len < 0) {
			dev_dbg(&client->dev, "failed to read a block!\n");
			readCount = len;
			goto nexit;
		}

		buf    += len;
		readCount += len;
		count -= len;
	} while(count > 0);

nexit:
	return readCount;
}

static int mc24xx256_attach_adapter(struct i2c_adapter *adapter)
{
	return i2c_probe(adapter, &addr_data, mc24xx256_detect);
}

/* This function is called by i2c_probe */
static int mc24xx256_detect(struct i2c_adapter *adapter, int address, int kind)
{
	struct i2c_client *new_client;
	struct bin_attribute* bin_attr;
	int err = 0;
	u32 size = getEepromSize(address);

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE))
		goto nexit;

	if (!(new_client = kzalloc(sizeof(struct i2c_client), GFP_KERNEL))) {
		err = -ENOMEM;
		goto nexit;
	}

	new_client->addr = address;
	new_client->adapter = adapter;
	new_client->driver = &mc24xx256_driver;
	new_client->flags = 0;

	/* Fill in the remaining client fields */
	strlcpy(new_client->name, getClientName(address), I2C_NAME_SIZE);

	/* Tell the I2C layer a new client has arrived */
	if ((err = i2c_attach_client(new_client)))
		goto nexit;

	if (!(bin_attr = kzalloc(sizeof(struct bin_attribute), GFP_KERNEL))) {
		err = -ENOMEM;
		goto exit_binattr;
	}

	bin_attr->attr.name	= BIN_ATTR_NAME;
	bin_attr->attr.mode	= S_IRUGO | S_IWUSR;
	bin_attr->attr.owner	= THIS_MODULE;
	bin_attr->size		= size;
	bin_attr->read		= mc24xx256_read;
	bin_attr->write		= mc24xx256_write;

	i2c_set_clientdata(new_client, bin_attr);

	/* create the sysfs eeprom file */
	err = sysfs_create_bin_file(&new_client->dev.kobj, bin_attr);
	if (err)
		goto exit_detach;

	return 0;

exit_binattr:
exit_detach:
	kfree(bin_attr);	
	i2c_detach_client(new_client);
nexit:
	return err;
}

static int mc24xx256_detach_client(struct i2c_client *client)
{
	int err;
	struct bin_attribute* bin_attr = i2c_get_clientdata(client);

	sysfs_remove_bin_file(&client->dev.kobj, bin_attr);

	err = i2c_detach_client(client);
	if (err)
		return err;

	kfree(i2c_get_clientdata(client));

	return 0;
}

static int __init mc24xx256_init(void)
{
	return i2c_add_driver(&mc24xx256_driver);
}

static void __exit mc24xx256_exit(void)
{
	i2c_del_driver(&mc24xx256_driver);
}


MODULE_AUTHOR("Embedded Artists AB <support@embeddedartists.com>");
MODULE_DESCRIPTION("24xx256 (EEPROM) driver");
MODULE_LICENSE("GPL");

module_init(mc24xx256_init);
module_exit(mc24xx256_exit);
