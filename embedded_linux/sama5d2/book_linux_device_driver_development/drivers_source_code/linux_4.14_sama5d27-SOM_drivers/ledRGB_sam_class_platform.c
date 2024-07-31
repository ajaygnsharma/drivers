
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/leds.h>

#define PIOB_SODR1_offset 0x50
#define PIOB_CODR1_offset 0x54
#define PIOB_CFGR1_offset 0x44
#define PIOB_MSKR1_offset 0x40

#define PIOA_SODR1_offset 0x10
#define PIOA_CODR1_offset 0x14
#define PIOA_CFGR1_offset 0x04
#define PIOA_MSKR1_offset 0x00

/* Declare masks to configure the different registers, PA10= RED, PB1 = GREEN, PA31 = BLUE */
#define PIO_PA10_MASK (1 << 10) /* red */
#define PIO_PB1_MASK (1 << 1) 	/* green */
#define PIO_PA31_MASK (1 << 31) /* blue */
#define PIO_CFGR1_MASK (1 << 8) /* masked bits direction (output), no PUEN, no PDEN */
#define PIOA_MASK_ALL_LEDS (PIO_PA10_MASK | PIO_PA31_MASK)
#define PIOB_MASK_ALL_LEDS PIO_PB1_MASK 

struct led_dev
{
	u32 led_mask; /* different mask if led is R,G or B */
	const char *led_name;
	void __iomem *base;
	struct led_classdev cdev;
};

static void led_control(struct led_classdev *led_cdev, enum led_brightness b)
{

	struct led_dev *led = container_of(led_cdev, struct led_dev, cdev);

	/* Switch off all the leds */
	iowrite32(PIOA_MASK_ALL_LEDS, led->base + PIOA_CODR1_offset);
	iowrite32(PIOB_MASK_ALL_LEDS, led->base + PIOB_CODR1_offset);

	if (b != LED_OFF) {	/* LED ON */
		if(!strcmp(led->led_name, "ledred"))
			iowrite32(led->led_mask, led->base + PIOA_SODR1_offset);
		else if(!strcmp(led->led_name, "ledgreen"))
			iowrite32(led->led_mask, led->base + PIOB_SODR1_offset);
		else if(!strcmp(led->led_name, "ledblue"))
			iowrite32(led->led_mask, led->base + PIOA_SODR1_offset);
		else {
			pr_info("Bad value\n");
			return -EINVAL;
		}
	}
	else {
		if(!strcmp(led->led_name, "ledred"))
			iowrite32(led->led_mask, led->base + PIOA_CODR1_offset);
		else if(!strcmp(led->led_name, "ledgreen"))
			iowrite32(led->led_mask, led->base + PIOB_CODR1_offset);
		else if(!strcmp(led->led_name, "ledblue"))
			iowrite32(led->led_mask, led->base + PIOA_CODR1_offset);
		else {
			pr_info("Bad value\n");
			return -EINVAL;
		}
	}
}

static int __init ledclass_probe(struct platform_device *pdev)
{

	void __iomem *g_ioremap_addr;
	struct device_node *child;
	struct resource *r;
	struct device *dev = &pdev->dev;
	int count, ret;

	dev_info(dev, "platform_probe enter\n");

	/* get our first memory resource from device tree */
	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r) {
		dev_err(dev, "IORESOURCE_MEM, 0 does not exist\n");
		return -EINVAL;
	}
	dev_info(dev, "r->start = 0x%08lx\n", (long unsigned int)r->start);
	dev_info(dev, "r->end = 0x%08lx\n", (long unsigned int)r->end);

	/* ioremap our memory region */
	g_ioremap_addr = devm_ioremap(dev, r->start, resource_size(r));
	if (!g_ioremap_addr) {
		dev_err(dev, "ioremap failed \n");
		return -ENOMEM;
	}

	count = of_get_child_count(dev->of_node);
	if (!count)
		return -EINVAL;

	dev_info(dev, "there are %d nodes\n", count);

	/* Enable all leds and set dir to output */
	iowrite32(PIOA_MASK_ALL_LEDS, g_ioremap_addr + PIOA_MSKR1_offset);
	iowrite32(PIOB_MASK_ALL_LEDS, g_ioremap_addr + PIOB_MSKR1_offset);
	iowrite32(PIO_CFGR1_MASK, g_ioremap_addr + PIOA_CFGR1_offset);
	iowrite32(PIO_CFGR1_MASK, g_ioremap_addr + PIOB_CFGR1_offset);

	/* Switch off all the leds */
	iowrite32(PIOA_MASK_ALL_LEDS, g_ioremap_addr + PIOA_CODR1_offset);
	iowrite32(PIOB_MASK_ALL_LEDS, g_ioremap_addr + PIOB_CODR1_offset);

	for_each_child_of_node(dev->of_node, child){

		struct led_dev *led_device;
		struct led_classdev *cdev;
		led_device = devm_kzalloc(dev, sizeof(*led_device), GFP_KERNEL);
		if (!led_device)
			return -ENOMEM;

		cdev = &led_device->cdev;

		led_device->base = g_ioremap_addr;

		of_property_read_string(child, "label", &cdev->name);

		if (strcmp(cdev->name,"red") == 0) {
			led_device->led_mask = PIO_PA10_MASK;
			led_device->led_name ="ledred";
			led_device->cdev.default_trigger = "heartbeat";
		}
		else if (strcmp(cdev->name,"green") == 0) {
			led_device->led_mask = PIO_PB1_MASK;
			led_device->led_name ="ledgreen";
		}
		else if (strcmp(cdev->name,"blue") == 0) {
			led_device->led_mask = PIO_PA31_MASK;
			led_device->led_name ="ledblue";
		}
		else {
			dev_info(dev, "Bad device tree value\n");
			return -EINVAL;
		}

		/* Disable timer trigger until led is on */
		led_device->cdev.brightness = LED_OFF;
		led_device->cdev.brightness_set = led_control;

		ret = devm_led_classdev_register(dev, &led_device->cdev);
		if (ret) {
			dev_err(dev, "failed to register the led %s\n", cdev->name);
			of_node_put(child);
			return ret;
		}
	}
	
	dev_info(dev, "leds_probe exit\n");

	return 0;
}

static int __exit ledclass_remove(struct platform_device *pdev)
{

	dev_info(&pdev->dev, "leds_remove enter\n");
	dev_info(&pdev->dev, "leds_remove exit\n");

	return 0;
}

static const struct of_device_id my_of_ids[] = {
	{ .compatible = "arrow,RGBclassleds"},
	{},
};

MODULE_DEVICE_TABLE(of, my_of_ids);

static struct platform_driver led_platform_driver = {
	.probe = ledclass_probe,
	.remove = ledclass_remove,
	.driver = {
		.name = "RGBclassleds",
		.of_match_table = my_of_ids,
		.owner = THIS_MODULE,
	}
};

static int ledRGBclass_init(void)
{
	int ret_val;
	pr_info("demo_init enter\n");

	ret_val = platform_driver_register(&led_platform_driver);
	if (ret_val !=0)
	{
		pr_err("platform value returned %d\n", ret_val);
		return ret_val;

	}

	pr_info("demo_init exit\n");
	return 0;
}

static void ledRGBclass_exit(void)
{
	pr_info("led driver enter\n");

	platform_driver_unregister(&led_platform_driver);

	pr_info("led driver exit\n");
}

module_init(ledRGBclass_init);
module_exit(ledRGBclass_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("This is a driver that turns on/off RGB leds \
		           using the LED subsystem");

