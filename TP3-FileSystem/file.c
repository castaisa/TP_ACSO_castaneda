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
        return -1; // Error al obtener el inodo
    }

    int blockNumReal = inode_indexlookup(fs, &in, blockNum);
    if (blockNumReal < 0) {
        return -1; // Error al obtener el bloque
    }

    if (diskimg_readsector(fs->dfd, blockNumReal, buf) < 0) {
        return -1; // Error al leer el bloque del disco
    }

    return DISKIMG_SECTOR_SIZE; // Retorna el tamaño del bloque leído
}

