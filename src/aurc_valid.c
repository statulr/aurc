#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <search.h>

// Function to check if an action is valid
int isValidAction(const char *action)
{
    // Check for NULL pointer
    if (action == NULL)
    {
        return 0;
    }

    // Define valid actions
    char *validActions[] = {
        "install", "install-force", "remove", "query", "search", "remove-dep",
        "clear-aur-cache", "update", "refresh", "modify-repo", "remove-force",
        "remove-force-dep", "search-aur", "install-aur", "install-local", "remove-orp",
        "github", "config"};

    // Calculate the size of validActions array dynamically
    size_t numValidActions = sizeof(validActions) / sizeof(validActions[0]);

    // Create a hash set for valid actions
    hcreate(numValidActions);
    ENTRY e, *ep;

    for (size_t i = 0; i < numValidActions; i++)
    {
        e.key = validActions[i];
        e.data = (void *)0;
        hsearch(e, ENTER);
    }

    // Check if action is valid
    e.key = (char *)action;
    ep = hsearch(e, FIND);
    hdestroy(); // Destroy the hash set (Frees up memory once done :P)

    return ep != NULL;
}