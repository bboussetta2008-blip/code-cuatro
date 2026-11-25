#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE 1024
#define MAX_ARGS 100

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

        if (args[0] == NULL)
            continue;

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
            for (int j = 1; args[j] != NULL; j++)
                printf("%s ", args[j]);
            printf("\n");
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

        remplacer_alias(args[0]);

        pid_t pid = fork();

        if (pid == 0)
        {
            execvp(args[0], args);
            perror(args[0]);
            exit(EXIT_FAILURE);
        }
        else if (pid > 0)
        {
            wait(NULL);
        }
        else
        {
            perror("fork");
        }
    }

    printf("Au revoir !\n");
    return 0;
}
