#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/**
 * TODO
 */
int directory_findname(struct unixfilesystem *fs, const char *name,
		int dirinumber, struct direntv6 *dirEnt) {
      struct inode dirInode;
    
    // Obtener el inodo del directorio
    if (inode_iget(fs, dirinumber, &dirInode) < 0) {
        return -1;  // Error al obtener el inodo del directorio
    }
    
    // Verificar que es un directorio
    if ((dirInode.i_mode & IFMT) != IFDIR) {
        return -1;  // No es un directorio
    }
    
    // Obtener el tamaño del directorio
    int dirSize = inode_getsize(&dirInode);
    
    // Verificar que el directorio tenga bloques
    int numBlocks = (dirSize + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE;
    
    // Calcular la longitud del nombre a buscar (máximo 14 caracteres)
    int name_len = strlen(name);
    if (name_len > 14) {
        name_len = 14;  // Limitar a 14 caracteres para Unix V6
    }
    
    for (int blockNum = 0; blockNum < numBlocks; blockNum++) {
        char buf[DISKIMG_SECTOR_SIZE];
        
        // Obtener el bloque correspondiente
        int bytesRead = file_getblock(fs, dirinumber, blockNum, buf);
        if (bytesRead <= 0) {
            continue;  // Error al leer el bloque o bloque vacío
        }
        
        // Procesar las entradas del directorio en el bloque
        struct direntv6 *entries = (struct direntv6 *)buf;
        int entriesInBlock = bytesRead / sizeof(struct direntv6);
        
        for (int i = 0; i < entriesInBlock; i++) {
            // Comparar el nombre con el de la entrada de directorio
            // En Unix V6, los nombres de archivo pueden no estar terminados en nulo
            if (entries[i].d_inumber != 0 && strncmp(entries[i].d_name, name, name_len) == 0 &&
                (name_len == 14 || entries[i].d_name[name_len] == '\0')) {
                // Si encontramos la entrada, copiamos la entrada encontrada
                *dirEnt = entries[i];
                return 0;  // Se encontró el archivo/directorio
            }
        }
    }
    
    // Si no se encontró el nombre en ninguna entrada
    return -1;
}
