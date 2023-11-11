#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/stat.h>

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
    size_t remaining = 512 - strlen(userp) - 1;
    if (size * nmemb < remaining) {
        strncat(userp, buffer, size * nmemb);
    } else {
        strncat(userp, buffer, remaining);
    }
    return size * nmemb;
}

void queryAurRepo(char *packageName, char *message) {
    char url[500];
    int ret = snprintf(url, sizeof(url), "https://aur.archlinux.org/rpc/?v=5&type=search&arg=%s", packageName);
    if (ret < 0 || ret >= (int)sizeof(url)) {
        printf("URL is too long or snprintf failed.\n");
        return;
    }

    for (char *p = packageName; *p != '\0'; p++) {
        if (!isalnum((unsigned char)*p)) {
            *p = '_';
        }
    }

    char buffer[512] = {0};

    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL *curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        CURLcode res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

void searchAurPackage(char *packageName) {
    queryAurRepo(packageName, "Search results for");
}

void existingAurPackage(char *packageName) {
    queryAurRepo(packageName, "Available & similar packages for");
}

void displayPKGBUILD(char *packageName, char *downloadDir) {
    char displayCommand[300];
    snprintf(displayCommand, sizeof(displayCommand), "less %s/%s/PKGBUILD", downloadDir, packageName);
    system(displayCommand);
}

void installAurPackages(char **packageNames, int numPackages) {
    char downloadDir[256];
    snprintf(downloadDir, sizeof(downloadDir), "%s", "~/.cache/aurc/");

    mkdir(downloadDir, 0755);

    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize curl\n");
        return;
    }

    for (int i = 0; i < numPackages; ++i) {
        char *packageName = packageNames[i];

        char url[256];
        snprintf(url, sizeof(url), "https://aur.archlinux.org/cgit/aur.git/snapshot/%s.tar.gz", packageName);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            printf("Error: Package '%s' not found.\n", packageName);
            existingAurPackage(packageName);
            continue;
        }

        FILE *fp = fopen(downloadDir, "wb");
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        fclose(fp);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            continue;
        }

        char extractCommand[300];
        snprintf(extractCommand, sizeof(extractCommand), "tar -xzf %s/%s.tar.gz -C %s", downloadDir, packageName, downloadDir);
        system(extractCommand);

        char userInput[10];
        printf("Do you want to view the PKGBUILD for '%s' before installation? (Recommended) (y/n): ", packageName);
        fgets(userInput, sizeof(userInput), stdin);

        if (tolower(userInput[0]) == 'y') {
            displayPKGBUILD(packageName, downloadDir);

            printf("Do you want to continue with the installation of '%s'? (y/n): ", packageName);
            fgets(userInput, sizeof(userInput), stdin);

            if (tolower(userInput[0]) != 'y') {
                printf("Installation of '%s' aborted.\n", packageName);
                char cleanupCommand[300];
                snprintf(cleanupCommand, sizeof(cleanupCommand), "rm -rf %s/%s %s/%s.tar.gz", downloadDir, packageName, downloadDir, packageName);
                system(cleanupCommand);
                continue;
            }
        }

        char buildCommand[300];
        snprintf(buildCommand, sizeof(buildCommand), "cd %s/%s && makepkg -si", downloadDir, packageName);
        system(buildCommand);

        printf("Do you want to clean up the build cache for '%s'? (Recommended) (y/n): ", packageName);
        fgets(userInput, sizeof(userInput), stdin);

        if (tolower(userInput[0]) == 'y') {
            char cleanupCommand[300];
            snprintf(cleanupCommand, sizeof(cleanupCommand), "rm -rf %s/%s %s/%s.tar.gz", downloadDir, packageName, downloadDir, packageName);
            system(cleanupCommand);
        }
    }

    curl_easy_cleanup(curl);
}

void clearAurBuildCache() {
    char downloadDir[256];
    // Use snprintf to avoid buffer overflow
    snprintf(downloadDir, sizeof(downloadDir), "~/.cache/aurc/");

    char cleanupCommand[300];
    // Use snprintf to avoid buffer overflow
    snprintf(cleanupCommand, sizeof(cleanupCommand), "rm -rf %s/*", downloadDir);
    system(cleanupCommand);
}
