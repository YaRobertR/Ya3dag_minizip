/*
minizip.c
Version 1.01e, February 12th, 2005

  Copyright (C) 1998-2005 Gilles Vollant
*/

/*
04.07.2008 RR: modified to allow wildcards in the files to add

  NOTE: This tools don't replace existing entries with the same name.
  Adding a file twice, it is two times in the archive.

29.08.2026 RR: * Reworked AddFileToZipRecursive() skip of current/parent directory.
                 Now, explicit test for current or parent directory.
                 Before the file/directory was skipped if the first character was a dot.
                 This allows files or directories beginning with a dot.
               * For none silent mode added printout of compressed files per sub directory.

*/

//x/#define USE_TEST_PRINTS    1  // Define this to output some test prints

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#include <direct.h>
#include <io.h>

#include "zip.h"

#ifdef WIN32
#define USEWIN32IOAPI
#include "iowin32.h"
#endif


#define WRITEBUFFERSIZE (16384)
#define MAXFILENAME (256)

//--------------------------------------------------------------------

/*
 * Disable MSVCRT command line wildcard expansion (__getmainargs called from
 * mingw startup code, see init.c in mingw runtime).
 */

#ifdef WIN32
int _CRT_glob = 0;    // disable wildcard globbing
#endif

//--------------------------------------------------------------------

uLong filetime( char *f,                /* name of file to get info on */
                tm_zip *tmzip,             /* return value: access, modific. and creation times */
                uLong *dt)             /* dostime */
{
  int ret = 0;
  {
    FILETIME ftLocal;
    HANDLE hFind;
    WIN32_FIND_DATA  ff32;

    hFind = FindFirstFile(f,&ff32);
    if (hFind != INVALID_HANDLE_VALUE)
    {
      FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
      FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
      FindClose(hFind);
      ret = 1;
    }
  }
  return ret;
}

// ignore files

#define MAX_IGNOREFILES 64

static int nIgnoreFiles;

static char *IgnoreFiles[ MAX_IGNOREFILES];

// ...

int check_exist_file(filename)
const char* filename;
{
  FILE* ftestexist;
  int ret = 1;
  ftestexist = fopen(filename,"rb");
  if (ftestexist==NULL)
    ret = 0;
  else
    fclose(ftestexist);
  return ret;
}

void do_banner()
{
  printf("Ya3dag_minizip\n");
}

void do_help()
{
  printf("Usage : Ya3dag_minizip [-o] [-a] [-s] [-n] [-0 to -9] [-i string] file.zip [files_to_add]\n\n" \
    "  -o  Overwrite existing file.zip\n" \
    "  -a  Append to existing file.zip\n" \
    "  -s  silent (less text output)\n" \
    "  -n  no recursive directory walking\n" \
    "  -i  ignore if string is part of filename to add\n" \
    "  -0  Store only\n" \
    "  -1  Compress faster\n" \
    "  -9  Compress better\n\n");
}

/* calculate the CRC32 of a file,
because to encrypt a file, we need known the CRC32 of the file before */
int getFileCrc(const char* filenameinzip,void*buf,unsigned long size_buf,unsigned long* result_crc)
{
  unsigned long calculate_crc=0;
  int err=ZIP_OK;
  FILE * fin = fopen(filenameinzip,"rb");
  unsigned long size_read = 0;
  unsigned long total_read = 0;
  if (fin==NULL)
  {
    err = ZIP_ERRNO;
  }

  if (err == ZIP_OK) {

    do {

      err = ZIP_OK;
      size_read = (int)fread(buf,1,size_buf,fin);

      if (size_read < size_buf) {

        if (feof(fin)==0) {
          printf("error in reading %s\n",filenameinzip);
          err = ZIP_ERRNO;
        }

        if( size_read > 0) {
          calculate_crc = crc32(calculate_crc,buf,size_read);
        }
        total_read += size_read;
      }
    } while ((err == ZIP_OK) && (size_read>0));

    if (fin)
      fclose(fin);

    *result_crc=calculate_crc;
    printf( "file %s crc %x\n", filenameinzip, (unsigned int)calculate_crc);
    return err;
  }

  return( err);
}


// --- add file

static void AddFileToZip( zipFile zf, char *filenameinzip,
                         char* password, void* buf, int size_buf, int opt_compress_level,
                         int *pnFiles)
{
  zip_fileinfo zi;
  unsigned long crcFile=0;
  int err=0;
  FILE * fin = NULL;
  int size_read;
  int i;
  char filename_try[MAXFILENAME+16];

  // test ignore files

  strcpy( filename_try, filenameinzip);
  _strlwr( filename_try);

  for( i = 0; i < nIgnoreFiles; i++) {

    if( strstr( filename_try, IgnoreFiles[ i]) != NULL) {   // if is in ignore list
      return;                                               // nothing to do
    }
  }

  // ...

  zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
    zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
  zi.dosDate = 0;
  zi.internal_fa = 0;
  zi.external_fa = 0;
  filetime(filenameinzip,&zi.tmz_date,&zi.dosDate);

  /*
  err = zipOpenNewFileInZip(zf,filenameinzip,&zi,
  NULL,0,NULL,0,NULL / * comment * /,
  (opt_compress_level != 0) ? Z_DEFLATED : 0,
  opt_compress_level);
  */
  if ((password != NULL) && (err==ZIP_OK))
    err = getFileCrc(filenameinzip,buf,size_buf,&crcFile);

  err = zipOpenNewFileInZip3(zf,filenameinzip,&zi,
    NULL,0,NULL,0,NULL /* comment*/,
    (opt_compress_level != 0) ? Z_DEFLATED : 0,
    opt_compress_level,0,
    /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
    -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
    password,crcFile);

  if (err != ZIP_OK)
    printf("error in opening %s in zipfile\n",filenameinzip);
  else
  {
    fin = fopen(filenameinzip,"rb");
    if (fin==NULL)
    {
      err=ZIP_ERRNO;
      printf("error in opening %s for reading\n",filenameinzip);
    }
  }

  if (err == ZIP_OK) {

    *pnFiles += 1;

    do {

      err = ZIP_OK;
      size_read = (int)fread(buf,1,size_buf,fin);

      if (size_read < size_buf) {

        if (feof(fin)==0) {

          printf("error in reading %s\n",filenameinzip);
          err = ZIP_ERRNO;
        }

        if( size_read > 0) {

          err = zipWriteInFileInZip (zf,buf,size_read);
          if (err<0)
          {
            printf("error in writing %s in the zipfile\n",
              filenameinzip);
          }
        }
      }
    } while ((err == ZIP_OK) && (size_read>0));
  }

  if (fin)
    fclose(fin);

  if (err<0)
    err=ZIP_ERRNO;
  else
  {
    err = zipCloseFileInZip(zf);
    if (err!=ZIP_OK)
      printf("error in closing %s in the zipfile\n",
      filenameinzip);
  }
}

// --- add file

static void AddFileToZipRecursive( char *pDir, char *pFilemask, zipFile zf,
                                   char* password, void* buf, int size_buf, int opt_compress_level,
                                   int *pnFiles, int opt_recursive, int opt_silent)
{
  HANDLE hFind;
  WIN32_FIND_DATA  ff32;
  int FilesBefore;
  char filenameinzip[ 512];

  // test for files

  FilesBefore = *pnFiles;                         // Remember number of compressed files until now

  strcpy( filenameinzip, pDir);                   // merge directory part
  if( pDir[ 0] != (char)'\0') {
    strcat( filenameinzip, "\\");                 // add path delimitter
  }
  strcat( filenameinzip, pFilemask);              // with filename

  hFind = FindFirstFile( filenameinzip, &ff32);

  if (hFind != INVALID_HANDLE_VALUE)
  {

    for( ; ; ) {

      if( ff32.cFileName[ 0] == '.'  &&           // skip current directory
          ff32.cFileName[ 1] == '\0') {
        goto SkipAddFile1;
      }

      if( ff32.cFileName[ 0] == '.'  &&           // skip parent directory
          ff32.cFileName[ 1] == '.'  &&
          ff32.cFileName[ 2] == '\0') {
        goto SkipAddFile1;
      }

      if( ff32.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {     // a directory

        // skip directories here

        goto SkipAddFile1;
      }

      strcpy( filenameinzip, pDir);              // merge directory part
      if( pDir[ 0] != (char)'\0') {
        strcat( filenameinzip, "\\");            // add path delimitter
      }
      strcat( filenameinzip, ff32.cFileName);    // with filename

      //printf("File %s --> %s\n", ff32.cFileName, filenameinzip);

      AddFileToZip( zf, filenameinzip, password, buf, size_buf, opt_compress_level, pnFiles);


SkipAddFile1:

      if( FindNextFile( hFind, &ff32) == 0) {
        break;
      }
    }
    FindClose(hFind);
  } else {

    //x/printf("file %s not found\n", filenameinzip);
  }

  // Print files for this directory if not silent

  if( opt_silent == 0 &&                          // More output
      *pnFiles - FilesBefore > 0) {               // and compressed any files

    printf( "%s added %d file(s)\n", pDir, *pnFiles - FilesBefore);
  }

  // test for directories

  strcpy( filenameinzip, pDir);                   // merge directory part
  if( pDir[ 0] != (char)'\0') {
    strcat( filenameinzip, "\\");                 // add path delimitter
  }
  strcat( filenameinzip, "*");                    // want all files

  hFind = FindFirstFile( filenameinzip, &ff32);

  if (hFind != INVALID_HANDLE_VALUE)
  {

    for( ; ; ) {

      if( ff32.cFileName[ 0] == '.'  &&           // skip current directory
          ff32.cFileName[ 1] == '\0') {
        goto SkipAddFile2;
      }

      if( ff32.cFileName[ 0] == '.'  &&           // skip parent directory
          ff32.cFileName[ 1] == '.'  &&
          ff32.cFileName[ 2] == '\0') {
        goto SkipAddFile2;
      }

      if( ff32.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {     // a directory

        if( opt_recursive) {                                      // recursive directory walk is enabled

          char dir[ 512];

          // recurisve walk the directories

          strcpy( dir, pDir);
          if( pDir[ 0] != (char)'\0') {
            strcat( dir, "\\");                    // add path delimitter
          }
          strcat( dir, ff32.cFileName);            // with filename

          AddFileToZipRecursive( dir, pFilemask, zf, password, buf, size_buf, opt_compress_level, pnFiles, opt_recursive, opt_silent);
        }

        goto SkipAddFile2;
      }

      // skip data files here

SkipAddFile2:

      if( FindNextFile( hFind, &ff32) == 0) {
        break;
      }
    }
    FindClose(hFind);
  } else {

    //x/printf("file %s not found\n", filenameinzip);
  }
}

// --- main

int main(argc,argv)
int argc;
char *argv[];
{
  int i;
  int opt_overwrite=0;
  int opt_compress_level=Z_DEFAULT_COMPRESSION;
  int opt_silent = 0;
  int opt_recursive = 1;  // allow recurisive directory walk, default is on
  int zipfilenamearg = 0;
  char filename_try[MAXFILENAME+16];
  int zipok;
  int err=0;
  int size_buf=0;
  void* buf=NULL;
  char* password=NULL;


  if (argc==1)
  {
    do_banner();
    do_help();
    return 0;
  }
  else
  {
    for (i=1;i<argc;i++)
    {
      if ((*argv[i])=='-')
      {
        const char *p=argv[i]+1;

        while ((*p)!='\0')
        {
          char c=*(p++);;

          if ((c=='o') || (c=='O')) {

            opt_overwrite = 1;

          } else if ((c=='a') || (c=='A')) {

            opt_overwrite = 2;

          } else if ((c=='s') || (c=='S')) {

            opt_silent = 2;

          } else if ((c=='n') || (c=='N')) {

            opt_recursive = 0;  // NO recurisive directory walk

          } else if ((c>='0') && (c<='9')) {

            opt_compress_level = c-'0';

          } else if (((c=='p') || (c=='P')) && (i+1<argc)) {

            password=argv[i+1];
            i++;

          } else if (((c=='i') || (c=='i')) && (i+1<argc)) {

            if( nIgnoreFiles < MAX_IGNOREFILES) {
              IgnoreFiles[ nIgnoreFiles] = argv[i+1];
              _strlwr( IgnoreFiles[ nIgnoreFiles]);
              nIgnoreFiles++;
            }
            i++;
          }
        }
      }
      else
        if (zipfilenamearg == 0)
          zipfilenamearg = i ;
    }
  }

  if( opt_silent == 0) {
    do_banner();
  }

  size_buf = WRITEBUFFERSIZE;
  buf = (void*)malloc(size_buf);
  if (buf==NULL)
  {
    printf("Error allocating memory\n");
    return ZIP_INTERNALERROR;
  }

  if (zipfilenamearg==0)
    zipok=0;
  else
  {
    int i,len;
    int dot_found=0;

    zipok = 1 ;
    strncpy(filename_try, argv[zipfilenamearg],MAXFILENAME-1);
    /* strncpy doesnt append the trailing NULL, of the string is too long. */
    filename_try[ MAXFILENAME ] = '\0';

#ifdef USE_TEST_PRINTS
    printf( "main: TP 1: %s\n", filename_try)
#endif

    len=(int)strlen(filename_try);
    for( i = 0; i < len; i++) {

      if (filename_try[i]=='.')
        dot_found=1;
    }

    if (dot_found==0)
      strcat(filename_try,".zip");

    if (opt_overwrite==2) {
      /* if the file don't exist, we not append file */
      if (check_exist_file(filename_try)==0)
        opt_overwrite=1;

    } else {

      if (opt_overwrite==0) {

        if (check_exist_file(filename_try)!=0) {

          char rep=0;
          do
          {
            char answer[128];
            int ret;
            printf("The file %s exists. Overwrite ? [y]es, [n]o, [a]ppend : ",filename_try);
            ret = scanf("%1s",answer);
            if (ret != 1)
            {
              exit(EXIT_FAILURE);
            }
            rep = answer[0] ;
            if ((rep>='a') && (rep<='z'))
              rep -= 0x20;
          }
          while ((rep!='Y') && (rep!='N') && (rep!='A'));

          if (rep=='N')
            zipok = 0;

          if (rep=='A')
            opt_overwrite = 2;

        }
      }
    }
  }

  if (zipok==1)
  {
    zipFile zf;
    int errclose;
#        ifdef USEWIN32IOAPI
    zlib_filefunc_def ffunc;
    fill_win32_filefunc(&ffunc);
    zf = zipOpen2(filename_try,(opt_overwrite==2) ? 2 : 0,NULL,&ffunc);
#        else
    zf = zipOpen(filename_try,(opt_overwrite==2) ? 2 : 0);
#        endif

    if (zf == NULL)
    {
      printf("error opening %s\n",filename_try);
      err= ZIP_ERRNO;
    }
    else {
      //printf("creating %s\n",filename_try);
    }

    for (i=zipfilenamearg+1;(i<argc) && (err==ZIP_OK);i++)
    {
      if (!((((*(argv[i]))=='-') || ((*(argv[i]))=='/')) &&
        ((argv[i][1]=='o') || (argv[i][1]=='O') ||
        (argv[i][1]=='a') || (argv[i][1]=='A') ||
        (argv[i][1]=='s') || (argv[i][1]=='S') ||
        (argv[i][1]=='n') || (argv[i][1]=='N') ||
        (argv[i][1]=='i') || (argv[i][1]=='I') ||
        ((argv[i][1]>='0') || (argv[i][1]<='9'))) &&
        (strlen(argv[i]) == 2)))
      {
        char dir[ 512];
        char filemask[ 512];
        char *pPathDelimiter;
        int nFiles;

        strcpy( dir, argv[i]);
        pPathDelimiter = strrchr( dir, '\\');          // find last backslash
        if( pPathDelimiter == NULL) {
          pPathDelimiter = strrchr( dir, '/');         // find last slash
        }

        if( pPathDelimiter == NULL) {
          strcpy( filemask, dir);                      // filemask for search
          dir[ 0] = '\0';                              // have no path
        } else {
          strcpy( filemask, pPathDelimiter + 1);       // filemask for search
          *pPathDelimiter = '\0';                      // cat off add end of path
        }

        nFiles = 0;

        if( opt_silent != 0) {                // Less output
          printf("add %s ", argv[i]);
        }

        AddFileToZipRecursive( dir, filemask, zf, password, buf, size_buf, opt_compress_level, &nFiles, opt_recursive, opt_silent);

        if( opt_silent != 0) {                // Less output
          printf(" %d file(s)\n", nFiles);

        } else {

           printf("Total number of file(s) added: %d\n", nFiles);
        }
      }
    }
    errclose = zipClose(zf,NULL);
    if (errclose != ZIP_OK)
      printf("error in closing %s\n",filename_try);
  }
  else
  {
    do_help();
  }

  free(buf);
  return 0;
}
