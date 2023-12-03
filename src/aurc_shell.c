#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

char *getCurrentUserShell()
{
    struct passwd *pw = getpwuid(getuid());
    if (pw == NULL || pw->pw_shell == NULL)
    {
        perror("getpwuid");
        return NULL;
    }

    char *shellName = strrchr(pw->pw_shell, '/');
    if (shellName != NULL)
    {
        shellName++;
    }
    else
    {
        shellName = pw->pw_shell;
    }

    char *shell = strdup(shellName);
    if (shell == NULL)
    {
        perror("strdup");
    }

    return shell;
}

void executeCommandWithUserShell(char *command)
{
    if (command == NULL || strlen(command) == 0)
    {
        fprintf(stderr, "Invalid command. Exiting.\n");
        exit(EXIT_FAILURE);
    }
    size_t commandLength = strlen(command);
    char *commandWithReset = malloc(commandLength + strlen("; echo -e '\033[0m'") + 1);
    if (commandWithReset == NULL)
    {
        perror("Failed to allocate memory for commandWithReset");
        exit(EXIT_FAILURE);
    }
    strncpy(commandWithReset, command, commandLength);
    strncpy(commandWithReset + commandLength, "; echo -e '\033[0m'", strlen("; echo -e '\033[0m'") + 1);

    char *userShell = getCurrentUserShell();

    if (userShell == NULL)
    {
        fprintf(stderr, "Unable to detect user's shell. Fallback to system() for command execution.\n");

        if (system(commandWithReset) == -1)
        {
            perror("Failed to execute command with system");
            exit(EXIT_FAILURE);
        }

        free(commandWithReset);
        return;
    }
    printf("Executing command with user's shell: %s\n", userShell);

    char *argv[] = {userShell, "-c", commandWithReset, NULL};

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("Failed to fork process");
        free(userShell);
        free(commandWithReset);
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        if (execvp(userShell, argv) == -1)
        {
            perror("Failed to execute command with execvp");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        // Parent process
        int status;
        if (waitpid(pid, &status, 0) == -1)
        {
            perror("Failed to wait for child process");
            free(userShell);
            free(commandWithReset);
            exit(EXIT_FAILURE);
        }
    }

    free(userShell);
    free(commandWithReset);
}