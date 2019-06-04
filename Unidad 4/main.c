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

/*** Funciones para el analizador sintactico ***/

void programa(tSimbolo, FILE * archivo);
void bloque(tSimbolo, FILE * archivo);
void proposicion(tSimbolo, FILE * archivo);
void condicion(tSimbolo, FILE * archivo);
void expresion(tSimbolo, FILE * archivo);
void termino(tSimbolo, FILE * archivo);
void factor(tSimbolo, FILE * archivo);

/*** MAIN ***/

int main(int argc, char* argv[]) {
    // if (argc != 2) {
    //     printf("Uso: lexer FUENTE.PL0\n");
    // } else {
        FILE* archivo;
        // archivo = fopen(argv[1], "r");
        archivo = fopen("ejemplo.pl0", "r");    // Harcodea nombre de archivo
        if (archivo == NULL) {
            printf("Error de apertura del archivo [%s].\n", argv[1]);
        } else {
            tSimbolo s;
            s = aLex(archivo);
            programa(s, archivo);
            if (s.simbolo == FIN_DE_ARCHIVO) {
                printf("Archivo procesado exitosamente\n");
            } else {
                // error(algo, algo);
                printf("Error: sobra algo despues del punto\n");
            }
            // Para probar el analizador lexico
            // do {
            //     s = aLex(archivo);
            //     imprimir(s);
            // } while(s.simbolo != FIN_DE_ARCHIVO);
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

void error(int codigo, tSimbolo s) {
    switch(codigo) {
        case 0:
            printf("Error, %s\n", s.cadena);
            break;
        case 1:
            printf("Error, se esperaba un IDENT: %s\n", s.cadena);
            break;
        case 2:
            printf("Error, se esperaba un IGUAL: %s\n", s.cadena);
            break;
        case 3:
            printf("Error, se esperaba un NUMERO: %s\n", s.cadena);
            break;
        case 4:
            printf("Error, se esperaba un PTOCOMA: %s\n", s.cadena);
            break;
        // COMPLETAR TODOS LOS CASOS DE ERROR!
        // case 1:
        //     printf("Error, se esperaba un IDENT: %s\n", s.cadena);
        //     break;
        // case 1:
        //     printf("Error, se esperaba un IDENT: %s\n", s.cadena);
        //     break;
        // case 1:
        //     printf("Error, se esperaba un IDENT: %s\n", s.cadena);
        //     break;
        // case 1:
        //     printf("Error, se esperaba un IDENT: %s\n", s.cadena);
        //     break;
        // case 1:
        //     printf("Error, se esperaba un IDENT: %s\n", s.cadena);
        //     break;
        // case 1:
        //     printf("Error, se esperaba un IDENT: %s\n", s.cadena);
        //     break;
        default:
            printf("Error generico\n");
            break;
    }
}

/*** Analizador sintactico ***/

void programa(tSimbolo s, FILE *archivo) {
    bloque(s, archivo);
    if (s.simbolo == PUNTO) s = aLex(archivo);
    else error(0, s);
}

void bloque(tSimbolo s, FILE *archivo) {
    if (s.simbolo == CONST) {
        s = aLex(archivo);
        if (s.simbolo == IDENT) s = aLex(archivo);
        else error(1, s);
        if (s.simbolo == IGUAL) s = aLex(archivo);
        else error(2, s);
        if (s.simbolo == NUMERO) s = aLex(archivo);
        else error(3, s);
        while (s.simbolo == COMA) {
            s = aLex(archivo);
            if (s.simbolo == IDENT) s = aLex(archivo);
            else error(1, s);
            if (s.simbolo == IGUAL) s = aLex(archivo);
            else error(2, s);
            if (s.simbolo == NUMERO) s = aLex(archivo);
            else error(3, s);
        }
        if (s.simbolo == PTOCOMA) s = aLex(archivo);
        else error(4, s);
    }
    if (s.simbolo == VAR) {
        s = aLex(archivo);
        if (s.simbolo == IDENT) s = aLex(archivo);
        else error(1, s);
        while (s.simbolo == COMA) {
            if (s.simbolo == IDENT) s = aLex(archivo);
            else error(1, s);
        }
        if (s.simbolo == PTOCOMA) s = aLex(archivo);
        else error(4, s);
    }
    while (s.simbolo == PROCEDURE) {
        s = aLex(archivo);
        if (s.simbolo == IDENT) s = aLex(archivo);
        else 
    }
    proposicion(s, archivo);
}

void proposicion(tSimbolo s, FILE *archivo) {
    switch(s.simbolo){
    case IDENT:
        s = aLex(archivo);
        if (s.simbolo == ASIGNACION) {
            s = aLex(archivo);
        }
        else error(6, s); // Se esperaba asignacion
        expresion(s, archivo);
        break;
    case CALL:
        s = aLex(archivo);
        if (s.simbolo == IDENT)
            s = aLex(archivo);
        else error(2, s);
        break;
    case BEGIN:
        s = aLex(archivo);
        proposicion(s, archivo);
        while (s.simbolo == PTOCOMA) {
            s = alex;
            proposicion(s, archivo);
        }
        if (s.simbolo == END) {
            s = aLex(archivo);
            else error(7, s); // Se esperaba END
        }
        break;
    case IF:
        s = aLex(archivo);
        condicion(s, archivo);
        if (s.simbolo == THEN)
            s = aLex(archivo);
        else error(8, s); // Se esperaba THEN
        proposicion(s, archivo);
        break;
    case WHILE:
        s = aLex(archivo);
        condicion(s, archivo);
        if (s.simbolo == DO)
            s = aLex(archivo);
        else error(9, s); // Se esperaba DO
        proposicion(s, archivo);
        break;
    case READLN:
        s = aLex(archivo);
        if (s.simbolo == ABREPAREN)
            s = aLex(archivo);
        else error(10, s); // Se esperaba ABREPAREN
        if (s.simbolo == IDENT)
            s = aLex(archivo);
        else error(2, s); // Se esperaba IDENT
        while (s.simbolo == COMA) {
            s = aLex(archivo);
            if (s.simbolo == IDENT)
                s = aLex(archivo);
            else error(2, s); // Se esperaba IDENT
        }
        if (s.simbolo == CIERRAPAREN)
            s = aLex(archivo);
        else error(11, s); // Se esperaba CIERRAPAREN
        break;
    case WRITE:
        s = aLex(archivo);
        if (s.simbolo == ABREPAREN)
            s = aLex(archivo);
        else error(10, s);
        if (s.simbolo == CADENA)
            s = aLex(archivo);
        else expresion(s, archivo);
        while (s.simbolo == COMA) {
            s = aLex(archivo);
            if (s.simbolo == CADENA)
                s = aLex(archivo);
            else expresion(s, archivo);
        }
        else expresion(s, archivo);
        if (s.simbolo == CIERRAPAREN)
            s = aLex(archivo);
        else error(11, s); // Se esperaba CIERRAPAREN
        break;
    case WRITELN:
        // probar esto, medio dudoso y mal explicado
        s = aLex(archivo);
        if (s.simbolo == ABREPAREN) {
            s = aLex(archivo);
            if (s.simbolo == CADENA)
                s = aLex(archivo);
            else expresion(s, archivo);
            while (s.simbolo == COMA) {
                s = aLex(archivo);
                if (s.simbolo == CADENA)
                    s = aLex(archivo);
                else expresion(s, archivo);
            }
            else expresion(s, archivo);
            if (s.simbolo == CIERRAPAREN)
                s = aLex(archivo);
            else error(11, s); // Se esperaba CIERRAPAREN
        }
    }
}

void condicion(tSimbolo s, FILE *archivo) {
    if (s.simbolo == ODD) {
         s = aLex(archivo);
         expresion(s, archivo);
    } else {
        expresion(s, archivo);
        switch(s.simbolo) {
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
}

void expresion(tSimbolo s, FILE *archivo) {
    if (s.simbolo == MAS) s = aLex(archivo);
    else if (s.simbolo = MENOS) s = aLex(archivo);
    termino(s, archivo);
    while (s.simbolo == MAS || s.simbolo == MENOS) {
        s = aLex(archivo);
        termino(s, archivo);
    }
}

void termino(tSimbolo s, FILE *archivo) {
    factor(s, archivo);
    while (s.simbolo == POR || s.simbolo == DIVIDIDO) {
        aLex(archivo);
        factor(s, archivo);
    }
}

void factor(tSimbolo s, FILE *archivo) {
    switch(s.simbolo) {
        case IDENT:
            s = aLex(archivo);
            break;
        case NUMERO:
            s = aLex(archivo);
            break;
        case ABREPAREN:
            s = aLex(archivo);
            expresion(s, archivo);
            if (s.simbolo == CIERRAPAREN)
                s = aLex(archivo);
            else error(11, s); // Se esperaba CIERRAPAREN
            break;
    }
}


// Al terminar el switch de la funcion error, meter exit(0); para detener ejecucion

