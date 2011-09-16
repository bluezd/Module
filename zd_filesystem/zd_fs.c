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
#include <linux/mpage.h>
#include <linux/ext3_fs_i.h>
#include <linux/pagemap.h>

/*static struct file_system_type zd_fs_type = {
        .owner          = THIS_MODULE,
        .name           = "zd",
        .get_sb         = zd_get_sb,
        .kill_sb        = kill_block_super,
        .fs_flags       = FS_REQUIRES_DEV,
};*/

//#define DNAME_INLINE_LEN (sizeof(struct dentry)-offsetof(struct dentry,d_iname))

#define BUFFER_SIZE 4096

static void destory_group_descriptor(struct buffer_head *bh);
static int read_inode_file(struct super_block *sb,int block,unsigned long offset);
static void print_data(struct inode *inode,int block,loff_t pos);


static void __init init_zd_fs(void)
{
    int err,i,ino;
    int rec_len;
    int db_count;
    int result;
    int found = 0;
    int block_count;
    int data_block;
    static int block[EXT3_N_BLOCKS];
    int offset,dsize;

    unsigned long first_meta_bg;
    unsigned long desc, group_desc, block_group,offset_new;

    char *name = "ext3";

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
    struct ext3_inode_info *file_ei =NULL;

    struct ext3_dir_entry_2 *disk_entry = NULL;
    //struct ext3_dir_entry *disk_entry = NULL;
    struct inode *inode;
    struct inode *file_inode;

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
    
    // printk(KERN_NOTICE"first data block = %d\n",le32_to_cpu(es->s_first_data_block));
    //printk(KERN_NOTICE"s_first_ino      = %d\n",sbi->s_first_ino);

    //db_count = (sbi->s_groups_count + EXT3_DESC_PER_BLOCK(s) - 1)/EXT3_DESC_PER_BLOCK(s);
    //printk(KERN_NOTICE"db_count      = %d\n",db_count);

    //first_meta_bg =  le32_to_cpu(es->s_first_meta_bg);
    //printk(KERN_NOTICE"first_meta_bg = %d\n",first_meta_bg);
    
    //result = EXT3_HAS_INCOMPAT_FEATURE(s, EXT3_FEATURE_INCOMPAT_META_BG);
    //printk("result = %d\n",result); 
   
    /*bh = sbi->s_group_desc[0];
    gdp = (struct ext3_group_desc *)bh->b_data;
    
    for(i=0; i < sbi->s_groups_count; i++)
    {
        block_count = le32_to_cpu(gdp[i].bg_inode_table);
        printk(KERN_INFO"group %d bg_inode_table %d\n\n",i,block_count);
    }*/

   // printk("block bitmap number = %d\n",le32_to_cpu(gdp[0].bg_block_bitmap));
   // printk("inode bitmap number = %d\n",le32_to_cpu(gdp[0].bg_inode_bitmap));*/

    //destory_group_descriptor(bh);

    /**
      *  Print the hd_struct 
      *
    bdev = s->s_bdev;

    if (bdev->bd_contains != bdev)
    {
        printk("sdb1\n");
        
        hd = bdev->bd_part;
        printk("start sect sdb1 = %d\n",hd->start_sect); 
    }*/

    //printk("%d\n",DNAME_INLINE_LEN);

    /*bh = sbi->s_sbh;
    buffer_es = (struct ext3_super_block *)bh->b_data;

    printk("s_inodes_count = %d\n",es->s_inodes_count);
    printk("New buffer s_inodes_count = %d\n",buffer_es->s_inodes_count);
    printk("New buffer s_first_ino = %d\n",buffer_es->s_first_ino);*/


    /*
     *  sb_bread()  
     */
    /*bh = sb_bread(s,1);
    
    if(bh)
    {
        gdp = (struct ext3_group_desc *)bh->b_data;
        printk("New Group descriptor bg_block_bitmap = %d\n",gdp->bg_block_bitmap);
    }*/
    
    /*bh = sb_bread(s,485);
    
    if(bh)
    {
         raw_inode = (struct ext3_inode *)bh->b_data;
         printk("inode 1.i_blocks = %d\n",raw_inode->i_blocks);
         
         raw_inode = (struct ext3_inode *)(bh->b_data + 128);
         printk("inode 2(root).i_blocks = %d\n",raw_inode->i_blocks);

         raw_inode = (struct ext3_inode *)(bh->b_data + 1408);
         printk("inode 11(lost+found).i_blocks = %d\n",raw_inode->i_blocks);
         
    }

    brelse(bh);*/

/*    err = register_filesystem(&zd_fs_type);
    if(!err)
        printk(KERN_NOTICE"register zd file system success\n");
    else
        printk(KERN_NOTICE"register zd file system fail\n");
    */

    root = s->s_root;
    inode = root->d_inode;
    //printk("inode 2 i_nlink = %d\n",inode->i_nlink);
   
    ei = EXT3_I(inode);
    
    /*if (ext3_should_order_data(inode))
         printk("ext3_ordered_aops\n");
    else if (ext3_should_writeback_data(inode))
         printk("ext3_writeback_aops\n");
    else
         printk("ext3_journalled_aops\n");*/

    //printk("i_size = %d\n",inode->i_size);    
    //printk("i_dir_start_lookup = %d\n",ei->i_dir_start_lookup);    

    /**
      * root dentry blocks
      */
    for (i = 0; i < EXT3_N_BLOCKS; i++)
    {
           block[i] = ei->i_data[i];
           //printk("block[i] = %d  ",block[i]);
           if (!block[i])
               continue;
           else
           {
                printk("Starting find block %d......\n",block[i]);
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


                    if(!strcmp(disk_entry->name,"feichengwurao.rmvb"))
                    {
                        // find the directory "lost+found"
                        found = 1;

                       // printk("Find the \"zhudong\" file\n");
                        //printk("inode number = %d\n",disk_entry->inode);
                        //printk("name = %s\n",disk_entry->name);
                        
                        printk("| inode number | name_len | file_type | name |\n");
                        printk("%11d%11d%11d    %11s\n",disk_entry->inode,  disk_entry->name_len, disk_entry->file_type, disk_entry->name);

                        ino = disk_entry->inode;
                         
                        /**
                          *  Get the inode of zhudong
                          */
                        /*block_group = (ino - 1) / EXT3_INODES_PER_GROUP(s);

                        if (block_group >= EXT3_SB(s)->s_groups_count)
                        {
                             printk("error block_group\n");
                             break;
                        }

                        group_desc = block_group >> EXT3_DESC_PER_BLOCK_BITS(s);

                        desc = block_group & (EXT3_DESC_PER_BLOCK(s) - 1);

                        group_bh = EXT3_SB(s)->s_group_desc[group_desc];    

                        gdp = (struct ext3_group_desc *)group_bh->b_data;*/

                        /*
                         * Figure out the offset within the block group inode table
                         */
                        //offset_new = ((ino - 1) % EXT3_INODES_PER_GROUP(s)) *  EXT3_INODE_SIZE(s);

                        // the inode block number
                        //block_count = le32_to_cpu(gdp[desc].bg_inode_table) + (offset_new >> EXT3_BLOCK_SIZE_BITS(s));

                        //data_block = read_inode_file(s,block_count,offset_new);
       

                        file_inode = iget(s,ino);

                        file_ei = EXT3_I(file_inode);

                        data_block = file_ei->i_data[0];                  
 
                        printk("Get the file inode ,the data block %d\n\n",data_block);
  
                         /**
                          * print the data block
                          */
                        print_data(file_inode,data_block,0);
                        print_data(file_inode,data_block,4092);


                           
                        break;
                    }
                    //printk(KERN_INFO"file name = %s\n",disk_entry->name);
                    //printk(KERN_INFO"inode number = %d\n",le32_to_cpu(disk_entry->inode));
                    //printk(KERN_INFO"\n");

                    offset += disk_entry->rec_len;
                    //offset += (((disk_entry->name_len + 3)/4)*4 + 8);

                    rec_len += disk_entry->rec_len;

                    if (rec_len >= BUFFER_SIZE - 8){
                           printk("search %d block ok ...\n",block[i]);

                           break;
                    }
               }

               brelse(bh);

               if(found)
                   break;

           }
    }
    if (!found)
        printk("Not find \n");

}

static void destory_group_descriptor(struct buffer_head *bh)
{
    memset(bh->b_data,0,4096);

    mark_buffer_dirty(bh);

    sync_dirty_buffer(bh);
}

static int read_inode_file(struct super_block *sb,int block,unsigned long offset)
{
    int data_block;
    struct buffer_head *bh;
    struct ext3_inode *inode;

    bh = sb_bread(sb,block);

    inode = (struct ext3_inode *)(((char *)bh->b_data) + offset);

    data_block = inode->i_block[0];

    brelse(bh);
    return data_block;
}



static void print_data(struct inode *inode,int block,loff_t pos)
{
       int error;
       char *data;
       unsigned long index;
       unsigned long offset;

       struct buffer_head *bh;
       struct page *page;
       struct page *cached_page;
       struct address_space *mapping;
       struct file *filp = current->files;

       cached_page = NULL;
       mapping = inode->i_mapping;

       if (mapping->host != inode){
             printk("serious error\n");
      
             goto out;
       }    

       index = pos >> PAGE_CACHE_SHIFT;
   
       page = find_get_page(mapping,index);

       if (unlikely(page == NULL)){
              printk("Not find  alloc new page \n");

              goto no_cached_page;
       }

       printk("Find the page from file inode i_data\n\n");

       if (!PageUptodate(page))
              goto page_not_up_to_date;


page_ok:
       printk("Print the file data\n");

       data = ((char *)(page_address(page))) + pos;
       printk("=================================================================\n");
       printk("%s",data);
       printk("=================================================================\n");

       goto out;

page_not_up_to_date:
       /* Get exclusive access to the page ... */
       lock_page(page);

       if (!page->mapping){
            unlock_page(page);

            printk("page_not_up_to_date error");
            goto out;  
       }

       /* Did somebody else fill it already? */
       if (PageUptodate(page)) {
                unlock_page(page);
                goto page_ok;
       }
       printk("Read the file again\n");


readpage:

      // error = mpage_readpage(page, ext3_get_block);     
       error = mapping->a_ops->readpage(filp, page);
      
       if (error)
              goto out;

       if (!PageUptodate(page)) {

              lock_page(page);

              // transmit ok ......
              unlock_page(page);           
       } 

       goto page_ok;


no_cached_page: 
       if (!cached_page){
              cached_page  = page_cache_alloc_cold(mapping);

              if (!cached_page){
                      printk("Alloc page fail\n");
                      goto out;
              }  
              printk("page->index = %d\n",cached_page->index);

              if (PageHighMem(cached_page))
                      printk("high memory \n");
              printk("The virtual address = %p\n",page_address(cached_page));
       }

       //error = add_to_page_cache_lru(cached_page, mapping,index, GFP_KERNEL);
       error = add_to_page_cache(cached_page, mapping,index, GFP_KERNEL);

       if (error)
            goto out;

       page = cached_page;
       cached_page = NULL;

       goto readpage;

out:
       return; 
}



static void __exit clear_zd_fs(void)
{
  //  unregister_filesystem(&zd_fs_type);
    printk(KERN_NOTICE"exit\n");
}

module_init(init_zd_fs);
module_exit(clear_zd_fs);
