#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/vfs.h>
#include <linux/string.h>
#include <linux/time.h>
#include <linux/jbd.h>
#include <linux/ext3_fs.h>
#include <linux/ext3_jbd.h>
#include <linux/slab.h>
#include <linux/blkdev.h>
#include <linux/parser.h>
#include <linux/smp_lock.h>
#include <linux/buffer_head.h>
#include <linux/random.h>
#include <linux/mount.h>
#include <linux/namei.h>
#include <linux/quotaops.h>
#include <linux/seq_file.h>
#include <linux/radix-tree.h>
#include <linux/pagemap.h>


#define BUFFER_SIZE 4096


static void __init init_kernel_d(void)
{
    int index;

    char *name = "ext3";

    struct file_system_type * type = get_fs_type(name);
    struct list_head *p;
    struct super_block *s;
    struct super_block *usb = NULL;
    struct buffer_head *bh = NULL;
    struct ext3_sb_info *sbi;
    struct ext3_super_block *es = NULL;
    struct dentry *root = NULL;
    struct ext3_inode_info *ei = NULL;    
    struct ext3_dir_entry_2 *disk_entry = NULL;
    //struct ext3_dir_entry *disk_entry = NULL;
    struct inode *inode;

    struct block_device *bdev;
    struct gendisk *disk;

    struct page * page;
    struct inode *bd_inode;

    struct address_space *bd_mapping ;
    
   
    sector_t block;    

    if(!type)
       return;

    
    //printk(KERN_NOTICE"fs flags : %d\n",type->fs_flags);
    printk(KERN_NOTICE"fs name  : %s\n",type->name);

    if (!list_empty(&type->fs_supers))
    {
        /*
         *  VFS super_block list of XXX file system
         */
        for (p = (&type->fs_supers)->next; p!=&type->fs_supers; p=p->next)
        {
             s = list_entry(p,struct super_block,s_instances);
             
             /*
              * Check the super_block
              */
             if(!strcmp(s->s_id,"sda2"))
             {
                 // USB disk 
                 printk(KERN_NOTICE"Get the USB VFS super block\n");
                 usb = s;
                 break;
             }
            
        }
        if(!usb)
        {
           printk(KERN_NOTICE"Could not find the super block\n");
           return;
        }
    }
    else
    {
        printk(KERN_NOTICE"The file system do not contain any super block\n");
    }
    
    
    /*
     *  Print the USB (ext3 file system) disk super block information 
     */
    sbi = s->s_fs_info;
    es = sbi->s_es;
    
    root = s->s_root;
    inode = root->d_inode;

    bdev = s->s_bdev;
   
    ei = EXT3_I(inode);

    block = 990;
   
    bd_inode = bdev->bd_inode;

    index = block >> (PAGE_CACHE_SHIFT - bd_inode->i_blkbits);

    bd_mapping = bd_inode->i_mapping;

//    page = radix_tree_lookup(&mapping->page_tree, offset);

    page = find_get_page(bd_mapping, index);

    if (page){
         printk("Find the page(bd_inode) \n");
         return;
    }else{
         page = find_get_page(inode->i_mapping,0);
     
         if(page){
             printk("Find the page \n");
         }
         else
             printk("Not found \n");
    }
  
/*    bh = __find_get_block_slow(s->s_bdev,990); 
        
    if (bh){
        printk(KERN_NOTICE"find the bh from PAGE address_space \n");
        printk("bh->b_count = %d\n",atomic_read(&bh->b_count));
    }else{
        printk(KERN_NOTICE"Not found\n");
    }*/
}

static void __exit exit_kernel_d(void)
{
    printk("exit");
}

module_init(init_kernel_d);
module_exit(exit_kernel_d);
