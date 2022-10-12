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
})

DEF_CMD (IN, 3, 0,
{
    arg_t val = 0;
    ScanArg (&val);
    StackPush (&(cpu -> stk), val);
})

DEF_CMD (OUT, 4, 0,
{
    arg_t val = 0;
    int err = OK;

    err |= StackPop (&(cpu -> stk), &val);

    if (err) return EMPTY_STACK;

    PrintArg (val);
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

    StackPush (&(cpu -> stk), x1 * x2);

})

DEF_CMD (DIV, 8, 0,
{
    arg_t x1 = 0, x2 = 0;
    int err = OK;

    err |= StackPop (&(cpu -> stk), &x1);
    err |= StackPop (&(cpu -> stk), &x2);

    if (err)     return EMPTY_STACK;
    if (x1 == 0) return DIV_BY_ZERO;

    StackPush (&(cpu -> stk), x2 / x1);
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

    if (arg >= cpu -> code_size) return INCORRECT_JMP_IP;

    cpu -> ip = arg;
})
