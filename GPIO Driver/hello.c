#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include<linux/fs.h>
#include <linux/cdev.h>
#include <linux/gpio.h>

/*Meta information*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ahmed Osama");
MODULE_DESCRIPTION("First Driver");
MODULE_PARM_DESC(number,"Driver major number");
/*Meta information*/

//for static major number reserving

/*int number=0;
module_param(number,int,0664);//0664  user permetions allowed for the cnt to have a file so it can be edited during runtime 
int majornumber=0;*/

//for static major number reserving

dev_t dynamicDeivcenumber;
struct cdev st_characterDevice;
struct class *myclass;
struct device *mydevice;
#define SIZE 6
#define SIZE_READ 3
#define LED_PIN 2
#define TOUCH_LED 3
static unsigned char buffer[SIZE]="";

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

    /*switch case set the PIN (in this case PIN 2 to the desired user input from the user buffer intake*/
    switch (buffer[0])
    {
        case '1':
            gpio_set_value(LED_PIN,1);
        break;

        case '0':
            gpio_set_value(LED_PIN,0);
        break;
    }
    /*switch case set the PIN (in this case PIN 2 to the desired user input from the user buffer intake*/

    if (not_copied)
    {
        return -1;
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

    if (count + *offs > SIZE) // count 10 offs: 3:size 9

    {
        count=SIZE-*offs;
    }

    //not_copied = copy_to_user(user_buffer, &buffer[*offs], count);
    printk("the value of the button is :%d\n",gpio_get_value(TOUCH_LED));

    temp[0]=gpio_get_value(TOUCH_LED)+'0';
    temp[1]='\n';

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

    //for static major number reserving

    /*majornumber=number;
    retval=register_chrdev(majornumber,"my_first_driver",&fops);*/

    //for static major number reserving

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
        printk("file creation process failed !");
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

    //set the direction of the GPIO pin 2 to output
    if(gpio_request(LED_PIN,"rpi_gpio_2"))
    {
        printk("cannot allocate for gpio 2 \n");
        goto GPIO_REQUEST_ERROR;
    }
    if (gpio_direction_output(LED_PIN,0))
    {
        printk("cannot set the pin to be output\n");
        goto GPIO_DIR_ERROR;
    }
    //set the direction of the GPIO pin 2 to output

    //set the direction of the GPIO pin 3 to input
    if(gpio_request(TOUCH_LED,"rpi_gpio_3"))
    {
        printk("cannot allocate for gpio 3 \n");
        goto GPIO_DIR_ERROR;
    }
    if (gpio_direction_input(TOUCH_LED))
    {
        printk("cannot set the pin to be input\n");
        goto GPIO_TOUCH_ERROR;
    }
   //set the direction of the GPIO pin 3 to input

    return 0;

    //error handling code
    GPIO_TOUCH_ERROR:
        gpio_free(TOUCH_LED);
    GPIO_DIR_ERROR:
        gpio_free(LED_PIN);
    GPIO_REQUEST_ERROR:
        device_destroy(myclass,dynamicDeivcenumber);
    DEVICE_ERROR:
        class_destroy (myclass);
    CLASS_ERROR:
        cdev_del (&st_characterDevice) ;
    CHARACTER_ERROR:
        unregister_chrdev_region(dynamicDeivcenumber,1);
    return -1;
    //error handling code
}

static void __exit Hellodriver_exit(void)
{
    //for static major number destroying

    //unregister_chrdev(majornumber,"my_first_driver");

    //for static major number destroying

    gpio_set_value(LED_PIN,0);//to make sure when you re insert the module you start with GPIO pin on LOW
    gpio_free(LED_PIN);
    gpio_free(TOUCH_LED);
    cdev_del (&st_characterDevice) ;//delete dynamicDeivcenumber
    device_destroy(myclass,dynamicDeivcenumber);//delete device
    class_destroy (myclass);//delete the device class
    unregister_chrdev_region(dynamicDeivcenumber,1);//delete the reserved number
    printk("GoodBye World\n");
}





module_init(Hellodriver_init);

module_exit(Hellodriver_exit);
