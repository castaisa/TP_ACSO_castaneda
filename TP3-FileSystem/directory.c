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
    
      // Obtener el tamaño del directorio
      int dirSize = inode_getsize(&dirInode);
      
      // Verificar que el directorio tenga bloques
      int numBlocks = (dirSize + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE;  // Número de bloques a leer
      
      for (int blockNum = 0; blockNum < numBlocks; blockNum++) {
        char buf[DISKIMG_SECTOR_SIZE];
        
        // Obtener el bloque correspondiente
        int bytesRead = file_getblock(fs, dirinumber, blockNum, buf);
        if (bytesRead < 0) {
          return -1;  // Error al leer el bloque
        }
    
        // Procesar las entradas del directorio en el bloque
        int entriesInBlock = bytesRead / sizeof(struct direntv6);  // Asumimos que cada entrada tiene el tamaño de struct direntv6
        for (int i = 0; i < entriesInBlock; i++) {
          struct direntv6 *entry = (struct direntv6 *)(buf + i * sizeof(struct direntv6));
    
          // Comparar el nombre con el de la entrada de directorio
          if (strcmp(entry->d_name, name) == 0) {
            // Si encontramos la entrada, copiamos la entrada encontrada
            *dirEnt = *entry;
            return 0;  // Se encontró el archivo/directorio
          }
        }
      }
    
      // Si no se encontró el nombre en ninguna entrada
      return -1;
}
