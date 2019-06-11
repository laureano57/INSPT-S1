// Para usar isgraph
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ################################ CONSTANTES ################################
// Longitud neta, sin los apóstrofos ni el cero final
#define MAX_LONGITUD_CADENA 128
// Longitud de la tabla de identificadores
#define MAX_CANT_IDENT 1024

// #define ARCHIVO "ejemplo.pl0"
// #define ARCHIVO "BIEN-00.PL0"
#define ARCHIVO "test.pl0"


// ############################### NUEVOS TIPOS ###############################

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
  END,
  FIN_DE_ARCHIVO,
  IDENT,
  IF,
  IGUAL,
  MAS,
  MAYOR,
  MAYORIGUAL,
  MENOR,
  MENORIGUAL,
  MENOS,
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


// ###################### NUEVAS FUNCIONES (PROTOTIPOS) ######################

tSimbolo aLex(FILE *);

void imprimir(tSimbolo);

void concatenar(char *, char);

void uppercase(char *);

void error(int codigo, tSimbolo s);

int buscarIdent(char *, tablaDeIdent, int, int);

// ################### Funciones del analizador sintactico ###################

tSimbolo programa(tSimbolo, FILE *archivo);
tSimbolo bloque(tSimbolo, FILE *archivo, tablaDeIdent, int base);
tSimbolo proposicion(tSimbolo, FILE *archivo, tablaDeIdent, int posUltimoIdent);
tSimbolo condicion(tSimbolo, FILE *archivo, tablaDeIdent, int posUltimoIdent);
tSimbolo expresion(tSimbolo, FILE *archivo, tablaDeIdent, int posUltimoIdent);
tSimbolo termino(tSimbolo, FILE *archivo, tablaDeIdent, int posUltimoIdent);
tSimbolo factor(tSimbolo, FILE *archivo, tablaDeIdent, int posUltimoIdent);

// ################################### MAIN ###################################

int main(int argc, char *argv[]) {
  // if (argc != 2) {
  //     printf("Uso: lexer FUENTE.PL0\n");
  // } else {
  FILE *archivo;
  // archivo = fopen(argv[1], "r");
  archivo = fopen(ARCHIVO, "r"); // Harcodea nombre de archivo
  if (archivo == NULL) {
    printf("Error de apertura del archivo [%s].\n", argv[1]);
  } else {
    tSimbolo s;
    s = aLex(archivo);
    s = programa(s, archivo);
    if (s.simbolo == FIN_DE_ARCHIVO) {
      printf("Archivo procesado exitosamente\n");
    } else {
      error(0, s);
      // printf("Error: sobra algo despues del punto\n");
    }

    // ########## Para probar el analizador lexico ##########
    // do {
    //     s = aLex(archivo);
    //     imprimir(s);
    // } while(s.simbolo != FIN_DE_ARCHIVO);
    // fclose(archivo);
    // ######################################################
  }
  // }
  return 0;
}

// ##################### NUEVAS FUNCIONES (DEFINICIONES) #####################

tSimbolo aLex(FILE *fp) {
  tSimbolo a;
  a.cadena[0] = '\0';
  char c;
  do {
    c = getc(fp);
  } while (c != EOF && !isgraph(c));
  // Corta cuando c == EOF o cuando isgraph(c) == true

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
      // Corta cuando c==EOF o cuando c no es letra ni dígito

      ungetc(c, fp);
      // El char que provoco el fin de la cadena debe volver a
      // leerse en el próximo llamado a aLex

      char cadenaAux[MAX_LONGITUD_CADENA + 3];
      // Más los apóstrofos y el cero final

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
      else if (strcmp(cadenaAux, "IF") == 0)
        a.simbolo = IF;
      else if (strcmp(cadenaAux, "ODD") == 0)
        a.simbolo = ODD;
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
      // Corta cuando c==EOF o cuando c no es dígito

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
        // Corta cuando c==EOF o c=='\n' o cuando c es un apóstrofo

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
  case FIN_DE_ARCHIVO:
    printf("FIN_DE_ARCHIVO");
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
  if (strlen(s) < MAX_LONGITUD_CADENA + 2) { // Si cabe uno mas
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
    printf("Error, se esperaba un ASIGNACION: %s\n", s.cadena);
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
    printf("Error semantico, se repite el identificador(?): %s\n", s.cadena);
    break;
  case 13:
    printf("Error semantico, se esperaba un VAR: %s\n", s.cadena);
    break;
  default:
    printf("Error generico\n");
    break;
  }
  exit(1);
}

// ########################## Analizador sintactico ##########################

tSimbolo programa(tSimbolo s, FILE *archivo) {
  // Declaramos tabla de identificadores para el analizador semantico
  tablaDeIdent tabla;

  s = bloque(s, archivo, tabla, 0);
  if (s.simbolo == PUNTO)
    s = aLex(archivo);
  else
    error(1, s);
  return s;
}

tSimbolo bloque(tSimbolo s, FILE *archivo, tablaDeIdent tabla, int base) {
  int desplazamiento = 0;

  if (s.simbolo == CONST) {
    int p;
    s = aLex(archivo);
    if (s.simbolo == IDENT) {
      p = buscarIdent(s.cadena, tabla, base, (base + desplazamiento - 1));
      if (p == -1) {
        tabla[base + desplazamiento].tipo = CONST;
        strcpy(tabla[base + desplazamiento].nombre, s.cadena);
      } else {
        error(12, s);
      }
      s = aLex(archivo);
    } else
      error(2, s);
    if (s.simbolo == IGUAL)
      s = aLex(archivo);
    else
      error(3, s);
    if (s.simbolo == NUMERO) {
      tabla[base + desplazamiento].valor = atoi(s.cadena);
      desplazamiento++;
      s = aLex(archivo);
    } else
      error(4, s);
    while (s.simbolo == COMA) {
      s = aLex(archivo);
      if (s.simbolo == IDENT)
        s = aLex(archivo);
      else
        error(2, s);
      if (s.simbolo == IGUAL)
        s = aLex(archivo);
      else
        error(3, s);
      if (s.simbolo == NUMERO)
        s = aLex(archivo);
      else
        error(4, s);
    }
    if (s.simbolo == PTOCOMA)
      s = aLex(archivo);
    else
      error(5, s);
  }


  if (s.simbolo == VAR) {
    int p;
    s = aLex(archivo);
    if (s.simbolo == IDENT) {
      p = buscarIdent(s.cadena, tabla, base, (base + desplazamiento - 1));
      if (p == -1) {
        tabla[base + desplazamiento].tipo = VAR;
        strcpy(tabla[base + desplazamiento].nombre, s.cadena);
        tabla[base + desplazamiento].valor = 0;
        desplazamiento++;
      } else {
        error(12, s);
      }

      s = aLex(archivo);
    } else
      error(2, s);
    while (s.simbolo == COMA) {
      s = aLex(archivo);
      if (s.simbolo == IDENT)
        s = aLex(archivo);
      else
        error(2, s);
    }

    if (s.simbolo == PTOCOMA)
      s = aLex(archivo);
    else
      error(5, s);
  }


  while (s.simbolo == PROCEDURE) {
    int p;
    s = aLex(archivo);
    if (s.simbolo == IDENT) {
      p = buscarIdent(s.cadena, tabla, base, (base + desplazamiento - 1));
      if (p == -1) {
        tabla[base + desplazamiento].tipo = PROCEDURE;
        strcpy(tabla[base + desplazamiento].nombre, s.cadena);
        tabla[base + desplazamiento].valor = 0;
        desplazamiento++;
      } else {
        error(12, s);
      }

      s = aLex(archivo);
    } else
      error(2, s);
    if (s.simbolo = PTOCOMA)
      s = aLex(archivo);
    else
      error(5, s);
    s = bloque(s, archivo, tabla, base + desplazamiento);
    if (s.simbolo == PTOCOMA)
      s = aLex(archivo);
    else
      error(5, s);
  }


  s = proposicion(s, archivo, tabla, (base + desplazamiento - 1));
  return s;
}

tSimbolo proposicion(tSimbolo s, FILE *archivo, tablaDeIdent tabla, int posUltimoIdent) {

  switch (s.simbolo) {
  case IDENT:
    s = aLex(archivo);
    if (s.simbolo == ASIGNACION) {
      s = aLex(archivo);
    } else
      error(6, s); // Se esperaba asignacion
    s = expresion(s, archivo, tabla, posUltimoIdent);
    break;
  case CALL:
    s = aLex(archivo);
    if (s.simbolo == IDENT)
      s = aLex(archivo);
    else
      error(2, s);
    break;
  case BEGIN:
    s = aLex(archivo);
    s = proposicion(s, archivo, tabla, posUltimoIdent);
    while (s.simbolo == PTOCOMA) {
      s = aLex(archivo);
      s = proposicion(s, archivo, tabla, posUltimoIdent);
    }
    if (s.simbolo == END)
      s = aLex(archivo);
    else
      error(7, s); // Se esperaba END
    break;
  case IF:
    s = aLex(archivo);
    s = condicion(s, archivo, tabla, posUltimoIdent);
    if (s.simbolo == THEN)
      s = aLex(archivo);
    else
      error(8, s); // Se esperaba THEN
    s = proposicion(s, archivo, tabla, posUltimoIdent);
    break;
  case WHILE:
    s = aLex(archivo);
    s = condicion(s, archivo, tabla, posUltimoIdent);
    if (s.simbolo == DO)
      s = aLex(archivo);
    else
      error(9, s); // Se esperaba DO
    s = proposicion(s, archivo, tabla, posUltimoIdent);
    break;
  case READLN:
    // int p;
    s = aLex(archivo);
    if (s.simbolo == ABREPAREN)
      s = aLex(archivo);
    else
      error(10, s); // Se esperaba ABREPAREN
    if (s.simbolo == IDENT) {
      int p = buscarIdent(s.cadena, tabla, 0, posUltimoIdent);
      if (p != -1) {
        if (tabla[p].tipo != VAR) {
          error(13, s);
        }
      }
      s = aLex(archivo);
    } else
      error(2, s); // Se esperaba IDENT
    while (s.simbolo == COMA) {
      s = aLex(archivo);
      if (s.simbolo == IDENT)
        s = aLex(archivo);
      else
        error(2, s); // Se esperaba IDENT
    }
    if (s.simbolo == CIERRAPAREN)
      s = aLex(archivo);
    else
      error(11, s); // Se esperaba CIERRAPAREN
    break;
  case WRITE:
    s = aLex(archivo);
    if (s.simbolo == ABREPAREN)
      s = aLex(archivo);
    else
      error(10, s);
    if (s.simbolo == CADENA)
      s = aLex(archivo);
    else
      s = expresion(s, archivo, tabla, posUltimoIdent);
    while (s.simbolo == COMA) {
      s = aLex(archivo);
      if (s.simbolo == CADENA)
        s = aLex(archivo);
      else
        s = expresion(s, archivo, tabla, posUltimoIdent);
    }
    // else expresion(s, archivo, posUltimoIdent);
    if (s.simbolo == CIERRAPAREN)
      s = aLex(archivo);
    else
      error(11, s); // Se esperaba CIERRAPAREN
    break;
  case WRITELN:
    // probar esto, medio dudoso y mal explicado
    s = aLex(archivo);
    if (s.simbolo == ABREPAREN) {
      s = aLex(archivo);
      if (s.simbolo == CADENA)
        s = aLex(archivo);
      else
        s = expresion(s, archivo, tabla, posUltimoIdent);
      while (s.simbolo == COMA) {
        s = aLex(archivo);
        if (s.simbolo == CADENA)
          s = aLex(archivo);
        else
          s = expresion(s, archivo, tabla, posUltimoIdent);
      }
      if (s.simbolo == CIERRAPAREN)
        s = aLex(archivo);
      else
        error(11, s); // Se esperaba CIERRAPAREN
    } else
      error(10, s);
  }
  return s;
}

tSimbolo condicion(tSimbolo s, FILE *archivo, tablaDeIdent tabla, int posUltimoIdent) {

  if (s.simbolo == ODD) {
    s = aLex(archivo);
    s = expresion(s, archivo, tabla, posUltimoIdent);
  } else {
    s = expresion(s, archivo, tabla, posUltimoIdent);
    switch (s.simbolo) {
    case IGUAL:
      s = aLex(archivo);
      break;
    case DISTINTO:
      s = aLex(archivo);
      break;
    case MAYOR:
      s = aLex(archivo);
      break;
    case MENOR:
      s = aLex(archivo);
      break;
    case MAYORIGUAL:
      s = aLex(archivo);
      break;
    case MENORIGUAL:
      s = aLex(archivo);
      break;
    default:
      error(12, s); // Se esperaba un operador relacional
    }
  }
  return s;
}

tSimbolo expresion(tSimbolo s, FILE *archivo, tablaDeIdent tabla, int posUltimoIdent) {

  if (s.simbolo == MAS)
    s = aLex(archivo);
  else if (s.simbolo = MENOS)
    s = aLex(archivo);
  s = termino(s, archivo, tabla, posUltimoIdent);
  while (s.simbolo == MAS || s.simbolo == MENOS) {
    s = aLex(archivo);
    s = termino(s, archivo, tabla, posUltimoIdent);
  }
  return s;
}

tSimbolo termino(tSimbolo s, FILE *archivo, tablaDeIdent tabla, int posUltimoIdent) {

  s = factor(s, archivo, tabla, posUltimoIdent);
  while (s.simbolo == POR || s.simbolo == DIVIDIDO) {
    aLex(archivo);
    s = factor(s, archivo, tabla, posUltimoIdent);
  }
  return s;
}

tSimbolo factor(tSimbolo s, FILE *archivo, tablaDeIdent tabla, int posUltimoIdent) {

  switch (s.simbolo) {
  case IDENT:
    s = aLex(archivo);
    break;
  case NUMERO:
    s = aLex(archivo);
    break;
  case ABREPAREN:
    s = aLex(archivo);
    s = expresion(s, archivo, tabla, posUltimoIdent);
    if (s.simbolo == CIERRAPAREN)
      s = aLex(archivo);
    else
      error(11, s); // Se esperaba CIERRAPAREN
    break;
  }
  return s;
}

int buscarIdent(char *id, tablaDeIdent tabla, int posPrimerIdent, int posUltimoIdent) {

  char cadenaAux[MAX_LONGITUD_CADENA + 3];
  strcpy(cadenaAux, id);
  uppercase(cadenaAux);
  // SIEMPRE lee de abajo para arriba, para encontrar la ultima definicion de la
  // variable
  // o procedimiento
  int i = posUltimoIdent;
  while (i >= posPrimerIdent && strcmp(cadenaAux, tabla[i].nombre) != 0)
    i--;
  return (i >= posPrimerIdent ? i : -1);
}
