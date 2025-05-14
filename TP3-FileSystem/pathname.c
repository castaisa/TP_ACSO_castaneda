
#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/**
 * Busca un archivo usando su ruta (pathname) y devuelve su inumber.
 * Devuelve -1 si hay algun error.
 */
int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
    //Manejo los casos borde
    if (pathname == NULL || fs == NULL || pathname[0] != '/') {
        return -1;
    }
    int inumber = ROOT_INUMBER;
    
    if (strcmp(pathname, "/") == 0) {
        return inumber;
    }
    
    //Copia el path
    char path[strlen(pathname) + 1]; 
    strncpy(path, pathname, sizeof(path));
    path[sizeof(path) - 1] = '\0';
    
    
    char *saveptr;
    char *pedazo_ruta_actual = strtok_r(path, "/", &saveptr);
    
    while (pedazo_ruta_actual != NULL) {
        struct inode in;
        if (inode_iget(fs, inumber, &in) < 0) {
            return -1;
        }
        
        //Se fija que sea un directorio y si no es devuelve -1
        if ((in.i_mode & IFMT) != IFDIR) {
            return -1;
        }
        
        struct direntv6 dirEnt;

      
        if (strlen(pedazo_ruta_actual) > 14) { 
            return -1;//Devuelve -1 porque el nombre es muy largo
        }

        if (directory_findname(fs, pedazo_ruta_actual, inumber, &dirEnt) < 0) {
            return -1;
        }
        
        //Actualiza el inumber
        inumber = dirEnt.d_inumber;
        
        //Busca la siguiente parte del path
        pedazo_ruta_actual = strtok_r(NULL, "/", &saveptr);
    }
    
    return inumber;
}
