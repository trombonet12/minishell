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
#include <unistd.h>
#include <errno.h>

//imprmir prompt per pantalla
void imprimir_prompt()
{
    //obtenir valor de les variables desitjades
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
        //comprovar si missatge introduït és un comentari
        if (token[0] == '#')
        {
            printf("Parse_args()-->token %d : %s \n", counter, token);
            printf("Parse_args()-->token %d : corregido (null) \n", counter);

            token = strtok(NULL, s);
            //assignam a la posicio counter-èssima el valor de token
            args[counter] = token;
            break;
        }

        //imprimim per pantalla el valor de counter i de token
        printf("Parse_args()-->token %d : %s \n", counter, token);
        //incrementam countre amb una unitat
        counter++;
        token = strtok(NULL, s);
        //assignam a la posicio counter-èssima el valor de token
        args[counter] = token;
    }
    //imprimim per pantalla el valor de counter i de token
    printf("Parse_args()-->token %d : %s \n", counter, token);

    return counter;
}
//canviar el directori
int internal_cd(char **args)
{

    char buffer[1024];
    //obtenir el directori anterior al desitjat
    if (getcwd(buffer, sizeof(buffer)) == NULL)
    {
        //control d'error al fer una cridada al sistema
        fprintf(stderr, "Error %d: %s \n", errno, strerror(errno));
    }
    else
    { //imprimir per pantalla el directori anterior al desitjat
        printf("Directori Anterior %s \n", buffer);
    }
    //comprovar si el comando cd no te cap agrument
    if (args[1] == NULL)
    {
        //canviar direcori a /home
        chdir("/home");
        //assignar a la variable d'entron PWD el directori /home
        setenv("PWD", "/home", 1);
    }
    //assignar nou directori introduït per pantalla
    else if (chdir(args[1]) == -1)
    {
        //control d'errors de la cridada al sistema
        printf("Chdir: No such file or directory \n");
    }
    //obtenir el directori actual
    else if (getcwd(buffer, sizeof(buffer)) == NULL)
    {
        //control d'errors de la cridada al sistema
        fprintf(stderr, "Error %d: %s \n", errno, strerror(errno));
    }
    else
    {
        //assignar el nou directori a la variable d'entorn PWD
        setenv("PWD", buffer, 1);
        //imprimir per pantalla el directori actualitzat
        printf("Directori Actual %s \n", buffer);
    }
}
//canviar una variable d'entorn
int internal_export(char **args)
{
    const char s[2] = "=\n";
    char *nom;
    char *valor;
    if (args[1] != NULL)
    {   
        //dividim el segon token en variable i valor introduit
        nom = strtok(args[1], s);
        printf("Parse_args()-->token: %s \n", nom);
        valor = strtok(NULL, s);
        printf("Parse_args()-->token: %s \n", valor);

        //comprovam que hi hagi arguments necessaris per realitzar l'operació
        if ((nom == NULL) && (valor == NULL))
        {   
            //avisam a l'usuari de l'error de sintaxis
            printf("Sintaxis Incorrecta. Us correcte: export NOM=VALOR \n");
        }
        else
        {   
            //comprovam que la variable d'entorn introduida existeix
            if (getenv(nom) == NULL && valor != NULL)
            {   
                //informam al usuari que la variable que ha introduit no existeix
                printf("No existeix aquesta variable d'entorn: %s \n", nom);
            }
            else
            {   
                printf("Valor Inicial %s \n", getenv(nom));
                //comprovam que hi hagi un valor a introduir a la variable d'entorn
                if (valor != NULL)
                {   
                    //assignam el nou valor a la variable introduida
                    setenv(nom, valor, 1);
                    printf("Valor Final %s \n", getenv(nom));
                }
                else
                {   
                    //avisam a l'usuari de l'error de sintaxis
                    printf("Sintaxis Incorrecta. Us correcte: export NOM=VALOR \n");
                }
            }
        }
    }
    else
    {   
        //avisam a l'usuari de l'error de sintaxis
        printf("Sintaxis Incorrecta. Us correcte: export NOM=VALOR \n");
    }
}

int internal_source(char **args)
{
    printf("Ejecuta las líneas de comandos del fichero nombre \n");
}

int internal_jobs(char **args)
{
    printf("Muestra el PID de los procesos que se están ejecutando en background \n");
}

int internal_fg(char **args)
{
    printf("Envía un trabajo del background al foreground, o reactiva la ejecución en foreground de un trabajo que había sido detenido \n");
}

int internal_bg(char **args)
{
    printf("Reactiva un proceso detenido para que siga ejecutándose pero en segundo plano \n");
}

//comprovar si el comando passat per paràmetre és intern
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

    if (args[0] == NULL)
    {
        //Evitam una violacio de segment
    }
    //comprovar si es el comando cd
    else if (strcmp(strs[0], args[0]) == 0)
    {
        internal_cd(args);
        return 1;
    }
    //comprovar si es el comando export
    else if (strcmp(strs[1], args[0]) == 0)
    {
        internal_export(args);
        return 1;
    }
    //comprovar si es el comando source
    else if (strcmp(strs[2], args[0]) == 0)
    {
        internal_source(args);
        return 1;
    }
    //comprovar si es el comando jobs
    else if (strcmp(strs[3], args[0]) == 0)
    {
        internal_jobs(args);
        return 1;
    }
    //comprovar si es el comando fg
    else if (strcmp(strs[4], args[0]) == 0)
    {
        internal_fg(args);
        return 1;
    }
    //comprovar si es el comando bg
    else if (strcmp(strs[5], args[0]) == 0)
    {
        internal_bg(args);
        return 1;
    }
    //comprovar si es el comando exit
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

    printf("El nombre de tokens és: %d \n", parse_args(args, line));
    //executa el mètode check_internal
    check_internal(args);
}
//metode main
int main()
{
    //declaracio variable char
    char line[COMMAND_LINE_SIZE];

    //bucle infinit perquè la consola estigu constantment esperant un fluxe d'entrada de dades
    while (1)
    {

        //comprov que la longuitud de la linia de comandos es major que 0, i executa el mètode read_line
        if (read_line(line))
        {
            //executa el mètode execute_line
            execute_line(line);
        }
    }
}
