#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

void existingAurPackage(const char *packageName);
void displayPkgBuild(const char *packageName, const char *downloadDir);

size_t writeData(void *buffer, size_t size, size_t nmemb, void *userp)
{
    size_t remaining = 512 - strlen(userp);
    size_t realSize = size * nmemb;
    if (realSize < remaining)
    {
        strncat(userp, buffer, realSize);
    }
    else
    {
        strncat(userp, buffer, remaining);
    }
    return realSize;
}

void installAurPackages(char **packageNames, int numPackages)
{
    char downloadDir[256];
    char command[300];
    snprintf(downloadDir, sizeof(downloadDir), "%s", "~/.cache/aurc/");

    char mkdirCommand[200];
    snprintf(mkdirCommand, sizeof(mkdirCommand), "mkdir -p %s", downloadDir);
    system(mkdirCommand);

    for (int i = 0; i < numPackages; ++i)
    {
        char *packageName = packageNames[i];

        char url[256];
        snprintf(url, sizeof(url), "https://aur.archlinux.org/cgit/aur.git/snapshot/%s.tar.gz", packageName);

        char checkUrlCommand[300];
        snprintf(checkUrlCommand, sizeof(checkUrlCommand), "curl -o /dev/null --silent --head --fail %s", url);
        int urlStatus = system(checkUrlCommand);

        if (urlStatus != 0)
        {
            printf("Error: Package '%s' not found.\n", packageName);
            existingAurPackage(packageName);
            continue;
        }
        char downloadCommand[300];
        snprintf(downloadCommand, sizeof(downloadCommand), "curl -L -o %s/%s.tar.gz %s", downloadDir, packageName, url);
        system(downloadCommand);
        char extractCommand[300];
        snprintf(extractCommand, sizeof(extractCommand), "tar -xzf %s/%s.tar.gz -C %s", downloadDir, packageName, downloadDir);
        system(extractCommand);

        char userInput[10];
        printf("Do you want to view the PKGBUILD for '%s' before installation? (Recommended) (y/n): ", packageName);
        fgets(userInput, sizeof(userInput), stdin);

        if (userInput[0] == '\n')
        {
            userInput[0] = 'y';
        }

        if (tolower(userInput[0]) == 'y')
        {
            displayPkgBuild(packageName, downloadDir);

            printf("Do you want to continue with the installation of '%s'? (y/n): ", packageName);
            fgets(userInput, sizeof(userInput), stdin);

            if (userInput[0] == '\n')
            {
                userInput[0] = 'y';
            }

            if (tolower(userInput[0]) != 'y')
            {
                printf("Installation of '%s' aborted.\n", packageName);
                char cleanupCommand[300];
                snprintf(cleanupCommand, sizeof(cleanupCommand), "rm -rf %s/%s %s/%s.tar.gz", downloadDir, packageName, downloadDir, packageName);
                system(cleanupCommand);
                continue;
            }
        }
        char buildCommand[300];
        snprintf(buildCommand, sizeof(buildCommand), "cd %s/%s && makepkg -si", downloadDir, packageName);

        pid_t pid = fork();
        if (pid == -1)
        {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            execlp("sh", "sh", "-c", buildCommand, (char *)NULL);
            _exit(EXIT_FAILURE);
        }
        else
        {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
            {
                printf("Installation of '%s' failed.\n", packageName);
                char cleanupCommand[300];
                snprintf(cleanupCommand, sizeof(cleanupCommand), "rm -rf %s/%s %s/%s.tar.gz", downloadDir, packageName, downloadDir, packageName);
                system(cleanupCommand);
                continue;
            }
        }

        printf("Do you want to clean up the build cache for '%s'? (Recommended) (y/n): ", packageName);
        fgets(userInput, sizeof(userInput), stdin);

        if (userInput[0] == '\n')
        {
            userInput[0] = 'y';
        }

        if (tolower(userInput[0]) == 'y')
        {
            char cleanupCommand[300];
            snprintf(cleanupCommand, sizeof(cleanupCommand), "rm -rf %s/%s %s/%s.tar.gz", downloadDir, packageName, downloadDir, packageName);
            system(cleanupCommand);
        }
    }
}

void queryAurRepo(const char *packageName, char *message)
{
    char url[500];
    int ret = snprintf(url, sizeof(url), "https://aur.archlinux.org/rpc/?v=5&type=search&arg=%s", packageName);
    if (ret < 0 || ret >= (int)sizeof(url))
    {
        printf("URL is too long or snprintf failed.\n");
        return;
    }

    char buffer[512] = {0};

    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL *curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

void searchAurPackage(char *packageName)
{
    queryAurRepo(packageName, "Search results for");
}

void existingAurPackage(const char *packageName)
{
    queryAurRepo(packageName, "Available & similar packages for");
}

void displayPkgBuild(const char *packageName, const char *downloadDir)
{
    char displayCommand[300];
    snprintf(displayCommand, sizeof(displayCommand), "less %s/%s/PKGBUILD", downloadDir, packageName);
    system(displayCommand);
}

void clearAurBuildCache()
{
    char downloadDir[256];
    snprintf(downloadDir, sizeof(downloadDir), "~/.cache/aurc/");

    char cleanupCommand[300];
    snprintf(cleanupCommand, sizeof(cleanupCommand), "rm -rf %s/*", downloadDir);
    system(cleanupCommand);
}