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
#include <math.h>
#include <time.h>
#include "stack.h"
#include "txtfuncs.h"

const int VERSION = 15;
const int SIGNATURE = 0x54ABC228;

const size_t BUFLEN = 128;
const size_t MAX_LABEL_LEN = 20;

const int NUM_OF_ASM = 2;

const size_t MAX_NUM_OF_ARGS = 2;
const size_t ARG_SIZE = sizeof (arg_t);
const size_t CMD_SIZE = sizeof (cmd_t);

const int CODE_SHIFT = 4;
const size_t INFO_SIZE = sizeof (cmd_t) * CODE_SHIFT;

const size_t ERROR_MSG_SIZE = 100;
FILE *ERROR_STREAM = stdout;

const int CMD_MASK = 0x000000FF;

const int NUM_OF_REGS = 5;
const int RAM_SIZE = 100;

const size_t WIDTH  = 10;
const size_t HEIGHT = 10;
const size_t PIXEL_WIDTH  = 5;
const size_t PIXEL_HEIGTH = 3;

const size_t      STACK_BASE_CAPACITY = 16;
const size_t CALL_STACK_BASE_CAPACITY =  8;

const int SECS_IN_DAY = 24 * 60 * 60;

const char ACCURACY_CMD_NAME [] = "#ACCURACY";

struct Cpu_t
{
    int ip;
    cmd_t *code;
    int code_size;

    Stack_t      stk;
    Stack_t call_stk;

    arg_t regs [NUM_OF_REGS];
    arg_t ram  [RAM_SIZE];

    int accuracy_coef;
};

struct Label_t
{
    char name [MAX_LABEL_LEN + 1];
    int ip;
};

struct Label_list_t
{
    size_t     num;
    size_t max_num;
    Label_t *list;
};

enum REGISTERS
{
    RAX = 1, 
    RBX = 2, 
    RCX = 3,
    RDX = 4,
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

#define DEF_CMD(name, num, arg, ...) CMD_##name = num,

enum CMDS
{
    #include "cmd.h"
};

#undef DEF_CMD


enum ARG_TYPES
{
    ARG_IM  = 0x100,
    ARG_REG = 0x200,
    ARG_MEM = 0x400,
};

// asm funcs ----------------------------------------------------------------------------------------------------------

int Compile (struct Text *txt, cmd_t **cmds_p);

int Assemble (Text *txt, cmd_t *cmds, Label_list_t *label_list, int pass);

int SetAccuracyCoef (cmd_t *cmds, char *line);

int LabelListCtor (Label_list_t *label_list);

int AddLabel (char *cmd, Label_list_t *label_list, int ip, size_t line);

int CheckLabelName (char **name_p, Label_list_t *label_list, size_t line);

int ExpandLabelList (Label_list_t *label_list);

int GetLabelIp (char *name, Label_list_t *label_list);

int WriteCmds (const char *output_file_name, cmd_t *cmds);

int PutArgs (char *args, cmd_t *cmds, int *ip, Label_list_t *label_list, size_t line, int loop);

char *DeleteSpaces (char *str);

void AsmErr (int err, FILE *stream);

//---------------------------------------------------------------------------------------------------------------------
// proc funcs ---------------------------------------------------------------------------------------------------------

int CpuCtor (Cpu_t *cpu);

int ReadCode (const char *input_file_name, Cpu_t *cpu);

int InfoCheck (Cpu_t *cpu);

int RunCode (Cpu_t *cpu);

int GetArgs (Cpu_t *cpu, cmd_t cmd, arg_t *arg);

void FreeCpu (Cpu_t *cpu);

int PrintArg (arg_t arg, int accuracy_coef);

int ScanArg (arg_t *arg);

void CpuErr (Cpu_t *cpu, int err, FILE *stream);

void PrintCode (Cpu_t *cpu, FILE *stream);

void PrintRegs (Cpu_t *cpu, FILE *stream);

int PrintMem (Cpu_t *cpu);

int MondayToday (void);
//---------------------------------------------------------------------------------------------------------------------

#endif