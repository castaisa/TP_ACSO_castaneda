#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "file.h"
#include "inode.h"
#include "diskimg.h"
//Importo lo necesario para poder usar las funciones
#include <string.h> 
#include "diskimg.h"

/**
 * Lee un bloque específico de un archivo y lo carga en el buffer (que le pasan como parametro).
 * Obtiene el bloque identificandolo con el inode.
 * Devuelve la cantidad de bytes leidos, 
 * 0 si el bloque está fuera de rango o -1 en caso de que haya algun error.
 */
int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf) {
    
    struct inode in;
    if (inode_iget(fs, inumber, &in) < 0) {
        return -1;
    }
    
    int fileSize = inode_getsize(&in);
    int startByte = blockNum * DISKIMG_SECTOR_SIZE;
    
    //Caso donde el comienzo esta mas alla del fin de archivo
    if (startByte >= fileSize) {
        return 0;
    }
    
    //Variable que guarda la cantidad de bytes a devolver
    int bytesToReturn = DISKIMG_SECTOR_SIZE;
    if (startByte + bytesToReturn > fileSize) {
        bytesToReturn = fileSize - startByte;
    }
    
    int diskBlockNum = inode_indexlookup(fs, &in, blockNum);
    
    //Caso donde el bloque no esta asignado
    if (diskBlockNum <= 0) {
        memset(buf, 0, bytesToReturn);
        return bytesToReturn;
    }

    if (diskimg_readsector(fs->dfd, diskBlockNum, buf) < 0) {
        return -1;
    }

    
    return bytesToReturn;
}

