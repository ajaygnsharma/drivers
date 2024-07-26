#include <linux/module.h>
#include <linux/ktime.h>

static int num = 10;
static ktime_t start_time;

module_param(num, int, S_IRUGO);

static void say_hello(void){
  int i;
  for(i = 1; i <= num; i++){
    pr_info("[%d,%d] Hello!\n", i, num);
  }
}

static int __init first_init(void)
{
	start_time = ktime_get();
	pr_info("loading first\n");
	say_hello();
	return 0;
}

static void __exit first_exit(void)
{
	ktime_t end_time;
	end_time = ktime_get();	
	long time_diff = ktime_sub(end_time, start_time);
	pr_info("Unloading module after %ld seconds\n", (long)ktime_to_ns(time_diff)/(1000*1000*1000));
	say_hello();
}

module_init(first_init);
module_exit(first_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Liberal <aliberal@arroweurope.com>");
MODULE_DESCRIPTION("This module a prints time when loaded");
