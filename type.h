#ifndef TYPE_H
#define TYPE_H

#define BLOCK_SIZE        1024
#define BITS_PER_BLOCK    (8*BLOCK_SIZE)
#define INODES_PER_BLOCK  (BLOCK_SIZE/sizeof(INODE))

// Block number of EXT2 FS on FD
#define SUPERBLOCK        1
#define GDBLOCK           2
#define BBITMAP           3
#define IBITMAP           4
#define INODEBLOCK        5
#define ROOT_INODE        2

// Default dir and regular file modes with macros to check mode
#define DIR_MODE          0040777 
#define IS_DIR(x) (x & 0xF000) == 0x4000
#define FILE_MODE         0100644
#define IS_FILE(x) (x & 0xF000) == 0x8000
#define LINK_MODE         0xA1FF
#define IS_LINK(x) (x & 0xF000) == 0xA000
#define SUPER_MAGIC       0xEF53
#define SUPER_USER        0

//Permission
#define FILE_PERMISSION 0x81A4

// Table sizes
#define NMINODES         100
#define NMOUNT            10
//#define NPROC             10
#define NFD               10
#define NOFT             100


//Mailman's Algorithm Macros
#define IBLOCK(ino,offset) ((ino - 1) / INODES_PER_BLOCK + offset)
#define INUMBER(ino) (ino - 1) % INODES_PER_BLOCK

//calculate dir entry ideal_len
#define RECLEN(nlen) 4*((8 + nlen + 3)/4)


/*
// define shorter TYPES, save typing efforts


typedef struct ext2_super_block{
  u32 s_inodes_count;
  u32 s_blocks_count;
  u32 s_r_blocks_count;
  u32 s_free_blocks_count;
  u32 s_free_inodes_count;
  u32 s_first_data_block;
  u32 s_log_block_size;
  u32 s_log_frag_size;
  u32 s_blocks_per_group;
  u32 s_frags_per_group;
  u32 s_inodes_per_group;
  u32 s_mtime, s_wtime;
  u16 s_mnt_count;
  u16 s_magic;            // 0xEF53
}SUPER;
*/


/*


// Open File Table
typedef struct oft{
  int   mode;
  int   refCount;
  struct minode *inodeptr;
  int   offset;
} OFT;
/*
// PROC structure
typedef struct proc{
  int   uid;
  int   pid;
  int   gid;
  int   ppid;
  struct proc *next;
  int   status;
  struct minode *cwd;
  struct oft   *fd[NFD];
} PROC;
      
// In-memory inodes structure
typedef struct minode{		
  INODE    inode;               // disk inode
  int      dev, ino;
  int      refCount;
  int      dirty;
  int      mounted;
  struct mount *mountptr;
} MINODE;

// Mount Table structure
typedef struct mount{
  int    ninodes;
  int    nblocks;
  int    dev;
  int    bmap, imap, iblk;
  struct minode *mounted_inode;
  char   name[64]; 
  char   mount_name[64];
} MOUNT;

typedef enum {false, true} bool;





//minode array and root to load
MINODE minode[NMINODES], *root;

//processes that will be running
PROC *P0, *P1, *running, procs[NPROC];

//oft array
OFT ofts[NOFT];

//Mount array
MOUNT MT[NMOUNT], *mp;

//represents the whole path
char *names[64];
//number of names in path
int nameCount;

int inodeTable, imap, bmap, ninodes, nblocks, nproc, ifree, bfree;
//root device number
int fd;

char pathname[64], parameter[64], cmd[32];

bool DEBUG = true;
*/
#endif
