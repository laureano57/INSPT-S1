; Se ingresa una cadena, intercambia mayusculas por minusculas
;
;
; En Windows (1 en la consola de NASM; 2 y 3 en la consola de Visual Studio):
; 1) nasm -f win32 ej3.asm --PREFIX _
; 2) link /out:ej3.exe ej3.obj libcmt.lib
; 3) ej3
;
; En Windows (1 en la consola de NASM; 2 y 3 en la consola de Windows, dentro de la carpeta [ajustando los nros. de version]: C:\Qt\Qt5.3.1\Tools\mingw482_32\bin ):
; 1) nasm -f win32 ej3.asm --PREFIX _
; 2) gcc ej3.obj -o ej3.exe
; 3) ej3
;
; En GNU/Linux:
; 1) nasm -f elf ej3.asm
; 2) ld -s -o ej3 ej3.o -lc -I /lib/ld-linux.so.2
; 3) ./ej3
;
; En GNU/Linux de 64 bits (Previamente, en Ubuntu, hay que ejecutar: sudo apt-get install libc6-dev-i386):
; 1) nasm -f elf ej3.asm
; 2) ld -m elf_i386 -s -o ej3 ej3.o -lc -I /lib/ld-linux.so.2
; 3) ./ej3


        global main              ; ETIQUETAS QUE MARCAN EL PUNTO DE INICIO DE LA EJECUCION
        global _start

        extern printf            ;
        extern scanf             ; FUNCIONES DE C (IMPORTADAS)
        extern exit              ;
        extern gets              ; GETS ES MUY PELIGROSA. SOLO USARLA EN EJERCICIOS BASICOS, JAMAS EN EL TRABAJO!!!



section .bss                     ; SECCION DE LAS VARIABLES

numero:
        resd    1                ; 1 dword (4 bytes)

cadena:
        resb    0x0100           ; 256 bytes

caracter:
        resb    1                ; 1 byte (dato)
        resb    3                ; 3 bytes (relleno)



section .data                    ; SECCION DE LAS CONSTANTES

fmtInt:
        db    "%d", 0            ; FORMATO PARA NUMEROS ENTEROS

fmtString:
        db    "%s", 0            ; FORMATO PARA CADENAS

fmtChar:
        db    "%c", 0            ; FORMATO PARA CARACTERES

fmtLF:
        db    0xA, 0             ; SALTO DE LINEA (LF)



section .text                    ; SECCION DE LAS INSTRUCCIONES

leerCadena:                      ; RUTINA PARA LEER UNA CADENA USANDO GETS
        push cadena
        call gets
        add esp, 4
        ret

leerNumero:                      ; RUTINA PARA LEER UN NUMERO ENTERO USANDO SCANF
        push numero
        push fmtInt
        call scanf
        add esp, 8
        ret

mostrarCadena:                   ; RUTINA PARA MOSTRAR UNA CADENA USANDO PRINTF
        push cadena
        push fmtString
        call printf
        add esp, 8
        ret

mostrarNumero:                   ; RUTINA PARA MOSTRAR UN NUMERO ENTERO USANDO PRINTF
        push dword [numero]
        push fmtInt
        call printf
        add esp, 8
        ret

mostrarCaracter:                 ; RUTINA PARA MOSTRAR UN CARACTER USANDO PRINTF
        push dword [caracter]
        push fmtChar
        call printf
        add esp, 8
        ret

mostrarSaltoDeLinea:             ; RUTINA PARA MOSTRAR UN SALTO DE LINEA USANDO PRINTF
        push fmtLF
        call printf
        add esp, 4
        ret

salirDelPrograma:                ; PUNTO DE SALIDA DEL PROGRAMA USANDO EXIT
        push 0
        call exit

_start:
main:                            ; PUNTO DE INICIO DEL PROGRAMA
        call leerCadena
        mov edi,0
        mov eax, 0
seguir:
        mov al,[edi + cadena]       ; muevo a 'al' el primero de los caracteres de la cadena leida
        cmp al,0                    ; Comparo con 0 (por si ya leyo toda la cadena), si es igual salto al fin de programa
        je finPrograma
        cmp al, 65                  ; Compara con una 'A', si es menor salta a 'dejar' (no es letra)
        jb dejar                    ; Jump if below
        cmp al, 122                 ; Compara con una 'z', si es mayor salta a 'dejar' (no es letra)
        ja dejar                    ; Jump if above
        cmp al, 91                  ; Si es mayuscula, la hace minuscula
        jb minus                    ; Salta a minuscula
        cmp al, 96                  ; Caso contrario, la hace mayuscula
        ja mayus                    ; Hace mayuscula
minus:
        add al, 32                  ; Suma 32 y salta a dejar
        jmp dejar
mayus:
        sub al, 32                  ; Resta 32 y salta a dejar
dejar:
        mov [caracter], al          ; Mueve la letra a la variable caracter
        call mostrarCaracter        ; Muestra por pantalla el caracter
        inc edi                     ; Apunta a la siguiente letra del string
        jmp seguir                  ; Sigue con seguir
finPrograma:
        call mostrarSaltoDeLinea
        jmp salirDelPrograma
