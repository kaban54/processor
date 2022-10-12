#include "proc.h"


#define Ret_if_err(func)                    \
    err = func;                             \
    if (err)                                \
    {                                       \
        AsmErr (err, ERROR_STREAM);         \
        return err;                         \
    }


int main (int argc, char *argv[])
{
    const char * input_file_name = nullptr;
    const char *output_file_name = nullptr;

    if (argc >= 2)  input_file_name = argv [1];
    else            input_file_name = "in.txt";

    if (argc >= 3) output_file_name = argv [2];
    else           output_file_name =      "a";


    cmd_t *commands = nullptr;
    struct Text txt = {};

    int err = OK;

    Ret_if_err (ReadText (input_file_name, &txt));

    Ret_if_err (Compile (&txt, &commands));

    FreeText (&txt);

    Ret_if_err (WriteCmds (output_file_name, commands));

    free (commands);

    return OK;
}

//---------------------------------------------------------------------------------------------------------------------

int ReadText (const char *input_file_name, struct Text *txt)
{    
    if (input_file_name == nullptr) return NULLPTR_ARG;
    if             (txt == nullptr) return NULLPTR_ARG;

    FILE *inp_file = fopen (input_file_name, "r");
    if (inp_file == nullptr) return FOPEN_ERROR;

    size_t filesize = GetSize (inp_file);

    txt -> buffer = (char *) calloc (filesize + 1, sizeof((txt -> buffer)[0]));
    if ((txt -> buffer) == nullptr) return ALLOC_ERROR;

    txt -> buflen = fread (txt -> buffer, sizeof((txt -> buffer)[0]), filesize, inp_file);
    *(txt -> buffer + txt -> buflen) = '\0';
    
    fclose (inp_file);

    txt -> len = CharReplace (txt -> buffer, '\n', '\0') + 1;

    SetLines (txt);

    return OK;
}


size_t GetSize (FILE *inp_file)
{
    if (inp_file == nullptr) return 0;
    struct stat stat_buf = {};

    fstat (fileno (inp_file), &stat_buf);
    return stat_buf.st_size;
}


size_t CharReplace (char *str, char ch1, char ch2)
{
    if (str == nullptr) return 0;

    size_t count = 0;
    str = strchr (str, ch1);

    while (str != nullptr)
    {
        count++;
        *str = ch2;
        str = strchr (str + 1, ch1);
    }

    return count;
}


int SetLines (struct Text *txt)
{
    if (txt           == nullptr) return NULLPTR_ARG;
    if (txt -> buffer == nullptr) return NULLPTR_ARG;

    txt -> lines = (char **) calloc (txt -> len, sizeof ((txt -> lines)[0]));
    if ((txt -> lines) == nullptr) return ALLOC_ERROR;

    char *str_ptr = txt -> buffer;

    for (size_t index = 0; index < txt -> len; index++)
    {
        txt -> lines [index] = str_ptr;
        str_ptr += strlen (str_ptr) + 1;
    }

    return OK;
}

void FreeText (struct Text *txt)
{
    free (txt -> buffer);
    free (txt ->  lines);
}

//---------------------------------------------------------------------------------------------------------------------

int Compile (struct Text *txt, cmd_t **cmds_p)
{
    if (txt           == nullptr) return NULLPTR_ARG;
    if (txt -> buffer == nullptr) return NULLPTR_ARG;
    if (txt ->  lines == nullptr) return NULLPTR_ARG;
    if (cmds_p        == nullptr) return NULLPTR_ARG;

    size_t size = (MAX_NUM_OF_ARGS * ARG_SIZE + CMD_SIZE) * (txt -> len) + INFO_SIZE;

    *cmds_p = (cmd_t *) (calloc (1, size));

    cmd_t *cmds = *cmds_p;

    if (cmds == nullptr) return ALLOC_ERROR;

    cmds [0] = SIGNATURE;
    cmds [1] = VERSION;

    int *ip = cmds + CODE_SHIFT;

    for (size_t line = 0; line < txt -> len; line++)
    {
        int symbs_read = 0;
        char cmd [BUFLEN] = "";

        sscanf (txt -> lines [line], "%s%n", cmd, &symbs_read);

        if (stricmp (cmd, "") == 0) continue;

#define DEF_CMD(name, num, arg, ...)                                                           \
    if (stricmp (cmd, #name) == 0)                                                             \
    {                                                                                          \
        *(ip++) |= CMD_##name;                                                                 \
        if (arg) if (PutArgs (txt -> lines [line] + symbs_read, &ip, line)) return COMP_ERROR; \
    }                                                                                          \
    else         
        
        #include "cmd.h"

#undef DEF_CMD

    /* else */
        {
            fprintf (ERROR_STREAM, "Compilation error:\nunknown command at line (%Iu):\n(%s)\n", line + 1, cmd);
            return COMP_ERROR;
        }
    }

    cmds[2] = ip - (cmds + CODE_SHIFT);

    return OK;
}

int PutArgs (char *args, cmd_t **ip_p, size_t line)
{
    cmd_t *ip = *ip_p;

    args = DeleteSpaces (args);

    if (*args == '\0')
    {
        fprintf (ERROR_STREAM, "Compilation error:\nmissing argument at line (%Iu)\n", line + 1);
        return COMP_ERROR;
    }

    size_t len = strlen (args);

    if (strchr (args, '['))
    {
        if (strchr (args, ']') != args + len - 1 || strchr (args + 1, '['))
        {
            fprintf (ERROR_STREAM, "Compilation error:\nincorrect argument format at line (%Iu)\n", line + 1);
            return COMP_ERROR;
        }

        args [len - 1] = '\0';
        args++;
        len -= 2;

        *(ip - 1) |= ARG_MEM;
    }
    
    char *arg1 = args;
    char *arg2 = strchr (args, '+');

    int got_im  = 0;
    int got_reg = 0;

    if (arg2)
    {
        *(arg2++) = '\0';
        arg2 = DeleteSpaces (arg2);

        if (arg2 [0] == 'r' && strlen (arg2) == 3 && arg2 [2] == 'x')
        {
            *(ip++) = arg2 [1] - 'a' + 1;
            got_reg = 1;
        }
        else if (isdigit (arg2 [0]))
        {
            sscanf (arg2, "%d", ip + 1);
            got_im = 1;
        }
        else
        {
            fprintf (ERROR_STREAM, "Compilation error:\nincorrect argument format at line (%Iu)\n", line + 1);
            return COMP_ERROR;
        }
    }

    arg1 = DeleteSpaces (arg1);

    if (!got_reg && arg1 [0] == 'r' && strlen (arg1) == 3 && arg1 [2] == 'x')
    {
        *(ip++) = arg1 [1] - 'a' + 1;
        got_reg = 1;
        if (got_im) ip++;
    }
    else if (!got_im && isdigit (arg1 [0]))
    {
        sscanf (arg1, "%d", ip++);
        got_im = 1;
    }
    else
    {
        fprintf (ERROR_STREAM, "Compilation error:\nincorrect argument format at line (%Iu)\n", line + 1);
        return COMP_ERROR;
    }

    if (arg2) *(ip - 3) |= ARG_IM | ARG_REG;
    else      *(ip - 2) |= got_im ? ARG_IM : ARG_REG;

    *ip_p = ip;

    return OK;
}

char *DeleteSpaces (char *str)
{
    if (str == nullptr) return nullptr;

    while (isspace (*str)) str++;

    size_t len = strlen (str);

    while (isspace (str [len - 1]) && len > 0) len--;
    str [len] = '\0';

    return str;
}

int WriteCmds (const char *output_file_name, cmd_t *cmds)
{
    if (output_file_name == nullptr) return NULLPTR_ARG;
    if             (cmds == nullptr) return NULLPTR_ARG;

    FILE *out_file = fopen (output_file_name, "wb");
    if (out_file == nullptr) return FOPEN_ERROR;

    if (fwrite ((void *) cmds, CMD_SIZE, cmds [2] + CODE_SHIFT, out_file) != cmds [2] + CODE_SHIFT) return FWRITE_ERROR;

    fclose (out_file);

    return OK;
}

void AsmErr (int err, FILE *stream)
{
    fprintf (stream, "ERROR: %d\n", err);
    
         if (err == OK)          fprintf (stream, "OK.\n");
    else if (err == NULLPTR_ARG) fprintf (stream, "Nullptr error");
    else if (err == FOPEN_ERROR) fprintf (stream, "Cannot open the file.\n");
    else if (err == ALLOC_ERROR) fprintf (stream, "Cannot allocate memory.\n");
    else if (err ==  COMP_ERROR) fprintf (stream, "Compilation error.\n");
    else                         fprintf (stream, "Unknown error.\n");
}