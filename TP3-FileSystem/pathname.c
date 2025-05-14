
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
    char *path = strdup(pathname);
    if (path == NULL) {
        return -1; // Error al duplicar la cadena
    }

    char *token = strtok(path, "/");
    int inumber = 1; // Inodo raíz

    while (token != NULL) {
        struct direntv6 dirEnt;
        if (directory_findname(fs, token, inumber, &dirEnt) < 0) {
            free(path);
            return -1; // Error al buscar el nombre en el directorio
        }
        inumber = dirEnt.d_inumber;
        token = strtok(NULL, "/");
    }

    free(path);
    return inumber; // Retorna el número de inodo encontrado
}
