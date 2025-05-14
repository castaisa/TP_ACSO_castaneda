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
        
        // Determinar si es un archivo grande (large file)
        int is_large_file = (inp->i_mode & ILARG);
        
        if (!is_large_file) {
            // Archivo normal: todos los bloques son directos
            if (blockNum < 8) {  // Unix v6 tiene 8 entradas en i_addr para bloques directos
                int direct_block = inp->i_addr[blockNum];
                if (direct_block == 0) {
                    return -1; // bloque no asignado
                }
                return direct_block;
            }
            return -1; // Fuera de rango para archivo normal
        } else {
            // Archivo grande (ILARG): 7 indirectos + 1 doblemente indirecto
            
            // Caso indirecto simple: i_addr[0] a i_addr[6]
            if (blockNum < 7 * 256) {  // 7 bloques indirectos × 256 entradas por bloque
                int indirect_index = blockNum / 256;  // Determina cuál de los 7 bloques indirectos
                int block_offset = blockNum % 256;    // Índice dentro del bloque indirecto
                
                if (indirect_index >= 7) {
                    return -1; // Fuera de rango para los bloques indirectos
                }
                
                int indirect_block = inp->i_addr[indirect_index];
                if (indirect_block == 0) {
                    return -1; // Bloque indirecto no asignado
                }
                
                uint16_t block_data[256];
                if (diskimg_readsector(fs->dfd, indirect_block, block_data) < 0) {
                    return -1;
                }
                
                int result = block_data[block_offset];
                if (result == 0) {
                    return -1; // Bloque no asignado
                }
                
                return result;
            }
            
            // Caso indirecto doble: i_addr[7]
            if (blockNum < 7 * 256 + 256 * 256) {  // 7 bloques indirectos + 1 doblemente indirecto
                int offset = blockNum - (7 * 256);
                int first_index = offset / 256;    // Índice en el primer nivel
                int second_index = offset % 256;   // Índice en el segundo nivel
                
                int double_indirect_block = inp->i_addr[7];
                if (double_indirect_block == 0) {
                    return -1; // Bloque indirecto doble no asignado
                }
                
                uint16_t first_level[256];
                if (diskimg_readsector(fs->dfd, double_indirect_block, first_level) < 0) {
                    return -1;
                }
                
                int second_level_block = first_level[first_index];
                if (second_level_block == 0) {
                    return -1; // Bloque de segundo nivel no asignado
                }
                
                uint16_t second_level[256];
                if (diskimg_readsector(fs->dfd, second_level_block, second_level) < 0) {
                    return -1;
                }
                
                int result = second_level[second_index];
                if (result == 0) {
                    return -1; // Bloque final no asignado
                }
                
                return result;
            }
            
            // Fuera de rango
            return -1;
        }
}

int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1); 
}
