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
    
    // Obtener el inodo del directorio y verificar que es un directorio
    if (inode_iget(fs, dirinumber, &dirInode) < 0 || ((dirInode.i_mode & IFMT) != IFDIR)) {
      return -1;
  }
  
  // Obtener el tamaño del directorio
  int tamano_dir = inode_getsize(&dirInode);
  if (tamano_dir <= 0 || tamano_dir % sizeof(struct direntv6) != 0) {
      return -1;
  }
  
  // Preparar el nombre para comparación (máximo 14 caracteres)
  char copia_name[15]; // 14 caracteres + nulo
  memset(copia_name, 0, sizeof(copia_name));
  strncpy(copia_name, name, 14);
  
  // Procesar el directorio bloque por bloque
  int bytes_procesados = 0;
  int blockNum = 0;
  
  while (bytes_procesados < tamano_dir) {
      char buf[DISKIMG_SECTOR_SIZE];
      int bytesRead = file_getblock(fs, dirinumber, blockNum, buf);
      if (bytesRead <= 0) break;
      
      // Examinar cada entrada en este bloque
      struct direntv6 *entries = (struct direntv6 *)buf;
      int numEntries = bytesRead / sizeof(struct direntv6);
      
      for (int i = 0; i < numEntries; i++) {
          if (entries[i].d_inumber == 0) continue; // Entrada vacía
          
          // Comparar nombres - en Unix v6 no necesariamente terminan en nulo
          char entryName[15];
          memset(entryName, 0, sizeof(entryName));
          memcpy(entryName, entries[i].d_name, 14);
          
          if (strcmp(copia_name, entryName) == 0) {
              *dirEnt = entries[i];
              return 0; // Encontrado
          }
      }
      
      bytes_procesados += bytesRead;
      blockNum++;
  }
  
  return -1; // No encontrado
}
