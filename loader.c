#include "type.h"
#include "util.h"

int load(char *filename, int segment){
  u16 iblk, i, nodeNumber;
  u16 pathCount = 0;
  u32 tsize, dsize, bsize, *temp;

  strcpy(path, filename);
  tokenize(path, '/');
  while(pathList[pathCount++]);

  get_block(2, fsbuf); // put GD block into buf1;
  gp = (GD*)fsbuf;

  iblk = (u16)gp->bg_inode_table;
  myprintf("bg_inode_table: %u\n", iblk);

  get_block(iblk, fsbuf);

  ip = (INODE*)fsbuf + 1;

  myprintf("i_uid: %d\n", ip->i_uid);
  nodeNumber = iblk;

  for(i = 0; i < pathCount; i++){
    nodeNumber = findInode(ip, pathList[i]);
    myprintf("searching for %s...\n", pathList[i]);
    if(nodeNumber < 0){
      myprintf("failed: %s path not found!\n", filename);
      return 0;
    }
    myprintf("success: found %s\n", pathList[i]);
    nodeNumber --;
    get_block((iblk+(nodeNumber/8)),fsbuf);
    ip = (INODE *)fsbuf + (nodeNumber % 8);
  }

  get_block((u16)ip->i_block[0], fsbuf); //i_block[0] = header iblock

  hp = (HEADER *)fsbuf;

  tsize = hp->tsize;dsize = hp->dsize;bsize = hp->bsize;

  setes(segment);

  //load direct i_blocks to memory
  for(i = 0; i < 12; i++){
    if(!ip->i_block[i]){
      break;
    }
    get_block((u16)ip->i_block[i],0);
    inces();
  }
  //load indirect i_blocks into buffer
  if(ip->i_block[12]){
    temp = (u32*)ip;
    while(*temp){
      get_block((u16)*temp, 0);
      inces();
      temp++;
    }
  }

  for(i = 0; i <= (tsize+dsize);i++)
    put_word(get_word(segment+2,i),segment,i);

  clear_bss(segment, tsize, dsize, bsize);

  setes(0x1000);
  myprintf("load done\n");
  return 1;
}

int get_block(u16 blk, char *buf){
  //diskr(cyl,head,sector,buf);

  diskr((blk/18), (((2*blk)%36)/18), (((2*blk)%36)%18), buf);
}

int findInode(INODE *tip, char *name){
  int i;
  char debugBuf[64];

  for(i = 0; tip->i_block[i] && i < 12; i++){
    get_block((u16)tip->i_block[i], fsbuf);    
    dp = (DIR*)fsbuf;

    while((char*)dp< &fsbuf[BLKSIZE]){
      printf("searching...\n");
      strncpy(&debugBuf, dp->name, dp->name_len);
      printf("%s\n", debugBuf);
      if(!mystrncmp(dp->name,name,(u16)dp->name_len)){
	return (u16)dp->inode;
      }
      dp = dp + dp->rec_len;
    }
  }
  return -1;
}
