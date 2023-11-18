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
CommandType getCommandType(const char* str)
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

#endif // COMMANDS_H