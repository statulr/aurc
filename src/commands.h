// commands.h

#ifndef COMMANDS_H
#define COMMANDS_H

#include <string.h>
#include <string.h>
#include "aurc_colors.h"

// Define an enum for the command types
typedef enum
{
    CMD_VERSION,
    CMD_HELP,
    CMD_UNKNOWN
} CommandType;

// Function to map strings to CommandType values
CommandType getCommandType(const char *str)
{
    if (strncmp(str, "--version", 9) == 0 || strncmp(str, "-v", 2) == 0)
    {
        return CMD_VERSION;
    }
    if (strncmp(str, "--help", 6) == 0 || strncmp(str, "-h", 2) == 0)
    {
        return CMD_HELP;
    }
    return CMD_UNKNOWN;
}

void handleConfigCommand(int argc, char *argv[])
{
    if (argc < 3 || (strncmp(argv[2], "--editor", 8) != 0 && strncmp(argv[2], "-e", 2) != 0))
    {
        fprintf(stderr, RED "Usage: %s config -e <editor>\n" RESET, argv[0]);
        exit(1);
    }

    if (argc == 3)
    {
        fprintf(stderr, RED "Argument required (Ex: %s config -e vim)\n" RESET, argv[0]);
        exit(1);
    }

    char *editor = argv[3];
    char *home = getenv("HOME");
    char configPath[256];
    snprintf(configPath, sizeof(configPath), "%s/.aurcrc", home);

    FILE *file = fopen(configPath, "w");
    if (!file)
    {
        fprintf(stderr, RED "Failed to open configuration file\n" RESET);
        exit(1);
    }

    fprintf(file, "editor=%s\n", editor);
    fclose(file);

    printf(GREEN "Successfully set editor to %s\n" RESET, editor);
}
#endif // COMMANDS_H