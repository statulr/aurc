#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

// Aurc Version
#define VERSION "1.1.7"

// Function to display the version
void displayVersion() {
    printf("\n");
    printf("Aurc Package Manager Version %s\n", VERSION);
    printf("Copyright (C) 2023 Statulr\n");
    printf("\n");
    printf("This program may be freely redistributed under\n");
    printf("the terms of the GNU General Public License.\n");
    printf("\n");
}
