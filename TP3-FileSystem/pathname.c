
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
     // Caso base: ruta vacía o raíz
     if (pathname == NULL || pathname[0] == '\0') {
        return -1;
    }

    int inumber = ROOT_INUMBER;  // inodo raíz = 1
    char path[1024];
    strncpy(path, pathname, sizeof(path));
    path[sizeof(path) - 1] = '\0';

    // Si es solo "/", ya está
    if (strcmp(path, "/") == 0) {
        return inumber;
    }

    // Eliminar barra inicial
    char *token = strtok(path, "/");
    while (token != NULL) {
        struct inode in;
        if (inode_iget(fs, inumber, &in) < 0) {
            return -1;
        }
    if ((in.i_mode & IFMT) != IFDIR) {
            return -1; // No es un directorio
    }
    }
    return -1;
}
