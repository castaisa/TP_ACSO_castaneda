#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "inode.h"
#include "diskimg.h"


/**
 * TODO
 */
int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    //Implement Code Here
    if (inumber < 1) {
        return -1;  // número de inodo inválido
    }

    int inodes_per_block = DISKIMG_SECTOR_SIZE / sizeof(struct inode);
    int inode_block_offset = (inumber - 1) / inodes_per_block;
    int inode_index_in_block = (inumber - 1) % inodes_per_block;

    int sector = INODE_START_SECTOR + inode_block_offset;

    struct inode buffer[DISKIMG_SECTOR_SIZE / sizeof(struct inode)];

    if (diskimg_readsector(fs->dfd, buffer, sector) < 0) {
        return -1;  // error al leer el sector
    }

    *inp = buffer[inode_index_in_block];
    return 0;  // éxito
}

/**
 * TODO
 */
int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp,
    int blockNum) {  
        //Implement code here
    return 0;
}

int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1); 
}
