#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;

// Array de bytes donde se van a volcar los datos
byte bytesArray[8192];

typedef struct memoria {
  int posicion;
  byte * bytesArray;
} memoria;

void getNombre(char *arg, char **output) {
    char *token;
    int len = 0;
    token = strtok(arg, ".");
    
    while (token[len] != '\0') {
        len++;
    }

    strcat(*output, token);
    strcat(*output, ".exe");
}

int main(int argc, char *argv[]) {
    char *fileName;
    getNombre(argv[0], &fileName);
    printf("%s", fileName);
    return 0;
}
