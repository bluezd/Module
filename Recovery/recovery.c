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


#define BUFFER_SIZE 4096
                                                    
static int read_ext3_inode(struct super_block *sb,int block_count,unsigned long offset_new,char *name,int number);

static void __init init_recovery(void)
{
    int err,i,ino;
    int db_count;
    int result;
    int rec_len;
    int found = 0;
    int block_count;
    int data_block;
    static int block[EXT3_N_BLOCKS];
    int offset,dsize;

    unsigned long first_meta_bg;
    unsigned long desc, group_desc, block_group,offset_new;

    char *name = "ext3";
    char *file_name = "zd";

    struct file_system_type * type = get_fs_type(name);
    struct list_head *p;
    struct super_block *s;
    struct super_block *usb = NULL;
    struct buffer_head *bh = NULL;
    struct buffer_head *group_bh = NULL;
    struct buffer_head *new_bh = NULL;
    struct ext3_sb_info *sbi;
    struct ext3_super_block *es = NULL;
    struct ext3_super_block *buffer_es = NULL;
    struct ext3_group_desc *gdp = NULL;
    struct ext3_inode *raw_inode=NULL;
    struct dentry *root = NULL;
    struct ext3_inode_info *ei = NULL;
    struct ext3_dir_entry_2 *disk_entry = NULL;
    //struct ext3_dir_entry *disk_entry = NULL;
    struct inode *inode;

    struct block_device *bdev;
    struct gendisk *disk;
    struct hd_struct *hd;

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
             if(!strcmp(s->s_id,"sdb1"))
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

    ei = EXT3_I(inode);
    for (i = 0; i < EXT3_N_BLOCKS; i++)
    {
           block[i] = ei->i_data[i];
           //printk("block[i] = %d  ",block[i]);
           if (!block[i])
               continue;
           else
           {
                printk("Starting find block %d......\n\n",block[i]);
                
                bh = sb_bread(s,block[i]);

                if(!bh)
                  return;

                offset = 0;
                rec_len = 0;

                while(1)
                {
                     disk_entry = (struct ext3_dir_entry_2 *)(((char *)bh->b_data) + offset);

                     if (disk_entry->rec_len <= 0 || (disk_entry->inode == 0 && disk_entry->name_len == 0 && disk_entry->name == NULL)){
                           printk("error\n");
                           break;
                     }
                   
                     disk_entry->name[disk_entry->name_len] = '\0';

                     if(!strcmp(disk_entry->name,file_name))
                     {
                         // find the directory "lost+found"
                         found = 1;

                         printk("Find the \"%s\" file\n",file_name);
                         printk("inode number = %d\n",disk_entry->inode);
                         printk("name = %s\n",disk_entry->name);

                         ino = disk_entry->inode;

                        
                         //  Get the inode of zhudong
                         
                         block_group = (ino - 1) / EXT3_INODES_PER_GROUP(s);

                         if (block_group >= EXT3_SB(s)->s_groups_count)
                         {
                              printk("error block_group\n");
                              break;
                         }

                         group_desc = block_group >> EXT3_DESC_PER_BLOCK_BITS(s);

                         desc = block_group & (EXT3_DESC_PER_BLOCK(s) - 1);

                         group_bh = EXT3_SB(s)->s_group_desc[group_desc];

                         gdp = (struct ext3_group_desc *)group_bh->b_data;

                        
                         // Figure out the offset within the block group inode table
                         
                         offset_new = ((ino - 1) % EXT3_INODES_PER_GROUP(s)) *  EXT3_INODE_SIZE(s);

                         // the inode block number
                         block_count = le32_to_cpu(gdp[desc].bg_inode_table) + (offset_new >> EXT3_BLOCK_SIZE_BITS(s));

                         /*
                          *  Now we get the file inode (inode table) block number
                          *  read the inode (ext3_inode)
                          */
                         result = read_ext3_inode(s,block_count,offset_new,file_name,ino);
                      
                         return;
                     
                     }

                     offset += disk_entry->rec_len;
                     //offset += (((disk_entry->name_len + 3)/4)*4 + 8);

                     rec_len += disk_entry->rec_len;
                     //rec_len += offset;

                     if (rec_len >= BUFFER_SIZE - 8){
                             break;
                     }

               }

               brelse(bh);

           }
    }
}

static int read_ext3_inode(struct super_block *sb,int block_count,unsigned long  offset_new,char *name,int number)
{
      
      int result = 0;

      struct buffer_head *bh;
      struct ext3_inode *inode;

      bh = sb_bread(sb,block_count);

      if (!bh)
          return -1;

      inode = (struct ext3_ionde *)(((char *)bh->b_data) + offset_new);

      printk("write the inode information to the log\n");
  
      printk("== file name = %s   inode number = %d\n",name,number);
      printk("== i_links_count = %d \n",inode->i_links_count);

      inode = (struct ext3_inode *)((unsigned long )inode + EXT3_INODE_SIZE(sb));

      printk("== inode number = %d\n",number + 1);
      printk("== i_links_count = %d \n",inode->i_links_count);
      printk("== i_blocks = %d \n",inode->i_blocks);
      
      printk("== i_block  = %d \n",inode->i_block[0]);

      brelse(bh);

      return result;

}


static void __exit clear_recovery(void)
{
  //  unregister_filesystem(&zd_fs_type);
    printk(KERN_NOTICE"exit\n");
}

module_init(init_recovery);
module_exit(clear_recovery);


