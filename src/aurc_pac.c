#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "aurc_colors.c"
#include <bsd/string.h>  // Include for strlcat
#include "constants.h"

// Function to execute a pacman command
void executePacmanCommand(int argc, char *argv[], const char *commandPrefix, const char *usageMessage) {
    if (argc >= 3) {
        size_t commandLength = strlen(commandPrefix);
        for (int i = 2; i < argc; ++i) {
            commandLength += strlen(argv[i]) + 1;
        }

        if (commandLength > MAX_COMMAND_LENGTH) {
            fprintf(stderr, "Command too long\n");
            return;
        }

        char *command = malloc(commandLength + 1);
        if (command == NULL) {
            perror("malloc");
            return;
        }

        strcpy(command, commandPrefix);
        for (int i = 2; i < argc; ++i) {
            strlcat(command, argv[i], commandLength);
            if (i < argc - 1) {
                strlcat(command, " ", commandLength);
            }
        }

        executeCommandWithUserShell(command);
        free(command);
    } else {
        fprintf(stderr, RED "Usage: %s %s\n" RESET, argv[0], usageMessage);
    }
}

// Function to install local packages
void installLocalPackages(int argc, char *argv[]) {
    if (argc >= 3) {
        char command[MAX_COMMAND_LENGTH];
        strcpy(command, "sudo pacman -U ");
        strcat(command, argv[2]);
        command[MAX_COMMAND_LENGTH - 1] = '\0'; // Ensure null terminator
        system(command);
    } else {
        fprintf(stderr, RED "Usage: %s install-local <package_path1> <package_path2> ...\n" RESET, argv[0]);
    }
}

// Function to remove packages
void removePackagesForce(int argc, char *argv[]) {
    executePacmanCommand(argc, argv, REMOVE_FORCE_COMMAND, "remove-force <package_name1> <package_name2> ...");
}

// Function to install packages with force
void installPackagesForce(int argc, char *argv[]) {
    executePacmanCommand(argc, argv, INSTALL_FORCE_COMMAND, "install-force <package_name1> <package_name2> ...");
}

// Function to remove packages
void removePackagesForceWithDependencies(int argc, char *argv[]) {
    executePacmanCommand(argc, argv, "sudo pacman -Rdds ", "remove <package_name1> <package_name2> ...");
}

// Function to install packages
void installPackages(int argc, char *argv[]) {
    if (argc >= 3) {
        char command[MAX_COMMAND_LENGTH];
        snprintf(command, sizeof(command), "sudo pacman -S %s", argv[2]);
        command[MAX_COMMAND_LENGTH - 1] = '\0'; // Ensure null terminator
        executeCommandWithUserShell(command);
    } else {
        fprintf(stderr, RED "Usage: %s install <package_name1> <package_name2> ...\n" RESET, argv[0]);
    }
}

// Function to remove packages
void removePackages(int argc, char *argv[]) {
    if (argc >= 3) {
        char command[MAX_COMMAND_LENGTH];
        snprintf(command, sizeof(command), "sudo pacman -R %s", argv[2]);
        command[MAX_COMMAND_LENGTH - 1] = '\0'; // Ensure null terminator
        executeCommandWithUserShell(command);
    } else {
        fprintf(stderr, RED "Usage: %s remove <package_name1> <package_name2> ...\n" RESET, argv[0]);
    }
}

// Function to query if a package is installed
void queryPackage(char *packageName) {
    char command[MAX_COMMAND_LENGTH];
    snprintf(command, sizeof(command), "pacman -Q %s", packageName);
    int result = system(command);
    if (result != 0) {
        fprintf(stderr, RED "%s is not installed.\n" RESET, packageName);
    } else {
        printf(GREEN "%s is installed.\n" RESET, packageName);
    }
}

// Function to search for a package in the repository
void searchPackage(char *packageName) {
    char command[MAX_COMMAND_LENGTH];
    snprintf(command, sizeof(command), "pacman -Ss %s", packageName);
    system(command);
}

// Function to remove packages
void removePackagesWithDependencies(int argc, char *argv[]) {
    if (argc >= 3) {
        char command[MAX_COMMAND_LENGTH];
        snprintf(command, sizeof(command), "sudo pacman -Rs %s", argv[2]);
        command[MAX_COMMAND_LENGTH - 1] = '\0'; // Ensure null terminator
        executeCommandWithUserShell(command);
    } else {
        fprintf(stderr, RED "Usage: %s remove-dep <package_name1> <package_name2> ...\n" RESET, argv[0]);
    }
}

// Function to remove orphan packages
void removeOrphanPackages() {
    executeCommandWithUserShell("sudo pacman -Rns $(pacman -Qdtq)");
}

// Function to update system
void updateSystem() {
    executeCommandWithUserShell("sudo pacman -Syyu");
}

// Function to update system
void refreshRepo() {
    executeCommandWithUserShell("sudo pacman -Syy");
}

// Function to modify repo
void modifyRepo() {
    executeCommandWithUserShell("sudo vim /etc/pacman.d/mirrorlist");
}
