/*
 * tmp102.c
 *
 *  Created on: Aug 26, 2014
 *      Author: user1
 */
/*
    tmp102.c - i2c temperature sensor

    Copyright (C) 2008 Embedded Artists AB <info@embeddedartists.com>

    Based on pca9532.c

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.
*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/hwmon-sysfs.h>

/* Addresses to scan */
static unsigned short normal_i2c[] = {0x48,0x49,0x4A,0x4B,I2C_CLIENT_END};

/* Insmod parameters */
I2C_CLIENT_INSMOD_1(tmp102);

enum tmp102_cmd
{
	TMP102_TEMPERATURE_REG		= 0,
	TMP102_CONFIG_REG			= 1,
	TMP102_TLOW_REG             = 2,
	TMP102_THIGH_REG            = 3,
};

static int tmp102_attach_adapter(struct i2c_adapter *adapter);
static int tmp102_detect(struct i2c_adapter *adapter, int address, int kind);
static int tmp102_detach_client(struct i2c_client *client);

int
test_it4(struct kobject* kobj, char* buffer, loff_t off, size_t count, int val)
{
	printk("test_it4: count=%d, val=%d\n", count, val);
	return 0;
}
EXPORT_SYMBOL_GPL(test_it4);

/* This is the driver that will be inserted */
static struct i2c_driver tmp102_driver = {
	.driver = {
		.name	= "tmp102",
	},
	.attach_adapter	= tmp102_attach_adapter,
	.detach_client	= tmp102_detach_client,
};

struct tmp102_data {
	struct i2c_client client;
};

/* following are the sysfs callback functions */
static ssize_t tmp102_show(struct device *dev, struct device_attribute *attr,
			    char *buf)
{
	struct sensor_device_attribute *psa = to_sensor_dev_attr(attr);
	struct i2c_client *client = to_i2c_client(dev);
	return sprintf(buf, "%d\n", i2c_smbus_read_word_data(client,
							     psa->index)); /* temperature is 16 bits */
}

static ssize_t tmp102_store(struct device *dev, struct device_attribute *attr,
			     const char *buf, size_t count)
{
	struct sensor_device_attribute *psa = to_sensor_dev_attr(attr);
	struct i2c_client *client = to_i2c_client(dev);
	unsigned long val = simple_strtoul(buf, NULL, 0);
	printk("val=%u\n",val);
	if (val > 0xff)
		return -EINVAL;
	i2c_smbus_write_byte_data(client, psa->index, val);
	return count;
}

/* Define the device attributes */
#define TMP102_ENTRY_RO(name, cmd_idx) \
	static SENSOR_DEVICE_ATTR(name, S_IRUGO, tmp102_show, NULL, cmd_idx)

#define TMP102_ENTRY_RW(name, cmd_idx) \
	static SENSOR_DEVICE_ATTR(name, S_IRUGO | S_IWUSR, tmp102_show, \
				  tmp102_store, cmd_idx)

TMP102_ENTRY_RO(temperature_reg, TMP102_TEMPERATURE_REG);
TMP102_ENTRY_RW(config_reg,   TMP102_CONFIG_REG);
TMP102_ENTRY_RW(tlow_reg,   TMP102_TLOW_REG);
TMP102_ENTRY_RW(thigh_reg,   TMP102_THIGH_REG);


static struct attribute *tmp102_attributes[] = {
	&sensor_dev_attr_temperature_reg.dev_attr.attr,
	&sensor_dev_attr_config_reg.dev_attr.attr,
	&sensor_dev_attr_tlow_reg.dev_attr.attr,
	&sensor_dev_attr_thigh_reg.dev_attr.attr,
	NULL
};

static struct attribute_group tmp102_defattr_group = {
	.attrs = tmp102_attributes,
};

static int tmp102_attach_adapter(struct i2c_adapter *adapter)
{
	return i2c_probe(adapter, &addr_data, tmp102_detect);
}

/* This function is called by i2c_probe */
static int tmp102_detect(struct i2c_adapter *adapter, int address, int kind)
{
	struct i2c_client *new_client;
	struct tmp102_data *data;
	int err = 0;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		goto exit;

	/* OK. For now, we presume we have a valid client. We now create the
	   client structure, even though we cannot fill it completely yet. */
	if (!(data = kzalloc(sizeof(struct tmp102_data), GFP_KERNEL))) {
		err = -ENOMEM;
		goto exit;
	}

	new_client = &data->client;
	i2c_set_clientdata(new_client, data);
	new_client->addr = address;
	new_client->adapter = adapter;
	new_client->driver = &tmp102_driver;
	new_client->flags = 0;

	if (kind < 0) {
		/* Detection: the pca9532 only has 10 registers (0-9).
		   A read of 9 should succeed, but a read of 10 should fail. */
		if ((i2c_smbus_read_byte_data(new_client, 9) < 0) ||
		    (i2c_smbus_read_byte_data(new_client, 10) >= 0))
			goto exit_kfree;
	}

	strlcpy(new_client->name, "tmp102", I2C_NAME_SIZE);

	/* Tell the I2C layer a new client has arrived */
	if ((err = i2c_attach_client(new_client)))
		goto exit_kfree;

	/* Register sysfs hooks */
	err = sysfs_create_group(&new_client->dev.kobj,
				 &tmp102_defattr_group);
	if (err)
		goto exit_detach;

	return 0;

exit_detach:
	i2c_detach_client(new_client);
exit_kfree:
	kfree(data);
exit:
	return err;
}

static int tmp102_detach_client(struct i2c_client *client)
{
	int err;

	sysfs_remove_group(&client->dev.kobj, &tmp102_defattr_group);

	if ((err = i2c_detach_client(client)))
		return err;

	kfree(i2c_get_clientdata(client));
	return 0;
}

static int __init tmp102_init(void)
{
	return i2c_add_driver(&tmp102_driver);
}

static void __exit tmp102_exit(void)
{
	i2c_del_driver(&tmp102_driver);
}

MODULE_AUTHOR("Ajay Sharma <ajaygnsharma12@gmail.com>");
MODULE_DESCRIPTION("TMP102 driver");
MODULE_LICENSE("GPL");

module_init(tmp102_init);
module_exit(tmp102_exit);


