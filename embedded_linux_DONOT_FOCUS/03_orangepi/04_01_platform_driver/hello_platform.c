#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/of_address.h>

/* For user space */
static int my_dev_open(struct inode *inode, struct file *file){
    pr_info("my_dev_open() is called. \n");
    return 0;
}

static int my_dev_close(struct inode *inode, struct file *file){
    pr_info("my_dev_close() is called\n");
    return 0;
}

static long my_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
    pr_info("mydev ioctl called, cmd=%d, arg=%ld\n", cmd, arg);
    return 0;
}

/* user space Hooks */
static const struct file_operations my_dev_fops = {
    .owner = THIS_MODULE,
    .open = my_dev_open,
    .release = my_dev_close,
    .unlocked_ioctl = my_dev_ioctl,
};

/* For userspace still. Just binds the top structure */
static struct miscdevice helloworld_miscdevice = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "mydev",
    .fops = &my_dev_fops,
};

/* -------------------------Kernel hooks --------------------- */
/* Note the __init and __exit part that ensures that the 
function calls are called at init and exit c call.
 */
static int __init my_probe(struct platform_device *pdev){
    int ret_val;
    pr_info("my_probe(), function is called \n");

    /* THis is special and may change. */
    ret_val = misc_register(&helloworld_miscdevice);

    if(ret_val != 0){
        pr_err("Could not regiater the misc device mydev");
        return ret_val;
    }

    pr_info("mydev: got minor %i\n",helloworld_miscdevice.minor);
    return 0;
}

static int __exit my_remove(struct platform_device *pdev){
    pr_info("my_remove() function is called.\n");
    misc_deregister(&helloworld_miscdevice);
    return 0;
}

/* For now we can insmod the device driver and the my_probe will be
called. But otherwise, the dts will be used and the probe 
will be called based on that. Essentially there are two ways
of calling the probe function. At boot using DTS and manually 
using insmod. */
static const struct of_device_id my_of_ids[] = {
    { .compatible = "ajay,hello_dts" },
    {},
};

MODULE_DEVICE_TABLE(of, my_of_ids);

static struct platform_driver my_platform_driver = {
    .probe = my_probe,
    .remove = my_remove,
    .driver = {
        .name = "hello world dts module",
        .of_match_table = my_of_ids,
        .owner = THIS_MODULE,
    }
};

module_platform_driver(my_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ajay Sharma");
MODULE_DESCRIPTION("This is a DTS based platform driver.");



