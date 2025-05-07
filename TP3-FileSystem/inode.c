#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "inode.h"
#include "diskimg.h"

#define INODES_PER_BLOCK 16

/**
 * TODO
 */
int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    //Implement Code Here
    if (inumber < 1) {
        return -1;
    }

    int sector = INODE_START_SECTOR + (inumber - 1) / INODES_PER_BLOCK;
    int offset = (inumber - 1) % INODES_PER_BLOCK;

    struct inode inodes[INODES_PER_BLOCK];

    if (diskimg_readsector(fs->dfd, sector, inodes) < 0) {
        return -1;
    }

    *inp = inodes[offset];  // Copiar el inode solicitado

    return 0;
}

/**
 * TODO
 */
int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp,
    int blockNum) {  
        if (blockNum < 0) {
            return -1;
        }
    
        // Casos directos: i_addr[0] a i_addr[5]
        if (blockNum < 6) {
            return inp->i_addr[blockNum];
        }
    
        // Caso indirecto simple
        if (blockNum < 6 + 128) {
            int indirect_block = inp->i_addr[6];
            if (indirect_block == 0) {
                return -1; // No está asignado
            }
    
            uint16_t block_data[256];  // 512 bytes / 2 bytes = 256 punteros
    
            if (diskimg_readsector(fs->dfd, indirect_block, block_data) < 0) {
                return -1;
            }
    
            int indirect_index = blockNum - 6;
            return block_data[indirect_index];
        }
    
        // No soportamos dobles indirectos
        return -1;
}

int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1); 
}
