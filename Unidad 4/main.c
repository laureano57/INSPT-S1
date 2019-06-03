#include <stdio.h>
#include <ctype.h>   /* Para usar isgraph, etc. */
#include <string.h>  /* Para usar strlen, etc. */

/*** CONSTANTES ***/

#define MAX_LONGITUD_CADENA 128  /* es a longitud neta, sin los apóstrofos ni el cero final */

/*** NUEVOS TIPOS ***/

typedef enum {
    NULO, 
    FIN_DE_ARCHIVO, 
    BEGIN,// DESDE ACA 
    CALL,
    CONST,
    DO,
    IF,
    ODD,
    PROCEDURE,
    THEN,
    WHILE,
    END,// HASTA ACA
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
    PTOCOMA, // Y DESDE ACA OTRA VEZ
    ASIGNACION,
    POR,
    DIVIDIDO,
    MAS,
    MENOS,
    IGUAL,
    MENOR,
    MENORIGUAL,
    MAYOR,
    MAYORIGUAL,
    DISTINTO
} tTerminal;

typedef struct {
    tTerminal simbolo;
    char cadena[MAX_LONGITUD_CADENA + 3];  /* más los apóstrofos y el cero final */
} tSimbolo;

/*** NUEVAS FUNCIONES (PROTOTIPOS) ***/

tSimbolo aLex(FILE*);

void imprimir(tSimbolo);

void concatenar(char*, char);

void uppercase(char*);

/*** MAIN ***/

int main(int argc, char* argv[]) {
    // if (argc != 2) {
    //     printf("Uso: lexer FUENTE.PL0\n");
    // } else {
        FILE* archivo;
        // archivo = fopen(argv[1], "r");
        archivo = fopen("ejemplo.pl0", "r");
        if (archivo == NULL) {
            printf("Error de apertura del archivo [%s].\n", argv[1]);
        } else {
            tSimbolo s;
            do {
                s = aLex(archivo);
                imprimir(s);
            } while(s.simbolo != FIN_DE_ARCHIVO);
            fclose(archivo);
        }
    // }
    return 0;
}

/*** NUEVAS FUNCIONES (DEFINICIONES) ***/

tSimbolo aLex(FILE* fp) {
    tSimbolo a;
    a.cadena[0] = '\0';
    char c;
    do {
        c = getc(fp);
    } while (c != EOF && !isgraph(c));  /* corta cuando c==EOF o cuando isgraph(c)==true */

    if (c == EOF){
        a.simbolo = FIN_DE_ARCHIVO;
    } else {
        concatenar(a.cadena, c);
        if (isalpha(c)) {
            do {
                c = getc(fp);
                if (isalpha(c) || isdigit(c)) concatenar(a.cadena, c);
            } while (c != EOF && (isalpha(c) || isdigit(c))); /* corta cuando c==EOF o cuando c no es letra ni dígito */
            ungetc(c, fp); /* el char que provocó el fin de la cadena debe volver a leerse en el próximo llamado a aLex */
            char cadenaAux[MAX_LONGITUD_CADENA + 3];  /* más los apóstrofos y el cero final */
            strcpy(cadenaAux, a.cadena);
            uppercase(cadenaAux);
            if (strcmp(cadenaAux, "BEGIN") == 0) a.simbolo = BEGIN;
            else if (strcmp(cadenaAux, "CALL") == 0) a.simbolo = CALL;
            else if (strcmp(cadenaAux, "CONST") == 0) a.simbolo = CONST;
            else if (strcmp(cadenaAux, "DO") == 0) a.simbolo = DO;
            else if (strcmp(cadenaAux, "IF") == 0) a.simbolo = IF;
            else if (strcmp(cadenaAux, "ODD") == 0) a.simbolo = ODD;
            else if (strcmp(cadenaAux, "PROCEDURE") == 0) a.simbolo = PROCEDURE;
            else if (strcmp(cadenaAux, "THEN") == 0) a.simbolo = THEN;
            else if (strcmp(cadenaAux, "WHILE") == 0) a.simbolo = WHILE;
            else if (strcmp(cadenaAux, "END") == 0) a.simbolo = END;
            else if (strcmp(cadenaAux, "READLN") == 0) a.simbolo = READLN;
            else if (strcmp(cadenaAux, "VAR") == 0) a.simbolo = VAR;
            else if (strcmp(cadenaAux, "WRITE") == 0) a.simbolo = WRITE;
            else if (strcmp(cadenaAux, "WRITELN") == 0) a.simbolo = WRITELN;
            else a.simbolo = IDENT;
        } else if (isdigit(c)) {
            do {
                c = getc(fp);
                if (isdigit(c)) concatenar(a.cadena, c);
            } while (c != EOF && isdigit(c)); /* corta cuando c==EOF o cuando c no es dígito */
            ungetc(c, fp); /* el char que provocó el fin de la cadena debe volver a leerse en el próximo llamado a aLex */
            a.simbolo = NUMERO;
        } else switch (c) {
            case '\'':
                do {
                    c = getc(fp);
                    if (c != EOF && c != '\n') concatenar(a.cadena, c);
                } while (c != EOF && c != '\n' && c != '\''); /* corta cuando c==EOF o c=='\n' o cuando c es un apóstrofo */
                if (c == EOF || c == '\n') {
                    a.simbolo = NULO;
                    ungetc(c, fp);
                } else a.simbolo = CADENA;
                break;
            case '.': a.simbolo = PUNTO;
                break;
            case ',': a.simbolo = COMA;
                break;
            case ';': a.simbolo = PTOCOMA;
                break;
            case '+': a.simbolo = MAS;
                break;
            case '(': a.simbolo = ABREPAREN;
                break;
            case ')': a.simbolo = CIERRAPAREN;
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
                }
                else {
                    ungetc(c, fp);
                    a.simbolo = MAYOR;
                }
                break;
            case '=': a.simbolo = IGUAL;
                break;
            case ':': 
                c = getc(fp);
                if (c == '=') a.simbolo = ASIGNACION;
                else{ 
                    ungetc(c, fp);
                    a.simbolo = NULO;
                }
                break;
            // case '<': a.simbolo = MENOR;
            //     break;
            // case '<': a.simbolo = MENOR;
            //     break;
            // case '<': a.simbolo = MENOR;
            //     break;
            // case '<': a.simbolo = MENOR;
            //     break;
            // case '<': a.simbolo = MENOR;
            //     break;
            // case '<': a.simbolo = MENOR;
            //     break;
            default: a.simbolo = NULO;
        }
    }
    return a;
}

void imprimir(tSimbolo simb) {
    printf("Cadena leida: \"%s\"\t Simbolo correspondiente: ", simb.cadena);
    switch(simb.simbolo){
        case NULO: printf("NULO");
            break;
        case FIN_DE_ARCHIVO: printf("FIN_DE_ARCHIVO");
            break;
        case CALL: printf("CALL");
            break;
        case CONST: printf("CONST");
            break;
        case DO: printf("DO");
            break;
        case IF: printf("IF");
            break;
        case ODD: printf("ODD");
            break;
        case PROCEDURE: printf("PROCEDURE");
            break;
        case THEN: printf("THEN");
            break;
        case WHILE: printf("WHILE");
            break;
        case NUMERO: printf("NUMERO");
            break;
        case BEGIN: printf("BEGIN");
            break;
        case END: printf("END");
            break;
        case READLN: printf("READLN");
            break;
        case WRITE: printf("WRITE");
            break;
        case WRITELN: printf("WRITELN");
            break;
        case VAR: printf("VAR");
            break;
        case CADENA: printf("CADENA");
            break;
        case IDENT: printf("IDENT");
            break;
        case PUNTO: printf("PUNTO");
            break;
        case COMA: printf("COMA");
            break;
        case ABREPAREN: printf("ABREPAREN");
            break;
        case CIERRAPAREN: printf("CIERRAPAREN");
            break;
        case PTOCOMA: printf("PTOCOMA");
            break;
        case ASIGNACION: printf("ASIGNACION");
            break;
        case POR: printf("POR");
            break;
        case DIVIDIDO: printf("DIVIDIDO");
            break;
        case MAS: printf("MAS");
            break;
        case MENOS: printf("MENOS");
            break;
        case IGUAL: printf("IGUAL");
            break;
        case MENOR: printf("MENOR");
            break;
        case MENORIGUAL: printf("MENORIGUAL");
            break;
        case MAYOR: printf("MAYOR");
            break;
        case MAYORIGUAL: printf("MAYORIGUAL");
            break;
        case DISTINTO: printf("DISTINTO");
            break;
    }
    printf ("\n");
}

void concatenar(char* s, char c) {
    if (strlen(s) < MAX_LONGITUD_CADENA + 2){  /* si cabe uno más */
        for (; *s; s++);
        *s++ = c;
        *s++ = '\0';
    }
}

void uppercase(char* s) {
    for (; *s; s++)
        *s = toupper(*s);
}
