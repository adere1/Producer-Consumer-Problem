#include <linux/module.h>  
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/vmalloc.h>
#include <linux/semaphore.h>
#include <linux/mutex.h> 



static DEFINE_SEMAPHORE(empty);
static DEFINE_SEMAPHORE(full);
static DEFINE_MUTEX(mutex);

static int maxval = 1000 ;
module_param(maxval, int, 0);

int init_module(void);
void cleanup_module(void);

static struct miscdevice my_dev;

static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);		


static struct file_operations fops = {
	.read = device_read,
	.write = device_write
	
};


char ** prod_consumer;
int counter1;
int resultdata = 0;
int num = 0;
int index1 = 0;
int index2 = 0;



int returnval;
int init_module(void) 
{
   /*int i = 1;
   int j = 3;
   int r = i+j;
   printk("My Addition is %d.\n",r);
   return 0;
*/
    
    prod_consumer = (char **)vmalloc(maxval * sizeof(char*));   
    
    my_dev.minor = MISC_DYNAMIC_MINOR;
    my_dev.name = "add";
    my_dev.fops = &fops;
    returnval = misc_register(&my_dev);    
    
    if(returnval < 0){
        printk(KERN_ALERT "Registration failed\n");
        return returnval;
    } 
    printk(KERN_ALERT "Registration Successfull\n");
    sema_init(&empty, maxval);
    sema_init(&full, 0);
    mutex_init(&mutex);
    return 0;
}


static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	resultdata = 0; 
       

	if(down_interruptible(&empty)){
		return -1;
	}
	mutex_lock_interruptible(&mutex);
	
	prod_consumer[index1] = (char*) vmalloc(len);
	resultdata = copy_from_user(prod_consumer[index1], buff, len);
	num = strlen(prod_consumer[index1]);

	if(resultdata == 0){		
		if (index1 != maxval-1){
                     index1 = index1+1;
                     //printk(KERN_INFO "Inside"); 
                }else{
                    index1 = 0;
                }
		mutex_unlock(&mutex);
		up(&full);

		return num;
	} else {
		mutex_unlock(&mutex);
		up(&empty); 
		return -1;
	}
}

static ssize_t device_read(struct file *filp,	
			   char *buffer,	
			   size_t length,	
			   loff_t * offset){
	resultdata = 0;
        

	if(down_interruptible(&full)){
		return -1;
	}
	mutex_lock_interruptible(&mutex);
	resultdata = copy_to_user(buffer, prod_consumer[index2], length);
	num = strlen(prod_consumer[index2]);
	vfree(prod_consumer[index2]);

	if(resultdata == 0){
		//printk(KERN_INFO "Inside");
		if (index2 != maxval-1){
                     index2 = index2+1;
                }else{
                    index2 = 0;
                }
		mutex_unlock(&mutex);
		up(&empty);
		
		return num;
	} else {
		mutex_unlock(&mutex);
		up(&full);
		
		return -1;
	}

}



void cleanup_module(void)
{
  //printk("Goodbye Addition\n");
  
  misc_deregister(&my_dev);
  printk(KERN_ALERT "Device removed\n");      
  
}  



MODULE_LICENSE("GPL");
MODULE_AUTHOR("Aditya");
MODULE_DESCRIPTION("character device");


