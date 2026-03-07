#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ajay Sharma");
MODULE_DESCRIPTION("Hello-world kernel module");
MODULE_VERSION("1.0");

static int __init hello_init(void)
{
    pr_info("kmod-hello: hello from STM32F469!\n");
    return 0;
}

static void __exit hello_exit(void)
{
    pr_info("kmod-hello: goodbye!\n");
}

module_init(hello_init);
module_exit(hello_exit);
