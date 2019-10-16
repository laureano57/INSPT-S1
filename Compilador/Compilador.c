// ####################################################################################################################
// ####################################  INSPT - Sistemas de computacion I - 2019  ####################################
// #################################### Compilador PL/0 - Gonzalez, Olmedo, Stella ####################################
// ####################################################################################################################

// Librerias
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ####################################################################################################################
// #################################################### Constantes ####################################################
// ####################################################################################################################

// Codigo especifico de Win/Linux
#if defined(_WIN32) || defined(_WIN64)
  #define OS_NAME "windows"
  #define PRINT_STRING 0x3E0
  #define NEW_LINE 0x410
  #define TEXT_START 0x200
  #define VAR_LENGTH_START 0x700
  #define PRINT_EAX 0x420
  #define READ_EAX 0x590
  #define END_PROGRAM 0x588
#elif defined(__linux__)
  #define OS_NAME "linux"
  #define PRINT_STRING 0x170
  #define NEW_LINE 0x180
  #define TEXT_START 0xE0
  #define VAR_LENGTH_START 0x480
  #define PRINT_EAX 0x190
  #define READ_EAX 0x310
  #define END_PROGRAM 0x300
#else
  #define OS_NAME "other"
#endif

// Longitud neta, sin los apóstrofos ni el cero final
#define MAX_LONGITUD_CADENA 128

// Longitud de la tabla de identificadores
#define MAX_CANT_IDENT 1024

// ####################################################################################################################
// ################################################## Tipos de datos ##################################################
// ####################################################################################################################

// Byte de chars sin signo para que use el rango de 0 a 255 (en vez de usar el rango -127 - 126)
typedef unsigned char byte;

typedef struct memStruct {
  int topeMemoria;
  byte * bytesArray;
} memStruct;

typedef enum {
  ABREPAREN,
  ASIGNACION,
  BEGIN,
  CADENA,
  CALL,
  CIERRAPAREN,
  COMA,
  CONST,
  DISTINTO,
  DIVIDIDO,
  DO,
  ELSE,
  END,
  FIN_DE_ARCHIVO,
  FOR,
  HALT,
  IDENT,
  IF,
  IGUAL,
  MAS,
  MAYOR,
  MAYORIGUAL,
  MENOR,
  MENORIGUAL,
  MENOS,
  NOT,
  NULO,
  NUMERO,
  ODD,
  POR,
  PROCEDURE,
  PTOCOMA,
  PUNTO,
  READLN,
  THEN,
  VAR,
  WHILE,
  WRITE,
  WRITELN,
} tTerminal;

typedef struct {
  tTerminal simbolo;
  char cadena[MAX_LONGITUD_CADENA + 3]; // Mas los apostrofos y el cero final
} tSimbolo;

typedef struct {
  tTerminal tipo;
  char nombre[MAX_LONGITUD_CADENA + 3];
  int valor;
} tEntradaTabla;

typedef tEntradaTabla tablaDeIdent[MAX_CANT_IDENT];

// ####################################################################################################################
// #################################################### Signatures ####################################################
// ####################################################################################################################

// ##################################################### Helpers ######################################################

void imprimir(tSimbolo);

void concatenar(char *, char);

void uppercase(char *);

void error(int, tSimbolo);

int buscarIdent(char *, tablaDeIdent, int, int);

void cargarByte(memStruct *, byte);

void cargarInt(memStruct *, int);

void cargarByteEn(memStruct *, byte, int);

void cargarIntEn(memStruct *, int, int);

int leerIntDe(memStruct *, int);

void cargarHeader(memStruct *);

void codigoFinal(memStruct *, int);

void writeString(memStruct *, tSimbolo);

void dumpToFile(memStruct *, char *);

void getOutputFilename(char *, char *);

// ########################################## Analizador lexico y sintactico ##########################################

tSimbolo aLex(FILE *);
tSimbolo programa(tSimbolo, FILE *archivo, memStruct *);
tSimbolo bloque(tSimbolo, FILE *archivo, memStruct *, tablaDeIdent, int *desplVarMem, int base);
tSimbolo proposicion(tSimbolo, FILE *archivo, memStruct *, tablaDeIdent, int posUltimoIdent, int *desplVarMem);
tSimbolo condicion(tSimbolo, FILE *archivo, memStruct *, tablaDeIdent, int posUltimoIdent);
tSimbolo expresion(tSimbolo, FILE *archivo, memStruct *, tablaDeIdent, int posUltimoIdent);
tSimbolo termino(tSimbolo, FILE *archivo, memStruct *, tablaDeIdent, int posUltimoIdent);
tSimbolo factor(tSimbolo, FILE *archivo, memStruct *, tablaDeIdent, int posUltimoIdent);

// ####################################################################################################################
// ####################################################### MAIN #######################################################
// ####################################################################################################################

int main(int argc, char *argv[]) {
  // Si el OS no es win32/win64 o linux, sale
  if (strcmp(OS_NAME, "other") == 0) {
    printf("Sistema operativo no soportado, saliendo...");
    return 0;
  }
  // Array de bytes donde se van a ir volcando los datos
  byte bytesArray[8192];

  // Puntero al nombre del compilado de salida
  char outputFilename[80];

  // Inicializo memoria para empezar a guardar bytes en bytesArray (array de salida)
  memStruct *memoria = malloc(sizeof(memStruct));
  memoria->topeMemoria = 0;
  memoria->bytesArray = bytesArray;

  // Cargo el header y los bytes de entrada y salida al array de bytes de memoria
  cargarHeader(memoria);

  if (argc != 2) {
      printf("No se ingreso ningun archivo por parametro, saliendo...\n");
  } else {
    FILE *archivo;
    archivo = fopen(argv[1], "r");

    if (archivo == NULL) {
      printf("Error de apertura del archivo [%s].\n", argv[1]);
    } else {
      tSimbolo s;
      s = aLex(archivo);
      s = programa(s, archivo, memoria);
      if (s.simbolo != FIN_DE_ARCHIVO) error(0, s);

      printf("Archivo procesado exitosamente\n");

      // Vuelca array de memoria en archivo
      getOutputFilename(argv[1], outputFilename);

      printf("\nArchivo generado: %s\n", outputFilename);
      dumpToFile(memoria, outputFilename);
    }
  }
  return 0;
}

// ####################################################################################################################
// ##################################################### Helpers ######################################################
// ####################################################################################################################

void imprimir(tSimbolo simb) {
  printf("Cadena leida: \"%s\"\t Simbolo correspondiente: ", simb.cadena);
  switch (simb.simbolo) {
  case ABREPAREN:
    printf("ABREPAREN");
    break;
  case ASIGNACION:
    printf("ASIGNACION");
    break;
  case BEGIN:
    printf("BEGIN");
    break;
  case CADENA:
    printf("CADENA");
    break;
  case CALL:
    printf("CALL");
    break;
  case CIERRAPAREN:
    printf("CIERRAPAREN");
    break;
  case COMA:
    printf("COMA");
    break;
  case CONST:
    printf("CONST");
    break;
  case DISTINTO:
    printf("DISTINTO");
    break;
  case DIVIDIDO:
    printf("DIVIDIDO");
    break;
  case DO:
    printf("DO");
    break;
  case END:
    printf("END");
    break;
  case ELSE:
    printf("ELSE");
    break;
  case FIN_DE_ARCHIVO:
    printf("FIN_DE_ARCHIVO");
    break;
  case FOR:
    printf("FOR");
    break;
  case HALT:
    printf("HALT");
    break;
  case IDENT:
    printf("IDENT");
    break;
  case IF:
    printf("IF");
    break;
  case IGUAL:
    printf("IGUAL");
    break;
  case MAS:
    printf("MAS");
    break;
  case MAYOR:
    printf("MAYOR");
    break;
  case MAYORIGUAL:
    printf("MAYORIGUAL");
    break;
  case MENOR:
    printf("MENOR");
    break;
  case MENORIGUAL:
    printf("MENORIGUAL");
    break;
  case MENOS:
    printf("MENOS");
    break;
  case NOT:
    printf("NOT");
    break;
  case NULO:
    printf("NULO");
    break;
  case NUMERO:
    printf("NUMERO");
    break;
  case ODD:
    printf("ODD");
    break;
  case POR:
    printf("POR");
    break;
  case PROCEDURE:
    printf("PROCEDURE");
    break;
  case PTOCOMA:
    printf("PTOCOMA");
    break;
  case PUNTO:
    printf("PUNTO");
    break;
  case READLN:
    printf("READLN");
    break;
  case THEN:
    printf("THEN");
    break;
  case VAR:
    printf("VAR");
    break;
  case WHILE:
    printf("WHILE");
    break;
  case WRITE:
    printf("WRITE");
    break;
  case WRITELN:
    printf("WRITELN");
    break;
  }
  printf("\n");
}

void concatenar(char *s, char c) {
  if (strlen(s) < MAX_LONGITUD_CADENA + 2) {
    for (; *s; s++);
    *s++ = c;
    *s++ = '\0';
  }
}

void uppercase(char *s) {
  for (; *s; s++)
    *s = toupper(*s);
}

void error(int codigo, tSimbolo s) {
  switch (codigo) {
  case 0:
    printf("Error al leer final de archivo: %s\n", s.cadena);
    break;
  case 1:
    printf("Error, se esperaba un PUNTO: %s\n", s.cadena);
    break;
  case 2:
    printf("Error, se esperaba un IDENT: %s\n", s.cadena);
    break;
  case 3:
    printf("Error, se esperaba un IGUAL: %s\n", s.cadena);
    break;
  case 4:
    printf("Error, se esperaba un NUMERO: %s\n", s.cadena);
    break;
  case 5:
    printf("Error, se esperaba un PTOCOMA: %s\n", s.cadena);
    break;
  case 6:
    printf("Error, se esperaba una ASIGNACION: %s\n", s.cadena);
    break;
  case 7:
    printf("Error, se esperaba un END: %s\n", s.cadena);
    break;
  case 8:
    printf("Error, se esperaba un THEN: %s\n", s.cadena);
    break;
  case 9:
    printf("Error, se esperaba un DO: %s\n", s.cadena);
    break;
  case 10:
    printf("Error, se esperaba un ABREPAREN: %s\n", s.cadena);
    break;
  case 11:
    printf("Error, se esperaba un CIERRAPAREN: %s\n", s.cadena);
    break;
  case 12:
    printf("Error, se esperaba un operador relacional: %s\n", s.cadena);
    break;
  case 13:
    printf("Error semantico, se repite el identificador: %s\n", s.cadena);
    break;
  case 14:
    printf("Error semantico, se esperaba un VAR: %s\n", s.cadena);
    break;
  case 15:
    printf("Error semantico, el identificador %s no esta declarado.\n", s.cadena);
    break;
  case 16:
    printf("Error semantico, el identificador %s no es de tipo PROCEDURE\n", s.cadena);
    break;
  case 17:
    printf("Error semantico, el identificador \"%s\" no es de tipo VAR\n", s.cadena);
    break;
  case 18:
    printf("Error semantico, el identificador \"%s\" no es de tipo NUMERO\n", s.cadena);
    break;
  default:
    printf("Error generico\n");
    break;
  }
  exit(1);
}

// Busca identificador en la tabla de identificadores y devuelve la posicion o -1
int buscarIdent(char *id, tablaDeIdent tabla, int posPrimerIdent, int posUltimoIdent) {
  char cadenaAux[MAX_LONGITUD_CADENA + 3];
  strcpy(cadenaAux, id);
  uppercase(cadenaAux);

  // SIEMPRE lee de abajo para arriba, para encontrar la ultima definicion de la variable o procedimiento
  int i = posUltimoIdent;
  while (i >= posPrimerIdent && strcmp(cadenaAux, tabla[i].nombre) != 0) i--;
  return (i >= posPrimerIdent ? i : -1);
}

// Carga un byte en el array de memoria e incrementa la posicion del indice
void cargarByte(memStruct *memoria, byte dato) {
  int i = memoria->topeMemoria;
  memoria->bytesArray[i] = dato;
  memoria->topeMemoria += 1;
}

// Carga un int (4 bytes) en el array de memoria e incrementa la posicion del indice
void cargarInt(memStruct *memoria, int dato) {
  int i = memoria->topeMemoria;
  unsigned int k = 4294967295; // -1 en 4 bytes (FF FF FF FF)
  unsigned int d;

  if (dato < 0) {
    d = k + dato + 1;
  } else {
    d = (unsigned int) dato;
  }
  // Como la PC es little endian, se insertan al reves, de a pares
  cargarByte(memoria, d % 0x100); // Ultimos 2 digitos
  cargarByte(memoria, d / 0x100 % 0x100); // los 2 siguientes
  cargarByte(memoria, d / 0x10000 % 0x100); // los 2 siguientes
  cargarByte(memoria, d / 0x1000000 % 0x100); // los 2 siguientes
}

// Carga un byte en el array de memoria en la posicion dada
void cargarByteEn(memStruct *memoria, byte dato, int pos) {
  memoria->bytesArray[pos] = dato;
}

// Carga un int (4 bytes) en el array de memoria en la posicion dada
void cargarIntEn(memStruct *memoria, int dato, int pos) {
  unsigned int k = 4294967295; // -1 en 4 bytes (FF FF FF FF)
  unsigned int d;

  if (dato < 0) {
    d = k + dato + 1;
  } else {
    d = (unsigned int) dato;
  }

  // Como la PC es little endian, se insertan al reves, de a pares
  cargarByteEn(memoria, d % 0x100, pos); // Ultimos 2 digitos
  cargarByteEn(memoria, d / 0x100 % 0x100, pos + 1); // los 2 siguientes
  cargarByteEn(memoria, d / 0x10000 % 0x100, pos + 2); // los 2 siguientes
  cargarByteEn(memoria, d / 0x1000000 % 0x100, pos + 3); // los 2 siguientes
}

// Devuelve un int de una posicion de memoria determinada
int leerIntDe(memStruct *memoria, int pos) {
  return  memoria->bytesArray[pos] +
          memoria->bytesArray[pos+1] * 256 +
          memoria->bytesArray[pos+2] * 256 * 256 +
          memoria->bytesArray[pos+3] * 256 * 256 * 256;
}

// POP EAX con optimizacion de codigo (elimina un PUSH EAX previo para evitar redundancia)
void cargarPopEax(memStruct *memoria) {
  int i = memoria->topeMemoria;
  if (memoria->bytesArray[i - 1] == 0x50) {
    memoria->topeMemoria -= 1;
  } else {
    cargarByte(memoria, 0x58);
  }
}

// Carga los bytes del header al array de bytes segun sistema operativo
void cargarHeader(memStruct *memoria) {
  if (strcmp(OS_NAME, "windows")  == 0) {
    // Header de bytes para windows PE32
    byte header[] = {0x4D, 0x5A, 0x60, 0x01, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x60, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00, 0x00, 0x0E, 0x1F, 0xBA, 0x0E, 0x00, 0xB4, 0x09, 0xCD, 0x21, 0xB8, 0x01, 0x4C, 0xCD, 0x21, 0x54, 0x68, 0x69, 0x73, 0x20, 0x70, 0x72, 0x6F, 0x67, 0x72, 0x61, 0x6D, 0x20, 0x69, 0x73, 0x20, 0x61, 0x20, 0x57, 0x69, 0x6E, 0x33, 0x32, 0x20, 0x63, 0x6F, 0x6E, 0x73, 0x6F, 0x6C, 0x65, 0x20, 0x61, 0x70, 0x70, 0x6C, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x2E, 0x20, 0x49, 0x74, 0x20, 0x63, 0x61, 0x6E, 0x6E, 0x6F, 0x74, 0x20, 0x62, 0x65, 0x20, 0x72, 0x75, 0x6E, 0x20, 0x75, 0x6E, 0x64, 0x65, 0x72, 0x20, 0x4D, 0x53, 0x2D, 0x44, 0x4F, 0x53, 0x2E, 0x0D, 0x0A, 0x24, 0x00, 0x00, 0x00, 0x00, 0x50, 0x45, 0x00, 0x00, 0x4C, 0x01, 0x01, 0x00, 0x00, 0x00, 0x53, 0x4C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x02, 0x01, 0x0B, 0x01, 0x01, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x10, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2E, 0x74, 0x65, 0x78, 0x74, 0x00, 0x00, 0x00, 0x0C, 0x06, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6E, 0x10, 0x00, 0x00, 0x7C, 0x10, 0x00, 0x00, 0x8C, 0x10, 0x00, 0x00, 0x98, 0x10, 0x00, 0x00, 0xA4, 0x10, 0x00, 0x00, 0xB6, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4B, 0x45, 0x52, 0x4E, 0x45, 0x4C, 0x33, 0x32, 0x2E, 0x64, 0x6C, 0x6C, 0x00, 0x00, 0x6E, 0x10, 0x00, 0x00, 0x7C, 0x10, 0x00, 0x00, 0x8C, 0x10, 0x00, 0x00, 0x98, 0x10, 0x00, 0x00, 0xA4, 0x10, 0x00, 0x00, 0xB6, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x45, 0x78, 0x69, 0x74, 0x50, 0x72, 0x6F, 0x63, 0x65, 0x73, 0x73, 0x00, 0x00, 0x00, 0x47, 0x65, 0x74, 0x53, 0x74, 0x64, 0x48, 0x61, 0x6E, 0x64, 0x6C, 0x65, 0x00, 0x00, 0x00, 0x00, 0x52, 0x65, 0x61, 0x64, 0x46, 0x69, 0x6C, 0x65, 0x00, 0x00, 0x00, 0x00, 0x57, 0x72, 0x69, 0x74, 0x65, 0x46, 0x69, 0x6C, 0x65, 0x00, 0x00, 0x00, 0x47, 0x65, 0x74, 0x43, 0x6F, 0x6E, 0x73, 0x6F, 0x6C, 0x65, 0x4D, 0x6F, 0x64, 0x65, 0x00, 0x00, 0x00, 0x00, 0x53, 0x65, 0x74, 0x43, 0x6F, 0x6E, 0x73, 0x6F, 0x6C, 0x65, 0x4D, 0x6F, 0x64, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0xA2, 0x1C, 0x11, 0x40, 0x00, 0x31, 0xC0, 0x03, 0x05, 0x2C, 0x11, 0x40, 0x00, 0x75, 0x0D, 0x6A, 0xF5, 0xFF, 0x15, 0x04, 0x10, 0x40, 0x00, 0xA3, 0x2C, 0x11, 0x40, 0x00, 0x6A, 0x00, 0x68, 0x30, 0x11, 0x40, 0x00, 0x6A, 0x01, 0x68, 0x1C, 0x11, 0x40, 0x00, 0x50, 0xFF, 0x15, 0x0C, 0x10, 0x40, 0x00, 0x09, 0xC0, 0x75, 0x08, 0x6A, 0x00, 0xFF, 0x15, 0x00, 0x10, 0x40, 0x00, 0x81, 0x3D, 0x30, 0x11, 0x40, 0x00, 0x01, 0x00, 0x00, 0x00, 0x75, 0xEC, 0x58, 0xC3, 0x00, 0x57, 0x72, 0x69, 0x74, 0x65, 0x20, 0x65, 0x72, 0x72, 0x6F, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x31, 0xC0, 0x03, 0x05, 0xCC, 0x11, 0x40, 0x00, 0x75, 0x37, 0x6A, 0xF6, 0xFF, 0x15, 0x04, 0x10, 0x40, 0x00, 0xA3, 0xCC, 0x11, 0x40, 0x00, 0x68, 0xD0, 0x11, 0x40, 0x00, 0x50, 0xFF, 0x15, 0x10, 0x10, 0x40, 0x00, 0x80, 0x25, 0xD0, 0x11, 0x40, 0x00, 0xF9, 0xFF, 0x35, 0xD0, 0x11, 0x40, 0x00, 0xFF, 0x35, 0xCC, 0x11, 0x40, 0x00, 0xFF, 0x15, 0x14, 0x10, 0x40, 0x00, 0xA1, 0xCC, 0x11, 0x40, 0x00, 0x6A, 0x00, 0x68, 0xD4, 0x11, 0x40, 0x00, 0x6A, 0x01, 0x68, 0xBE, 0x11, 0x40, 0x00, 0x50, 0xFF, 0x15, 0x08, 0x10, 0x40, 0x00, 0x09, 0xC0, 0x61, 0x90, 0x75, 0x08, 0x6A, 0x00, 0xFF, 0x15, 0x00, 0x10, 0x40, 0x00, 0x0F, 0xB6, 0x05, 0xBE, 0x11, 0x40, 0x00, 0x81, 0x3D, 0xD4, 0x11, 0x40, 0x00, 0x01, 0x00, 0x00, 0x00, 0x74, 0x05, 0xB8, 0xFF, 0xFF, 0xFF, 0xFF, 0xC3, 0x00, 0x52, 0x65, 0x61, 0x64, 0x20, 0x65, 0x72, 0x72, 0x6F, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x89, 0xC6, 0x30, 0xC0, 0x02, 0x06, 0x74, 0x08, 0x46, 0xE8, 0xE1, 0xFE, 0xFF, 0xFF, 0xEB, 0xF2, 0x61, 0x90, 0xC3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x30, 0xE8, 0xC9, 0xFE, 0xFF, 0xFF, 0xC3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB0, 0x0D, 0xE8, 0xB9, 0xFE, 0xFF, 0xFF, 0xB0, 0x0A, 0xE8, 0xB2, 0xFE, 0xFF, 0xFF, 0xC3, 0x00, 0x3D, 0x00, 0x00, 0x00, 0x80, 0x75, 0x4E, 0xB0, 0x2D, 0xE8, 0xA2, 0xFE, 0xFF, 0xFF, 0xB0, 0x02, 0xE8, 0xCB, 0xFF, 0xFF, 0xFF, 0xB0, 0x01, 0xE8, 0xC4, 0xFF, 0xFF, 0xFF, 0xB0, 0x04, 0xE8, 0xBD, 0xFF, 0xFF, 0xFF, 0xB0, 0x07, 0xE8, 0xB6, 0xFF, 0xFF, 0xFF, 0xB0, 0x04, 0xE8, 0xAF, 0xFF, 0xFF, 0xFF, 0xB0, 0x08, 0xE8, 0xA8, 0xFF, 0xFF, 0xFF, 0xB0, 0x03, 0xE8, 0xA1, 0xFF, 0xFF, 0xFF, 0xB0, 0x06, 0xE8, 0x9A, 0xFF, 0xFF, 0xFF, 0xB0, 0x04, 0xE8, 0x93, 0xFF, 0xFF, 0xFF, 0xB0, 0x08, 0xE8, 0x8C, 0xFF, 0xFF, 0xFF, 0xC3, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x7D, 0x0B, 0x50, 0xB0, 0x2D, 0xE8, 0x4C, 0xFE, 0xFF, 0xFF, 0x58, 0xF7, 0xD8, 0x3D, 0x0A, 0x00, 0x00, 0x00, 0x0F, 0x8C, 0xEF, 0x00, 0x00, 0x00, 0x3D, 0x64, 0x00, 0x00, 0x00, 0x0F, 0x8C, 0xD1, 0x00, 0x00, 0x00, 0x3D, 0xE8, 0x03, 0x00, 0x00, 0x0F, 0x8C, 0xB3, 0x00, 0x00, 0x00, 0x3D, 0x10, 0x27, 0x00, 0x00, 0x0F, 0x8C, 0x95, 0x00, 0x00, 0x00, 0x3D, 0xA0, 0x86, 0x01, 0x00, 0x7C, 0x7B, 0x3D, 0x40, 0x42, 0x0F, 0x00, 0x7C, 0x61, 0x3D, 0x80, 0x96, 0x98, 0x00, 0x7C, 0x47, 0x3D, 0x00, 0xE1, 0xF5, 0x05, 0x7C, 0x2D, 0x3D, 0x00, 0xCA, 0x9A, 0x3B, 0x7C, 0x13, 0xBA, 0x00, 0x00, 0x00, 0x00, 0xBB, 0x00, 0xCA, 0x9A, 0x3B, 0xF7, 0xFB, 0x52, 0xE8, 0x18, 0xFF, 0xFF, 0xFF, 0x58, 0xBA, 0x00, 0x00, 0x00, 0x00, 0xBB, 0x00, 0xE1, 0xF5, 0x05, 0xF7, 0xFB, 0x52, 0xE8, 0x05, 0xFF, 0xFF, 0xFF, 0x58, 0xBA, 0x00, 0x00, 0x00, 0x00, 0xBB, 0x80, 0x96, 0x98, 0x00, 0xF7, 0xFB, 0x52, 0xE8, 0xF2, 0xFE, 0xFF, 0xFF, 0x58, 0xBA, 0x00, 0x00, 0x00, 0x00, 0xBB, 0x40, 0x42, 0x0F, 0x00, 0xF7, 0xFB, 0x52, 0xE8, 0xDF, 0xFE, 0xFF, 0xFF, 0x58, 0xBA, 0x00, 0x00, 0x00, 0x00, 0xBB, 0xA0, 0x86, 0x01, 0x00, 0xF7, 0xFB, 0x52, 0xE8, 0xCC, 0xFE, 0xFF, 0xFF, 0x58, 0xBA, 0x00, 0x00, 0x00, 0x00, 0xBB, 0x10, 0x27, 0x00, 0x00, 0xF7, 0xFB, 0x52, 0xE8, 0xB9, 0xFE, 0xFF, 0xFF, 0x58, 0xBA, 0x00, 0x00, 0x00, 0x00, 0xBB, 0xE8, 0x03, 0x00, 0x00, 0xF7, 0xFB, 0x52, 0xE8, 0xA6, 0xFE, 0xFF, 0xFF, 0x58, 0xBA, 0x00, 0x00, 0x00, 0x00, 0xBB, 0x64, 0x00, 0x00, 0x00, 0xF7, 0xFB, 0x52, 0xE8, 0x93, 0xFE, 0xFF, 0xFF, 0x58, 0xBA, 0x00, 0x00, 0x00, 0x00, 0xBB, 0x0A, 0x00, 0x00, 0x00, 0xF7, 0xFB, 0x52, 0xE8, 0x80, 0xFE, 0xFF, 0xFF, 0x58, 0xE8, 0x7A, 0xFE, 0xFF, 0xFF, 0xC3, 0x00, 0xFF, 0x15, 0x00, 0x10, 0x40, 0x00, 0x00, 0x00, 0xB9, 0x00, 0x00, 0x00, 0x00, 0xB3, 0x03, 0x51, 0x53, 0xE8, 0xA2, 0xFD, 0xFF, 0xFF, 0x5B, 0x59, 0x3C, 0x0D, 0x0F, 0x84, 0x34, 0x01, 0x00, 0x00, 0x3C, 0x08, 0x0F, 0x84, 0x94, 0x00, 0x00, 0x00, 0x3C, 0x2D, 0x0F, 0x84, 0x09, 0x01, 0x00, 0x00, 0x3C, 0x30, 0x7C, 0xDB, 0x3C, 0x39, 0x7F, 0xD7, 0x2C, 0x30, 0x80, 0xFB, 0x00, 0x74, 0xD0, 0x80, 0xFB, 0x02, 0x75, 0x0C, 0x81, 0xF9, 0x00, 0x00, 0x00, 0x00, 0x75, 0x04, 0x3C, 0x00, 0x74, 0xBF, 0x80, 0xFB, 0x03, 0x75, 0x0A, 0x3C, 0x00, 0x75, 0x04, 0xB3, 0x00, 0xEB, 0x02, 0xB3, 0x01, 0x81, 0xF9, 0xCC, 0xCC, 0xCC, 0x0C, 0x7F, 0xA8, 0x81, 0xF9, 0x34, 0x33, 0x33, 0xF3, 0x7C, 0xA0, 0x88, 0xC7, 0xB8, 0x0A, 0x00, 0x00, 0x00, 0xF7, 0xE9, 0x3D, 0x08, 0x00, 0x00, 0x80, 0x74, 0x11, 0x3D, 0xF8, 0xFF, 0xFF, 0x7F, 0x75, 0x13, 0x80, 0xFF, 0x07, 0x7E, 0x0E, 0xE9, 0x7F, 0xFF, 0xFF, 0xFF, 0x80, 0xFF, 0x08, 0x0F, 0x8F, 0x76, 0xFF, 0xFF, 0xFF, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x88, 0xF9, 0x80, 0xFB, 0x02, 0x74, 0x04, 0x01, 0xC1, 0xEB, 0x03, 0x29, 0xC8, 0x91, 0x88, 0xF8, 0x51, 0x53, 0xE8, 0xC3, 0xFD, 0xFF, 0xFF, 0x5B, 0x59, 0xE9, 0x53, 0xFF, 0xFF, 0xFF, 0x80, 0xFB, 0x03, 0x0F, 0x84, 0x4A, 0xFF, 0xFF, 0xFF, 0x51, 0x53, 0xB0, 0x08, 0xE8, 0x7A, 0xFC, 0xFF, 0xFF, 0xB0, 0x20, 0xE8, 0x73, 0xFC, 0xFF, 0xFF, 0xB0, 0x08, 0xE8, 0x6C, 0xFC, 0xFF, 0xFF, 0x5B, 0x59, 0x80, 0xFB, 0x00, 0x75, 0x07, 0xB3, 0x03, 0xE9, 0x25, 0xFF, 0xFF, 0xFF, 0x80, 0xFB, 0x02, 0x75, 0x0F, 0x81, 0xF9, 0x00, 0x00, 0x00, 0x00, 0x75, 0x07, 0xB3, 0x03, 0xE9, 0x11, 0xFF, 0xFF, 0xFF, 0x89, 0xC8, 0xB9, 0x0A, 0x00, 0x00, 0x00, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x7D, 0x08, 0xF7, 0xD8, 0xF7, 0xF9, 0xF7, 0xD8, 0xEB, 0x02, 0xF7, 0xF9, 0x89, 0xC1, 0x81, 0xF9, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x85, 0xE6, 0xFE, 0xFF, 0xFF, 0x80, 0xFB, 0x02, 0x0F, 0x84, 0xDD, 0xFE, 0xFF, 0xFF, 0xB3, 0x03, 0xE9, 0xD6, 0xFE, 0xFF, 0xFF, 0x80, 0xFB, 0x03, 0x0F, 0x85, 0xCD, 0xFE, 0xFF, 0xFF, 0xB0, 0x2D, 0x51, 0x53, 0xE8, 0xFD, 0xFB, 0xFF, 0xFF, 0x5B, 0x59, 0xB3, 0x02, 0xE9, 0xBB, 0xFE, 0xFF, 0xFF, 0x80, 0xFB, 0x03, 0x0F, 0x84, 0xB2, 0xFE, 0xFF, 0xFF, 0x80, 0xFB, 0x02, 0x75, 0x0C, 0x81, 0xF9, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x84, 0xA1, 0xFE, 0xFF, 0xFF, 0x51, 0xE8, 0x14, 0xFD, 0xFF, 0xFF, 0x59, 0x89, 0xC8, 0xC3};
    for (int i = 0; i < sizeof(header); i++) {
      cargarByte(memoria, header[i]);
    }
  } else if (strcmp(OS_NAME, "linux")  == 0) {
    // Header de bytes para linux ELF
    byte header[] = { 0x7f, 0x45, 0x4c, 0x46, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x80, 0x84, 0x04, 0x08, 0x34, 0x00, 0x00, 0x00, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x00, 0x20, 0x00, 0x01, 0x00, 0x28, 0x00, 0x03, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x04, 0x08, 0x00, 0x80, 0x04, 0x08, 0x97, 0x05, 0x00, 0x00, 0x97, 0x05, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x2e, 0x73, 0x68, 0x73, 0x74, 0x72, 0x74, 0x61, 0x62, 0x00, 0x2e, 0x74, 0x65, 0x78, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x54, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0xe0, 0x80, 0x04, 0x08, 0xe0, 0x00, 0x00, 0x00, 0xb7, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x51, 0x53, 0x50, 0xb8, 0x04, 0x00, 0x00, 0x00, 0xbb, 0x01, 0x00, 0x00, 0x00, 0x89, 0xe1, 0xba, 0x01, 0x00, 0x00, 0x00, 0xcd, 0x80, 0x58, 0x5b, 0x59, 0x5a, 0xc3, 0x55, 0x89, 0xe5, 0x81, 0xec, 0x24, 0x00, 0x00, 0x00, 0x52, 0x51, 0x53, 0xb8, 0x36, 0x00, 0x00, 0x00, 0xbb, 0x00, 0x00, 0x00, 0x00, 0xb9, 0x01, 0x54, 0x00, 0x00, 0x8d, 0x55, 0xdc, 0xcd, 0x80, 0x81, 0x65, 0xe8, 0xf5, 0xff, 0xff, 0xff, 0xb8, 0x36, 0x00, 0x00, 0x00, 0xbb, 0x00, 0x00, 0x00, 0x00, 0xb9, 0x02, 0x54, 0x00, 0x00, 0x8d, 0x55, 0xdc, 0xcd, 0x80, 0x31, 0xc0, 0x50, 0xb8, 0x03, 0x00, 0x00, 0x00, 0xbb, 0x00, 0x00, 0x00, 0x00, 0x89, 0xe1, 0xba, 0x01, 0x00, 0x00, 0x00, 0xcd, 0x80, 0x81, 0x4d, 0xe8, 0x0a, 0x00, 0x00, 0x00, 0xb8, 0x36, 0x00, 0x00, 0x00, 0xbb, 0x00, 0x00, 0x00, 0x00, 0xb9, 0x02, 0x54, 0x00, 0x00, 0x8d, 0x55, 0xdc, 0xcd, 0x80, 0x58, 0x5b, 0x59, 0x5a, 0x89, 0xec, 0x5d, 0xc3, 0xb8, 0x04, 0x00, 0x00, 0x00, 0xbb, 0x01, 0x00, 0x00, 0x00, 0xcd, 0x80, 0xc3, 0x90, 0x90, 0x90, 0xb0, 0x0a, 0xe8, 0x59, 0xff, 0xff, 0xff, 0xc3, 0x04, 0x30, 0xe8, 0x51, 0xff, 0xff, 0xff, 0xc3, 0x3d, 0x00, 0x00, 0x00, 0x80, 0x75, 0x4e, 0xb0, 0x2d, 0xe8, 0x42, 0xff, 0xff, 0xff, 0xb0, 0x02, 0xe8, 0xe3, 0xff, 0xff, 0xff, 0xb0, 0x01, 0xe8, 0xdc, 0xff, 0xff, 0xff, 0xb0, 0x04, 0xe8, 0xd5, 0xff, 0xff, 0xff, 0xb0, 0x07, 0xe8, 0xce, 0xff, 0xff, 0xff, 0xb0, 0x04, 0xe8, 0xc7, 0xff, 0xff, 0xff, 0xb0, 0x08, 0xe8, 0xc0, 0xff, 0xff, 0xff, 0xb0, 0x03, 0xe8, 0xb9, 0xff, 0xff, 0xff, 0xb0, 0x06, 0xe8, 0xb2, 0xff, 0xff, 0xff, 0xb0, 0x04, 0xe8, 0xab, 0xff, 0xff, 0xff, 0xb0, 0x08, 0xe8, 0xa4, 0xff, 0xff, 0xff, 0xc3, 0x3d, 0x00, 0x00, 0x00, 0x00, 0x7d, 0x0b, 0x50, 0xb0, 0x2d, 0xe8, 0xec, 0xfe, 0xff, 0xff, 0x58, 0xf7, 0xd8, 0x3d, 0x0a, 0x00, 0x00, 0x00, 0x0f, 0x8c, 0xef, 0x00, 0x00, 0x00, 0x3d, 0x64, 0x00, 0x00, 0x00, 0x0f, 0x8c, 0xd1, 0x00, 0x00, 0x00, 0x3d, 0xe8, 0x03, 0x00, 0x00, 0x0f, 0x8c, 0xb3, 0x00, 0x00, 0x00, 0x3d, 0x10, 0x27, 0x00, 0x00, 0x0f, 0x8c, 0x95, 0x00, 0x00, 0x00, 0x3d, 0xa0, 0x86, 0x01, 0x00, 0x7c, 0x7b, 0x3d, 0x40, 0x42, 0x0f, 0x00, 0x7c, 0x61, 0x3d, 0x80, 0x96, 0x98, 0x00, 0x7c, 0x47, 0x3d, 0x00, 0xe1, 0xf5, 0x05, 0x7c, 0x2d, 0x3d, 0x00, 0xca, 0x9a, 0x3b, 0x7c, 0x13, 0xba, 0x00, 0x00, 0x00, 0x00, 0xbb, 0x00, 0xca, 0x9a, 0x3b, 0xf7, 0xfb, 0x52, 0xe8, 0x30, 0xff, 0xff, 0xff, 0x58, 0xba, 0x00, 0x00, 0x00, 0x00, 0xbb, 0x00, 0xe1, 0xf5, 0x05, 0xf7, 0xfb, 0x52, 0xe8, 0x1d, 0xff, 0xff, 0xff, 0x58, 0xba, 0x00, 0x00, 0x00, 0x00, 0xbb, 0x80, 0x96, 0x98, 0x00, 0xf7, 0xfb, 0x52, 0xe8, 0x0a, 0xff, 0xff, 0xff, 0x58, 0xba, 0x00, 0x00, 0x00, 0x00, 0xbb, 0x40, 0x42, 0x0f, 0x00, 0xf7, 0xfb, 0x52, 0xe8, 0xf7, 0xfe, 0xff, 0xff, 0x58, 0xba, 0x00, 0x00, 0x00, 0x00, 0xbb, 0xa0, 0x86, 0x01, 0x00, 0xf7, 0xfb, 0x52, 0xe8, 0xe4, 0xfe, 0xff, 0xff, 0x58, 0xba, 0x00, 0x00, 0x00, 0x00, 0xbb, 0x10, 0x27, 0x00, 0x00, 0xf7, 0xfb, 0x52, 0xe8, 0xd1, 0xfe, 0xff, 0xff, 0x58, 0xba, 0x00, 0x00, 0x00, 0x00, 0xbb, 0xe8, 0x03, 0x00, 0x00, 0xf7, 0xfb, 0x52, 0xe8, 0xbe, 0xfe, 0xff, 0xff, 0x58, 0xba, 0x00, 0x00, 0x00, 0x00, 0xbb, 0x64, 0x00, 0x00, 0x00, 0xf7, 0xfb, 0x52, 0xe8, 0xab, 0xfe, 0xff, 0xff, 0x58, 0xba, 0x00, 0x00, 0x00, 0x00, 0xbb, 0x0a, 0x00, 0x00, 0x00, 0xf7, 0xfb, 0x52, 0xe8, 0x98, 0xfe, 0xff, 0xff, 0x58, 0xe8, 0x92, 0xfe, 0xff, 0xff, 0xc3, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0xb8, 0x01, 0x00, 0x00, 0x00, 0xbb, 0x00, 0x00, 0x00, 0x00, 0xcd, 0x80, 0x90, 0x90, 0x90, 0x90, 0xb9, 0x00, 0x00, 0x00, 0x00, 0xb3, 0x03, 0x51, 0x53, 0xe8, 0xde, 0xfd, 0xff, 0xff, 0x5b, 0x59, 0x3c, 0x0a, 0x0f, 0x84, 0x34, 0x01, 0x00, 0x00, 0x3c, 0x7f, 0x0f, 0x84, 0x94, 0x00, 0x00, 0x00, 0x3c, 0x2d, 0x0f, 0x84, 0x09, 0x01, 0x00, 0x00, 0x3c, 0x30, 0x7c, 0xdb, 0x3c, 0x39, 0x7f, 0xd7, 0x2c, 0x30, 0x80, 0xfb, 0x00, 0x74, 0xd0, 0x80, 0xfb, 0x02, 0x75, 0x0c, 0x81, 0xf9, 0x00, 0x00, 0x00, 0x00, 0x75, 0x04, 0x3c, 0x00, 0x74, 0xbf, 0x80, 0xfb, 0x03, 0x75, 0x0a, 0x3c, 0x00, 0x75, 0x04, 0xb3, 0x00, 0xeb, 0x02, 0xb3, 0x01, 0x81, 0xf9, 0xcc, 0xcc, 0xcc, 0x0c, 0x7f, 0xa8, 0x81, 0xf9, 0x34, 0x33, 0x33, 0xf3, 0x7c, 0xa0, 0x88, 0xc7, 0xb8, 0x0a, 0x00, 0x00, 0x00, 0xf7, 0xe9, 0x3d, 0x08, 0x00, 0x00, 0x80, 0x74, 0x11, 0x3d, 0xf8, 0xff, 0xff, 0x7f, 0x75, 0x13, 0x80, 0xff, 0x07, 0x7e, 0x0e, 0xe9, 0x7f, 0xff, 0xff, 0xff, 0x80, 0xff, 0x08, 0x0f, 0x8f, 0x76, 0xff, 0xff, 0xff, 0xb9, 0x00, 0x00, 0x00, 0x00, 0x88, 0xf9, 0x80, 0xfb, 0x02, 0x74, 0x04, 0x01, 0xc1, 0xeb, 0x03, 0x29, 0xc8, 0x91, 0x88, 0xf8, 0x51, 0x53, 0xe8, 0xcb, 0xfd, 0xff, 0xff, 0x5b, 0x59, 0xe9, 0x53, 0xff, 0xff, 0xff, 0x80, 0xfb, 0x03, 0x0f, 0x84, 0x4a, 0xff, 0xff, 0xff, 0x51, 0x53, 0xb0, 0x08, 0xe8, 0x0a, 0xfd, 0xff, 0xff, 0xb0, 0x20, 0xe8, 0x03, 0xfd, 0xff, 0xff, 0xb0, 0x08, 0xe8, 0xfc, 0xfc, 0xff, 0xff, 0x5b, 0x59, 0x80, 0xfb, 0x00, 0x75, 0x07, 0xb3, 0x03, 0xe9, 0x25, 0xff, 0xff, 0xff, 0x80, 0xfb, 0x02, 0x75, 0x0f, 0x81, 0xf9, 0x00, 0x00, 0x00, 0x00, 0x75, 0x07, 0xb3, 0x03, 0xe9, 0x11, 0xff, 0xff, 0xff, 0x89, 0xc8, 0xb9, 0x0a, 0x00, 0x00, 0x00, 0xba, 0x00, 0x00, 0x00, 0x00, 0x3d, 0x00, 0x00, 0x00, 0x00, 0x7d, 0x08, 0xf7, 0xd8, 0xf7, 0xf9, 0xf7, 0xd8, 0xeb, 0x02, 0xf7, 0xf9, 0x89, 0xc1, 0x81, 0xf9, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x85, 0xe6, 0xfe, 0xff, 0xff, 0x80, 0xfb, 0x02, 0x0f, 0x84, 0xdd, 0xfe, 0xff, 0xff, 0xb3, 0x03, 0xe9, 0xd6, 0xfe, 0xff, 0xff, 0x80, 0xfb, 0x03, 0x0f, 0x85, 0xcd, 0xfe, 0xff, 0xff, 0xb0, 0x2d, 0x51, 0x53, 0xe8, 0x8d, 0xfc, 0xff, 0xff, 0x5b, 0x59, 0xb3, 0x02, 0xe9, 0xbb, 0xfe, 0xff, 0xff, 0x80, 0xfb, 0x03, 0x0f, 0x84, 0xb2, 0xfe, 0xff, 0xff, 0x80, 0xfb, 0x02, 0x75, 0x0c, 0x81, 0xf9, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x84, 0xa1, 0xfe, 0xff, 0xff, 0x51, 0xe8, 0x04, 0xfd, 0xff, 0xff, 0x59, 0x89, 0xc8, 0xc3};
    for (int i = 0; i < sizeof(header); i++) {
      cargarByte(memoria, header[i]);
    }
  }
}

// Escribe archivo binario con el contenido del array de bytes
void dumpToFile(memStruct *memoria, char *nombreArchivo) {
  FILE * fp;
  fp = fopen(nombreArchivo, "wb");
  fwrite(memoria->bytesArray, sizeof(byte), memoria->topeMemoria, fp);
  fclose(fp);
}

// Devuelve nombre de archivo ejecutable
void getOutputFilename(char *arg, char *output) {
  char *token;
  char filename[80];
  strcpy(filename, arg);
  token = strtok(filename, ".");
  strcpy(output, token);
  if (strcmp(OS_NAME, "windows") == 0) strcat(output, ".exe");
}

// ####################################################################################################################
// ################################################ Analizador Lexico #################################################
// ####################################################################################################################

tSimbolo aLex(FILE *fp) {
  tSimbolo a;
  a.cadena[0] = '\0';
  char c;
  do {
    c = getc(fp);
  } while (c != EOF && !isgraph(c));

  if (c == EOF) {
    a.simbolo = FIN_DE_ARCHIVO;
  } else {
    concatenar(a.cadena, c);
    if (isalpha(c)) {
      do {
        c = getc(fp);
        if (isalpha(c) || isdigit(c))
          concatenar(a.cadena, c);
      } while (c != EOF && (isalpha(c) || isdigit(c)));

      ungetc(c, fp);
      // El char que provoco el fin de la cadena debe volver a
      // leerse en el próximo llamado a aLex

      char cadenaAux[MAX_LONGITUD_CADENA + 3]; // Más los apóstrofos y el \0 final

      strcpy(cadenaAux, a.cadena);
      uppercase(cadenaAux);

      if (strcmp(cadenaAux, "BEGIN") == 0)
        a.simbolo = BEGIN;
      else if (strcmp(cadenaAux, "CALL") == 0)
        a.simbolo = CALL;
      else if (strcmp(cadenaAux, "CONST") == 0)
        a.simbolo = CONST;
      else if (strcmp(cadenaAux, "DO") == 0)
        a.simbolo = DO;
      else if (strcmp(cadenaAux, "END") == 0)
        a.simbolo = END;
      else if (strcmp(cadenaAux, "ELSE") == 0)
        a.simbolo = ELSE;
      else if (strcmp(cadenaAux, "FOR") == 0)
        a.simbolo = FOR;
      else if (strcmp(cadenaAux, "HALT") == 0)
        a.simbolo = HALT;
      else if (strcmp(cadenaAux, "IF") == 0)
        a.simbolo = IF;
      else if (strcmp(cadenaAux, "ODD") == 0)
        a.simbolo = ODD;
      else if (strcmp(cadenaAux, "NOT") == 0)
        a.simbolo = NOT;
      else if (strcmp(cadenaAux, "PROCEDURE") == 0)
        a.simbolo = PROCEDURE;
      else if (strcmp(cadenaAux, "READLN") == 0)
        a.simbolo = READLN;
      else if (strcmp(cadenaAux, "THEN") == 0)
        a.simbolo = THEN;
      else if (strcmp(cadenaAux, "VAR") == 0)
        a.simbolo = VAR;
      else if (strcmp(cadenaAux, "WHILE") == 0)
        a.simbolo = WHILE;
      else if (strcmp(cadenaAux, "WRITE") == 0)
        a.simbolo = WRITE;
      else if (strcmp(cadenaAux, "WRITELN") == 0)
        a.simbolo = WRITELN;
      else
        a.simbolo = IDENT;
    } else if (isdigit(c)) {
      do {
        c = getc(fp);
        if (isdigit(c)) concatenar(a.cadena, c);
      } while (c != EOF && isdigit(c));

      ungetc(c, fp);
      // El char que provoco el fin de la cadena debe volver a
      // leerse en el próximo llamado a aLex

      a.simbolo = NUMERO;
    } else {
      switch (c) {

      case '\'':
        do {
          c = getc(fp);
          if (c != EOF && c != '\n') concatenar(a.cadena, c);
        } while (c != EOF && c != '\n' && c != '\'');

        if (c == EOF || c == '\n') {
          a.simbolo = NULO;
          ungetc(c, fp);
        } else {
          a.simbolo = CADENA;
        }
        break;

      case '.':
        a.simbolo = PUNTO;
        break;

      case ',':
        a.simbolo = COMA;
        break;

      case ';':
        a.simbolo = PTOCOMA;
        break;

      case '+':
        a.simbolo = MAS;
        break;

      case '-':
        a.simbolo = MENOS;
        break;

      case '*':
        a.simbolo = POR;
        break;

      case '/':
        a.simbolo = DIVIDIDO;
        break;

      case '(':
        a.simbolo = ABREPAREN;
        break;

      case ')':
        a.simbolo = CIERRAPAREN;
        break;

      case '<':
        c = getc(fp);
        if (c == '=') {
          concatenar(a.cadena, c);
          a.simbolo = MENORIGUAL;
        } else if (c == '>') {
          concatenar(a.cadena, c);
          a.simbolo = DISTINTO;
        } else {
          ungetc(c, fp);
          a.simbolo = MENOR;
        }
        break;

      case '>':
        c = getc(fp);
        if (c == '=') {
          concatenar(a.cadena, c);
          a.simbolo = MAYORIGUAL;
        } else {
          ungetc(c, fp);
          a.simbolo = MAYOR;
        }
        break;

      case '=':
        a.simbolo = IGUAL;
        break;

      case ':':
        c = getc(fp);
        if (c == '=') {
          concatenar(a.cadena, c);
          a.simbolo = ASIGNACION;
        } else {
          ungetc(c, fp);
          a.simbolo = NULO;
        }
        break;
      default:
        a.simbolo = NULO;
      }
    }
  }

  imprimir(a);

  return a;
}

// ####################################################################################################################
// ##################################################### Programa #####################################################
// ####################################################################################################################

tSimbolo programa(tSimbolo s, FILE *archivo, memStruct *memoria) {
  // Declaramos tabla de identificadores para el analizador semantico
  tablaDeIdent tabla;

  int desplVarMem = 0;                                              // Desplazamiento de variables en memoria, aumenta de 4 en 4

  // La primera instrucción del código traducido será la inicialización del registro EDI para que apunte a la dirección
  // a partir de la cual estarán alojados los valores de las variables. Como esta dirección aún no se conoce, deberá
  // reservarse el lugar grabando BF 00 00 00 00.
  cargarByte(memoria, 0xBF);                                        // MOV EDI,   pos. 1792
  cargarInt(memoria, 0);                                            // 0 0 0 0    pos. 1793, Inserta 4 bytes en 0

  s = bloque(s, archivo, memoria, tabla, &desplVarMem, 0);
  if (s.simbolo != PUNTO) error(1, s);                              // Se esperaba un PUNTO
  s = aLex(archivo);

  codigoFinal(memoria, desplVarMem);                                // Carga el final del código específico según la plataforma win/linux

  return s;
}

// Generacion del final del código
void codigoFinal(memStruct *memoria, int desplVarMem) {
  int base;

  int distanciaFin = END_PROGRAM - (memoria->topeMemoria + 5);      // Pag. 31 ver. Windows, parrafo 1
  cargarByte(memoria, 0xE9);                                        // Calcula la distancia hasta la instrucción de finalización de programa (0x588 para windows)
  cargarInt(memoria, distanciaFin);                                 // Escribe un salto negativo de memoria

  if (strcmp(OS_NAME, "windows") == 0) {
    int imageBase = leerIntDe(memoria, 212);                        // memoria[212] es image base
    int baseOfCode = leerIntDe(memoria, 200);                       // memoria[200] es base of code
    base = imageBase + baseOfCode;
  } else if (strcmp(OS_NAME, "linux") == 0) {
    base = leerIntDe(memoria, 193);                                 // memoria[193] es Address
  }
                                                                    // Pag. 31 ver. Windows, parrafo 3
  int distBase = base + memoria->topeMemoria - TEXT_START;          // se carga en la posicion VAR_LENGTH_START el size de de la parte text,
  cargarIntEn(memoria, distBase, VAR_LENGTH_START + 1);             // VAR_LENGTH_START es la posicion donde termina la parte de long. fija
                                                                    // En windows, el codigo empieza en 0x701 / 1793 en linux en 0x481 / 1153

  for (int i = 0; i < desplVarMem / 4; i++) {                       // Pag. 31 ver. Windows, parrafo 4
    cargarInt(memoria, 0);                                          // Carga 1 cero por cada variable (4 bytes, ints de 32 bits)
  }

  if (strcmp(OS_NAME, "windows") == 0) {
    cargarIntEn(memoria, memoria->topeMemoria - TEXT_START, 416);   // Pag. 31, parrafo 5 - Escribe el tamaño de la sección .text en la pos. 416 (virtual size)

    int fileAlignment = leerIntDe(memoria, 220);                    // Pag. 31, parrafo 6

    while (memoria->topeMemoria % fileAlignment != 0) {             // Se rellena con 0's para que sea multiplo de 512 (requisito de windows)
      cargarByte(memoria, 0x00);
    }

    cargarIntEn(memoria, memoria->topeMemoria - TEXT_START, 188);   // Pag. 31, parrafo 7
    cargarIntEn(memoria, memoria->topeMemoria - TEXT_START, 424);   // Ajusta campos SizeOfCodeSection y SizeOfRawData con el tamaño final de .text

    int sizeOfCodeSection = leerIntDe(memoria, 188);                // Pag. 31, parrafo 8
    int sizeOfRawData = leerIntDe(memoria, 424);                    // Ajusta los campos SizeOfImage (240) y BaseOfData (208) según los campos
    int sectionAlignment = leerIntDe(memoria, 216);                 // SizeOfCodeSection (188), SizeOfRawData (424) y SectionAlignment (216).
    cargarIntEn(memoria, (2 + sizeOfCodeSection / sectionAlignment) * sectionAlignment, 240);
    cargarIntEn(memoria, (2 + sizeOfRawData / sectionAlignment) * sectionAlignment, 208);

  } else if (strcmp(OS_NAME, "linux") == 0) {
    cargarIntEn(memoria, memoria->topeMemoria, 68);                 // Pag. 29, parrafo 4
    cargarIntEn(memoria, memoria->topeMemoria, 72);                 // Ajusta campos FileSize y MemorySize con el tamaño final del archivo ejecutable
    cargarIntEn(memoria, memoria->topeMemoria - 0xE0, 201);         // Pag. 29, parrafo 5
  }
}

tSimbolo bloque(tSimbolo s, FILE *archivo, memStruct *memoria, tablaDeIdent tabla, int *desplVarMem, int base) {
  int desplazamiento = 0;
  int p;

  // Agregar salto condicional al inicio de bloque, antes de todo (E9 00 00 00 00)
  cargarByte(memoria, 0xE9);                                        // JMP (en cero, al final se vuelve para corregir el salto)
  cargarInt(memoria, 0);                                            // ....

  int inicioBloque = memoria->topeMemoria;                          // Guardo esta posicion para hacer el fixup del salto anterior

  if (s.simbolo == CONST) {
    do {
      s = aLex(archivo);
      if (s.simbolo != IDENT) error(2, s);                          // Se esperaba un IDENT
      p = buscarIdent(s.cadena, tabla, base, (base + desplazamiento - 1));

      // Si no encuentro el identificador en la tabla, lo agrego
      if (p != -1) error(13, s);                                    // El ident ya estaba declarado
      tabla[base + desplazamiento].tipo = CONST;
      uppercase(s.cadena);
      strcpy(tabla[base + desplazamiento].nombre, s.cadena);

      s = aLex(archivo);
      if (s.simbolo != IGUAL) error(3, s);                          // Se esperaba un IGUAL

      s = aLex(archivo);
      if (s.simbolo != NUMERO) error(4, s);                         // Se esperaba un NUMERO
      tabla[base + desplazamiento].valor = atoi(s.cadena);
      desplazamiento++;

      s = aLex(archivo);
    } while (s.simbolo == COMA);

    if (s.simbolo != PTOCOMA) error(5, s);                          // Se esperaba PTOCOMA
    s = aLex(archivo);
  }

  if (s.simbolo == VAR) {
    do {
      s = aLex(archivo);
      if (s.simbolo != IDENT) error(2, s);                          // Se esperaba un IDENT

      p = buscarIdent(s.cadena, tabla, base, (base + desplazamiento - 1));
      if (p != -1) error(13, s);                                    // Ya estaba declarado en la tabla

      tabla[base + desplazamiento].tipo = VAR;
      uppercase(s.cadena);
      strcpy(tabla[base + desplazamiento].nombre, s.cadena);
      tabla[base + desplazamiento].valor = *desplVarMem;            // La dirección de memoria a que se refiere la variable
                                                                    // (sólo el desplazamiento, es una posicion relativa a EDI)
      *desplVarMem += 4;
      desplazamiento++;

      s = aLex(archivo);
    } while (s.simbolo == COMA);

    if (s.simbolo != PTOCOMA) error(5, s);                          // Se esperaba punto y coma
    s = aLex(archivo);
  }

  while (s.simbolo == PROCEDURE) {
    s = aLex(archivo);

    if (s.simbolo != IDENT) error(2, s);                            // Se esperaba un IDENT

    p = buscarIdent(s.cadena, tabla, base, (base + desplazamiento - 1));
    if (p != -1) error(13, s);                                      // El identificador ya estaba declarado

    tabla[base + desplazamiento].tipo = PROCEDURE;
    uppercase(s.cadena);
    strcpy(tabla[base + desplazamiento].nombre, s.cadena);
    tabla[base + desplazamiento].valor = memoria->topeMemoria;      // La dirección de memoria donde comienza la proposición a ejecutar en el bloque
    desplazamiento++;

    s = aLex(archivo);

    if (s.simbolo != PTOCOMA) error(5, s);                          // Se esperaba PTOCOMA

    s = aLex(archivo);

    s = bloque(s, archivo, memoria, tabla, desplVarMem, base + desplazamiento);

    cargarByte(memoria, 0xC3);                                      // RET, vuelve al punto donde se llamó a la subrutina (procedure)

    if (s.simbolo != PTOCOMA) error(5, s);                          // Se esperaba PTOCOMA
    s = aLex(archivo);
  }

  int sizeBloque = memoria->topeMemoria - inicioBloque;
  if (sizeBloque > 0) {                                             // Si el tamaño del bloque es mayor que cero, hago el fixup del salto
      cargarIntEn(memoria, sizeBloque, inicioBloque - 4);           // incondicional del principio del bloque, sino, borro directamente el salto
  } else {
    memoria->topeMemoria -= 5;
  }

  s = proposicion(s, archivo, memoria, tabla, (base + desplazamiento - 1), desplVarMem);
  return s;
}

tSimbolo proposicion(tSimbolo s, FILE *archivo, memStruct *memoria, tablaDeIdent tabla, int posUltimoIdent, int *desplVarMem) {
  int p, posPrevCond, posPostCond, origSalto, destSalto, origSaltoElse, destSaltoElse, dist;

  switch (s.simbolo) {

    case IDENT:
      p = buscarIdent(s.cadena, tabla, 0, posUltimoIdent);

      if (p == -1) error(15, s);                                    // Identificador no encontrado
      if (tabla[p].tipo != VAR) error(17, s);                       // Error semantico, se esperaba un VAR

      s = aLex(archivo);
      if (s.simbolo != ASIGNACION) error(6, s);                     // Error sintactico, se esperaba asignacion

      s = aLex(archivo);
      s = expresion(s, archivo, memoria, tabla, posUltimoIdent);

      cargarPopEax(memoria);                                        //  POP EAX
      cargarByte(memoria, 0x89);                                    //  MOV [EDI + ....], EAX
      cargarByte(memoria, 0x87);                                    //  ....
      cargarInt(memoria, tabla[p].valor);                           //  Offset de memoria respecto de EDI
      break;

    case CALL:
      s = aLex(archivo);
      if (s.simbolo != IDENT) error(2, s);                          // Se esperaba un ident

      p = buscarIdent(s.cadena, tabla, 0, posUltimoIdent);
      if (p == -1) error(15, s);                                    // Identificador no encontrado
      if (tabla[p].tipo != PROCEDURE) error(16, s);                 // Error semantico, se esperaba un PROCEDURE

      cargarByte(memoria, 0xE8);                                        // CALL dir
      cargarInt(memoria, tabla[p].valor - (memoria->topeMemoria + 4));  // Salto hacia atras (inicio del procedure)

      s = aLex(archivo);
      break;

    case BEGIN:
      s = aLex(archivo);
      s = proposicion(s, archivo, memoria, tabla, posUltimoIdent, desplVarMem);
      while (s.simbolo == PTOCOMA) {
        s = aLex(archivo);
        s = proposicion(s, archivo, memoria, tabla, posUltimoIdent, desplVarMem);
      }
      if (s.simbolo != END) error(7, s);                            // Se esperaba END
      s = aLex(archivo);
      break;

    case IF:
      s = aLex(archivo);
      s = condicion(s, archivo, memoria, tabla, posUltimoIdent);

      origSalto = memoria->topeMemoria;                             // Guardamos posicion previa a la proposicion para el fixup de condicion

      if (s.simbolo != THEN) error(8, s);                           // Se esperaba THEN

      s = aLex(archivo);
      s = proposicion(s, archivo, memoria, tabla, posUltimoIdent, desplVarMem);

      destSalto = memoria->topeMemoria;                             // Guardamos posicion posterior a la proposicion
      cargarIntEn(memoria, destSalto - origSalto, origSalto - 4);   // Arreglamos salto de condicion

      if (s.simbolo == ELSE) {
        cargarIntEn(memoria, destSalto - origSalto + 5, origSalto - 4);   // Agregamos +5 al salto incondicional inicial, ya que tiene que saltar
                                                                          // el E9 de la proposicion del ELSE

        origSaltoElse = memoria->topeMemoria;                       // Guardo posicion de mem. antes de la prop. del else
        cargarByte(memoria, 0xE9);                                  // Cargo un salto incond. para arreglar despues
        cargarInt(memoria, 0);

        s = aLex(archivo);
        s = proposicion(s, archivo, memoria, tabla, posUltimoIdent, desplVarMem);

        destSaltoElse = memoria->topeMemoria;                       // Dest. del salto
        cargarIntEn(memoria, destSaltoElse - origSaltoElse, origSaltoElse - 4);
      }

      break;

    case WHILE:
      s = aLex(archivo);

      posPrevCond = memoria->topeMemoria;
      s = condicion(s, archivo, memoria, tabla, posUltimoIdent);
      posPostCond = memoria->topeMemoria;

      if (s.simbolo != DO) error(9, s);                             // Se esperaba DO

      s = aLex(archivo);
      s = proposicion(s, archivo, memoria, tabla, posUltimoIdent, desplVarMem);

      destSalto = memoria->topeMemoria;

      dist = posPrevCond - (memoria->topeMemoria + 5);              // Cantidad de bytes a saltar hacia atras
      cargarByte(memoria, 0xE9);                                    // JMP dir, salto incondicional hacia atras
      cargarInt(memoria, dist);                                     // ... (vuelve hasta el inicio de la condicion)
      cargarIntEn(memoria, memoria->topeMemoria - posPostCond, posPostCond - 4);    // Fixup del salto E9 de la condicion del while
                                                                                    // (para saltar todo el while si la condicion da false)
      break;

    case FOR:
      s = aLex(archivo);

      if (s.simbolo != ABREPAREN) error(10, s);

      s = aLex(archivo);


      // 1er parametro
      if (s.simbolo != IDENT) error(2, s);

      p = buscarIdent(s.cadena, tabla, 0, posUltimoIdent);

      if (p == -1) error(15, s);                                    // Identificador no encontrado
      if (tabla[p].tipo != VAR) error(17, s);                       // Error semantico, se esperaba un VAR

      s = aLex(archivo);
      if (s.simbolo != ASIGNACION) error(6, s);                     // Error sintactico, se esperaba asignacion

      s = aLex(archivo);
      s = expresion(s, archivo, memoria, tabla, posUltimoIdent);
      cargarPopEax(memoria);                                        //  POP EAX
      cargarByte(memoria, 0x89);                                    //  MOV [EDI + ....], EAX
      cargarByte(memoria, 0x87);                                    //  ....
      cargarInt(memoria, tabla[p].valor);                           //  Offset de memoria respecto de EDI


      // 2do parametro
      if (s.simbolo != PTOCOMA) error(5, s);

      posPrevCond = memoria->topeMemoria;
      s = aLex(archivo);
      s = condicion(s, archivo, memoria, tabla, posUltimoIdent);
      if (s.simbolo != PTOCOMA) error(5, s);
      posPostCond = memoria->topeMemoria;


      // 3er parametro
      s = aLex(archivo);

      if (s.simbolo != IDENT) error(2, s);

      p = buscarIdent(s.cadena, tabla, 0, posUltimoIdent);

      if (p == -1) error(15, s);                                    // Identificador no encontrado
      if (tabla[p].tipo != VAR) error(17, s);                       // Error semantico, se esperaba un VAR

      s = aLex(archivo);
      if (s.simbolo != ASIGNACION) error(6, s);                     // Error sintactico, se esperaba asignacion

      s = aLex(archivo);
      s = expresion(s, archivo, memoria, tabla, posUltimoIdent);
      cargarPopEax(memoria);                                        //  POP EAX
      cargarByte(memoria, 0x89);                                    //  MOV [EDI + ....], EAX
      cargarByte(memoria, 0x87);                                    //  ....
      cargarInt(memoria, tabla[p].valor);                           //  Offset de memoria respecto de EDI

      if (s.simbolo != CIERRAPAREN) error(11, s);

      s = aLex(archivo);

      if (s.simbolo != DO) error(9, s);                             // Se esperaba DO

      s = aLex(archivo);
      s = proposicion(s, archivo, memoria, tabla, posUltimoIdent, desplVarMem);

      destSalto = memoria->topeMemoria;

      dist = posPrevCond - (memoria->topeMemoria + 5);              // Cantidad de bytes a saltar hacia atras
      cargarByte(memoria, 0xE9);                                    // JMP dir, salto incondicional hacia atras
      cargarInt(memoria, dist);                                     // ... (vuelve hasta el inicio de la condicion)
      cargarIntEn(memoria, memoria->topeMemoria - posPostCond, posPostCond - 4);    // Fixup del salto E9 de la condicion del while
                                                                                    // (para saltar todo el while si la condicion da false)
      break;

    case READLN:
      s = aLex(archivo);
      if (s.simbolo != ABREPAREN) error(10, s);                     // Se esperaba ABREPAREN

      do {
        s = aLex(archivo);
        if (s.simbolo != IDENT) error(2, s);                        // Se esperaba IDENT

        p = buscarIdent(s.cadena, tabla, 0, posUltimoIdent);
        if (p == -1) error(15, s);                                  // No se encontro el IDENT en la tabla
        if (tabla[p].tipo != VAR) error(14, s);                     // Se esperaba un VAR

        s = aLex(archivo);

        cargarByte(memoria, 0xE8);                                  // CALL dir
        int distanciaReadln = READ_EAX - (memoria->topeMemoria + 4);// (distancia)
        cargarInt(memoria, distanciaReadln);                        // ...
        cargarByte(memoria, 0x89);                                  // MOV [EDI + ....], EAX
        cargarByte(memoria, 0x87);                                  // ...
        cargarInt(memoria, tabla[p].valor);                         // MOV [EDI + ....], EAX
      } while (s.simbolo == COMA);

      if (s.simbolo != CIERRAPAREN) error(11, s);                   // Se esperaba CIERRAPAREN

      s = aLex(archivo);
      break;

    case WRITE:
      s = aLex(archivo);

      if (s.simbolo != ABREPAREN) error(10, s);

      do {
        s = aLex(archivo);
        if (s.simbolo == CADENA) {
          writeString(memoria, s);
          s = aLex(archivo);
        } else {
          s = expresion(s, archivo, memoria, tabla, posUltimoIdent);
          // Genera código para escribir un número por pantalla
          cargarPopEax(memoria);
          cargarByte(memoria, 0xE8);
          cargarInt(memoria, PRINT_EAX - (memoria->topeMemoria + 4));
        }
      } while (s.simbolo == COMA);

      if (s.simbolo != CIERRAPAREN) error(11, s);                   // Se esperaba CIERRAPAREN
      s = aLex(archivo);
      break;

    case WRITELN:
      s = aLex(archivo);
      if (s.simbolo == PTOCOMA || s.simbolo != ABREPAREN) {         // Si se lo invoca sin parametros, es un salto de linea nomas
        cargarByte(memoria, 0xE8);                                  // CALL
        cargarInt(memoria, NEW_LINE - (memoria->topeMemoria + 4));  // Salto de linea
        return s;
      }

      do {
        s = aLex(archivo);
        if (s.simbolo == CADENA) {
          writeString(memoria, s);
          s = aLex(archivo);
        } else {
          s = expresion(s, archivo, memoria, tabla, posUltimoIdent);
          // Genera código para escribir un número por pantalla
          cargarPopEax(memoria);
          cargarByte(memoria, 0xE8);
          cargarInt(memoria, PRINT_EAX - (memoria->topeMemoria + 4));
        }
      } while (s.simbolo == COMA);

      // Finaliza enviando un salto de linea
      cargarByte(memoria, 0xE8);                                    // CALL
      cargarInt(memoria, NEW_LINE - (memoria->topeMemoria + 4));    // Salto de linea

      if (s.simbolo != CIERRAPAREN) error(11, s);                   // Se esperaba CIERRAPAREN
      s = aLex(archivo);
      break;

    case HALT:
      codigoFinal(memoria, *desplVarMem);
      s = aLex(archivo);
      break;
  }
  return s;
}

void writeString(memStruct *memoria, tSimbolo s) {
  int base, baseOfCode, imageBase, posCadena;
  int sizeCadena = strlen(s.cadena);

  if (strcmp(OS_NAME, "windows") == 0) {
    baseOfCode = leerIntDe(memoria, 204);
    imageBase = leerIntDe(memoria, 212);
    base = baseOfCode + imageBase;
    posCadena = base + memoria->topeMemoria - TEXT_START + 15;
    cargarByte(memoria, 0xB8);                                      // MOV EAX, ....
    cargarInt(memoria, posCadena);                                  // Ubicacion absoluta de la cadena
    cargarByte(memoria, 0xE8);                                      // CALL dir
    cargarInt(memoria, PRINT_STRING - memoria->topeMemoria - 4);    // Rutina de impresion por pantalla
    cargarByte(memoria, 0xE9);                                      // JMP ....
    cargarInt(memoria, sizeCadena - 1);
    for (int i = 1; i < (sizeCadena - 1); i++) cargarByte(memoria, s.cadena[i]);
    cargarByte(memoria, 0);
  } else if (strcmp(OS_NAME, "linux") == 0) {
    base = leerIntDe(memoria, 193);
    posCadena = base + memoria->topeMemoria - TEXT_START + 20;
    cargarByte(memoria, 0xB9);                                      // MOV ECX, ....
    cargarInt(memoria, posCadena);                                  // Ubicacion absoluta de la cadena
    cargarByte(memoria, 0xBA);                                      // MOV EDX, ....
    cargarInt(memoria, sizeCadena - 2);                             // Longitud de la cadena sin el \0
    cargarByte(memoria, 0xE8);                                      // CALL dir
    cargarInt(memoria, PRINT_STRING - memoria->topeMemoria - 4);    // Rutina de impresion por pantalla
    cargarByte(memoria, 0xE9);                                      // JMP ....
    cargarInt(memoria, sizeCadena - 2);
    for (int i = 1; i < (sizeCadena - 1); i++) cargarByte(memoria, s.cadena[i]);
  }
}

tSimbolo condicion(tSimbolo s, FILE *archivo, memStruct *memoria, tablaDeIdent tabla, int posUltimoIdent) {
  int isIfNot = 0;
  tTerminal operador;

  if (s.simbolo == ODD) {
    s = aLex(archivo);
    s = expresion(s, archivo, memoria, tabla, posUltimoIdent);

    cargarPopEax(memoria);                                          // POP EAX
    cargarByte(memoria, 0xA8);                                      // TEST AL, ab
    cargarByte(memoria, 0x01);                                      // ... (ab)
    cargarByte(memoria, 0x7B);                                      // JPO ab     => Jump if parity odd
    cargarByte(memoria, 0x05);                                      // ... (ab)   => 5 bytes

  } else {

    if (s.simbolo == NOT) {
        isIfNot = 1;                                                // Guardo un flag si es un IF NOT
        s = aLex(archivo);
    };

    s = expresion(s, archivo, memoria, tabla, posUltimoIdent);

    operador = s.simbolo;                                           // Guardo el operador y leo la siguiente expresion

    s = aLex(archivo);
    s = expresion(s, archivo, memoria, tabla, posUltimoIdent);

    cargarPopEax(memoria);                                          // POP EAX
    cargarByte(memoria, 0x5B);                                      // POP EBX
    cargarByte(memoria, 0x39);                                      // CMP EBX, EAX     => Compara EBX y EAX segun los operadores del switch y salta
    cargarByte(memoria, 0xC3);                                      // ...              la cantidad de bytes indicadas despues del Jump If

    switch (operador) {
    case IGUAL:
      cargarByte(memoria, 0x74);                                    // JE dir     => Jump if equal
      cargarByte(memoria, 0x05);                                    // Salta 5 bytes (una instruccion)
      break;
    case DISTINTO:
      cargarByte(memoria, 0x75);                                    // JNE dir    => Jump if not equal
      cargarByte(memoria, 0x05);
      break;
    case MAYOR:
      cargarByte(memoria, 0x7F);                                    // JG dir    => Jump if greater than
      cargarByte(memoria, 0x05);
      break;
    case MENOR:
      cargarByte(memoria, 0x7C);                                    // JL dir    => Jump if less than
      cargarByte(memoria, 0x05);
      break;
    case MAYORIGUAL:
      cargarByte(memoria, 0x7D);                                    // JGE dir    => Jump if greater than or equal
      cargarByte(memoria, 0x05);
      break;
    case MENORIGUAL:
      cargarByte(memoria, 0x7E);                                    // JLE dir    => Jump if less than or equal
      cargarByte(memoria, 0x05);
      break;
    default:
      error(12, s);                                                 // Se esperaba un operador relacional
    }

    if (isIfNot) {                                                  // Si es un IF NOT, al darse la condicion true saltea esta instruccion
        cargarByte(memoria, 0xE9);                                  // y entra directamente en el E9 siguiente que saltea toda la proposicion
        cargarInt(memoria, 5);                                      // Si la condicion es FALSE, entra en este E9 y saltea el E9 siguiente 
    }
  }

  // Salto de memoria, según si entra ó no en la proposición que sigue despues de la condicion.
  // Si entra, salta al inicio de la prop, si no entra, salta al final de la prop.
  cargarByte(memoria, 0xE9);                                        // JMP ...
  cargarInt(memoria, 0);                                            // Fixup

  return s;
}

tSimbolo expresion(tSimbolo s, FILE *archivo, memStruct *memoria, tablaDeIdent tabla, int posUltimoIdent) {
  tTerminal termAux;

  if (s.simbolo == MAS) {
    s = aLex(archivo);
    s = termino(s, archivo, memoria, tabla, posUltimoIdent);
  } else if (s.simbolo == MENOS) {                                  // Si es un numero negativo
    s = aLex(archivo);
    s = termino(s, archivo, memoria, tabla, posUltimoIdent);
    cargarPopEax(memoria);                                          // POP EAX
    cargarByte(memoria, 0xF7);                                      // NEG EAX
    cargarByte(memoria, 0xD8);                                      // ...
    cargarByte(memoria, 0x50);                                      // PUSH EAX
  } else {
    s = termino(s, archivo, memoria, tabla, posUltimoIdent);
  }

  while (s.simbolo == MAS || s.simbolo == MENOS) {
    termAux = s.simbolo;
    s = aLex(archivo);
    s = termino(s, archivo, memoria, tabla, posUltimoIdent);
    if (termAux == MAS) {
      cargarPopEax(memoria);                                        // POP EAX
      cargarByte(memoria, 0x5B);                                    // POP EBX
      cargarByte(memoria, 0x01);                                    // ADD EAX, EBX
      cargarByte(memoria, 0xD8);                                    // ...
      cargarByte(memoria, 0x50);                                    // PUSH EAX
    } else if (termAux == MENOS) {
      cargarPopEax(memoria);                                        // POP EAX
      cargarByte(memoria, 0x5B);                                    // POP EBX
      cargarByte(memoria, 0x93);                                    // XCHG EAX, EBX
      cargarByte(memoria, 0x29);                                    // SUB EAX, EBX
      cargarByte(memoria, 0xD8);                                    // ...
      cargarByte(memoria, 0x50);                                    // PUSH EAX
    }
  }
  return s;
}

tSimbolo termino(tSimbolo s, FILE *archivo, memStruct *memoria, tablaDeIdent tabla, int posUltimoIdent) {
  s = factor(s, archivo, memoria, tabla, posUltimoIdent);
  while (s.simbolo == POR || s.simbolo == DIVIDIDO) {
    tTerminal operacion = s.simbolo;
    s = aLex(archivo);
    s = factor(s, archivo, memoria, tabla, posUltimoIdent);
    if (operacion == POR) {
      cargarPopEax(memoria);                                        // POP EAX
      cargarByte(memoria, 0x5B);                                    // POP EBX
      cargarByte(memoria, 0xF7);                                    // IMUL EBX
      cargarByte(memoria, 0xEB);                                    // ...
      cargarByte(memoria, 0x50);                                    // PUSH EAX
    } else {                                                        // Division
      cargarPopEax(memoria);                                        // POP EAX
      cargarByte(memoria, 0x5B);                                    // POP EBX
      cargarByte(memoria, 0x93);                                    // XCHG EAX, EBX
      cargarByte(memoria, 0x99);                                    // CDQ
      cargarByte(memoria, 0xF7);                                    // IDIV EBX
      cargarByte(memoria, 0xFB);                                    // ...
      cargarByte(memoria, 0x50);                                    // PUSH EAX
    }
  }
  return s;
}

tSimbolo factor(tSimbolo s, FILE *archivo, memStruct *memoria, tablaDeIdent tabla, int posUltimoIdent) {
  int p;
  switch (s.simbolo) {
    case IDENT:
      p = buscarIdent(s.cadena, tabla, 0, posUltimoIdent);
      if (p == -1) error(15, s);                                    // identificador no encontrado
      if (tabla[p].tipo == PROCEDURE) {
        error(18, s);                                               // Error semantico, se esperaba un VAR o CONST
      } else if (tabla[p].tipo == VAR) {
        cargarByte(memoria, 0x8B);                                  // MOV EAX[EDI + ....] (8B 87 gh ef cd ab)
        cargarByte(memoria, 0x87);                                  // ....
        cargarInt(memoria, tabla[p].valor);                         // Direccionamiento indexado, el valor de VAR es el offset de bytes respecto de EDI
        cargarByte(memoria, 0x50);                                  // PUSH EAX
      } else if (tabla[p].tipo == CONST) {                          // CONST
        cargarByte(memoria, 0xB8);                                  // MOV EAX .... (B8 gh ef cd ab)
        cargarInt(memoria, tabla[p].valor);                         // Direccionamiento inmediato
        cargarByte(memoria, 0x50);                                  // PUSH EAX
      }
      s = aLex(archivo);
      break;
    case NUMERO:
      cargarByte(memoria, 0xB8);                                    // MOV EAX .... (B8 gh ef cd ab)
      cargarInt(memoria, atoi(s.cadena));                           // Direccionamiento inmediato
      cargarByte(memoria, 0x50);                                    // PUSH EAX
      s = aLex(archivo);
      break;
    case ABREPAREN:
      s = aLex(archivo);
      s = expresion(s, archivo, memoria, tabla, posUltimoIdent);
      if (s.simbolo != CIERRAPAREN) error(11, s);                   // Se esperaba CIERRAPAREN
      s = aLex(archivo);
      break;
  }
  return s;
}
