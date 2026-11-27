#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 1024
#define MAX_ARGS 100
int dernier_status = 0;
int dernier_background = 0;

void afficher_aide(void)
{
    printf("Bienvenue dans ocsh !\n");
    printf("Commandes : go, quit, path, say, info\n");
    printf("Aliases : list, show, copy, move, del, newdir, newfile\n");
}

void remplacer_alias(char *commande)
{
    if (strcmp(commande, "list") == 0)
        strcpy(commande, "ls");
    else if (strcmp(commande, "show") == 0)
        strcpy(commande, "cat");
    else if (strcmp(commande, "copy") == 0)
        strcpy(commande, "cp");
    else if (strcmp(commande, "move") == 0)
        strcpy(commande, "mv");
    else if (strcmp(commande, "del") == 0)
        strcpy(commande, "rm");
    else if (strcmp(commande, "newdir") == 0)
        strcpy(commande, "mkdir");
    else if (strcmp(commande, "newfile") == 0)
        strcpy(commande, "touch");
}
void redirection(char *args[])
{
    int i = 0;

    while (args[i] != NULL)
    {
        if (strcmp(args[i], ">") == 0)
        {
            if (args[i + 1] == NULL)
            {
                fprintf(stderr, "Erreur: fichier manquant\n");
                return;
            }

            int fd = open(args[i + 1],
                          O_WRONLY | O_CREAT | O_TRUNC, 0644);

            if (fd < 0)
            {
                perror("open");
                return;
            }

            dup2(fd, STDOUT_FILENO);
            close(fd);

            args[i] = NULL;
            return;
        }
        i++;
    }
}
void gestionnaire_sigint(int sig)
{
    (void)sig;
    write(STDOUT_FILENO, "\n", 1);
}

void gestionnaire_sigchld(int sig)
{
    (void)sig;

    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
}
void remplacer_variables(char *args[])
{
    for (int i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], "$?") == 0)
        {
            char valeur[20];
            sprintf(valeur, "%d", dernier_status);
            args[i] = strdup(valeur);
        }
        else if (strcmp(args[i], "$!") == 0)
        {
            char valeur[20];
            sprintf(valeur, "%d", dernier_background);
            args[i] = strdup(valeur);
        }
        else if (args[i][0] == '$')
        {
            char *valeur = getenv(args[i] + 1);
            args[i] = strdup(valeur ? valeur : "");
        }
    }
}
void executer_pipe(char *args[], int position)
{
    int tube[2];

    if (pipe(tube) == -1)
    {
        perror("pipe");
        return;
    }

    args[position] = NULL;

    



    pid_t pid1 = fork();

    if (pid1 == 0)
    {
        dup2(tube[1], STDOUT_FILENO);

        close(tube[0]);
        close(tube[1]);

        remplacer_alias(args[0]);

        execvp(args[0], args);
        perror("execvp");
        exit(EXIT_FAILURE);
    }

    pid_t pid2 = fork();

    if (pid2 == 0)
    {
        dup2(tube[0], STDIN_FILENO);

        close(tube[0]);
        close(tube[1]);

        char **commande2 = &args[position + 1];

        remplacer_alias(commande2[0]);

        execvp(commande2[0], commande2);
        perror("execvp");
        exit(EXIT_FAILURE);
    }

    close(tube[0]);
    close(tube[1]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}
int main(void)
{
    char ligne[MAX_LINE];
    char *args[MAX_ARGS];

    while (1)
    {
        printf("ocsh> ");
        fflush(stdout);

        if (fgets(ligne, sizeof(ligne), stdin) == NULL)
            break;

        ligne[strcspn(ligne, "\n")] = '\0';

        if (strlen(ligne) == 0)
            continue;

        int i = 0;
        char *token = strtok(ligne, " \t");

        while (token != NULL && i < MAX_ARGS - 1)
        {
            args[i++] = token;
            token = strtok(NULL, " \t");
        }
        args[i] = NULL;

        if (args[0] == NULL) {
    continue;
}
        remplacer_variables(args);

        if (strcmp(args[0], "quit") == 0)
            break;

        if (strcmp(args[0], "go") == 0)
        {
            if (args[1] == NULL)
                chdir(getenv("HOME"));
            else if (chdir(args[1]) != 0)
                perror("go");
            continue;
        }

        if (strcmp(args[0], "path") == 0)
        {
            char cwd[PATH_MAX];
            if (getcwd(cwd, sizeof(cwd)) != NULL)
                printf("%s\n", cwd);
            continue;
        }

       if (strcmp(args[0], "say") == 0)
{
    int sauvegarde = dup(STDOUT_FILENO);

    redirection(args);

    for (int j = 1; args[j] != NULL; j++)
        printf("%s ", args[j]);

    printf("\n");
    fflush(stdout);

    dup2(sauvegarde, STDOUT_FILENO);
    close(sauvegarde);

    continue;
}

        if (strcmp(args[0], "info") == 0)
        {
            afficher_aide();
            continue;
        }
        if (strcmp(args[0], "osinfo") == 0)
{
    printf("Systèmes d’Exploitation -- UIR/ESIN\n");
    continue;
}

if (strcmp(args[0], "past") == 0)
{
    printf("Historique non disponible pour le moment.\n");
    continue;
}
if (strcmp(args[0], "export") == 0)
{
    if (args[1] != NULL)
    {
        char *egal = strchr(args[1], '=');

        if (egal != NULL)
        {
            *egal = '\0';
            setenv(args[1], egal + 1, 1);
        }
    }

    continue;
}
        int position = -1;

for (int j = 0; args[j] != NULL; j++)
{
    if (strcmp(args[j], "|") == 0)
    {
        position = j;
        break;
    }
}

if (position != -1)
{
    executer_pipe(args, position);
    continue;
}
        remplacer_alias(args[0]);
        int background = 0;
int n = 0;

while (args[n] != NULL)
    n++;

if (n > 0 && strcmp(args[n - 1], "&") == 0)
{
    background = 1;
    args[n - 1] = NULL;
}

        pid_t pid = fork();

        if (pid == 0)
{
    redirection(args);

    execvp(args[0], args);

    perror(args[0]);
    exit(EXIT_FAILURE);
}
       else if (pid > 0)
{
    if (background)
    {
        dernier_background = pid;
        printf("[%d]\n", pid);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status))
            dernier_status = WEXITSTATUS(status);
    }
}
        
        else
        {
            perror("fork");
        }
    }

    printf("Au revoir !\n");
    return 0;
}
