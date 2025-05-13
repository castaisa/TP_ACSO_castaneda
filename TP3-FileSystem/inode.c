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
        
        // Verificar primero si es un dispositivo especial (no un archivo regular o directorio)
        // Esto es para evitar acceder a bloques inexistentes
        if ((inp->i_mode & IFMT) != IFDIR && (inp->i_mode & IFMT) != IFREG) {
            return -1; // No es un archivo regular o directorio
        }
        
        // Casos directos: i_addr[0] a i_addr[5]
        if (blockNum < 6) {
            return inp->i_addr[blockNum];  // Puede ser 0 para bloques no asignados
        }
        
        blockNum -= 6;  // Ajustar para índices indirectos
        
        // Caso indirecto simple: i_addr[6]
        if (blockNum < 256) {
            uint16_t indirect_block = inp->i_addr[6];
            if (indirect_block == 0) {
                return 0;  // Bloque indirecto no asignado
            }
            
            uint16_t block_data[256];
            if (diskimg_readsector(fs->dfd, indirect_block, block_data) < 0) {
                return -1;
            }
            
            return block_data[blockNum];  // Puede ser 0 para bloques no asignados
        }
        
        blockNum -= 256;  // Ajustar para índices indirectos dobles
        
        // Caso indirecto doble: i_addr[7]
        if (blockNum < 256 * 256) {
            uint16_t double_indirect_block = inp->i_addr[7];
            if (double_indirect_block == 0) {
                return 0;  // Bloque indirecto doble no asignado
            }
            
            int first_index = blockNum / 256;  // Índice en el primer nivel
            int second_index = blockNum % 256; // Índice en el segundo nivel
            
            uint16_t first_level[256];
            if (diskimg_readsector(fs->dfd, double_indirect_block, first_level) < 0) {
                return -1;
            }
            
            uint16_t second_level_block = first_level[first_index];
            if (second_level_block == 0) {
                return 0;  // Bloque de segundo nivel no asignado
            }
            
            uint16_t second_level[256];
            if (diskimg_readsector(fs->dfd, second_level_block, second_level) < 0) {
                return -1;
            }
            
            return second_level[second_index];  // Puede ser 0 para bloques no asignados
        }
        
        // Fuera de rango
        return -1;
}

int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1); 
}
