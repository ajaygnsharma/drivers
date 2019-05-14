/*
    pca9532.c - 16-bit I2C-bus and SMBus expander optimized for dimming LEDs in
                256 discrete steps. Input/outputs not used as LED drivers can
		be used as regular GPIOs

    Copyright (C) 2008 Embedded Artists AB <info@embeddedartists.com>

    Based on pca9539.c

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
static unsigned short normal_i2c[] = {0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67, I2C_CLIENT_END};

/* Insmod parameters */
I2C_CLIENT_INSMOD_1(pca9532);

enum pca9532_cmd
{
	PCA9532_INPUT_0		= 0,
	PCA9532_INPUT_1		= 1,
	PCA9532_PSC_0		= 2,
	PCA9532_PWM_0		= 3,
	PCA9532_PSC_1		= 4,
	PCA9532_PWM_1		= 5,
	PCA9532_LS_0		= 6,
	PCA9532_LS_1		= 7,
	PCA9532_LS_2		= 8,
	PCA9532_LS_3		= 9,
};

static int pca9532_attach_adapter(struct i2c_adapter *adapter);
static int pca9532_detect(struct i2c_adapter *adapter, int address, int kind);
static int pca9532_detach_client(struct i2c_client *client);

int
test_it4(struct kobject* kobj, char* buffer, loff_t off, size_t count, int val)
{
	printk("test_it4: count=%d, val=%d\n", count, val);
	return 0;
}
EXPORT_SYMBOL_GPL(test_it4);

/* This is the driver that will be inserted */
static struct i2c_driver pca9532_driver = {
	.driver = {
		.name	= "pca9532",
	},
	.attach_adapter	= pca9532_attach_adapter,
	.detach_client	= pca9532_detach_client,
};

struct pca9532_data {
	struct i2c_client client;
};

/* following are the sysfs callback functions */
static ssize_t pca9532_show(struct device *dev, struct device_attribute *attr,
			    char *buf)
{
	struct sensor_device_attribute *psa = to_sensor_dev_attr(attr);
	struct i2c_client *client = to_i2c_client(dev);
	return sprintf(buf, "%d\n", i2c_smbus_read_byte_data(client,
							     psa->index));
}

static ssize_t pca9532_store(struct device *dev, struct device_attribute *attr,
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

#define PCA9532_ENTRY_RO(name, cmd_idx) \
	static SENSOR_DEVICE_ATTR(name, S_IRUGO, pca9532_show, NULL, cmd_idx)

#define PCA9532_ENTRY_RW(name, cmd_idx) \
	static SENSOR_DEVICE_ATTR(name, S_IRUGO | S_IWUSR, pca9532_show, \
				  pca9532_store, cmd_idx)

PCA9532_ENTRY_RO(input0, PCA9532_INPUT_0);
PCA9532_ENTRY_RO(input1, PCA9532_INPUT_1);
PCA9532_ENTRY_RW(psc0,   PCA9532_PSC_0);
PCA9532_ENTRY_RW(pwm0,   PCA9532_PWM_0);
PCA9532_ENTRY_RW(psc1,   PCA9532_PSC_1);
PCA9532_ENTRY_RW(pwm1,   PCA9532_PWM_1);
PCA9532_ENTRY_RW(ls0,    PCA9532_LS_0);
PCA9532_ENTRY_RW(ls1,    PCA9532_LS_1);
PCA9532_ENTRY_RW(ls2,    PCA9532_LS_2);
PCA9532_ENTRY_RW(ls3,    PCA9532_LS_3);

static struct attribute *pca9532_attributes[] = {
	&sensor_dev_attr_input0.dev_attr.attr,
	&sensor_dev_attr_input1.dev_attr.attr,
	&sensor_dev_attr_psc0.dev_attr.attr,
	&sensor_dev_attr_pwm0.dev_attr.attr,
	&sensor_dev_attr_psc1.dev_attr.attr,
	&sensor_dev_attr_pwm1.dev_attr.attr,
	&sensor_dev_attr_ls0.dev_attr.attr,
	&sensor_dev_attr_ls1.dev_attr.attr,
	&sensor_dev_attr_ls2.dev_attr.attr,
	&sensor_dev_attr_ls3.dev_attr.attr,
	NULL
};

static struct attribute_group pca9532_defattr_group = {
	.attrs = pca9532_attributes,
};

static int pca9532_attach_adapter(struct i2c_adapter *adapter)
{
	return i2c_probe(adapter, &addr_data, pca9532_detect);
}

/* This function is called by i2c_probe */
static int pca9532_detect(struct i2c_adapter *adapter, int address, int kind)
{
	struct i2c_client *new_client;
	struct pca9532_data *data;
	int err = 0;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		goto exit;

	/* OK. For now, we presume we have a valid client. We now create the
	   client structure, even though we cannot fill it completely yet. */
	if (!(data = kzalloc(sizeof(struct pca9532_data), GFP_KERNEL))) {
		err = -ENOMEM;
		goto exit;
	}

	new_client = &data->client;
	i2c_set_clientdata(new_client, data);
	new_client->addr = address;
	new_client->adapter = adapter;
	new_client->driver = &pca9532_driver;
	new_client->flags = 0;

	if (kind < 0) {
		/* Detection: the pca9532 only has 10 registers (0-9).
		   A read of 9 should succeed, but a read of 10 should fail. */
		if ((i2c_smbus_read_byte_data(new_client, 9) < 0) ||
		    (i2c_smbus_read_byte_data(new_client, 10) >= 0))
			goto exit_kfree;
	}

	strlcpy(new_client->name, "pca9532", I2C_NAME_SIZE);

	/* Tell the I2C layer a new client has arrived */
	if ((err = i2c_attach_client(new_client)))
		goto exit_kfree;

	/* Register sysfs hooks */
	err = sysfs_create_group(&new_client->dev.kobj,
				 &pca9532_defattr_group);
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

static int pca9532_detach_client(struct i2c_client *client)
{
	int err;

	sysfs_remove_group(&client->dev.kobj, &pca9532_defattr_group);

	if ((err = i2c_detach_client(client)))
		return err;

	kfree(i2c_get_clientdata(client));
	return 0;
}

static int __init pca9532_init(void)
{
	return i2c_add_driver(&pca9532_driver);
}

static void __exit pca9532_exit(void)
{
	i2c_del_driver(&pca9532_driver);
}

MODULE_AUTHOR("Embedded Artists AB <support@embeddedartists.com>");
MODULE_DESCRIPTION("PCA9532 driver");
MODULE_LICENSE("GPL");

module_init(pca9532_init);
module_exit(pca9532_exit);

