//if you get the fault segmentation error it could be that you just need to add an overlay in the congig.txt in the /boot folder
//this repo : https://github.com/dotnet/iot/blob/main/Documentation/raspi-pwm.md is usefull to show you how to do that


#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include<linux/fs.h>
#include <linux/cdev.h>
#include <linux/pwm.h>
#include <linux/kernel.h>

/*Meta information*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ahmed Osama");
MODULE_DESCRIPTION("First Driver");
MODULE_PARM_DESC(number,"Driver major number");
/*Meta information*/



dev_t dynamicDeivcenumber;
struct cdev st_characterDevice;
struct class *myclass;
struct device *mydevice;

#define SIZE 10
#define SIZE_READ 3
#define LED_PIN 2
#define TOUCH_LED 3

static unsigned char buffer[SIZE]="";

struct pwm_device *pwm0 =NULL;
u32 pwm_high=500000000;
#define PERIOD 1000000000



static int driver_open(struct inode *device_file,struct file *instance)
{
    printk("%s functino was callen, open was called ! \n",__FUNCTION__);
    return 0;
}
static int driver_close(struct inode *device_file,struct file *instance)
{
    printk("%s function was called, close was called ! \n",__FUNCTION__);
    return 0;
}
ssize_t driver_write(struct file *file, const char __user *user_buff, size_t count, loff_t *offs)
{
    
    int not_copied;
    long value=0;    

    printk("%s: the count to write %d \n", __func__, count); // 35
    printk("%s: the offs %lld \n", __func__, *offs); // 0

    if (count + *offs > SIZE) // count 10 offs 3 size 9
    {
        count=SIZE-*offs;
    }
    if (!count)
    {
        printk("no space left");
        return -1;
    }

    not_copied = copy_from_user(&buffer[*offs], user_buff, count);


    if (kstrtol_from_user(user_buff,count-1,0,&value)==0)
    {
        printk("%s the value is %ld",__func__,value);
    }
    if (value > 1000)
    {
        printk("invalid value\n");
    }
    else
    {
        pwm_config(pwm0,value*1000000,1000000000);
    }

    *offs = count; // 15

    printk("%s: already done %d \n", __func__, count);
    printk("%s: message: %s \n", __func__, buffer);

    return count;

}
ssize_t driver_read(struct file *file, char __user *user_buffer, size_t count, loff_t *offs)
{
    int not_copied;
    char temp[SIZE_READ]="";

    printk("%s: the count to read %d \n", __func__, count);
    printk("%s: the offs %lld \n", __func__, *offs);

    if (count + *offs > SIZE_READ) // count 10 offs: 3:size 9

    {
        count=SIZE_READ-*offs;
    }

    not_copied = copy_to_user(user_buffer, &temp[*offs], count);

    if (not_copied)
    {
        return -1;
    }

    *offs = count;

    printk("%s: not copied %d \n", __func__, not_copied);
    printk("%s: message: %s \n", __func__, user_buffer);


    return count;//if you use  in the return the message will only appear in the kernel messages , you need to return the count for the driver to be able to 
                 //print the content of the file in the terminal
}


struct file_operations fops=
{
    .owner=THIS_MODULE,
    .open=driver_open,
    .release=driver_close,
    .read=driver_read,
    .write=driver_write
};
struct cdev st_characterDevice;





static int __init Hellodriver_init(void)
{
    int retval;
    printk("Hello World\n");



    retval=alloc_chrdev_region(&dynamicDeivcenumber,0,1,"my_first_driver_dynamic");
    if (retval==0)
    {
        printk("%s retval=0 -registered dynamicDeivcenumber major: %d,minor : %d\n",__FUNCTION__,MAJOR(dynamicDeivcenumber),MINOR(dynamicDeivcenumber));
    }
    else
    {
        printk("couldn't reserve a major number for the driver");
        return -1;

    }
    cdev_init(&st_characterDevice,&fops);
    retval=cdev_add(&st_characterDevice,dynamicDeivcenumber,1);
    
    if (retval!=0)
    {
        printk("file creation process failed !\n");
        goto CHARACTER_ERROR;
    }
    // 3- generate file ( class -device)
    if ((myclass = class_create(THIS_MODULE, "test_class"))== NULL)
    {
        printk("Device class can not be created!\n");
        goto CLASS_ERROR;
    }

    mydevice = device_create(myclass, NULL, dynamicDeivcenumber, NULL, "test_file");
    if (mydevice == NULL)
    {
        printk("Device class can not be created!\n");
        goto DEVICE_ERROR;
        
    }
    printk("Driver number & file created successfuly !");

    /*********************pwm setup*******************/
   pwm0=pwm_request(0, "mypwm");
   if(pwm0==NULL)
   {
        printk("pwm is not created \n");
        goto GPIO_REQUEST_ERROR;
   }
   pwm_config(pwm0,pwm_high,PERIOD);
   pwm_enable(pwm0);
    /*********************pwm setup*******************/

    return 0;

    /*****************error handling code***********/
    GPIO_REQUEST_ERROR:
        device_destroy(myclass,dynamicDeivcenumber);
    DEVICE_ERROR:
        class_destroy (myclass);
    CLASS_ERROR:
        cdev_del (&st_characterDevice) ;
    CHARACTER_ERROR:
        unregister_chrdev_region(dynamicDeivcenumber,1);
    return -1;
    /*****************error handling code***********/
}

static void __exit Hellodriver_exit(void)
{
    pwm_disable(pwm0);                              //disable the pwm
    pwm_free(pwm0);                                 //free the struct
    cdev_del (&st_characterDevice) ;                //delete dynamicDeivcenumber
    device_destroy(myclass,dynamicDeivcenumber);    //delete device
    class_destroy (myclass);                        //delete the device class
    unregister_chrdev_region(dynamicDeivcenumber,1);//delete the reserved number
    printk("GoodBye World\n");
}





module_init(Hellodriver_init);

module_exit(Hellodriver_exit);
