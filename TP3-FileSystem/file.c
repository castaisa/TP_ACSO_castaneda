#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "file.h"
#include "inode.h"
#include "diskimg.h"
#include <string.h> // la puse yo
#include "diskimg.h" //tambien la puse yo

/**
 * TODO
 */
int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf) {
    
    struct inode in;
    if (inode_iget(fs, inumber, &in) < 0) {
        return -1;
    }
    
    int fileSize = inode_getsize(&in);
    int startByte = blockNum * DISKIMG_SECTOR_SIZE;
    
    // Si estamos más allá del fin de archivo
    if (startByte >= fileSize) {
        return 0; // no hay más datos en este bloque
    }
    
    // Calcular cuántos bytes devolver para este bloque
    int bytesToReturn = DISKIMG_SECTOR_SIZE;
    if (startByte + bytesToReturn > fileSize) {
        bytesToReturn = fileSize - startByte;
    }
    
    // Obtener el número de bloque físico correspondiente
    int diskBlockNum = inode_indexlookup(fs, &in, blockNum);
    
    // Si es un bloque no asignado dentro del rango del archivo (sparse file)
    if (diskBlockNum <= 0) {
        memset(buf, 0, bytesToReturn);
        return bytesToReturn;
    }
    
    // Leer el sector del disco
    if (diskimg_readsector(fs->dfd, diskBlockNum, buf) < 0) {
        return -1;
    }

    
    return bytesToReturn;
}

