DEF_CMD (HLT, 0, 0,
{
    return OK;
})

DEF_CMD (PUSH, 1, 1,
{
    arg_t arg = 0;

    int err = GetArgs (cpu, cmd, &arg);
    if (err) return err;

    StackPush (&(cpu -> stk), arg);
})

DEF_CMD (POP, 2, 1,
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
            arg += (cpu -> regs [reg]) / cpu -> accuracy_coef;
        }

        if (cmd & ARG_IM ) arg += cpu -> code [(cpu -> ip)++];

        if (arg >= RAM_SIZE) return INCORRECT_RAM_ADRESS;

        val_ptr = (cpu -> ram + arg);
    }
    else
    {
        int reg = cpu -> code [(cpu -> ip)++];
        if (reg <= 0 || reg >= NUM_OF_REGS) return INCORRECT_REG;
                    
        val_ptr = cpu -> regs + reg;
    }   

    if ( StackPop (&(cpu -> stk), val_ptr) ) return EMPTY_STACK;

    if (cmd & ARG_MEM) PrintMem (cpu);
})

DEF_CMD (IN, 3, 0,
{
    arg_t val = 0;
    ScanArg (&val);
    StackPush (&(cpu -> stk), val * cpu -> accuracy_coef);
})

DEF_CMD (OUT, 4, 0,
{
    arg_t val = 0;
    int err = OK;

    err |= StackPop (&(cpu -> stk), &val);

    if (err) return EMPTY_STACK;

    PrintArg (val, cpu -> accuracy_coef);
})

DEF_CMD (ADD, 5, 0, 
{
    int x1 = 0, x2 = 0;
    int err = OK;
    
    err |= StackPop (&(cpu -> stk), &x1);
    err |= StackPop (&(cpu -> stk), &x2);

    if (err) return EMPTY_STACK;

    StackPush (&(cpu -> stk), x1 + x2);
})

DEF_CMD (SUB, 6, 0, 
{
    int x1 = 0, x2 = 0;
    int err = OK;

    err |= StackPop (&(cpu -> stk), &x1);
    err |= StackPop (&(cpu -> stk), &x2);

    if (err) return EMPTY_STACK;

    StackPush (&(cpu -> stk), x2 - x1);

    break;
})

DEF_CMD (MUL, 7, 0, 
{
    int x1 = 0, x2 = 0;
    int err = OK;

    err |= StackPop (&(cpu -> stk), &x1);
    err |= StackPop (&(cpu -> stk), &x2);

    if (err) return EMPTY_STACK;

    StackPush (&(cpu -> stk), x1 * x2 / cpu -> accuracy_coef);

})

DEF_CMD (DIV, 8, 0,
{
    arg_t x1 = 0, x2 = 0;
    int err = OK;

    err |= StackPop (&(cpu -> stk), &x1);
    err |= StackPop (&(cpu -> stk), &x2);

    if (err)     return EMPTY_STACK;
    if (x1 == 0) return DIV_BY_ZERO;

    StackPush (&(cpu -> stk), x2 * cpu -> accuracy_coef / x1);
})

DEF_CMD (DUMP, 9, 0,
{
    printf ("\nCPU DUMP:\n\n");
    PrintCode (cpu, stdout);
    PrintRegs (cpu, stdout);
    printf ("\n");
})

DEF_CMD (JMP, 10, 1,
{
    arg_t arg = 0;

    int err = GetArgs (cpu, cmd, &arg);
    if (err) return err;

    arg = arg / cpu -> accuracy_coef;

    if (arg >= cpu -> code_size) return INCORRECT_JMP_IP;

    cpu -> ip = arg;
})

#define DEF_JMP(name, num ,op)                              \
DEF_CMD (name, num, 1,                                      \
{                                                           \
    int x1 = 0, x2 = 0;                                     \
    int err = OK;                                           \
                                                            \
    err |= StackPop (&(cpu -> stk), &x1);                   \
    err |= StackPop (&(cpu -> stk), &x2);                   \
                                                            \
    if (err) return EMPTY_STACK;                            \
                                                            \
    if (!(x2 op x1))                                        \
    {                                                       \
        cpu -> ip ++;                                       \
        break;                                              \
    }                                                       \
                                                            \
    arg_t arg = 0;                                          \
                                                            \
    err |= GetArgs (cpu, cmd, &arg);                        \
    if (err) return err;                                    \
                                                            \
    arg = arg / cpu -> accuracy_coef;                       \
                                                            \
    if (arg >= cpu -> code_size) return INCORRECT_JMP_IP;   \
                                                            \
    cpu -> ip = arg;                                        \
})

DEF_JMP (JA , 11,  >)

DEF_JMP (JAE, 12, >=)

DEF_JMP (JB , 13,  <)

DEF_JMP (JBE, 14, <=)

DEF_JMP (JE , 15, ==)

DEF_JMP (JNE, 16, !=)
/*
DEF_NONARITHM_JMP (JMP, 17, 1)
DEF_NONARITHM_JMP (JMP, 17, 1)
*/
#undef DEF_JMP


DEF_CMD (CALL, 17, 1,
{
    arg_t arg = 0;

    int err = GetArgs (cpu, cmd, &arg);
    if (err) return err;

    arg = arg / cpu -> accuracy_coef;

    if (arg >= cpu -> code_size) return INCORRECT_JMP_IP;

    StackPush (&(cpu -> call_stk), cpu -> ip);

    cpu -> ip = arg;
})

DEF_CMD (RET, 18, 0,
{
    int ip = 0;

    int err = StackPop (&(cpu -> call_stk), &ip);
    if (err) return EMPTY_CALL_STACK;

    cpu -> ip = ip;
})

DEF_CMD (SQRT, 19, 0,
{
    arg_t x = 0;
    int err = OK;

    err |= StackPop (&(cpu -> stk), &x);
    if (err) return EMPTY_STACK;

    if (x < 0) return SQRT_OF_NEG;

    x = (arg_t) (sqrt (((double) x) / cpu -> accuracy_coef) * cpu -> accuracy_coef); 

    StackPush (&(cpu -> stk),  x);

})

DEF_CMD (JMON, 20, 1,
{    
    arg_t arg = 0;

    int err = GetArgs (cpu, cmd, &arg);
    if (err) return err;

    arg = arg / cpu -> accuracy_coef;

    if (arg >= cpu -> code_size) return INCORRECT_JMP_IP;
    
    if (MondayToday()) cpu -> ip = arg;
})