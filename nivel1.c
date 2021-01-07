//AUTORS: Miquel Vidal Cortés i Joan López Ferrer

#define _POSIX_C_SOURCE 200112L
#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64
#define PROMPT '$'
#define AMARILLO_T "\x1b[33m"
#define VERDE_T "\x1b[32m"
#define BLANCO_T "\x1b[37m"
#define VERMELL_T "\x1b[31m"
#define RESET_COLOR "\x1b[0m"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
void imprimir_prompt()
{
    char *PWD = getenv("PWD");
    char *USER = getenv("USER");

    printf(VERMELL_T "%s" VERDE_T ":~" AMARILLO_T "%s-SOI" BLANCO_T "%c " RESET_COLOR, USER, PWD, PROMPT);
}

//imprimir el prompt i llegir una linia de codi amb char *fgets(char *str, int n, sream FILE*)
char *read_line(char *line)
{
    //imprimir pront
    imprimir_prompt();
    //emmagatzamar el comando introduit per teclat dins line
    fgets(line, COMMAND_LINE_SIZE, stdin);
    //buidam el buffer
    fflush(stdin);

    return line;
}
//divideix la linia passada per paràmetre en fragments
int parse_args(char **args, char *line)
{
    //declaracions
    int counter = 0;
    const char s[2] = " \n";
    char *token;
    token = strtok(line, s);
    //assignam a la posicio counter-èssima el valor de token
    args[counter] = token;

    while (token != NULL)
    {
        if (token[0] == '#')
        {
            printf("error");
            break;
        }
        else
        {
            //imprimim per pantalla el valor de counter i de token
            printf("Parse_args()-->token %d : %s \n", counter, token);
            //incrementam countre amb una unitat
            counter++;
            token = strtok(NULL, s);
            //assignam a la posicio counter-èssima el valor de token
            args[counter] = token;
        }
    }
    //imprimim per pantalla el valor de counter i de token
    printf("Parse_args()-->token %d : %s \n", counter, token);

    return counter;
}

int internal_cd(char **args)
{
    printf("Para cambiar de directorio actual de trabajo \n");
    return 0;
}

int internal_export(char **args)
{
    printf("Para asignar variables de entorno \n");
    return 0;
}

int internal_source(char **args)
{
    printf("Ejecuta las líneas de comandos del fichero nombre \n");
    return 0;
}

int internal_jobs(char **args)
{
    printf("Muestra el PID de los procesos que se están ejecutando en background \n");
    return 0;
}

int internal_fg(char **args)
{
    printf("Envía un trabajo del background al foreground, o reactiva la ejecución en foreground de un trabajo que había sido detenido \n");
    return 0;
}

int internal_bg(char **args)
{
    printf("Reactiva un proceso detenido para que siga ejecutándose pero en segundo plano \n");
    return 0;
}

int check_internal(char **args)
{
    char *strs[6];

    strs[0] = "cd";
    strs[1] = "export";
    strs[2] = "source";
    strs[3] = "jobs";
    strs[4] = "fg";
    strs[5] = "bg";
    strs[6] = "exit";

    if (strcmp(strs[0], args[0]) == 0)
    {
        internal_cd(args);
        return 1;
    }
    else if (strcmp(strs[1], args[0]) == 0)
    {
        internal_export(args);
        return 1;
    }
    else if (strcmp(strs[2], args[0]) == 0)
    {
        internal_source(args);
        return 1;
    }
    else if (strcmp(strs[3], args[0]) == 0)
    {
        internal_jobs(args);
        return 1;
    }
    else if (strcmp(strs[4], args[0]) == 0)
    {
        internal_fg(args);
        return 1;
    }
    else if (strcmp(strs[5], args[0]) == 0)
    {
        internal_bg(args);
        return 1;
    }
    else if (strcmp(strs[6], args[0]) == 0)
    {
        exit(0);
    }
    else
    {
        return 0;
    }
}

//crida a la funcio parse_args() i passa a la funcio check_internal() el retorn de parse_args
int execute_line(char *line)
{
    //declaració varibale punter char
    char *args[ARGS_SIZE];

    printf("Contador: %d \n", parse_args(args, line));
    //executa el mètode check_internal
    check_internal(args);
    return 0;
}

int main()
{
    //declaracio variable char
    char line[COMMAND_LINE_SIZE];

    //bucle infinit perquè la consola estigu constantment esperant un fluxe d'entrada de dades
    while (1)
    {

        //comprov que la longuitud de la linia de comandos es major que 0, i executa el mètode read_line
        if (read_line(line) && strlen(line) > 0)
        {
            //executa el mètode execute_line
            execute_line(line);
        }
    }
}