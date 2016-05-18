#include <linux/init.h>
#include <linux/module.h>

#include <linux/fs.h>
#include <asm/uaccess.h>
#define BUFFER_SIZE 1024

static char device_buffer[BUFFER_SIZE];

ssize_t simple_char_driver_read (struct file *pfile, char __user *buffer, size_t length, loff_t *offset)
{
	/* *buffer is the userspace buffer to where you are writing the data you want to be read from the device file*/
	/*  length is the length of the userspace buffer*/
	/*  current position of the opened file*/
	/* copy_to_user function. source is device_buffer (the buffer defined at the start of the code) and destination is the userspace 		buffer *buffer */
	
	copy_to_user(buffer, device_buffer, length);
	printk(KERN_ALERT "%s\n", buffer);
	printk(KERN_ALERT "Read %zu bytes", length);
	return 0;
}



ssize_t simple_char_driver_write (struct file *pfile, const char __user *buffer, size_t length, loff_t *offset)
{
	/* *buffer is the userspace buffer where you are writing the data you want to be written in the device file*/
	/*  length is the length of the userspace buffer*/
	/*  current position of the opened file*/
	/* copy_from_user function. destination is device_buffer (the buffer defined at the start of the code) and source is the userspace 		buffer *buffer */
	int size = strlen(device_buffer);
	copy_from_user(device_buffer + size, buffer, length);
	//*offset += length;
	printk(KERN_ALERT "Wrote %zu bytes", strlen(buffer));
	return length;
}


int simple_char_driver_open (struct inode *pinode, struct file *pfile)
{
	/* print to the log file that the device is opened and also print the number of times this device has been opened until now*/
	static int opened = 0;
	opened++;
	printk(KERN_ALERT "Simple_Char_Driver is opened\n");
	printk(KERN_ALERT "Device has been opened %d times\n", opened);
	return 0;
}


int simple_char_driver_close (struct inode *pinode, struct file *pfile)
{
	/* print to the log file that the device is closed and also print the number of times this device has been closed until now*/
	static int closed = 0;
	closed++;
	printk(KERN_ALERT "Simple_Char_Driver is closed\n");
	printk(KERN_ALERT "Device has been closed %d times\n", closed);
	return 0;
}

struct file_operations simple_char_driver_file_operations = {
	.owner   = THIS_MODULE,
	
	.release = simple_char_driver_close,
	.open = simple_char_driver_open,
	.read = simple_char_driver_read,
	.write = simple_char_driver_write,	
};

static int simple_char_driver_init(void)
{
	/* print to the log file that the init function is called.*/
	/* register the device */
	register_chrdev(301, "simple_char_driver", &simple_char_driver_file_operations);
	printk(KERN_ALERT "inside %s function\n",__FUNCTION__);
	return 0;
}

static int simple_char_driver_exit(void)
{
	/* print to the log file that the exit function is called.*/
	/* unregister  the device using the register_chrdev() function. */
	printk(KERN_ALERT "inside %s function\n",__FUNCTION__);
	unregister_chrdev(301, "simple_char_driver");
	return 0;
}

module_init(simple_char_driver_init);
module_exit(simple_char_driver_exit);



/* add module_init and module_exit to point to the corresponding init and exit function*/
