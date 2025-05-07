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
    
    // Obtener el inodo del archivo
    if (inode_iget(fs, inumber, &in) < 0) {
        fprintf(stderr, "file_getblock: error al obtener el inodo %d\n", inumber);
        return -1;
    }

    // Traducir el número de bloque lógico a físico
    int diskBlockNum = inode_indexlookup(fs, &in, blockNum);
    if (diskBlockNum < 0) {
        fprintf(stderr, "file_getblock: error al obtener el bloque físico para inodo %d, bloque %d\n", inumber, blockNum);
        return -1;
    }

    // Leer el bloque físico desde el disco
    if (diskimg_readsector(fs->dfd, diskBlockNum, buf) < 0) {
        fprintf(stderr, "file_getblock: error al leer el sector %d del disco\n", diskBlockNum);
        return -1;
    }

    // Calcular tamaño del archivo y del bloque
    int fileSize = inode_getsize(&in);
    int startByte = blockNum * DISKIMG_SECTOR_SIZE;

    // Ver cuántos bytes del archivo hay en este bloque
    if (fileSize > startByte + DISKIMG_SECTOR_SIZE) {
        return DISKIMG_SECTOR_SIZE; // bloque completo
    } else if (fileSize > startByte) {
        return fileSize - startByte; // solo parte del bloque
    } else {
        return 0; // el bloque está más allá del final del archivo
    }
}

