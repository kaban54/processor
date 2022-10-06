#ifndef PROC_H
#define PROC_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>


typedef double arg_t;
typedef int    cmd_t;

const int VERSION = 1;
const int SIGNATURE = 0x54ABC228;

const size_t CMD_LEN = 64;

const size_t  ARG_SIZE = sizeof (arg_t);
const size_t  CMD_SIZE = sizeof (cmd_t);
const size_t INFO_SIZE = sizeof (SIGNATURE) + sizeof (VERSION) + sizeof (int);


struct Text
{
    size_t    len;
    size_t buflen;
    char  *buffer; 
    char  **lines;
};

enum ERRORS
{
    OK           = 0,
    NULLPTR_ARG  = 1,
    FOPEN_ERROR  = 2,
    ALLOC_ERROR  = 3,
    COMP_ERROR   = 4,
    FWRITE_ERROR = 5,
};

enum CMDS
{
    HLT  =  0,
    PUSH =  1,
    POP  =  2,
    IN   =  3,
    OUT  =  4,

    ADD  = 32,
    SUB  = 33,
    MUL  = 34,
    DIV  = 35,
    POW  = 36,
    ABS  = 37,
};


int ReadText (const char *input_file_name, struct Text *txt);

size_t GetSize (FILE *inp_file);

size_t CharReplace (char *str, char ch1, char ch2);

void FreeText (struct Text *txt);

int SetLines (struct Text *txt);

int SetCmds (struct Text *txt, cmd_t **cmds_p);

int WriteCmds (const char *output_file_name, cmd_t *cmds);


#endif