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
        return -1;
    }
    
    // Verificar que es un directorio
    if ((dirInode.i_mode & IFMT) != IFDIR) {
        return -1;
    }
    
    // Obtener el tamaño del directorio
    int dirSize = inode_getsize(&dirInode);
    if (dirSize <= 0 || dirSize % sizeof(struct direntv6) != 0) {
        return -1; // Tamaño de directorio inválido
    }
    
    // Calcular cuántas entradas de directorio hay en total
    int numEntries = dirSize / sizeof(struct direntv6);
    int entriesPerBlock = DISKIMG_SECTOR_SIZE / sizeof(struct direntv6);
    
    // Número de bloques necesarios para leer todo el directorio
    int numBlocks = (numEntries + entriesPerBlock - 1) / entriesPerBlock;
    
    // Preparar el nombre para comparación
    int nameLen = strlen(name);
    
    // Iterar por cada bloque del directorio
    for (int block = 0; block < numBlocks; block++) {
        char buf[DISKIMG_SECTOR_SIZE];
        
        // Leer un bloque del directorio
        int bytesRead = file_getblock(fs, dirinumber, block, buf);
        if (bytesRead < 0) {
            return -1;
        }
        
        // Determinar cuántas entradas contiene este bloque
        int entriesInBlock = bytesRead / sizeof(struct direntv6);
        struct direntv6 *entries = (struct direntv6 *)buf;
        
        // Examinar cada entrada en el bloque
        for (int i = 0; i < entriesInBlock; i++) {
            // Ignorar entradas con inodo 0 (no asignadas o eliminadas)
            if (entries[i].d_inumber == 0) {
                continue;
            }
            
            // Comparar los nombres
            // En Unix V6, los nombres de archivo son de un máximo de 14 caracteres
            // y pueden no terminar con un carácter nulo
            int match = 1;
            
            // Comparar hasta la longitud del nombre de búsqueda o hasta 14 caracteres
            for (int j = 0; j < nameLen && j < 14; j++) {
                if (name[j] != entries[i].d_name[j]) {
                    match = 0;
                    break;
                }
            }
            
            // Para nombres más cortos que 14, comprobar que el resto son nulos o el nombre termina
            if (match && nameLen < 14) {
                for (int j = nameLen; j < 14; j++) {
                    if (entries[i].d_name[j] != '\0') {
                        match = 0;
                        break;
                    }
                }
            }
            
            if (match) {
                // Encontramos una coincidencia, copiar la entrada al resultado
                *dirEnt = entries[i];
                return 0;
            }
        }
    }
    
    // No se encontró el nombre
    return -1;
}
