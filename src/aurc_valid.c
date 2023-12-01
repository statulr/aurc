#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <search.h>

// Define valid actions
char *validActions[] = {
    "install", "install-force", "remove", "query", "search", "remove-dep",
    "clear-aur-cache", "update", "refresh", "modify-repo", "remove-force",
    "remove-force-dep", "search-aur", "install-aur", "install-local", "remove-orp",
    "github", "config"};

// Calculate the size of validActions array dynamically
size_t numValidActions = sizeof(validActions) / sizeof(validActions[0]);

// Create a hash set for valid actions
void __attribute__((constructor)) createHashSet()
{
    hcreate(numValidActions);
    ENTRY e;

    for (size_t i = 0; i < numValidActions; i++)
    {
        e.key = validActions[i];
        e.data = (void *)0;
        hsearch(e, ENTER);
    }
}

// Function to check if an action is valid
int isValidAction(const char *action)
{
    // Error handling
    if (action == NULL)
    {
        return 0;
    }

    // Main logic
    ENTRY e, *ep;
    e.key = (char *)action;
    ep = hsearch(e, FIND);

    return ep != NULL;
}