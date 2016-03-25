#ifndef TYPE_H
#define TYPE_H

typedef unsigned char   u8;
typedef unsigned short u16;
typedef unsigned long  u32;

#define NPROC    9
#define SSIZE 1024
#define BLKSIZE 1024
#define PROCNAMELEN 32

/******* PROC status ********/
#define FREE     0
#define READY    1
#define RUNNING  2
#define STOPPED  3
#define SLEEP    4
#define ZOMBIE   5

/********** kernel.c wait.c t.c struct **********/
typedef struct proc{
    struct proc *next;
    int    *ksp;               // at offset 2

    int    uss, usp;           // at offsets 4,6
    int    inkmode;            // at offset 8

    int    pid;                // add pid for identify the proc
    int    status;             // status = FREE|READY|RUNNING|SLEEP|ZOMBIE    
    int    ppid;               // parent pid
    struct proc *parent;
    int    priority;
    int    event;
    int    exitCode;
    char   name[32];           // name string of PROC

    int    kstack[SSIZE];      // per proc stack area
}PROC;

/**********loader.c structs **********/
typedef struct ext2_group_desc {
  u32 bg_block_bitmap;
  u32 bg_inode_bitmap;
  u32 bg_inode_table;
  u16 bg_free_blocks_count;
  u16 bg_free_inodes_count;
  u16 bg_used_dirs_count;
  u16 bg_pad;
  u32 bg_reserved[3];
} GD;

typedef struct ext2_inode {
  u16 i_mode;
  u16 i_uid;
  u32 i_size;
  u32 i_atime;
  u32 i_ctime;
  u32 i_mtime;
  u32 i_dtime;
  u16 i_gid;
  u16 i_links_count;
  u32 i_blocks;
  u32 i_flags;
  u32 reserved;
  u32 i_block[15];
  u32 i_pad[7];
}INODE;

typedef struct ext2_dir_entry_2 {
  u32 inode;
  u16 rec_len;
  u8 name_len;
  u8 file_type;
  char name[255];
} DIR;    // need this for new version of e2fs

typedef struct head{
  u32 ID_space;              // 0x04100301: combined I&D or 0x04200301: separate I&D
  u32 magic_number;          // 0x00000020
  u32 tsize;                 // code section size in bytes
  u32 dsize;                 // initialized data section size in bytes
  u32 bsize;                 // bss section size in bytes
  u32 zero;                  // 0
  u32 total_size;            // total memory size, including heap
  u32 symbolTable_size;      // only if symbol table is present
}HEADER;

/**************************************************/

/********** GLOBALS **********/
PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList, *zombieList;
GD *gp;
DIR *dp;
INODE *ip;
HEADER *hp;
int procSize = sizeof(PROC);
int nproc = 0;
int rflag;
char path[32];
int color;

char *statusStrings[ ]  = {"FREE   ", "READY  ", "RUNNING", "STOPPED", "SLEEP  ", "ZOMBIE ", 0};

char *pname[]={"Sun", "Mercury", "Venus", "Earth",  "Mars", "Jupiter", 
               "Saturn", "Uranus", "Neptune" };
/******************************/
/********** funcions **********/
/******************************/
/********** int.c **********/
int kcinth();
int kgetpid();
int kps();
int chname(char *name);
int kkfork();
int ktswitch();
int kkwait(int *status);
int kkexit(int value);
/********** kernel.c **********/
PROC *kfork(char *filename);
int makeUimage(char *filename, PROC *p);
void do_tswitch();
int do_kfork();
int do_exit(int exitValue);
int do_wait(int *ustatus);
int body();
int kmode();
int do_ps();
int do_chname(char *newName);
int do_kkfork();
/********** loader.c **********//*
int load(char *filename, int segment);
int get_block(u16 blk, char *buf);
int findInode(INODE *tip, char *name);
*/
/********** t.c **********/
int init();
int scheduler();
int int80h();
int set_vector(u16 vector, u16 handler);
int main();
int printQueues();
int printQueue(PROC *queue, char *queueName);
/********** wait.c **********/
void ksleep(int event);
int kwait(int *status);
int ready(PROC *p);
void kwakeup(int event);
int kexit(int exitValue);
/********** fork_exec.c **********/
int copyImage(u16 seg1, u16 seg2, u16 size);
int fork();
int exec(char *filename);

#endif
