#include "mytar.h"



/* *---------------------------------------------------------------------------------------------------------* */
/* *---------------------------------------------------------------------------------------------------------* */
/* *---------------------------------------------------------------------------------------------------------* */
/* *--------------------------------------------Init/Fill----------------------------------------------------* */
/* *---------------------------------------------------------------------------------------------------------* */
/* *---------------------------------------------------------------------------------------------------------* */
/* *---------------------------------------------------------------------------------------------------------* */


int init(struct posix_header *pHeader){
  memset((*pHeader).name,'\0', 100);
  memset((*pHeader).mode, '\0', 8);
  memset((*pHeader).uid, '\0', 8);
  memset((*pHeader).gid, '\0',8);
  memset((*pHeader).size, '\0',20);
  memset((*pHeader).mtime, '\0',12);
  memset((*pHeader).chksum, '\0',8);
  memset((*pHeader).linkname, '\0',100);
  memset((*pHeader).magic, '\0',6);
  memset((*pHeader).version, '\0',2);
  memset((*pHeader).uname, '\0',32);
  memset((*pHeader).gname, '\0',32);
  memset((*pHeader).prefix, '\0',183);

  return 0;
}

unsigned int checksum (char ptr[], size_t sz, int chk) {
  int i = 0;
  while( (ptr[i] != '\0') && (i < sz) ){
    chk += ptr[i];
    i++;
  }
  return chk;
}

int fill(struct posix_header *pHeader, struct stat *fileStat){
  struct group *grp;
  struct passwd *pwd;

  grp = getgrgid((*fileStat).st_gid);
  pwd = getpwuid((*fileStat).st_uid);

  sprintf((*pHeader).mode, "%06o ", (int) (*fileStat).st_mode);
  sprintf((*pHeader).uid, "%06d ", (int) (*fileStat).st_uid);
  sprintf((*pHeader).gid, "%06d ", (int) (*fileStat).st_gid);
  sprintf((*pHeader).size, "%0*o", 11, (int) (*fileStat).st_size );
  sprintf((*pHeader).mtime, "%0*o", 11,(int) (*fileStat).st_mtime);
  sprintf((*pHeader).magic, "%s", TMAGIC);
  sprintf((*pHeader).version, "%s", TVERSION);
  sprintf((*pHeader).uname, "%s", pwd->pw_name);
  sprintf((*pHeader).gname, "%s", grp->gr_name);

  if( S_ISREG((*fileStat).st_mode) )
  (*pHeader).typeflag = REGTYPE;
  if( S_ISFIFO((*fileStat).st_mode) )
  (*pHeader).typeflag = FIFOTYPE;
  if( S_ISDIR((*fileStat).st_mode) )
  (*pHeader).typeflag = DIRTYPE;
  if( S_ISLNK((*fileStat).st_mode) ) {
    (*pHeader).typeflag = SYMTYPE;
    readlink ((*pHeader).name, (*pHeader).linkname, 99);
  }

  unsigned int chk = 0;
  chk = checksum((*pHeader).name, 100, chk);
  chk = checksum((*pHeader).mode, 8, chk);
  chk = checksum((*pHeader).uid, 8, chk);
  chk = checksum((*pHeader).gid, 8, chk);
  chk = checksum((*pHeader).size, 12, chk);
  chk = checksum((*pHeader).mtime, 12, chk);
  chk += (*pHeader).typeflag;
  chk += 8*32;
  chk = checksum((*pHeader).linkname, 100, chk);
  chk = checksum((*pHeader).magic, 6, chk);
  chk = checksum((*pHeader).version, 2, chk);
  chk = checksum((*pHeader).uname, 32, chk);
  chk = checksum((*pHeader).gname, 32, chk);
  chk = checksum((*pHeader).prefix, 183, chk);
  sprintf((*pHeader).chksum, "%0*o ", 6, chk);

  return 0;
}

int aRemplir(int x){
  if( x == 512 )
  return 0;
  else
  return (512 - (x % 512));
}


int tarfifo(char* source, char* buffer, int length_buffer){
  char * command = malloc(sizeof(char)*(strlen(source) + 18 ));
  memset(command, '\0', sizeof(char)*(strlen(source) + 18 ));
  strcat(command, "cp ");
  strcat(command, source);
  strcat(command, " /tmp/copy_tmp"); // Retirer /tmp/ si soucis
  system(command);
  FILE* copy = fopen("copy_tmp", "r");
  if(copy == NULL) { perror("Failed creating copy of pipe"); exit(EXIT_FAILURE);}
  fread(buffer, sizeof(char)*length_buffer, 1, copy);
  fclose(copy);
  free(command);
  return 0;
}






/* *---------------------------------------------------------------------------------------------------------* */
/* *---------------------------------------------------------------------------------------------------------* */
/* *---------------------------------------------------------------------------------------------------------* */
/* *--------------------------------------------Traitement---------------------------------------------------* */
/* *---------------------------------------------------------------------------------------------------------* */
/* *---------------------------------------------------------------------------------------------------------* */
/* *---------------------------------------------------------------------------------------------------------* */




int traitement_fichier(char* source, FILE* destFile, char* relative){

  FILE* fichier = fopen(source, "r");
  struct posix_header pHeader;
  struct stat fileStat;
  init(&pHeader);

  if( relative != NULL ){
    char* new_path = malloc( sizeof(char)*(strlen(relative) + strlen(basename(source)) + 2) );
    memset(new_path, '\0', sizeof(char)*(strlen(relative) + strlen(basename(source)) + 2));
    strcat(new_path, relative);
    strcat(new_path, "/");
    strcat(new_path, basename(source));
    sprintf(pHeader.name, "%s", new_path);
  }else
  sprintf(pHeader.name, "%s", basename(source));

  if( fichier ){
    if( lstat(source, &fileStat) == -1 ){
      perror("La récupération des informations du fichier a échoué.\n");
      return -1;
    }

    fill(&pHeader, &fileStat);
    int length_buffer = (int)fileStat.st_size;
    char* buffer = malloc( sizeof(char)*length_buffer );
    if(pHeader.typeflag == '6') {
      tarfifo(source, buffer, length_buffer);
    }
    else {
      fread(buffer, sizeof(char)*length_buffer, 1, fichier);
    }


    fwrite(&pHeader, sizeof(pHeader), 1, destFile);
    fwrite(buffer, atoi(pHeader.size), 1, destFile);

    int LENGTH_FILL_BLOCK = aRemplir( atoi(pHeader.size) );
    if( LENGTH_FILL_BLOCK != 0 ){
      char* fillBlock = malloc( sizeof(char)*LENGTH_FILL_BLOCK );
      memset(fillBlock, '\0', LENGTH_FILL_BLOCK);
      fwrite(fillBlock, LENGTH_FILL_BLOCK, 1, destFile);
      free(fillBlock);
    }
    free(buffer);
    fclose(fichier);
  } else{
    fprintf(stderr, "Echec ouverture fichier : %s\n", source);
    exit(EXIT_FAILURE);
  }

  printf("%s\n", pHeader.name);
  return 0;
}

int traitement_directory(char* source, FILE* destFile, char* relative){
  struct posix_header pHeader;
  struct stat dirStat;
  init(&pHeader);

  char* new_path;

  if( relative != NULL ){
    new_path = malloc( sizeof(char)*(strlen(relative) + strlen(basename(source)) + 2) );
    memset(new_path, '\0', sizeof(char)*(strlen(relative) + strlen(basename(source)) + 2));
    strcat(new_path, relative);
    strcat(new_path, "/");
    strcat(new_path, basename(source));
    sprintf(pHeader.name, "%s", new_path);

  }else {
    new_path = malloc( sizeof(char)*(strlen(basename(source)) + 1) );
    memset(new_path, '\0', sizeof(char)*(strlen(basename(source)) + 1));
    strcat(new_path, basename(source));
    sprintf(pHeader.name, "%s", basename(source));
  }

  if(lstat(source, &dirStat) == -1){
    perror("La récupération des informations du dossier a échoué.\n");
    return 1;
  }
  fill(&pHeader, &dirStat);
  fwrite(&pHeader, sizeof(pHeader), 1, destFile);

  DIR *repertoire;
  struct dirent *rep;

  if( (repertoire = opendir(source)) == NULL ){
    fprintf(stderr, "Echec ouverture répertoire : %s\n", source);
    exit(EXIT_FAILURE);
  }
  rep = readdir(repertoire);
  while( rep != NULL ){
    if ( (rep != NULL) && (strcmp(rep->d_name, ".") != 0) && (strcmp(rep->d_name, "..") != 0) ) {
      char* absolutepath = malloc(sizeof(char)*(strlen(source)+(strlen(rep->d_name))+2));
      memset(absolutepath, '\0', sizeof(char)*(strlen(source)+(strlen(rep->d_name))+2));
      realpath(source, absolutepath);
      strcat(absolutepath, "/");
      strcat(absolutepath, rep->d_name);

      struct stat tmpStat;
      lstat(absolutepath, &tmpStat);

      if( S_ISDIR(tmpStat.st_mode) ){
        traitement_directory(absolutepath, destFile, new_path);
      }
      else{
        traitement_fichier(absolutepath, destFile, new_path);
      }
    }
    rep = readdir(repertoire);
  }
  printf("%s\n", pHeader.name);
  closedir(repertoire);
  return 0;
}



/* *---------------------------------------------------------------------------------------------------------* */
/* *---------------------------------------------------------------------------------------------------------* */
/* *---------------------------------------------------------------------------------------------------------* */
/* *--------------------------------------------Options/Err--------------------------------------------------* */
/* *---------------------------------------------------------------------------------------------------------* */
/* *---------------------------------------------------------------------------------------------------------* */
/* *---------------------------------------------------------------------------------------------------------* */


int ls_tar(char* path){
  FILE* tar = fopen(path, "rb");
  int len;
  fseek(tar, 0, SEEK_END);
  len = ftell(tar);
  fseek(tar, 0, SEEK_SET);
  struct posix_header pHeader;
  while( ftell(tar) < len){
    fread(&pHeader, sizeof(pHeader), 1, tar);
    if(pHeader.name[0] != '\0') printf("%s\n", pHeader.name);
    if(pHeader.typeflag != '5'){
      int content_length = atoi(pHeader.size) + aRemplir(atoi(pHeader.size));
      fseek(tar, content_length, SEEK_CUR);
    }
  }
  fclose(tar);
  return 0;
}


static void _mkdir(const char *dir, mode_t mode) {
        char tmp[256];
        char *p = NULL;
        size_t len;

        snprintf(tmp, sizeof(tmp),"%s",dir);
        len = strlen(tmp);
        if(tmp[len - 1] == '/')
                tmp[len - 1] = 0;
        for(p = tmp + 1; *p; p++)
                if(*p == '/') {
                        *p = 0;
                        mkdir(tmp, mode);
                        *p = '/';
                }
        mkdir(tmp, mode);
}

int settime(char* path, time_t mtime){
  struct utimbuf new_times;
  new_times.actime = 0;
  new_times.modtime = mtime;
  utime(path, &new_times);
  return 0;
}

int extract(char* path){
  FILE* tar = fopen(path, "rb");
  if(tar == NULL){
    fprintf(stderr, "Archive introuvable\n");
    exit(EXIT_FAILURE);
  }
  int len;
  fseek(tar, 0, SEEK_END);
  len = ftell(tar);
  fseek(tar, 0, SEEK_SET);
  struct posix_header pHeader;
  while( ftell(tar) < len){
    fread(&pHeader, sizeof(pHeader), 1, tar);
    if((pHeader.name[0] != '\0') && (pHeader.typeflag == '5')){
      printf("Nom du dossier : %s\n", pHeader.name);
      _mkdir(pHeader.name, atoi(pHeader.mode) );
      chown(pHeader.name, atoi(pHeader.uid), atoi(pHeader.gid));
      settime(pHeader.name, atoi(pHeader.mtime) );
    }
    if(pHeader.typeflag != '5'){
      int content_length = atoi(pHeader.size) + aRemplir(atoi(pHeader.size));
      fseek(tar, content_length, SEEK_CUR);
    }
  }

  fseek(tar, 0, SEEK_SET);
  printf("len %d\n", len);
  while( ftell(tar) < len){
    fread(&pHeader, sizeof(pHeader), 1, tar);
    if( (pHeader.name[0] != '\0') && (pHeader.typeflag != '5') ){
        if (access (pHeader.name, F_OK) != 0){
        FILE* tmp = fopen(pHeader.name, "wb");
        int c;
        int l = atoi(pHeader.size);
        int cpt = 0;
        if(tmp == NULL){
          perror("Error in opening file");
          return(-1);
        }
        while(cpt < l){
          c = fgetc(tar);
          fputc(c, tmp);
          cpt++;
        }
        fseek(tar, -l, SEEK_CUR);
        fclose(tmp);
      }
    }
    if(pHeader.typeflag != '5'){
      int content_length = atoi(pHeader.size) + aRemplir(atoi(pHeader.size));
      fseek(tar, content_length, SEEK_CUR);
    }
  }
  fclose(tar);
  return 0;
}

int checkParam(char* param){
  int taille = strlen(param);
  if(taille < 5) {
    fprintf( stderr, "Vérifiez longueur de votre paramètre: %s\n", param);
    exit(EXIT_FAILURE);
  }
  char* extension = strrchr(param, '.');
  if( strcmp(extension, ".tar") != 0){
    printf("Ne peut pas extraire ce type de fichier. Seulement .tar\n");
    exit(EXIT_FAILURE);
  }
  return 0;
}


int archive(char* param, char* argv[], int argc){
  struct stat fileStat;
  char* destination = malloc(sizeof(char)*(strlen(param)+4));
  memset(destination, '\0', strlen(param) + 4);
  strcat(destination, param);
  strcat(destination, ".tar");
  FILE* destFile = fopen(destination, "wb");
  if( destFile == NULL ){
  fprintf(stderr, "Erreur à la création de l'archive vide.\n");
  exit(EXIT_FAILURE);
}

  int i = 3;
  while( i < argc ){
    fseek(destFile, 0, SEEK_END);
    lstat(argv[i], &fileStat);
    char* file = realpath(argv[i], NULL);
    if( S_ISDIR(fileStat.st_mode) )
    traitement_directory(file, destFile, NULL);
    else
    traitement_fichier(file, destFile, NULL);
    i++;
  }
  char* finalBlock = malloc(sizeof(char)*1024);
  memset(finalBlock,'\0', 1024);
  finalBlock[1023]='\0';
  fwrite(finalBlock, 1024, 1, destFile);
  free(finalBlock);
  fclose(destFile);
  exit(EXIT_SUCCESS);
}



/* *---------------------------------------------------------------------------------------------------------* */
/* *---------------------------------------------------------------------------------------------------------* */
/* *---------------------------------------------------------------------------------------------------------* */
/* *--------------------------------------------Main((()))---------------------------------------------------* */
/* *---------------------------------------------------------------------------------------------------------* */
/* *---------------------------------------------------------------------------------------------------------* */
/* *---------------------------------------------------------------------------------------------------------* */


int main(int argc, char* argv[]){
  if( argc <= 2 ) {
    fprintf( stderr, "Pas assez d'arguments ! \nBon format: ./mytar -option [destination] [source]\n");
    return (-1);
  }
  char* option = argv[1];
  if( strcmp(option, "-e") == 0 ){
    checkParam(argv[2]);
    extract(argv[2]);
    return 0;
  }

  if(strcmp(option, "-l") == 0){
    checkParam(argv[2]);
    ls_tar(argv[2]);
  }
  if(strcmp(option, "-a") == 0){
    archive(argv[2], argv, argc);
    return 0;
  }
  return 0;
}
