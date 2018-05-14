#include <linux/list.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/init.h>

#define MSG_SIZE (512)
static char *msg;

static DEFINE_RWLOCK(list_lock);

/*rwlock_t list_lock;
static int __init rwlock_init(void)
{
	rwlock_init(&list_lock);
}*/

//struct to hold the Name and Value data.
struct KeyValue {
   char *name;
   char *value;
   struct list_head list; //Kernel standard object for linked list.  list_head
};
//Linked List
struct KeyValue keyvalueList;

static void keyvalue_show_list(void)
{
  struct list_head *listptr;
  struct KeyValue *entry;
  printk(KERN_DEBUG "show list\n");
  printk(KERN_DEBUG "name = %s (list %p, prev = %p, next = %p)\n",
	keyvalueList.name,&keyvalueList.list,
        keyvalueList.list.prev, keyvalueList.list.next);
  
  read_lock(&list_lock);

  list_for_each(listptr, &keyvalueList.list) {
      entry = list_entry(listptr, struct KeyValue, list);
      printk(KERN_DEBUG "name = %s (list %p, prev = %p, next = %p)\n",
	entry->name,&entry->list,
        entry->list.prev, entry->list.next); 
  }

  read_unlock(&list_lock);

  printk(KERN_DEBUG "Leaving Show list");

}

static ssize_t write_proc(struct file *filp,const char *buf,size_t count,loff_t *offp)
{
   int actual_len = count;

   if(count > MSG_SIZE-1)
	actual_len = MSG_SIZE-1;
 
   memset(msg, 0, MSG_SIZE);

   copy_from_user(msg, buf, actual_len);

   char *nameparam;
   char *valueparam;
   nameparam = strsep(&msg,"=");
   valueparam = strsep(&msg,"=");

   struct KeyValue *aNewNode = NULL;
   aNewNode = kmalloc(sizeof(struct KeyValue), GFP_KERNEL);

   strcpy(aNewNode->name,nameparam);
   strcpy(aNewNode->value,valueparam);
   
   write_lock(&list_lock);

   list_add(&aNewNode->list, &keyvalueList.list);

   write_unlock(&list_lock); 

   keyvalue_show_list();

   return count;
}

static ssize_t read_proc(struct file *flip, char *buffer, size_t len, loff_t *offset)
{

   int read_len = len;
   
   if(read_len > MSG_SIZE-1)
	read_len = MSG_SIZE-1;

   memset(msg, 0, MSG_SIZE);
   copy_from_user(msg, buffer, read_len);
   

   struct KeyValue* aKeyVal;
   
   write_lock(&list_lock);

   list_for_each_entry(aKeyVal, &keyvalueList.list, list) {
	

        if(&aKeyVal->name == msg)
        {
	   copy_to_user(buffer,aKeyVal->name,len);
	   return len;
        }

   }
   
   write_unlock(&list_lock);
   
   return len;
}

struct file_operations proc_fops = {
	.owner = THIS_MODULE,
	.read = read_proc,
	.write = write_proc
};

static int proc_init (void) {

  if ((msg = kmalloc(MSG_SIZE, GFP_KERNEL)) == NULL)
    return -ENOMEM;

  proc_create("customdictwrite",0644,NULL,&proc_fops);

  INIT_LIST_HEAD(&keyvalueList.list);

  return 0;
}

static void proc_cleanup(void) {
    printk(KERN_DEBUG "Entering customdictwrite proc_cleanup.");

    remove_proc_entry("customdictwrite",NULL);
    kfree(msg);
    
    write_lock(&list_lock);

    struct  KeyValue *ptr;
    ptr = list_entry(keyvalueList.list.next, struct KeyValue, list);
    printk(KERN_DEBUG "Before while loop in cleanup customDiction module.");
    while(ptr != NULL)
    {
    	printk(KERN_DEBUG "Before list_del in customDiction module.");
    	list_del(keyvalueList.list.next);
    	printk(KERN_DEBUG "After Delete in in customDiction module.");
        if(ptr->name!=NULL)
	{
    		kfree(ptr);
        	printk("After the kfree on the ptr");

		ptr = list_entry(keyvalueList.list.next, struct KeyValue, list);
	}
	else
	{
		ptr = NULL;
	}


	printk("After ptr list_entry -- %p",ptr);
    }
    
    write_lock(&list_lock);

    printk(KERN_DEBUG "customdictwrite proc_cleanup successfully unloaded.");
}

//Meta Data the Kernel looks at.
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas Dumont");
MODULE_DESCRIPTION("A Linux module that implements a dictionary.");
MODULE_VERSION("1");

//Interface for Loadable Kernel Modules
module_init(proc_init); //Load the module - insmod command
module_exit(proc_cleanup);//Remove the module - rmmod command

