#include "proc.h"


int main (int argc, char *argv[])
{    
    const char *input_file_name = nullptr;

    if (argc >= 2)  input_file_name = argv [1];
    else            input_file_name = "a";

    struct Cpu_t cpu = {};
    int err = OK;

    err = ReadCode (input_file_name, &cpu);
    if (err)
    {
        CpuErr (&cpu, err, ERROR_STREAM);
        return err;
    }

    err = InfoCheck (&cpu);
    if (err)
    {
        CpuErr (&cpu, err, ERROR_STREAM);
        return err;
    }

    err = RunCode (&cpu);
    if (err)
    {
        CpuErr (&cpu, err, ERROR_STREAM);
        return err;
    }

    FreeCpu (&cpu);

    return OK;
}


int ReadCode (const char *input_file_name, Cpu_t *cpu)
{
    if (input_file_name == nullptr) return NULLPTR_ARG;
    if             (cpu == nullptr) return NULLPTR_ARG;

    FILE *inp_file = fopen (input_file_name, "rb");
    if (inp_file == nullptr) return FOPEN_ERROR;

    size_t filesize = GetSize (inp_file);
    cpu -> code_size = filesize / CMD_SIZE - 3;

    cpu -> code = (cmd_t *) calloc (1, filesize);
    if (cpu -> code == nullptr) return ALLOC_ERROR;
    
    fread (cpu -> code, 1, filesize, inp_file);
    cpu -> code += 3; 

    fclose (inp_file);

    return OK;
}

size_t GetSize (FILE *inp_file)
{
    if (inp_file == nullptr) return 0;
    struct stat stat_buf = {};

    fstat (fileno (inp_file), &stat_buf);
    return stat_buf.st_size;
}


int InfoCheck (Cpu_t *cpu)
{
    if (cpu == nullptr) return NULLPTR_ARG;

    if (cpu -> code [-3] != SIGNATURE)            return WRONG_SIGNATURE;
    if (cpu -> code [-2] != VERSION)              return WRONG_VERSION;
    if (cpu -> code [-1] != cpu -> code_size + 3) return WRONG_CODESIZE;
    
    return OK;
}


int RunCode (Cpu_t *cpu)
{
    if (cpu         == nullptr) return NULLPTR_ARG;
    if (cpu -> code == nullptr) return NULLPTR_ARG;

    StackCtor (&(cpu -> stk), cpu -> code_size / 8);
    cpu -> ip = 0;

    while (1)
    {
        cmd_t cmd = (cpu -> code [(cpu -> ip)++]);

        switch (cmd & CMD_MASK)
        {
            case CMD_HLT:
            {
                return OK;
            }
            case CMD_PUSH:
            {
                arg_t arg = 0;
                if (cmd & ARG_REG) 
                {
                    int reg = cpu -> code [(cpu -> ip)++];
                    if (reg <= 0 || reg >= NUM_OF_REGS) return INCORRECT_REG;
                    arg += cpu -> regs [reg];
                }

                if (cmd & ARG_IM ) arg += cpu -> code [(cpu -> ip)++];

                if (cmd & ARG_MEM)
                {
                    if (arg >= RAM_SIZE) return INCORRECT_RAM_ADRESS;
                    arg = cpu -> ram [arg];
                }

                StackPush (&(cpu -> stk), arg);
                
                break;
            }
            case CMD_POP:
            {
                if (cmd & ARG_IM && !(cmd & ARG_MEM)) return INCORRECT_ARG_TYPE;
                
                arg_t *val_ptr = nullptr;
                
                if (cmd & ARG_MEM)
                {   
                    arg_t arg = 0;

                    if (cmd & ARG_REG)
                    {
                        int reg = cpu -> code [(cpu -> ip)++];
                        if (reg <= 0 || reg >= NUM_OF_REGS) return INCORRECT_REG;
                        arg += cpu -> regs [reg];
                    }

                    if (cmd & ARG_IM ) arg += cpu -> code [(cpu -> ip)++];

                    if (arg >= RAM_SIZE) return INCORRECT_RAM_ADRESS;

                    val_ptr = cpu -> ram + arg;
                }
                else
                {
                    int reg = cpu -> code [(cpu -> ip)++];
                    if (reg <= 0 || reg >= NUM_OF_REGS) return INCORRECT_REG;
                    
                    val_ptr = cpu -> regs + reg;
                }   

                if ( StackPop (&(cpu -> stk), val_ptr) ) return EMPTY_STACK;

                break;
            }
            case CMD_ADD:
            {
                int x1 = 0, x2 = 0;
                int err = OK;

                err |= StackPop (&(cpu -> stk), &x1);
                err |= StackPop (&(cpu -> stk), &x2);

                if (err) return EMPTY_STACK;

                StackPush (&(cpu -> stk), x1 + x2);

                break;
            }
            case CMD_MUL:
            {
                int x1 = 0, x2 = 0;
                int err = OK;

                err |= StackPop (&(cpu -> stk), &x1);
                err |= StackPop (&(cpu -> stk), &x2);

                if (err) return EMPTY_STACK;

                StackPush (&(cpu -> stk), x1 * x2);

                break;
            }
            case CMD_SUB:
            {
                int x1 = 0, x2 = 0;
                int err = OK;

                err |= StackPop (&(cpu -> stk), &x1);
                err |= StackPop (&(cpu -> stk), &x2);

                if (err) return EMPTY_STACK;

                StackPush (&(cpu -> stk), x2 - x1);

                break;
            }
            case CMD_DIV:
            {
                arg_t x1 = 0, x2 = 0;
                int err = OK;

                err |= StackPop (&(cpu -> stk), &x1);
                err |= StackPop (&(cpu -> stk), &x2);

                if (err)     return EMPTY_STACK;
                if (x1 == 0) return DIV_BY_ZERO;

                StackPush (&(cpu -> stk), x2 / x1);

                break;
            }
            case CMD_OUT:
            {
                arg_t val = 0;
                int err = OK;

                err |= StackPop (&(cpu -> stk), &val);

                if (err) return EMPTY_STACK;

                PrintArg (val);

                break;
            }
            case CMD_IN:
            {
                arg_t val = 0;

                ScanArg (&val);

                StackPush (&(cpu -> stk), val);

                break;
            }
            default:
            {
                return UNKNOWN_CMD;
            }
        }
    }

}


void FreeCpu (Cpu_t *cpu)
{
    if (cpu == nullptr) return;

    free (cpu -> code - 3);
    
    cpu -> ip = 0;
    cpu -> code_size = 0;

    StackDtor (&(cpu -> stk));

    memset ((void *) (cpu -> regs), 0, sizeof (cpu -> regs [0]) * NUM_OF_REGS);
    memset ((void *) (cpu -> ram ), 0, sizeof (cpu -> ram  [0]) * RAM_SIZE);
}



int PrintArg (arg_t arg)
{
    return printf ("%d\n", arg);
}

int ScanArg (arg_t *arg)
{
    return scanf ("%d", arg);
}


void CpuErr (Cpu_t *cpu, int err, FILE *stream)
{
    fprintf (stream, "ERROR: %d\n", err);

    if (cpu == nullptr || err == NULLPTR_ARG) fprintf (stream, "Nullptr error.\n");
    else if (err == FOPEN_ERROR)              fprintf (stream, "Cannot open the file.\n");
    else if (err == ALLOC_ERROR)              fprintf (stream, "Cannot allocate memory.\n");
    else if (err == WRONG_SIGNATURE)          fprintf (stream, "Code file has wrong signature.\n");
    else if (err == WRONG_VERSION)            fprintf (stream, "Code version differs from cpu version:\n"
                                                               "code version - %d;\n"
                                                               "cpu  version - %d.\n", cpu -> code [-2], VERSION);
    else if (err == WRONG_CODESIZE)           fprintf (stream, "Code file has wrong code size:\n");
    else
    {
        PrintCode (cpu, stream);
             if (err == EMPTY_STACK)          fprintf (stream, "Cannot get a value from stack.\n");
        else if (err == DIV_BY_ZERO)          fprintf (stream, "Division by zero.\n");
        else if (err == UNKNOWN_CMD)          fprintf (stream, "Unknown command.\n");
        else if (err == INCORRECT_REG)        fprintf (stream, "Incorrect register name.\n");
        else if (err == INCORRECT_RAM_ADRESS) fprintf (stream, "Incorrect RAM adress.\n");
        else if (err == INCORRECT_ARG_TYPE)   fprintf (stream, "Incorrect argument type.\n");
    }
}

void PrintCode (Cpu_t *cpu, FILE *stream)
{
    int  left = cpu -> ip > 5 ? cpu -> ip - 5 : 0;
    int right = left + 11 < cpu -> code_size ? left + 11 : cpu -> code_size;

    fprintf (stream, "IP:   ");
    for (int index = left; index < right; index ++) fprintf (stream, " %04d ", index);
    fprintf (stream, "\nCMD:  ");
    for (int index = left; index < right; index ++) fprintf (stream, " %04X ", cpu -> code [index]);
    fprintf (stream, "\n"      );
    for (int index = left; index < right; index ++) fprintf (stream, index == cpu -> ip ? "   ^  " : "      " );
    fprintf (stream, "\n");
}
