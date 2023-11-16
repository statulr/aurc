// commands.h

#ifndef COMMANDS_H
#define COMMANDS_H

#include <string.h>

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
    if (strcmp(str, "--version") == 0 || strcmp(str, "-v") == 0)
    {
        return CMD_VERSION;
    }
    if (strcmp(str, "--help") == 0 || strcmp(str, "-h") == 0)
    {
        return CMD_HELP;
    }
    return CMD_UNKNOWN;
}

#endif // COMMANDS_H
