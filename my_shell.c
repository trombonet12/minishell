//AUTORS: Miquel Vidal Cortés i Joan López Ferrer

#define _POSIX_C_SOURCE 200112L
#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64
#define N_JOBS 64
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
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int n_pids = 0; //variable global per controlar el nombre de treballs actius
int execute_line(char *line);
int jobs_list_remove(int pos);

struct info_process
{
    pid_t pid;
    char status;                 // ‘N’, ’E’, ‘D’, ‘F’
    char cmd[COMMAND_LINE_SIZE]; // línea de comando
};

static struct info_process jobs_list[N_JOBS];

//imprmir prompt per pantalla
void imprimir_prompt()
{
    //obtenir valor de les variables desitjades
    char *PWD = getenv("PWD");
    char *USER = getenv("USER");

    printf(VERMELL_T "%s" VERDE_T ":~" AMARILLO_T "%s" BLANCO_T "%c " RESET_COLOR, USER, PWD, PROMPT);
}

//imprimir el prompt i llegir una linia de codi amb char *fgets(char *str, int n, sream FILE*)
char *read_line(char *line)
{
    //imprimir pront
    imprimir_prompt();
    //emmagatzamar el comando introduit per teclat dins line
    fgets(line, COMMAND_LINE_SIZE, stdin);
    //detectam si hi ha hagut un EOF
    if (feof(stdin))
    {
        //inserim un salt de linia per la claretat del shell
        printf("\n");
        exit(0);
    }
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
            token = strtok(NULL, s);
            //assignam a la posicio counter-èssima el valor de token
            args[counter] = token;
            break;
        }
        //incrementam countre amb una unitat
        counter++;
        token = strtok(NULL, s);
        //assignam a la posicio counter-èssima el valor de token
        args[counter] = token;
    }
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
        printf("Directori Anterior --> %s \n", buffer);
    }
    //comprovar si el comando cd no te cap agrument
    if (args[1] == NULL)
    {
        //canviar direcori a /home
        chdir("/home");
        //assignar a la variable d'entron PWD el directori /home
        setenv("PWD", "/home", 1);
        printf("Directori Actual --> /home \n");
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
        printf("Directori Actual --> %s \n", buffer);
    }
    return 0;
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
        valor = strtok(NULL, s);

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
                    printf("Valor Final --> %s \n", getenv(nom));
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
    return 0;
}

int internal_source(char **args)
{
    //comprovam que la sintaxis de l'ordre sigui correcte
    if (args[1] != NULL)
    {
        //Declaracio i inicialització de la variable FILE
        FILE *arxiu;
        arxiu = fopen(args[1], "r");
        //Declaracio d'un string per contenir una linia del arxiu
        char str[COMMAND_LINE_SIZE];
        //comprovam l'existencia del arxiu
        if (arxiu != NULL)
        {
            //bucle per llegir el contingut del arxiu, linia a linia
            while (fgets(str, COMMAND_LINE_SIZE, arxiu) != NULL)
            {
                fflush(arxiu);
                //pasam la linia llegida a execute_line
                execute_line(str);
            }
            //tancam l'arxiu
            fclose(arxiu);
        }
        else
        {
            printf("ERROR: L'arxiu introduit no existeix\n");
        }
    }
    else
    {
        printf("Sintaxis Incorrecta. Us correcte: source <nom del fitxer>\n");
    }
    return 0;
}
//Metode que imprimeix job list
int internal_jobs(char **args)
{
    for (int i = 1; i < (n_pids+1); i++)
    {
        printf("[%d]: PID: %d. COMMAND LINE: %s. STATUS: %c\n", i, jobs_list[i].pid, jobs_list[i].cmd, jobs_list[i].status);
        
    }
    return 0;
}
//metode que mou un proces al primer pla
int internal_fg(char **args)
{
    char cpos;
    int pos;
    if (args[1] != NULL)
    {
        cpos = *args[1];
        pos = (int)cpos - 48;
        if (!((pos > n_pids) || (pos == 0)))
        {
            kill(jobs_list[pos].pid, SIGCONT);
            jobs_list[0].pid = jobs_list[pos].pid;
            jobs_list[0].status = 'E';
            strncpy(jobs_list[0].cmd, jobs_list[pos].cmd, 64);
            jobs_list_remove(pos);
            for (int i = 0; i < 64; i++)
            {
                printf("%d ", jobs_list[pos].cmd[i]);
            }
            printf("\n");
            while (jobs_list[0].pid > 0)
            {
                pause();
            }
        }
        else
        {
            printf("FG--> No existeix el treball.\n");
        }
    }
    else
    {
        printf("FG--> Sintaxis Incorrecta.\n");
    }
    return 0;
}
//metode que envia un proces a segon pla
int internal_bg(char **args)
{
    char cpos;
    int pos;
    if (args[1] != NULL)
    {
        cpos = *args[1];
        pos = (int)cpos - 48;
        if (!((pos > n_pids) || (pos == 0)))
        {
            if (jobs_list[pos].status == 'E')
            {
                printf("El proces ja s'esta executant en segon pla\n");
            }
            else
            {
                jobs_list[pos].status = 'E';
                kill(jobs_list[pos].pid, SIGCONT);
            }
        }
        else
        {
            printf("BG--> No existeix el treball.\n");
        }
    }
    else
    {
        printf("BG--> Sintaxis Incorrecta.\n");
    }
    return 0;
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
        return 1;
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
    return 0;
}
//metode que afegeix una entrada a job list
int jobs_list_add(pid_t pid, char status, char *cmd)
{
    if (n_pids < N_JOBS)
    {
        n_pids++;
        jobs_list[n_pids].pid = pid;
        jobs_list[n_pids].status = status;
        for (int i = 0; i < COMMAND_LINE_SIZE; i++)
        {
            jobs_list[n_pids].cmd[i] = cmd[i];
        }
    }
    return 0;
}
//metode que cerca un pid a job list i retorna la seva posicio
int jobs_list_find(pid_t pid)
{
    for (int i = 0; i < n_pids; i++)
    {
        if (jobs_list[i].pid == pid)
        {
            return i;
        }
    }

    return -1;
}
//metdode que elimina una entrada de job list
int jobs_list_remove(int pos)
{
    jobs_list[pos] = jobs_list[n_pids];
    n_pids--;
    return 0;
}
//Metode que comprova si s'ha d'executar en segon pla
int is_background(char **args, int numArgs)
{
    if (numArgs > 1)
    {
        //comprovam si el darrer argument es &
        if (*args[numArgs - 1] == '&')
        {
            //eliminam & i enviam un true
            args[numArgs - 1] = NULL;
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}
//Metode enterrador
void reaper(int signum)
{
    signal(SIGCHLD, reaper);
    pid_t ended;
    if ((ended = (waitpid(-1, NULL, WNOHANG))) > 0)
    {
        fflush(stdout);
        if (ended == jobs_list[0].pid)
        {
            jobs_list[0].pid = 0;
        }
        else
        {
            if (jobs_list_find(ended) != -1)
            {
                jobs_list_remove(jobs_list_find(ended));
                printf("El fill que ha finalitzat es: %d\n", ended);
            }
        }
        jobs_list[0].pid = 0; //provisional
    }
}
//Metode que atura el proces en primer pla
void ctrlc(int signum)
{
    signal(SIGINT, ctrlc);
    //Comprovam si hi ha un proces en primer pla
    if (jobs_list[0].pid > 0)
    {
        //Comprovam si el proces en primer pla es el shell
        if (getppid() != getpid())
        {
            //enviam la senyal SIGKILL al proces en primer pla
            kill(jobs_list[0].pid, SIGKILL);
            printf("CTRL C executat amb exit\n");
        }
        else
        {
            printf("Señal SIGTERM no enviada debido a que el proceso en foreground es el shell\n");
        }
    }
    else
    {
        printf("Señal SIGTERM no enviada debido a que no hay proceso en foreground\n");
    }
}
//Metode que envia el proces en primer pla al segon pla
void ctrlz(int signum)
{
    signal(SIGTSTP, ctrlz);
    //Comprovam si hi ha un proces en primer pla
    if (jobs_list[0].pid > 0)
    {
        //Comprovam si el proces en primer pla es el shell
        if (jobs_list[0].pid != getpid())
        {
            //enviam la senyal SIGSTOP al proces en primer pla
            kill(jobs_list[0].pid, SIGSTOP);
            printf("CTRL-Z: Senyal SIGSTOP enviada al proces %d\n", jobs_list[0].pid);
            //ho registram a jobs list
            jobs_list_add(jobs_list[0].pid, 'D', jobs_list[0].cmd);
            jobs_list[0].pid = 0;
        }
        else
        {
            printf("Señal SIGSTOP no enviada debido a que el proceso en foreground es el shell\n");
        }
    }
    else
    {
        printf("Señal SIGTSTP no enviada debido a que no hay proceso en foreground\n");
    }
}

int is_output_redirection(char **args, int numArgs)
{
    int file;
    //bucle que cerca '>'
    for (int i = 0; i < numArgs; i++)
    {
        if (*args[i] == '>')
        {
            //comprovam que la sintaxis sigui correcte
            if (args[i + 1] != NULL)
            {
                //obrim l'artxiu
                file = open(args[i + 1], O_RDWR | O_APPEND | O_CREAT,0777);
                //associam stdout amb el fitxer
                dup2(file, 1);
                //tancam el fitxer
                close(file);
            }
            else
            {
                printf("Output Redirectionation--> Sintaxis Incorrecta.\n");
            }
            args[i] = NULL;
            return 1;
        }
    }
    return 0;
}
//crida a la funcio parse_args() i passa a la funcio check_internal() el retorn de parse_args
int execute_line(char *line)
{
    //declaració varibale punter chars
    char *args[ARGS_SIZE];
    //variable que conte el numero d'arguments introduits per l'usuari
    int numArgs = parse_args(args, line);
    //variable que indica si el proces s'ha d'executar en background
    int background = is_background(args, numArgs);
    //executa el mètode check_internal
    if (!check_internal(args))
    {
        pid_t pid;
        pid = fork();
        if (pid == 0)
        { // fill
            signal(SIGINT, SIG_IGN);
            signal(SIGTSTP, SIG_IGN);
            signal(SIGCHLD, SIG_DFL);
            //comprovam si s'ha de redireccionar la sortida a un artxiu
            is_output_redirection(args, numArgs);
            //ho executam
            execvp(args[0], args);
            printf("HIJO: Si ve este mensaje, el execvp no funcionó...\n");
            exit(-1);
        }
        else if (pid > 0)
        { // pare
            //comprovam si s'ha d'executar en segon pla
            if (background)
            {
                //ho afegim a jobs list
                jobs_list_add(pid, 'E', *args);
                printf("pocoloco");
                fflush(stdout);
                args[0]=NULL;
            }
            else
            {
                //afegim les dades del proces en foreground
                jobs_list[0].pid = pid;
                jobs_list[0].status = 'E';
                strncpy(jobs_list[0].cmd, *args, 64);
                //mentres hi ha un proces en foreground el pare espera
                while (jobs_list[0].pid > 0)
                {
                    pause();
                }
            }
        }
        else
        {
            //control d'errors de la cridada al sistema
            fprintf(stderr, "Error %d: %s \n", errno, strerror(errno));
        }
    }
    return 0;
}
//metode main
int main()
{
    //declaracio variable char
    char line[COMMAND_LINE_SIZE];

    //bucle infinit perquè la consola estigu constantment esperant un fluxe d'entrada de dades
    while (1)
    {
        //Associam les senyals als metodes que hem creat
        signal(SIGCHLD, reaper);
        signal(SIGINT, ctrlc);
        signal(SIGTSTP, ctrlz);
        //comprov que la longuitud de la linia de comandos es major que 0, i executa el mètode read_line
        if (read_line(line))
        {
            //executa el mètode execute_line
            execute_line(line);
        }
    }
}
