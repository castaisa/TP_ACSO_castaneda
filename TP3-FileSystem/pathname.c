
#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/**
 * TODO
 */
int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
    if (pathname == NULL || pathname[0] == '\0') {
        return -1;
    }
    
    int inumber = ROOT_INUMBER;  // inodo raíz = 1
    
    // Si es solo "/", ya está
    if (strcmp(pathname, "/") == 0) {
        return inumber;
    }
    
    // Copiar la ruta para tokenizarla
    char path[1024];
    strncpy(path, pathname, sizeof(path));
    path[sizeof(path) - 1] = '\0';
    
    // Procesar tokens de la ruta (segmentos separados por "/")
    char *saveptr;
    char *token = strtok_r(path, "/", &saveptr);
    
    while (token != NULL) {
        struct inode in;
        if (inode_iget(fs, inumber, &in) < 0) {
            return -1;
        }
        
        // Verificar que el inodo actual sea un directorio
        if ((in.i_mode & IFMT) != IFDIR) {
            return -1;  // No es un directorio
        }
        
        // Buscar el nombre del token en el directorio actual
        struct direntv6 dirEnt;
        if (directory_findname(fs, token, inumber, &dirEnt) < 0) {
            return -1;  // No se encontró el nombre
        }
        
        // Actualizar el inumber para el siguiente nivel
        inumber = dirEnt.d_inumber;
        
        // Obtener el siguiente token
        token = strtok_r(NULL, "/", &saveptr);
    }
    
    return inumber;
}
