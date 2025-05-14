#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>


/**
 *Busca una entrada en un directorio espesifico fijandose que coincida con 
 * el nombre que le pasan como parametro.
 * Devuelve 0 si la entrada se copio correctamente en dirEnt
 * y -1 si hubo algun error.
 */
int directory_findname(struct unixfilesystem *fs, const char *name,
		int dirinumber, struct direntv6 *dirEnt) {
    struct inode dirInode;
    
    //Obtiene el inode del directorio y se fija que sea un directorio
    if (inode_iget(fs, dirinumber, &dirInode) < 0 || ((dirInode.i_mode & IFMT) != IFDIR)) {
      return -1;
  }
  
  int tamano_dir = inode_getsize(&dirInode);
  if (tamano_dir <= 0 || tamano_dir % sizeof(struct direntv6) != 0) {
      return -1;
  }
  
  //Prepara el nombre para ser comparado
  char copia_name[15]; // 14 caracteres + nulo
  memset(copia_name, 0, sizeof(copia_name));
  strncpy(copia_name, name, 14);
  
  //Itera por los bloques del directorio
  int bytes_procesados = 0;
  int blockNum = 0;
  
  while (bytes_procesados < tamano_dir) {
      char buf[DISKIMG_SECTOR_SIZE];
      int bytesRead = file_getblock(fs, dirinumber, blockNum, buf);
      if (bytesRead <= 0) break;
      
      struct direntv6 *entries = (struct direntv6 *)buf;
      int numEntries = bytesRead / sizeof(struct direntv6);
      
      
      for (int i = 0; i < numEntries; i++) {
          if (entries[i].d_inumber == 0) continue;
          
          //Compara los nombres
          char entryName[15];
          memset(entryName, 0, sizeof(entryName));
          memcpy(entryName, entries[i].d_name, 14);
          
          if (strcmp(copia_name, entryName) == 0) {
              *dirEnt = entries[i];
              //Lo encontro
              return 0;
          }
      }
      
      bytes_procesados += bytesRead;
      blockNum++;
  }
  
  return -1;//No lo encontro
}
