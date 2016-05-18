#include <linux/kernel.h>
#include <linux/linkage.h>

asmlinkage int sys_simple_add(int numberA, int numberB, int* result) {
 printk(KERN_ALERT "Number 1: %d \n", numberA);
 printk(KERN_ALERT "Number 2: %d \n", numberB);
 *result = numberA + numberB;
 printk(KERN_ALERT "Result: %d \n", *result);
 return 0;
}

