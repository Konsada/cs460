typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned long  u32;

#define  GREEN  10         // color byte for putc()
#define  CYAN   11
#define  RED    12
#define  MAG    13
#define  YELLOW 14

#include "ext2.h"

struct partition {         // Partition table entry in MBR
	u8  drive;          // 0x80 - active 
	u8  head;	   // starting head 
	u8  sector;	   // starting sector 
	u8  cylinder;       // starting cylinder 
	u8  type;	   // partition type 
	u8  end_head;       // end head 
	u8  end_sector;	   // end sector 
	u8  end_cylinder;   // end cylinder 
	u32 start_sector;   // starting sector counting from 0 
	u32 nr_sectors;     // nr of sectors in partition 
};

struct dap{                // DAP for extended INT 13-42  
	u8   len;           // dap length=0x10 (16 bytes)
	u8   zero;          // must be 0  
	u16  nsector;       // number of sectors to read: 1 to 127
	u16  addr;          // memory address = (segment:addr)
	u16  segment;    
	u32  s1;            // low  4 bytes of sector#
	u32  s2;            // high 4 bytes of sector#
};

struct dap dap, *dp;       // global dap struct
u16 color = RED;           // initial color for putc()

#define  BOOTSEG 0x9000

// #include "bio.c" <========= WRITE YOUR OWN io.c file !!!!
#include "util.h"          // contains my io

char mbr[512];
char ans[64];
int partition, bsector;

// load a disk sector to (DS, addr), where addr is an offset in segment
int getSector(u32 sector, char *addr)
{
	dp->addr    = addr;
	dp->s1      = (u32)sector;
	diskr();    // call int13-43 in assembly
}
int get_block(u32 blk, char *buf)
{
	dp->addr = (u16)buf;
	dp->s1 = (u32)(bsector + (u32)(2*blk));
	diskr();
}

char buf[1024];
SUPER *sp;
GD *gp;
INODE *ip;
DIR *dirp;

int main()
{
	int i;
	struct partition *p;
	printf("booter start in main()\n");

	// initialize the dap struct
	dp = &dap;
	dp->len  = 0x10;        // must be 16
	dp->zero = 0;           // must be 0
	dp->nsector = 1;        // load one sector
	dp->addr = 0;           // will set to addr              
	dp->segment = BOOTSEG;  // load to which segment
	dp->s1 = 0;             // will set to LBA sector#
	dp->s2 = 0;             // high 4-byte address s2=0

	getSector((u32)0, (u16)mbr); // get MBR
	printf("show partition table\n");
	p = (struct partition *)(&mbr[0x1BE]);
	printf("p#    type    start_sector    nr_sectors\n");
	printf("----------------------------------------\n");
	for(i = 1; i<=4; i++)
	{
		printf("%d    %x    %l    %l\n", i, p->type, p->start_sector, p->nr_sectors);
		if(i==1)
		{
			partition = 1
				bsector = p->start_sector;
		}
		p++;
	}
	printf("----------------------------------------\n");

	color = CYAN;

	/*while(1){
	  printf("what's your name? ");
	  gets(ans);
	  if (strcmp(ans, "quit")==0){
	  printf("\nexit main()\n");
	  break;
	  }
	  printf("\nWelcome %s!\n", ans);
	  }*/
	// initialize dap to read 2 sectors
	dp = &dap;
	dp->len = 0x10;
	dp->zero = 0;
	dp->nsector = 2;
	dp->segment = BOOTSEG;
	dp->s2 = 0;
	color = RED;

	printf("*************** Keon's Booter ****************\n");
	printf("partition=%d bsector=%d\n", partition, bsector);
	getc();

	get_block((u32)1, buf);
	sp = (SUPER *)buf;
	printf("magic=%x\n", sp->s_magic);

	get_block((u32)2, buf);
	gp = (GD *)buf;
	printf("bmap=%l imap=%l inode_table=%l\n", gp->bg_blcok_bitmap, gp->bg_inode_bitmap, gp->bg_inode_table);
	printf("bmap=%d imap=%d inode_table=%d\n", (u16)gp->bg_block_bitmap, (u16)gp->bg_inode_bitmap, (u16)gp->bg_inode_table);
}
