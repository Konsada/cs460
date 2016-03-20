#include "type.h"
#include "util.h"

char fsbuf[BLKSIZE];
char fsbuf2[BLKSIZE];
u32 tsize, dsize, bsize;

int load(char *filename, u16 segment){
  u16 iblk, i, nodeNumber,blk;
  u16 pathCount = 0;
  u32  *temp;

  printf("load(%s, %x)\n", filename, segment);
  //  getc();
  strcpy(path, filename);
  tokenize(path, '/');
  while(pathList[pathCount++]);
  pathCount--;

  get_block(2, fsbuf); // put GD block into buf1;
  gp = (GD*)fsbuf;

  iblk = (u16)gp->bg_inode_table;

  myprintf("bg_inode_table: %u\n", iblk);

  get_block(iblk, fsbuf);

  ip = (INODE*)fsbuf + 1;

  myprintf("i_uid: %d\n", ip->i_uid);
  nodeNumber = iblk;

  for(i = 0; i < pathCount; i++){

    myprintf("searching[%d:%d] for %s...\n", i,pathCount,pathList[i]);
    nodeNumber = findInode(ip, pathList[i]);    
    if(nodeNumber < 0){
      myprintf("failed: %s path not found!\n", filename);
      return 0;
    }
    myprintf("success: found %s in inode %d\n", pathList[i], nodeNumber);
    nodeNumber --;

    get_block((iblk+(nodeNumber/8)),fsbuf); 
    ip = (INODE *)fsbuf + (nodeNumber % 8);

  }
  //  clear_buffer(&fsbuf);
  get_block((u16)ip->i_block[0], fsbuf2); //i_block[0] = header iblock -- works

  myprintf("debug1\n");
  //  getc();

  hp = (HEADER *)fsbuf2;

  tsize = hp->tsize;
  dsize = hp->dsize;
  bsize = hp->bsize;
  myprintf("hp->tsize: %l\nhp->dsize: %l\nhp->bsize: %l\n", tsize, dsize, bsize);
  myprintf("loading %s to segment %x\n", filename, segment);
  setes(segment);

  //load direct i_blocks to memory
  for(i = 0; i < 12; i++){
    if(ip->i_block[i]){
      myprintf("!ip->i_block[%d]: %x\n", i, ip->i_block[i]);
      //      getc();
    }
    else
      break;

    //    myprintf("ip->i_block[%d] = %x\n", i, (u16)ip->i_block);
    myprintf("getblk((u16)ip->i_block[%d], %x)...", i, 0);
    //    getc();
    getblk((u16)ip->i_block[i], 0x0000); //<--- now working properly
    //    get_block((u16)ip->i_block[i],0);
    myprintf("inces()\n");
    //    getc();
    inces();
    myprintf("ip->i_block[%d] written to memory!\n", i);
    //    getc();
  }
  myprintf("debug3\n");
  //  getc();

  //load indirect i_blocks into buffer
  myprintf("checking for indirect blocks...\n");
  if(ip->i_block[12]){
    myprintf("loading indirect blocks...\n");
    temp = (u32*)ip;
    while(*temp){
      get_block((u16)*temp, 0x0000);
      inces();
      temp++;
    }
  }

  for(i = 0; i <= (tsize+dsize);i++)
    put_word(get_word(segment+2,i),segment,i);

  myprintf("clearing bss...\n");
  clear_bss(segment, tsize, dsize, bsize);
  myprintf("moving ES back to MTX segment...\n");
  setes(0x1000);
  myprintf("load done\n");
  return 1;
}

int clear_buffer(char **buf){
  int i;
  for(i = 0; i < BLKSIZE; i++){
    *buf[i] = 0;
    myprintf("buf[%d] = %c\n", i, buf[i]);
    if(!(i%50))getc();
  }
}
int get_block(u16 blk, char *buf){
  //diskr(cyl,head,sector,buf);
  
    myprintf("blk: %u, buf: %x\n", blk, buf);   
    //    getc();
    myprintf("diskr(%d, %d, %d, %x)\n", blk/18, ((2*blk)%36)/18, (((2*blk)%36)%18), buf);
    //    getc();
  
  //diskr(blk/18, ((2*blk)%36)/18, (((2*blk)%36)%18), buf);
  diskr( blk/18, ((2*blk)%36)/18, (((2*blk)%36)%18), buf);

}

int findInode(INODE *tip, char *name){
  int i, n;
  char debugBuf[255];

  for(i = 0; tip->i_block[i] && i < 12; i++){
    get_block((u16)tip->i_block[i], fsbuf2);    
    dp = (DIR*)fsbuf2;

    while((char*)dp< &fsbuf2[BLKSIZE]){
      printf("searching...\n");
      

      for(i = 0; i < 255; i++)
	debugBuf[i] = 0;
      myprintf("dp->inode:    %l\n", dp->inode);
      strncpy(&debugBuf, dp->name, dp->name_len);
      myprintf("dp->name:     %s\n", debugBuf);
      myprintf("dp->name_len: %d\n", dp->name_len);
      myprintf("dp->rec_len:  %d\n", dp->rec_len);

      //      if(!mystrncmp(dp->name,name,(u16)dp->name_len)){
      if(mystrcmp(debugBuf, name) == 0){	
	myprintf("success: %s found in node %d!\n", debugBuf, dp->inode);
	return (u16)dp->inode;
      }
      
      dp = ((char *)dp + dp->rec_len);
    }
  }
  return -1;
}
