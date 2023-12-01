#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <bsd/string.h>
#include "constants.h"
#include "colors.h"

void executePacmanCommand(int argc, char *argv[], const char *commandPrefix, const char *usageMessage)
{
    if (argc < 3)
    {
        fprintf(stderr, RED "Usage: %s %s\n" RESET, argv[0], usageMessage);
        return;
    }

    size_t commandLength = strlen(commandPrefix);
    for (int i = 2; i < argc; ++i)
    {
        commandLength += strlen(argv[i]) + 1;
    }

    if (commandLength > MAX_COMMAND_LENGTH)
    {
        fprintf(stderr, "Command too long\n");
        return;
    }

    char *command = malloc(commandLength + 1);
    if (command == NULL)
    {
        perror(RED "malloc" RESET);
        return;
    }

    strncpy(command, commandPrefix, commandLength);
    for (int i = 2; i < argc; ++i)
    {
        strncat(command, argv[i], commandLength - strlen(command) - 1);
        if (i < argc - 1)
        {
            strncat(command, " ", commandLength - strlen(command) - 1);
        }
    }
    executeCommandWithUserShell(command);
    free(command);
}

void installLocalPackages(int argc, char *argv[])
{
    if (argc >= 3)
    {
        char command[MAX_COMMAND_LENGTH];
        strncpy(command, "sudo pacman -U ", MAX_COMMAND_LENGTH);
        strlcat(command, argv[2], MAX_COMMAND_LENGTH - strlen(command) - 1);
        command[MAX_COMMAND_LENGTH - 1] = '\0';
        system(command);
    }
    else
    {
        fprintf(stderr, RED "Usage: %s installLocal <packagePath1> <packagePath2> ...\n" RESET, argv[0]);
    }
}

void removePackagesForce(int argc, char *argv[])
{
    executePacmanCommand(argc, argv, REMOVE_FORCE_COMMAND, "removeForce <packageName1> <packageName2> ...");
}

void installPackagesForce(int argc, char *argv[])
{
    executePacmanCommand(argc, argv, INSTALL_FORCE_COMMAND, "installForce <packageName1> <packageName2> ...");
}

void removePackagesForceWithDependencies(int argc, char *argv[])
{
    executePacmanCommand(argc, argv, "sudo pacman -Rdds ", "remove <packageName1> <packageName2> ...");
}

void installPackages(int argc, char *argv[])
{
    if (argc >= 3)
    {
        char command[MAX_COMMAND_LENGTH];
        snprintf(command, sizeof(command), "sudo pacman -S %s", argv[2]);
        command[MAX_COMMAND_LENGTH - 1] = '\0';
        executeCommandWithUserShell(command);
    }
    else
    {
        fprintf(stderr, RED "Usage: %s install <packageName1> <packageName2> ...\n" RESET, argv[0]);
    }
}

void removePackages(int argc, char *argv[])
{
    if (argc >= 3)
    {
        char command[MAX_COMMAND_LENGTH];
        snprintf(command, sizeof(command), "sudo pacman -R %s", argv[2]);
        command[MAX_COMMAND_LENGTH - 1] = '\0';
        executeCommandWithUserShell(command);
    }
    else
    {
        fprintf(stderr, RED "Usage: %s remove <packageName1> <packageName2> ...\n" RESET, argv[0]);
    }
}

void queryPackage(char *packageName)
{
    char command[MAX_COMMAND_LENGTH];
    snprintf(command, sizeof(command), "pacman -Q %s", packageName);
    int result = system(command);
    if (result != 0)
    {
        fprintf(stderr, RED "%s is not installed.\n" RESET, packageName);
    }
    else
    {
        printf(GREEN "%s is installed.\n" RESET, packageName);
    }
}

void searchPackage(char *packageName)
{
    char command[MAX_COMMAND_LENGTH];
    snprintf(command, sizeof(command), "pacman -Ss %s", packageName);
    system(command);
}

void removePackagesWithDependencies(int argc, char *argv[])
{
    if (argc >= 3)
    {
        char command[MAX_COMMAND_LENGTH];
        snprintf(command, sizeof(command), "sudo pacman -Rs %s", argv[2]);
        command[MAX_COMMAND_LENGTH - 1] = '\0';
        executeCommandWithUserShell(command);
    }
    else
    {
        fprintf(stderr, RED "Usage: %s removeDep <packageName1> <packageName2> ...\n" RESET, argv[0]);
    }
}

void removeOrphanPackages()
{
    executeCommandWithUserShell("sudo pacman -Rns $(pacman -Qdtq)");
}

void updateSystem()
{
    executeCommandWithUserShell("sudo pacman -Syyu");
}

void refreshRepo()
{
    executeCommandWithUserShell("sudo pacman -Syy");
}

void modifyRepo()
{
    char editor[256] = "vi"; // default to Vi (Vim's Older Version)

    char *home = getenv("HOME");
    if (!home)
    {
        fprintf(stderr, "Could not get home directory\n");
        exit(1);
    }

    char configPath[256];
    snprintf(configPath, sizeof(configPath), "%s/.aurcrc", home);

    FILE *file = fopen(configPath, "r");
    if (file)
    {
        if (fscanf(file, "editor=%255s", editor) != 1)
        {
            fprintf(stderr, "Failed to read editor from configuration file\n");
        }
        fclose(file);
    }

    const size_t commandSize = 1024;
    char command[commandSize];

    int ret = snprintf(command, commandSize, "sudo %s /etc/pacman.d/mirrorlist", editor);
    if (ret < 0 || (size_t)ret >= commandSize)
    {
        fprintf(stderr, "Failed to open\n");
        return;
    }

    executeCommandWithUserShell(command);
}