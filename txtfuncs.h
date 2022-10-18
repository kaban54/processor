#ifndef TXTFUNCS_H
#define TXTFUNCS_H


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>
#include <time.h>


struct Text
{
    size_t    len;
    size_t buflen;
    char  *buffer; 
    char  **lines;
};

#ifndef ENUM_ERRORS
#define ENUM_ERRORS

enum ERRORS
{
    OK                   =  0,
    NULLPTR_ARG          =  1,
    FOPEN_ERROR          =  2,
    ALLOC_ERROR          =  3,
    COMP_ERROR           =  4,
    FWRITE_ERROR         =  5,
    WRONG_SIGNATURE      =  6,
    WRONG_VERSION        =  7,
    WRONG_CODESIZE       =  8,
    DIV_BY_ZERO          =  9,
    EMPTY_STACK          = 10,
    UNKNOWN_CMD          = 11,
    INCORRECT_REG        = 12,
    INCORRECT_RAM_ADRESS = 13,
    INCORRECT_ARG_TYPE   = 14,
    INCORRECT_JMP_IP     = 15,
    EMPTY_CALL_STACK     = 16,
    SQRT_OF_NEG          = 17,
};

#endif


int ReadText (const char *input_file_name, struct Text *txt);

size_t GetSize (FILE *inp_file);

size_t CharReplace (char *str, char ch1, char ch2);

void FreeText (struct Text *txt);

int SetLines (struct Text *txt);


#endif