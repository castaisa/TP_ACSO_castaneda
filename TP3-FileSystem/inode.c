#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "inode.h"
#include "diskimg.h"

PUNTEROS_POR_BLOQUE = DISKIMG_SECTOR_SIZE / sizeof(uint16_t); 

/**
 * Lee un inode del disco y lo carga en memoria.
 * Lo obtiene con su inumber, extrae el inode y lo copia en la estructura destino (inp).
 */
int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    //Chequeo casos borde y retorno -1 si los hay
    if (inumber < 1 || inp == NULL || fs == NULL) {
        return -1;
    }

    int INODES_PER_BLOCK = DISKIMG_SECTOR_SIZE / sizeof(struct inode);
    int sector = INODE_START_SECTOR + (inumber - 1) / INODES_PER_BLOCK;
    int offset = (inumber - 1) % INODES_PER_BLOCK;

    struct inode inodes[INODES_PER_BLOCK];

    if (diskimg_readsector(fs->dfd, sector, inodes) < 0) {
        return -1;
    }

    *inp = inodes[offset];

    return 0;
}

/**
 * Traduce un número de bloque lógico de un archivo al número de sector 
 * físico donde se almacena ese bloque de datos.
 * Diferencia entre los archivos grandes (que tienen 7 bloques indirectos y uno doblemente indirecto) 
 * y los normales (hasta 8 bloques directos).
 */
int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp,
    int blockNum) {  
        if (blockNum < 0) {
            return -1;
        }
        
        //Ver si es un archivo grande (large file)
        int es_largo = (inp->i_mode & ILARG);
        
        if (!es_largo) {
            //Caso donde el archivo es normal (no es grande)
            if (blockNum < 8) {
                int direct_block = inp->i_addr[blockNum];
                if (direct_block == 0) {
                    return -1;
                }
                return direct_block;
            }
            // Fuera de rango
            return -1;
        } else {
            //Caso donde el archivo es grande (ILARG): 7 indirectos y uno doblemente indirecto
            if (blockNum < 7 * PUNTEROS_POR_BLOQUE) {  
                //Caso indirecto simple: i_addr[0] a i_addr[6] (7 bloques de 256 punteros)

                int indirect_index = blockNum / PUNTEROS_POR_BLOQUE; 
                int block_offset = blockNum % PUNTEROS_POR_BLOQUE;
                
                if (indirect_index >= 7) {
                    return -1;
                }
                
                int indirect_block = inp->i_addr[indirect_index];
                if (indirect_block == 0) {
                    return -1;
                }
                
                uint16_t block_data[PUNTEROS_POR_BLOQUE];
                if (diskimg_readsector(fs->dfd, indirect_block, block_data) < 0) {
                    return -1;
                }
                
                int result = block_data[block_offset];
                if (result == 0) {
                    return -1;
                }
                
                return result;
            }
            
            //Caso indirecto doble: i_addr[7]
            if (blockNum < 7 * PUNTEROS_POR_BLOQUE + 256 * PUNTEROS_POR_BLOQUE) {
                // 7 bloques indirectos + 1 doblemente indirecto
                int offset = blockNum - (7 * PUNTEROS_POR_BLOQUE);
                int first_index = offset / PUNTEROS_POR_BLOQUE;
                int second_index = offset % PUNTEROS_POR_BLOQUE;
                
                int double_indirect_block = inp->i_addr[7];
                if (double_indirect_block == 0) {
                    return -1;
                }
                
                uint16_t first_level[PUNTEROS_POR_BLOQUE];
                if (diskimg_readsector(fs->dfd, double_indirect_block, first_level) < 0) {
                    return -1;
                }
                
                int second_level_block = first_level[first_index];
                if (second_level_block == 0) {
                    return -1;
                }
                
                uint16_t second_level[PUNTEROS_POR_BLOQUE];;
                if (diskimg_readsector(fs->dfd, second_level_block, second_level) < 0) {
                    return -1;
                }
                
                int result = second_level[second_index];
                if (result == 0) {
                    return -1;
                }
                
                return result;
            }
            
            //Fuera de rango
            return -1;
        }
}

int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1); 
}
