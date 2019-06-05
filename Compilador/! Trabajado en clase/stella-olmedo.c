#include <stdio.h>
#include <ctype.h>  /* Para usar isgraph, etc. */
#include <string.h> /* Para usar strlen, etc. */

/*** CONSTANTES ***/

#define MAX_LONGITUD_CADENA 128 /* es a longitud neta, sin los apóstrofos ni el cero final */

/*** NUEVOS TIPOS ***/

typedef enum { NULO,
               FIN_DE_ARCHIVO,
               BEGIN,
               END,
               ABREPAREN,
               CIERRAPAREN,
               READLN,
               WRITE,
               WRITELN,
               VAR,
               CADENA,
               IDENT,
               NUMERO,
               PUNTO,
               COMA,
               PTOCOMA,
               MAS,
               WHILE,
               DO,
               CALL,
               PROCEDURE,
               ODD,
               THEN,
               IF,
               EOF,
               CONST,
               MENOROIGUAL,
               DISTINTO } tTerminal;

typedef struct
{
  tTerminal simbolo;
  char cadena[MAX_LONGITUD_CADENA + 3]; /* más los apóstrofos y el cero final */
} tSimbolo;

/*** NUEVAS FUNCIONES (PROTOTIPOS) ***/

tSimbolo aLex(FILE *);

void imprimir(tSimbolo);

void concatenar(char *, char);

void uppercase(char *);

void error(int, char *);

/*** ANALISIS SINTACTICO ***/
void programa(tSimbolo, FILE *);

void bloque(tSimbolo, FILE *);
void proposicion(tSimbolo, FILE *);
void condicion(tSimbolo, FILE *);
void expresion(tSimbolo, FILE *);
void termino(tSimbolo, FILE *);
void factor(tSimbolo, FILE *);

/*** MAIN ***/

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printf("Uso: lexer FUENTE.PL0\n");
  }
  else
  {
    FILE *archivo;
    archivo = fopen(argv[1], "r");
    if (archivo == NULL)
    {
      printf("Error de apertura del archivo [%s].\n", argv[1]);
    }
    else
    {
      tSimbolo s;
      /*do {
                s = aLex(archivo);
                imprimir(s);
            } while(s.simbolo != FIN_DE_ARCHIVO);*/
      s = aLex(archivo);
      programa(s, archivo);
      if (s == EOF)
      {
        printf("Succedd");
      }
      else
      {
        error(1, s.cadena);
      }
      fclose(archivo);
    }
  }
  return 0;
}

/*** NUEVAS FUNCIONES (DEFINICIONES) ***/

tSimbolo aLex(FILE *fp)
{
  tSimbolo a;
  a.cadena[0] = '\0';
  char c;
  do
  {
    c = getc(fp);
  } while (c != EOF && !isgraph(c)); /* corta cuando c==EOF o cuando isgraph(c)==true */

  if (c == EOF)
  {
    a.simbolo = FIN_DE_ARCHIVO;
  }
  else
  {
    concatenar(a.cadena, c);
    if (isalpha(c))
    { //sale cuando es 0, osea false
      do
      {
        c = getc(fp);
        if (isalpha(c) || isdigit(c))
          concatenar(a.cadena, c);
      } while (c != EOF && (isalpha(c) || isdigit(c))); /* corta cuando c==EOF o cuando c no es letra ni dígito */
      ungetc(c, fp);                                    /* el char que provocó el fin de la cadena debe volver a leerse en el próximo llamado a aLex */
      char cadenaAux[MAX_LONGITUD_CADENA + 3];          /* más los apóstrofos y el cero final */
      strcpy(cadenaAux, a.cadena);
      uppercase(cadenaAux);
      if (strcmp(cadenaAux, "BEGIN") == 0)
        a.simbolo = BEGIN;
      else if (strcmp(cadenaAux, "END") == 0)
        a.simbolo = END;
      else if (strcmp(cadenaAux, "READLN") == 0)
        a.simbolo = READLN;
      else if (strcmp(cadenaAux, "VAR") == 0)
        a.simbolo = VAR;
      else if (strcmp(cadenaAux, "WRITE") == 0)
        a.simbolo = WRITE;
      else if (strcmp(cadenaAux, "WRITELN") == 0)
        a.simbolo = WRITELN;
      else if (strcmp(cadenaAux, "CALL") == 0)
        a.simbolo = CALL;
      else if (strcmp(cadenaAux, "DO") == 0)
        a.simbolo = DO;
      else if (strcmp(cadenaAux, "WHILE") == 0)
        a.simbolo = WHILE;
      else if (strcmp(cadenaAux, "IF") == 0)
        a.simbolo = IF;
      else if (strcmp(cadenaAux, "THEN") == 0)
        a.simbolo = THEN;
      else if (strcmp(cadenaAux, "PROCEDURE") == 0)
        a.simbolo = PROCEDURE;
      else if (strcmp(cadenaAux, "CONST") == 0)
        a.simbolo = CONST;
      else if (strcmp(cadenaAux, "ODD") == 0)
        a.simbolo = ODD;
      else if (strcmp(cadenaAux, "EOF") == 0)
        a.simbolo = EOF;
      else
        a.simbolo = IDENT;
    }
    else if (isdigit(c))
    {
      do
      {
        c = getc(fp);
        if (isdigit(c))
          concatenar(a.cadena, c);
      } while (c != EOF && isdigit(c)); /* corta cuando c==EOF o cuando c no es dígito */
      ungetc(c, fp);                    /* el char que provocó el fin de la cadena debe volver a leerse en el próximo llamado a aLex */
      a.simbolo = NUMERO;
    }
    else
      switch (c)
      {
      case '\'':
        do
        {
          c = getc(fp);
          if (c != EOF && c != '\n')
            concatenar(a.cadena, c);
        } while (c != EOF && c != '\n' && c != '\''); /* corta cuando c==EOF o c=='\n' o cuando c es un apóstrofo */
        if (c == EOF || c == '\n')
        {
          a.simbolo = NULO;
          ungetc(c, fp);
        }
        else
          a.simbolo = CADENA;
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
      case '(':
        a.simbolo = ABREPAREN;
        break;
      case ')':
        a.simbolo = CIERRAPAREN;
        break;
      case '<':
        //do {
        c = getc(fp);
        if (c != EOF && c != '\n')
          concatenar(a.cadena, c);
        // {while (c != EOF && c != '=' && c != '>'); /* corta cuando c==EOF o c=='\n' o cuando c es un apóstrofo */
        if (c != EOF && c == '=')
        {
          a.simbolo = MENOROIGUAL;
          //ungetc(c, fp);
        }
        else if (c != EOF && c == '>')
          a.simbolo = DISTINTO;
        else
          a.simbolo = MENOR; // ?? NULO tal vez
        break;
      case '>':
        //do {
        c = getc(fp);
        if (c != EOF && c != '\n')
          concatenar(a.cadena, c);
        //} //while (c != EOF && c != '=' && c != '\n'); /* corta cuando c==EOF o c=='\n' o cuando c es un apóstrofo */
        if (c != EOF && c == '=')
        {
          a.simbolo = MAYOROIGUAL;
        }
        else
        {
          ungetc(c, fp);
          a.simbolo = MAYOR;
        }
        break;
      case ':':
        do
        {
          c = getc(fp);
          if (c != EOF && c != '\n')
            concatenar(a.cadena, c);
        } //while (c != EOF && c != '=' && c != '<'); /* corta cuando c==EOF o c=='\n' o cuando c es un apóstrofo */
        if (c != EOF && c == '=')
        {
          a.simbolo = ;
        }
        else
          a.simbolo = MAYOR;
        ungetc(c, fp) // ??? NULO tal vez
            break;

      default:
        a.simbolo = NULO;
      }
  }
  return a;
}

void imprimir(tSimbolo simb)
{
  printf("Cadena leida: \"%s\"\t Simbolo correspondiente: ", simb.cadena);
  switch (simb.simbolo)
  {
  case NULO:
    printf("NULO");
    break;
  case FIN_DE_ARCHIVO:
    printf("FIN_DE_ARCHIVO");
    break;
  case NUMERO:
    printf("NUMERO");
    break;
  case BEGIN:
    printf("BEGIN");
    break;
  case END:
    printf("END");
    break;
  case READLN:
    printf("READLN");
    break;
  case WRITE:
    printf("WRITE");
    break;
  case WRITELN:
    printf("WRITELN");
    break;
  case VAR:
    printf("VAR");
    break;
  case CADENA:
    printf("CADENA");
    break;
  case IDENT:
    printf("IDENT");
    break;
  case PUNTO:
    printf("PUNTO");
    break;
  case COMA:
    printf("COMA");
    break;
  case ABREPAREN:
    printf("ABREPAREN");
    break;
  case CIERRAPAREN:
    printf("CIERRAPAREN");
    break;
  case PTOCOMA:
    printf("PTOCOMA");
    break;
  case MAS:
    printf("MAS");
    break;
  }
  printf("\n");
}

void concatenar(char *s, char c)
{
  if (strlen(s) < MAX_LONGITUD_CADENA + 2)
  { /* si cabe uno más */
    for (; *s; s++)
      ;
    *s++ = c;
    *s++ = '\0';
  }
}

void uppercase(char *s)
{
  for (; *s; s++)
    *s = toupper(*s);
}

void programa(tSimbolo s, FILE *x)
{
  bloque(s, x);

  if (s.simbolo == PUNTO)
    s = aLex(x);
  else
    error(0, s.cadena);
} //FIN PROGRAMA

void bloque(tSimbolo s, FILE *x)
{
  if (s.simbolo == CONST)
  {
    s = aLex(x);
    if (s.simbolo == IDENT)
      s = aLEX(x);
    else
      error(2, s.cadena);
    if (s.simbolo == IGUAL)
      s = aLex(x);
    else
      error(3, s.cadena);
    if (s.simbolo == NUMERO)
      s = aLex(x);
    else
      error(4, s.cadena);
  }
  while (s.simbolo == COMA)
  {
    s = aLex(x);
    if (s.simbolo == IDENT)
      s = aLex(x);
    else
      error(2, s.cadena);
    if (s.simbolo == IGUAL)
      s = aLex(x);
    else
      error(3, s.cadena);
    if (s.simbolo == NUMERO)
      s = aLex(x);
    else
      error(4, s.cadena);
  } // FIN DEL WHILE con COMA
  if (s.simbolo == PTOCOMA)
    s = aLex();
  else
    error(5, s.cadena);

  if (s.simbolo == VAR)
  {
    s = aLex(x);
    if (s.simbolo == IDENT)
      s = aLex;
    else
      error(2, s.cadena);

    while (s.simbolo == COMA)
    {
      s = aLex(x);
      if (s.simbolo == IDENT)
        s = aLex;
      else
        error(2, s.cadena);
    } //fin del while COMA
    if (s.simbolo == PTOCOMA)
      s = aLex(x);
    else
      error(5, s.cadena);
  } // FIN DEL VAR

  while (s.simbolo == PROCEDURE)
  {
    s = aLex();
    if (s.simbolo == IDENT)
      s = aLex(x);
    else
      error(2, s.cadena);
    if (s.simbolo = PTOCOMA)
      s = aLex(x);
    else
      error(5, s.cadena);
    bloque(s, x);
    if (s.simbolo == PTOCOMA)
      s = aLex(x);
    else
      error(6, s.cadena);
  } //fin while PROCEDURE

  proposicion(s, x);
} // FIN BLOQUE

void proposicion(tSimbolo s, FILE *x)
{
  switch (s.simbolo)
  {
  case IDENT:
    s = aLex;
    if (s.simbolo == ASIGNACION)
      s = aLex;
    else
    {
      error(6, s.cadena);
    }
    expresion(s, x);
    break;
  case CALL:
    S = aLex(x);
    if (s.simbolo == IDENT)
      s = aLex(x);
    else
      error(2, s.cadena);
    break;
  case BEGIN:
    s = aLex(x);
    proposicion(s, x);
    while (s.simbolo == PTOCOMA)
    {
      s = aLex(x);
      proposicion(s, x);
    } //fin while PTCOMA
    if (s.simbolo == END)
      s = aLex(x);
    else
      error(7, s.cadena);
    break;
  case IF:
    s = aLex(x);
    condicion(s, x);
    if (s.simbolo == THEN)
      s = aLex(x);
    else
      error(8, s.cadena);
    proposicion(s, x);
    break;
  case WHILE:
    s = aLex(x);
    condicion(s, x);
    if (s.simbolo == DO)
      s = aLex(x);
    else
      error(8, s.cadena);
    proposicion(s, x);
    break;
  case READLN:
    s = aLex(x);
    if (s.simbolo == APAREN)
      s = aLex(x);
    else
      error(10, s.cadena);
    if (s.simbolo == IDENT)
      s = aLex(x);
    else
      error(2, s.cadena);
        while(s.simbolo==COMA{
      s = aLex(x);
      if (s.simbolo == IDENT)
        s = aLex(x);
      else
        error(2, s.cadena);
              }//fin del WHILE IDENNT
              if(s.simbolo==CPARENT)s=aLex(x);
              else error(11, s.cadena);
        break;
    case WRITE:
        s=aLex(x);
        if (s.simbolo == APAREN)
        s=aLex(x);
        else error(10, s.cadena);
        if(s.simbolo== CADENA)
        s=aLex(x);
        else expresion(s, x);
        while (s.simbolo==COMA){
      s = aLex(x);
      if (s.simbolo == CADENA)
        s = aLex(x);
      else
        expresion(s, x);}
        if (s.simbolo==CPAREN)
            s=aLex(x);
        else error(11, s.cadena);
        break;
    case WRITELN:
       s=aLex(x);
        if (s.simbolo == APAREN){
      s = aLex(x);
      if (s.simbolo == CADENA)
        s = aLex(x);
      else
        expresion(s, x);
      while (s.simbolo == COMA)
      {
        s = aLex(x);
        if (s.simbolo == CADENA)
          s = aLex(x);
        else
          expresion(s, x);
      }
      if (s.simbolo == CPAREN)
        s = aLex(x);
      else
        error(11, s.cadena);}
        break;
  } //fin del CASE

} // FIN PROPOSICION

void condicion(tSimbolo s, FILE *x)
{
  if (s.simbolo == ODD)
  {
    s = aLex(x);
    expresion(s, x)
  }
  else
  {
    expresion(s, x);
    switch (s.simbolo)
    case IGUAL:
    s = aLex(x);
    break;
  default:
    error(12, s.cadena);
  }
  expresion(s, x);

} // FIN CONDICION

void expresion(tSimbolo s, FILE *x)
{
  if (s.simbolo == MAS)
    s = aLex(x);
  else if (s.simbolo == MENOS)
    s = aLex(x);
  termino(s, x);
  while (s.simbolo == MAS || s.simbolo == MENOS)
  {
    s = aLex(x);
    termino(s, x);
  }
} // FIN EXPRESION

void termino(tSimbolo s, FILE *x)
{
  factor(s, x);
  while (s.simbolo == POR || s.simbolo == DIVIDIDO)
  {

    s = aLex(x);
    factor(s.x);
  } //fin WHILE POR o DIVIDIDO

} // FIN TERMINO

void factor(tSimbolo s, FILE *x)
{
  switch (s.simbolo)
  {
  case IDENT:
    s = aLex(x);
    break;
  case NUMERO:
    s = aLex(x);
    break;
  case APAREN:
    s = aLex(x);
    expresion(s, x);
    if (s.simbolo == CPAREN)
      s = aLex(x);
    else
      error(11, s.cadena);
  }

} // FIN FACTOR

void error(int e, char cad)
{
  switch (e)
  {
  case 0:
    printf("Error se esperaba un punto en vez de %s", &cad);
    break;
  case 1:
    printf("Archivo invalido, despues del punto se leyo %s", &cad);
    break;
  case 2:
    printf("Se esperaba IDENT en vez de %s", &cad);
    break;
  case 3:
    printf("Se esperaba IGUAL en vez de %s", &cad);
    break;
  case 5:
    pritf("Error, se eperaba punto y coma en vez de %s", &cad);
    break;
  case 10:
    printf("Error, se esperarba abrir parentesis en vez de %s", &cad);
    break;
  case 11:
    printf("Error, se esperarba cerrar parentesis en vez de %s", &cad);
    break;
  }
  exit(0);
}