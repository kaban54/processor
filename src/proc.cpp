#include "proc.h"


#define Ret_if_err(func)                    \
    err = func;                             \
    if (err)                                \
    {                                       \
        CpuErr (&cpu, err, ERROR_STREAM);   \
        return err;                         \
    }


int main (int argc, char *argv[])
{    
    const char *input_file_name = nullptr;

    if (argc >= 2)  input_file_name = argv [1];
    else            input_file_name = "a";

    struct Cpu_t cpu = {};
    int err = OK;

    Ret_if_err (CpuCtor (&cpu));

    Ret_if_err (ReadCode (input_file_name, &cpu));

    Ret_if_err (InfoCheck (&cpu));

    Ret_if_err (RunCode (&cpu));

    FreeCpu (&cpu);

    return OK;
}


int CpuCtor (Cpu_t *cpu)
{
    if (cpu == nullptr) return NULLPTR_ARG;

    StackCtor (&(cpu -> call_stk), CALL_STACK_BASE_CAPACITY);
    StackCtor (&(cpu ->      stk),      STACK_BASE_CAPACITY);

    memset (cpu -> regs, 0, sizeof (cpu -> regs [0]) * NUM_OF_REGS);
    memset (cpu -> ram , 0, sizeof (cpu -> ram  [0]) * RAM_SIZE);
    
    cpu -> ip = 0;
    cpu -> accuracy_coef = 1;

    cpu -> code_size = 0;
    cpu -> code = nullptr;

    return OK;
}


int ReadCode (const char *input_file_name, Cpu_t *cpu)
{
    if (input_file_name == nullptr) return NULLPTR_ARG;
    if             (cpu == nullptr) return NULLPTR_ARG;

    FILE *inp_file = fopen (input_file_name, "rb");
    if (inp_file == nullptr) return FOPEN_ERROR;

    size_t filesize = GetSize (inp_file);
    cpu -> code_size = (int)(filesize / CMD_SIZE - CODE_SHIFT);

    cpu -> code = (cmd_t *) calloc (1, filesize);
    if (cpu -> code == nullptr) return ALLOC_ERROR;
    
    fread (cpu -> code, 1, filesize, inp_file);
    cpu -> code += CODE_SHIFT;

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

    if (cpu -> code [SIGNATURE_POS] != SIGNATURE)        return WRONG_SIGNATURE;
    if (cpu -> code [  VERSION_POS] != VERSION)          return WRONG_VERSION;
    if (cpu -> code [ CODESIZE_POS] != cpu -> code_size) return WRONG_CODESIZE;
    
    return OK;
}


int RunCode (Cpu_t *cpu)
{
    if (cpu         == nullptr) return NULLPTR_ARG;
    if (cpu -> code == nullptr) return NULLPTR_ARG;

    cpu -> accuracy_coef = cpu -> code [ACCURACY_POS];

    while (1)
    {
        cmd_t cmd = (cpu -> code [(cpu -> ip)++]);

#define DEF_CMD(name, num, arg, ...)  \
    case CMD_##name:                  \
    {                                 \
        __VA_ARGS__                   \
        break;                        \
    }

        switch (cmd & CMD_MASK)
        {

            #include "cmd.h"

            default:
            {
                return UNKNOWN_CMD;
            }
        }

#undef DEF_CMD

    }
    return OK;
}

int GetArgs (Cpu_t *cpu, cmd_t cmd, arg_t *arg_p)
{
    if (cpu   == nullptr) return NULLPTR_ARG;
    if (arg_p == nullptr) return NULLPTR_ARG;

    arg_t arg = 0;

    if (cmd & ARG_REG) 
    {
        int reg = cpu -> code [(cpu -> ip)++];
        if (reg <= 0 || reg >= NUM_OF_REGS) return INCORRECT_REG;
        arg += cpu -> regs [reg];
    }

    if (cmd & ARG_IM ) arg += (cpu -> code [(cpu -> ip)++]) * cpu -> accuracy_coef;

    if (cmd & ARG_MEM)
    {
        arg = arg / cpu -> accuracy_coef;
        if (arg >= RAM_SIZE) return INCORRECT_RAM_ADRESS;
        arg = cpu -> ram [arg];
    }
    
    *arg_p = arg;

    return OK;
}

int GetArgAdress (Cpu_t *cpu, cmd_t cmd, arg_t **val_ptr_p)
{
    if (cpu       == nullptr) return NULLPTR_ARG;
    if (val_ptr_p == nullptr) return NULLPTR_ARG;

    if (cmd & ARG_IM && !(cmd & ARG_MEM)) return INCORRECT_ARG_TYPE;
                
    arg_t *val_ptr = nullptr;

    if (cmd & ARG_REG)
    {
        int reg = cpu -> code [(cpu -> ip)++];
        if (reg <= 0 || reg >= NUM_OF_REGS) return INCORRECT_REG;
        val_ptr = cpu -> regs + reg;
    }

    if (cmd & ARG_MEM)
    {   
        if (val_ptr == nullptr) val_ptr = cpu -> ram;
        else                    val_ptr = cpu -> ram + *val_ptr / cpu -> accuracy_coef;

        if (cmd & ARG_IM ) val_ptr += cpu -> code [(cpu -> ip)++];

        if (val_ptr >= cpu -> ram + RAM_SIZE) return INCORRECT_RAM_ADRESS;
    }

    *val_ptr_p = val_ptr;
    return OK;
}


void FreeCpu (Cpu_t *cpu)
{
    if (cpu == nullptr) return;

    free (cpu -> code - CODE_SHIFT);
    
    cpu -> ip = 0;
    cpu -> code_size = 0;

    StackDtor (&(cpu ->      stk));
    StackDtor (&(cpu -> call_stk));

    memset ((void *) (cpu -> regs), 0, sizeof (cpu -> regs [0]) * NUM_OF_REGS);
    memset ((void *) (cpu -> ram ), 0, sizeof (cpu -> ram  [0]) * RAM_SIZE);
}


int PrintMem (Cpu_t *cpu)
{
    if (cpu == nullptr) return NULLPTR_ARG;

    printf ("\n\n");

    if (WIDTH * HEIGHT > RAM_SIZE)
    {
        printf ("WIDTH * HEIGHT > MEMORY SIZE.\n");
        return OK;
    }

    for (size_t y = 0; y < HEIGHT; y++)
    {
        for (size_t pix_y = 0; pix_y < PIXEL_HEIGTH; pix_y++)
        {
            for (size_t x = 0; x < WIDTH; x++)
            {
                char symb = cpu -> ram [y * WIDTH + x] ? '#' : '.';
                for (size_t pix_x = 0; pix_x < PIXEL_WIDTH; pix_x++) putchar (symb);
                putchar (' ');
            }
            putchar ('\n');
        }
    }

    printf ("\n\n");

    return OK;
}

int PrintArg (arg_t arg, int accuracy_coef)
{
    if (accuracy_coef == 1) return printf ("%d\n", arg);
    else                    return printf ("%.3lf\n", (double) arg / accuracy_coef);  
}

int ScanArg (arg_t *arg)
{
    return scanf ("%d", arg);
}


void CpuErr (Cpu_t *cpu, int err, FILE *stream)
{
    fprintf (stream, "ERROR: %d\n", err);

    if (cpu == nullptr || err == NULLPTR_ARG) fprintf (stream, "Nullptr error.\n");
    else if (err == OK)                       fprintf (stream, "OK.\n"); 
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
        else if (err == SQRT_OF_NEG)          fprintf (stream, "Square root of negative number.\n");
        else if (err == UNKNOWN_CMD)          fprintf (stream, "Unknown command.\n");
        else if (err == INCORRECT_REG)        fprintf (stream, "Incorrect register name.\n");
        else if (err == INCORRECT_RAM_ADRESS) fprintf (stream, "Incorrect RAM adress.\n");
        else if (err == INCORRECT_ARG_TYPE)   fprintf (stream, "Incorrect argument type.\n");
        else if (err == INCORRECT_JMP_IP)     fprintf (stream, "Incorrect ip to jump.\n");
        else if (err == EMPTY_CALL_STACK)     fprintf (stream, "Cannot return (call stack is empty).\n");
        else                                  fprintf (stream, "Unknown error.\n");
    }
}

void PrintCode (Cpu_t *cpu, FILE *stream)
{
    if (cpu == nullptr || stream == nullptr) return;

    int  left = cpu -> ip > 5 ? cpu -> ip - 5 : 0;
    int right = left + 11 < cpu -> code_size ? left + 11 : cpu -> code_size;

    fprintf (stream, "IP:   ");
    for (int index = left; index < right; index ++) fprintf (stream, " %04X ", index);
    fprintf (stream, "\nCMD:  ");
    for (int index = left; index < right; index ++) fprintf (stream, " %04X ", cpu -> code [index]);
    fprintf (stream, "\n      ");
    for (int index = left; index < right; index ++) fprintf (stream, index == cpu -> ip ? "   ^  " : "      " );
    fprintf (stream, "\n");
}

void PrintRegs (Cpu_t *cpu, FILE *stream)
{
    if (cpu == nullptr || stream == nullptr) return;
 
    fprintf (stream, "Registers:\n");

    for (size_t reg = 1; reg < NUM_OF_REGS; reg++)
        fprintf (stream, "    r%cx = %d\n", (char)('a' - 1 + reg), cpu -> regs [reg]);
}


int MondayToday (void)
{
    struct tm monday = {};
    monday.tm_year = 122;
    monday.tm_mon = 9;
    monday.tm_mday = 17;
    monday.tm_hour = 4;
    monday.tm_sec = 1;
    // 17.10.2022 03:00:01 AM (monday)

    long difdays = (time (NULL) - mktime (&monday)) / SECS_IN_DAY;

    if (difdays % 7 == 0) return 1;

    return 0;
}