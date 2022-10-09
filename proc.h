#ifndef PROC_H
#define PROC_H

typedef int    arg_t;
typedef int    cmd_t;
typedef arg_t Elem_t;

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include "stack.h"

const int VERSION = 2;
const int SIGNATURE = 0x54ABC228;

const size_t CMD_LEN = 64;

const size_t MAX_NUM_OF_ARGS = 2;
const size_t  ARG_SIZE = sizeof (arg_t);
const size_t  CMD_SIZE = sizeof (cmd_t);
const size_t INFO_SIZE = sizeof (SIGNATURE) + sizeof (VERSION) + sizeof (int);

const size_t ERROR_MSG_SIZE = 100;

FILE *ERROR_STREAM = stdout;

struct Text
{
    size_t    len;
    size_t buflen;
    char  *buffer; 
    char  **lines;
};

struct Cpu_t
{
    int ip;
    cmd_t *code;
    Stack_t stk;
    size_t code_size;

};


enum ERRORS
{
    OK              =  0,
    NULLPTR_ARG     =  1,
    FOPEN_ERROR     =  2,
    ALLOC_ERROR     =  3,
    COMP_ERROR      =  4,
    FWRITE_ERROR    =  5,
    WRONG_SIGNATURE =  6,
    WRONG_VERSION   =  7,
    WRONG_CODESIZE  =  8,
    DIV_BY_ZERO     =  9,
    EMPTY_STACK     = 10,
    UNKNOWN_CMD     = 11,
};

enum CMDS
{
    HLT  =  0,
    PUSH =  1,
    POP  =  2,
    IN   =  3,
    OUT  =  4,
    ADD  =  5,
    SUB  =  6,
    MUL  =  7,
    DIV  =  8,
};

// asm funcs ----------------------------------------------------------------------------------------------------------

int ReadText (const char *input_file_name, struct Text *txt);

size_t GetSize (FILE *inp_file);

size_t CharReplace (char *str, char ch1, char ch2);

void FreeText (struct Text *txt);

int SetLines (struct Text *txt);

int SetCmds (struct Text *txt, cmd_t **cmds_p);

int WriteCmds (const char *output_file_name, cmd_t *cmds);

//---------------------------------------------------------------------------------------------------------------------
// proc funcs ---------------------------------------------------------------------------------------------------------

int ReadCode (const char *input_file_name, Cpu_t *cpu);

int InfoCheck (Cpu_t *cpu);

int RunCode (Cpu_t *cpu);

void FreeCpu (Cpu_t *cpu);

int PrintArg (arg_t arg);

int ScanArg (arg_t *arg);

void CpuErr (Cpu_t *cpu, int err, FILE *stream);

void PrintCode (Cpu_t *cpu, FILE *stream);
//---------------------------------------------------------------------------------------------------------------------

#endif