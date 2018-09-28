#include <stdio.h>
#include <time.h>
#include <utime.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>
#include <pwd.h>
#include <grp.h>

#include "simple_tar.h"

#define BYTE unsigned char

int init(struct posix_header *pHeader);
unsigned int checksum (char ptr[], size_t sz, int chk);
int fill(struct posix_header *pHeader, struct stat *fileStat);
int aRemplir(int x);
int tarfifo(char* source, char* buffer, int length_buffer);
int traitement_fichier(char* source, FILE* destFile, char* relative);
int traitement_directory(char* source, FILE* destFile, char* relative);
int ls_tar(char* path);
static void _mkdir(const char *dir, mode_t mode);
int settime(char* path, time_t mtime);
int extract(char* path);
int checkParam(char* param);
int archive(char* param, char* argv[], int argc);
