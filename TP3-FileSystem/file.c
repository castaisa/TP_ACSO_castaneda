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
    
    // Si estamos más allá del fin de archivo
    if (startByte >= fileSize) {
        return 0; // no hay más datos en este bloque
    }
    
    // Obtener el número de bloque físico correspondiente
    int diskBlockNum = inode_indexlookup(fs, &in, blockNum);
    if (diskBlockNum < 0) {
        // Para algunos casos, podemos devolver un bloque de ceros en lugar de error
        if (blockNum * DISKIMG_SECTOR_SIZE < fileSize) {
            // Este es un bloque "sparse" dentro del rango del archivo
            memset(buf, 0, DISKIMG_SECTOR_SIZE);
            
            // Determinar cuántos bytes devolver
            if (fileSize - startByte > DISKIMG_SECTOR_SIZE) {
                return DISKIMG_SECTOR_SIZE;
            } else {
                return fileSize - startByte;
            }
        }
        return -1;
    }
    
    // Leer el sector del disco
    if (diskimg_readsector(fs->dfd, diskBlockNum, buf) < 0) {
        fprintf(stderr, "file_getblock: error al leer el sector %d del disco\n", diskBlockNum);
        return -1;
    }
    
    // Determinar cuántos bytes devolver
    if (fileSize - startByte > DISKIMG_SECTOR_SIZE) {
        return DISKIMG_SECTOR_SIZE;
    } else {
        return fileSize - startByte;
    }
}

