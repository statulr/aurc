#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <pwd.h>

// Get the current user's shell
char* getCurrentUserShell() {
    struct passwd *pw = getpwuid(getuid());
    if (pw == NULL) {
        perror("getpwuid");
        return NULL;
    }

    // Find the shell name in the shell path
    char* shellName = strrchr(pw->pw_shell, '/');
    if (shellName != NULL) {
        shellName++;  // Skip the slash
    } else {
        shellName = pw->pw_shell;  // If there's no slash, the whole string is the shell name
    }

    char* shell = strdup(shellName);
    if (shell == NULL) {
        perror("strdup");
        return NULL;
    }

    return shell;
}

// Execute a command with the user's shell the command is executed in
void executeCommandWithUserShell(char* command) {
    char* userShell = getCurrentUserShell();
    if (userShell != NULL) {
        char* argv[] = {userShell, "-c", command, NULL};

        printf("Executing command with user's shell: %s\n", userShell);
        execvp(userShell, argv);
        
        perror("execvp");
        free(userShell);  // Free userShell before exiting
        exit(EXIT_FAILURE);
    } else {
        printf("Unable to detect user's shell. Fallback to system() for command execution.\n");

        if (command != NULL && strlen(command) > 0) {
            if (system(command) == -1) {
                perror("system");
                exit(EXIT_FAILURE);
            }
        } else {
            printf("Invalid command. Exiting.\n");
            exit(EXIT_FAILURE);
        }
    }
}