#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "file.h"
#include "inode.h"
#include "diskimg.h"

/**
 * TODO
 */
int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf) {
    
    struct inode in;

    if (inode_iget(fs, inumber, &in) < 0) {
        fprintf(stderr, "file_getblock: error al obtener el inodo %d\n", inumber);
        return -1;
    }

    int fileSize = inode_getsize(&in);
    int startByte = blockNum * DISKIMG_SECTOR_SIZE;

    if (startByte >= fileSize) {
        return 0; // no hay más datos en este bloque
    }

    int diskBlockNum = inode_indexlookup(fs, &in, blockNum);
    if (diskBlockNum < 0) {
        fprintf(stderr, "file_getblock: error al obtener el bloque físico para inodo %d, bloque %d\n", inumber, blockNum);
        return -1;
    }

    if (diskimg_readsector(fs->dfd, diskBlockNum, buf) < 0) {
        fprintf(stderr, "file_getblock: error al leer el sector %d del disco\n", diskBlockNum);
        return -1;
    }

    if (fileSize > startByte + DISKIMG_SECTOR_SIZE) {
        return DISKIMG_SECTOR_SIZE;
    } else {
        return fileSize - startByte;
    }
}

