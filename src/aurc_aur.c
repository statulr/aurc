#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <regex.h>
#include <json-c/json.h>
#include "colors.h"

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

size_t WriteCallback(void *contents, size_t size, size_t nmemb, char *userp)
{
    size_t totalSize = size * nmemb;
    if (totalSize < 1024)
    {
        memcpy(userp, contents, totalSize);
        userp[totalSize] = 0;
    }
    return totalSize;
}

int existingAurPackage(const char *packageName)
{
    CURL *curl;
    CURLcode res;
    char readBuffer[1024] = {0};
    char url[256];

    sprintf(url, "https://aur.archlinux.org/rpc/?v=5&type=info&arg=%s", packageName);

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            return 0;
        }
        else
        {
            struct json_object *parsed_json;
            parsed_json = json_tokener_parse(readBuffer);
            struct json_object *resultcount;
            json_object_object_get_ex(parsed_json, "resultcount", &resultcount);
            int count = json_object_get_int(resultcount);

            json_object_put(parsed_json);

            return count > 0;
        }
    }

    return 0;
}

void installAurPackages(char **packageNames, unsigned int numPackages)
{
    char *home = getenv("HOME");
    if (!home)
    {
        fprintf(stderr, "Could not get home directory\n");
        exit(1);
    }
    regex_t regex;
    int reti;

    reti = regcomp(&regex, "^[a-zA-Z0-9_-]+$", REG_EXTENDED);
    if (reti)
    {
        fprintf(stderr, "Characters not allowed\n");
        exit(1);
    }

    char downloadDir[256];
    snprintf(downloadDir, sizeof(downloadDir), "%s/.cache/aurc/", home);

    mkdir(downloadDir, 0755);

    CURL *curl = curl_easy_init();
    if (!curl)
    {
        fprintf(stderr, "Failed to initialize curl\n");
        exit(1);
    }

    for (unsigned int i = 0; i < numPackages; ++i)
    {
        char *packageName = packageNames[i];
        if (!existingAurPackage(packageName))
        {
            fprintf(stderr, RED "Package %s does not exist\n", packageName);
            printf(RESET);
            continue;
        }
        printf("Package(s) Requested: ");
        for (unsigned int i = 0; i < numPackages; ++i)
        {
            printf("%s", packageNames[i]);
            if (i < numPackages - 1)
            {
                printf(", ");
            }
        }
        printf("\n");

        reti = regexec(&regex, packageName, 0, NULL, 0);
        if (!reti)
        {

            char url[256];
            snprintf(url, sizeof(url), "https://aur.archlinux.org/cgit/aur.git/snapshot/%s.tar.gz", packageName);
            char downloadCommand[300];
            snprintf(downloadCommand, sizeof(downloadCommand), "%s/%s.tar.gz", downloadDir, packageName);

            curl_easy_setopt(curl, CURLOPT_URL, url);
            FILE *file = fopen(downloadCommand, "wb");
            if (!file)
            {
                fprintf(stderr, "Failed to open file %s\n", downloadCommand);
                continue;
            }
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
            CURLcode res = curl_easy_perform(curl);
            fclose(file);
            if (res != CURLE_OK)
            {
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            }

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
                    fprintf(stderr, "Installation of '%s' aborted.\n", packageName);
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
                }
            }
        }
        else if (reti == REG_NOMATCH)
        {
            printf(YELLOW "Invalid package name '%s'.\n" RESET, packageName);
            continue;
        }
        else
        {
            char msgbuf[100];
            regerror(reti, &regex, msgbuf, sizeof(msgbuf));
            fprintf(stderr, "Regex match failed: %s\n", msgbuf);
            exit(1);
        }
    }

    regfree(&regex);
    curl_easy_cleanup(curl);
}

void queryAurRepo(const char *packageName, char *message)
{
    char url[500];
    int ret = snprintf(url, sizeof(url), "https://aur.archlinux.org/rpc/?v=5&type=search&arg=%s", packageName);
    if (ret < 0 || ret >= (int)sizeof(url))
    {
        fprintf(stderr, RED "URL is too long or snprintf failed.\n");
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

void existingPackage(const char *packageName)
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
    printf(YELLOW "Clearing AUR build cache...\n" RESET);
    printf(GREEN "Done.\n" RESET);
}