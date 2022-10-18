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

    cmds += CODE_SHIFT;

    Label_list_t label_list = {};
    int err = LabelListCtor (&label_list);
    if (err) return err;
    
    for (int pass = 0; pass < NUM_OF_ASM; pass++)
    {
        err = Assemble (txt, cmds, &label_list, pass);
        if (err) return err;
    }

    return OK;
}

int Assemble (Text *txt, cmd_t *cmds, Label_list_t *label_list, int pass)
{
    int ip = 0;

    for (size_t line = 0; line < txt -> len; line++)
    {
        if (line == 0)
        {   
            if (SetAccuracyCoef (cmds, txt -> lines [line])) return COMP_ERROR;
            continue;
        }

        int symbs_read = 0;

        char line_cpy [BUFLEN] = "";
        char cmd      [BUFLEN] = "";

        strncpy (line_cpy, txt -> lines [line], BUFLEN);
        
        sscanf (line_cpy, "%s%n", cmd, &symbs_read);

        if (stricmp (cmd, "") == 0) continue;


#define DEF_CMD(name, num, arg, ...)                                                                         \
    if (stricmp (cmd, #name) == 0)                                                                           \
    {                                                                                                        \
        cmds [ip++] |= CMD_##name;                                                                           \
        if (arg) if (PutArgs (line_cpy + symbs_read, cmds, &ip, label_list, line, pass)) return COMP_ERROR; \
    }                                                                                                        \
    else   
        
        #include "cmd.h"

#undef DEF_CMD


        /* else */ if (strchr (cmd, ':'))
        {   
            if (pass >= 1) continue;
            if (AddLabel (cmd, label_list, ip, line)) return COMP_ERROR;
        }
        else
        {
            fprintf (ERROR_STREAM, "Compilation error:\nunknown command at line (%Iu):\n(%s)\n", line + 1, cmd);
            return COMP_ERROR;
        }
    }

    cmds [-CODE_SHIFT + 2] = ip;  // Support for EMSL opcode (Entire Memory Shift Left, see Knappy Opcodes)

    return OK;
}

int SetAccuracyCoef (cmd_t *cmds, char *line)
{
    if (cmds == nullptr) return NULLPTR_ARG;
    if (line == nullptr) return NULLPTR_ARG;

    int accuracy_coef = 0;
    int symbs_read = 0;

    char cmd [BUFLEN] = "";
    sscanf (line, "%s%n", cmd, &symbs_read);
 
    if (stricmp (cmd, ACCURACY_CMD_NAME) || sscanf (line + symbs_read, "%d", &accuracy_coef) == 0)
    {
        fprintf (ERROR_STREAM, "Compilation error:\nfirst line has to contain accuracy coefficient.\n");
        return COMP_ERROR;
    }

    cmds [-CODE_SHIFT + 3] = accuracy_coef;

    return OK;
}

//----------------------------------------------------------------------------------------------------------------------

int LabelListCtor (Label_list_t *label_list)
{
    if (label_list == nullptr) return NULLPTR_ARG;

    label_list -> max_num = 32;
    label_list ->     num =  0;

    label_list -> list = (Label_t *) calloc (label_list -> max_num, sizeof (Label_t));
    if (label_list -> list == nullptr) return ALLOC_ERROR;

    return OK;
}

int AddLabel (char *cmd, Label_list_t *label_list, int ip, size_t line)
{
    if (cmd == nullptr || label_list == nullptr) return NULLPTR_ARG;

    int err = OK;

    err = CheckLabelName (&cmd, label_list, line);
    if (err) return err;

    err = ExpandLabelList (label_list);
    if (err) return err;

    strcpy (((label_list -> list) [label_list -> num]).name, cmd);
    ((label_list -> list) [label_list -> num++]).ip = ip;

    return OK;
}

int CheckLabelName (char **name_p, Label_list_t *label_list, size_t line)
{
    if (name_p == nullptr || *name_p == nullptr || label_list == nullptr) return NULLPTR_ARG;

    char *name = *name_p;

    name = DeleteSpaces (name);

    size_t len = strlen (name);

    if (name [len - 1] != ':')
    {
        fprintf (ERROR_STREAM, "Compilation error:\nincorrect label format at line (%Iu).\n", line + 1);
        return COMP_ERROR;
    }
    name [len - 1] = '\0';

    name = DeleteSpaces (name);

    len = strlen (name);

    if (strcmp (name, "") == 0 || isdigit (name [0]) || (len == 3 && name [0] == 'r' && name [2] == 'x'))
    {
        fprintf (ERROR_STREAM, "Compilation error:\nincorrect label name at line (%Iu).\n", line + 1);
        return COMP_ERROR;
    }

    if (len >= MAX_LABEL_LEN)
    {
        fprintf (ERROR_STREAM, "Compilation error:\nlabel name at line (%Iu) is too long.\n", line + 1);
        return COMP_ERROR;
    }

    for (size_t index = 0; index < label_list -> num; index++)
    {
        if (strcmp ((label_list -> list [index]).name, name) == 0)
        {
            fprintf (ERROR_STREAM, "Compilation error at line (%Iu):\nlabel (%s) has been already created.\n", line + 1, name);
            return COMP_ERROR;
        }
    }

    *name_p = name;

    return OK;
}

int ExpandLabelList (Label_list_t *label_list)
{
    if (label_list == nullptr) return NULLPTR_ARG;

    if (label_list -> num >= label_list -> max_num)
    {
        size_t old_num = label_list -> max_num;

        label_list -> list = (Label_t *) Recalloc ((void *) label_list -> list, old_num * 2, sizeof (Label_t), old_num);
        if (label_list -> list == nullptr) return ALLOC_ERROR;
        
        label_list -> max_num *= 2;
    }

    return OK;
}

int GetLabelIp (char *name, Label_list_t *label_list)
{
    if (name == nullptr || label_list == nullptr) return -1;

    for (size_t index = 0; index < label_list -> num; index++)
        if (strcmp ((label_list -> list [index]).name, name) == 0)
            return (label_list -> list [index]).ip;
    
    return -1;
}

//----------------------------------------------------------------------------------------------------------------------

int PutArgs (char *args, cmd_t *cmds, int *ip, Label_list_t *label_list, size_t line, int pass)
{
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
            printf ("(%s)\n", args);
            fprintf (ERROR_STREAM, "Compilation error:\nincorrect argument format at line (%Iu)\n", line + 1);
            return COMP_ERROR;
        }

        args [len - 1] = '\0';
        args++;
        len -= 2;

        cmds [*ip - 1] |= ARG_MEM;
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
            cmds [(*ip)++] = arg2 [1] - 'a' + 1;
            got_reg = 1;
        }
        else if (isdigit (arg2 [0]))
        {
            sscanf (arg2, "%d", cmds + *ip + 1);
            got_im = 1;
        }
        else
        {
            cmds [*ip + 1] = GetLabelIp (arg2, label_list);

            if (pass >= 2 && cmds [*ip + 1] == -1)
            {
                fprintf (ERROR_STREAM, "Compilation error:\nincorrect argument format at line (%Iu)\n", line + 1);
                return COMP_ERROR;
            }

            got_im = 1;
        }
    }

    arg1 = DeleteSpaces (arg1);

    if (!got_reg && arg1 [0] == 'r' && strlen (arg1) == 3 && arg1 [2] == 'x')
    {
        cmds [(*ip)++] = arg1 [1] - 'a' + 1;
        got_reg = 1;
        if (got_im) (*ip)++;
    }
    else if (!got_im && isdigit (arg1 [0]))
    {
        sscanf (arg1, "%d", cmds + (*ip)++);
        got_im = 1;
    }
    else
    {
        if (!got_im) cmds [(*ip)++] = GetLabelIp (arg1, label_list);
            
        if (got_im || (pass >= 2 && cmds [*ip - 1] == -1))
        {
            fprintf (ERROR_STREAM, "Compilation error:\nincorrect argument format at line (%Iu)\n", line + 1);
            return COMP_ERROR;
        }

        got_im = 1;
    }

    if (arg2) cmds [*ip - 3] |= ARG_IM | ARG_REG;
    else      cmds [*ip - 2] |= got_im ? ARG_IM : ARG_REG;

    return OK;
}

//----------------------------------------------------------------------------------------------------------------------

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

    if (fwrite ((void *) cmds, CMD_SIZE, cmds [2] + CODE_SHIFT, out_file) != (size_t) (cmds [2] + CODE_SHIFT)) return FWRITE_ERROR;

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


void *Recalloc (void *memptr, size_t num, size_t size, size_t old_num)
{
    memptr = realloc (memptr, num * size);
    if (memptr == NULL) return NULL;

    if (num > old_num) memset ((void *) ((char *) memptr + old_num * size), 0, (num - old_num) * size);

    return memptr;
}
