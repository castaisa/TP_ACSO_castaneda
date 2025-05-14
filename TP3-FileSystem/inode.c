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
    if (inumber < 1 || inumber > fs->superblock.s_isize * INODES_PER_BLOCK) {
        return -1; // Número de inodo inválido
    }
    int blockNum = (inumber - 1) / INODES_PER_BLOCK; // Número de bloque
    int offset = (inumber - 1) % INODES_PER_BLOCK; // Offset dentro del bloque
    char buf[DISKIMG_SECTOR_SIZE];
    if (diskimg_readsector(fs->dfd, blockNum, buf) < 0) {
        return -1; // Error al leer el bloque
    }
    struct inode *inodes = (struct inode *)buf;
    *inp = inodes[offset]; // Copiar el inodo solicitado
    return 0; // Éxito

    
}

/**
 * TODO
 */
int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp,
    int blockNum) {  
    //Implement Code Here
    if (blockNum < 0 || blockNum >= 8) {
        return -1; // Número de bloque inválido
    }
    int diskBlockNum = inp->i_addr[blockNum];
    if (diskBlockNum == 0) {
        return -1; // Bloque no asignado
    }
    return diskBlockNum; 

}

int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1); 
}
