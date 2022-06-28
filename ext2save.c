

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "header.h"
#include <linux/stat.h>

#define BASE_OFFSET 1024                   /* locates beginning of the super block (first group) */
#define FD_DEVICE "../img9.in"              


static unsigned int block_size = 0;        /* block size (to be calculated) */

static void read_dir(int, const struct ext2_inode*, const struct ext2_group_desc*);
static void read_inode(int, int, const struct ext2_group_desc*, struct ext2_inode*);
char name[256];
int main(int argc, char **argv)
{
	struct ext2_super_block super;
	struct ext2_group_desc group;

	struct ext2_inode inode;
	int fd;
	printf("%s \n",argv[1]);

	strcpy(name,argv[2]);
	printf("%s \n",name);
		

	/* open floppy device */

	if ((fd = open(argv[1], O_RDONLY)) < 0) {
		perror(argv[1]);
		printf("**\n");
		exit(1);  /* error while opening the floppy device */
		printf("**\n");
	}

	/* read super-block */

	lseek(fd, BASE_OFFSET, SEEK_SET); 
	read(fd, &super, sizeof(super));

	if (super.s_magic != 0xEF53) {
		fprintf(stderr, "Not a Ext2 filesystem\n");
		exit(1);
	}
		
	block_size = 1024 << super.s_log_block_size;

	/* read group descriptor */

	lseek(fd, BASE_OFFSET + block_size, SEEK_SET);
	read(fd, &group, sizeof(group));
	printf("[%d]\n",super.s_inodes_count);

	/* show entries in the root directory */
	read_inode(fd, 2, &group, &inode);   /* read inode 2 (root directory) */
	

	read_dir(fd, &inode, &group);

	close(fd);
	exit(0);
} /* main() */

static 
void read_inode(int fd, int inode_no, const struct ext2_group_desc *group, struct ext2_inode *inode)
{
	lseek(fd, BLOCK_OFFSET1(group->bg_inode_table)+(inode_no-1)*sizeof(struct ext2_inode), 
	      SEEK_SET);
	read(fd, inode, sizeof(struct ext2_inode));
} /* read_inode() */


int contains_jpg(char *arr,int n)
{
	int x=0;
	while(x<n)
	{

		if(strncmp((arr+x),".jpg",4)==0)
		{
			return 0;
		}
		x++;
	}
	return -1;

}
static void read_dir(int fd, const struct ext2_inode *inode, const struct ext2_group_desc *group)
{
	char *block;

	if (S_ISDIR(inode->i_mode)) {
		struct ext2_dir_entry *entry;
		unsigned int size = 0;

		if ((block = malloc(block_size)) == NULL) { /* allocate memory for the data block */
			fprintf(stderr, "Memory error\n");
			close(fd);
			exit(1);
		}
		
		lseek(fd, BLOCK_OFFSET1(inode->i_block[0]), SEEK_SET);
		int x=read(fd, block, block_size);                // read block from disk
		
		
		entry = (struct ext2_dir_entry *) block;  // first entry in the directory 
                // Notice that the list may be terminated with a NULL
                   //entry (entry->inode == NULL)
		while((size < inode->i_size) && entry->inode ) 
		{
			char file_name[254];
			

			
			x=0;

			struct ext2_inode in;
			read_inode(fd, entry->inode, group, &in);   /* read inode 2 (root directory) */
			printf("******************************\n");
			printf("Mode = %d\n",inode->i_mode);
			printf("length %d | %10u | %s\n",entry->name_len, entry->inode,entry->name);
			printf("Num Blocks : %d\n",in.i_blocks);
			printf("Size : %d\n",in.i_size);
			char arr[strlen(entry->name)+2];
			int i=0;
			strncpy(arr,name,strlen(name));
			arr[strlen(name)]='/';
			int j=strlen(name)+1;
			while(i<strlen(entry->name))
			{
				arr[j]=entry->name[i];

				i++;
				j++;
			}
			printf("file name copied: [%s]\n",arr);
			//arr[strlen(entry->name)+strlen(name)+1]='\0';
			
			if(contains_jpg(entry->name,strlen(entry->name))==0 || (strcmp(entry->name,".")!=0 && strcmp(entry->name,"..")!=0))
			{
				int fdw=open(arr,O_CREAT | O_WRONLY);
			
				if(fdw==-1)
				{
					exit(0);
				}
				
			
				while(x<in.i_blocks)
				{
					
					
					printf("reading inode's block %d of %d\n\n",x,in.i_blocks);					
					//printf("~%x\n",in.i_block[0]);
					//printf("%x\n",in.i_block[1]);
					//printf("%x\n",in.i_block[1]);
					uint8_t *blockn;
					if ((blockn = malloc(block_size*200)) == NULL)
					{ /* allocate memory for the data block */
						fprintf(stderr, "Memory error\n");
						close(fd);
						//exit(1);
					}
					lseek(fd, BLOCK_OFFSET1(in.i_block[x]), SEEK_SET);

					int valread=read(fd, blockn, in.i_size-1);
					printf("block read = %p\n",blockn);
					 i=0;
					int valwrote=write(fdw,blockn,valread);
					printf("wrote %d bytes\n",valwrote);
					
					
					printf("\n");
					x++;
					

							
				}
			}
			/*else
			{
				printf("directory of inode found %d\n",entry->inode);
				struct ext2_inode in1;
				read_inode(fd, entry->inode, group, &in1);   
				printf("new inode blocks found = %d\n",in1.i_blocks);
				
				
			}*/
			
			printf("******************************\n");


			
			entry = (void*) entry + entry->rec_len;
			
			size += entry->rec_len;
		}

		free(block);
	}
} /* read_dir() */
