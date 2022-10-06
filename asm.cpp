#include "proc.h"


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

    err = ReadText (input_file_name, &txt);
    if (err != OK) return err;

    err = SetCmds (&txt, &commands);
    if (err != OK) return err;
printf("ok");
    FreeText (&txt);

    err = WriteCmds (output_file_name, commands);
    if (err != OK) return err;
printf("ok");
    free (commands);

    return 0;
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

int SetCmds (struct Text *txt, cmd_t **cmds_p)
{
    if (txt           == nullptr) return NULLPTR_ARG;
    if (txt -> buffer == nullptr) return NULLPTR_ARG;
    if (txt ->  lines == nullptr) return NULLPTR_ARG;
    if (cmds_p        == nullptr) return NULLPTR_ARG;

    int cmds_in_arg = ARG_SIZE / CMD_SIZE;         //???
    if (cmds_in_arg * CMD_SIZE < ARG_SIZE) cmds_in_arg++; //???

    size_t size = (cmds_in_arg + 1) * CMD_SIZE * txt -> len + INFO_SIZE; 

    *cmds_p = (cmd_t *) (calloc (1, size));

    cmd_t *cmds = *cmds_p;
    
    if (cmds == nullptr) return ALLOC_ERROR;

    cmds [0] = SIGNATURE;
    cmds [1] = VERSION;

    int *cmd_ptr = cmds + 3;

    char cmd [CMD_LEN] = "";

    for (size_t line = 0; line < txt -> len; line++)
    {
        int symbs_read = 0;

        sscanf (txt -> lines [line], "%s%n", cmd, &symbs_read);

        if (stricmp (cmd, "") == 0) continue;

        int need_arg = 0;


        if      (stricmp (cmd, "PUSH") == 0)
        {
            need_arg = 1;
            *(cmd_ptr++) = PUSH; 
        }
        else if (stricmp (cmd,  "POP") == 0) *(cmd_ptr++) = POP;
        else if (stricmp (cmd,   "IN") == 0) *(cmd_ptr++) = IN;
        else if (stricmp (cmd,  "OUT") == 0) *(cmd_ptr++) = OUT;
        else if (stricmp (cmd,  "ADD") == 0) *(cmd_ptr++) = ADD;
        else if (stricmp (cmd,  "SUB") == 0) *(cmd_ptr++) = SUB;
        else if (stricmp (cmd,  "MUL") == 0) *(cmd_ptr++) = MUL;
        else if (stricmp (cmd,  "DIV") == 0) *(cmd_ptr++) = DIV;
        else if (stricmp (cmd,  "POW") == 0) *(cmd_ptr++) = POW;
        else if (stricmp (cmd,  "ABS") == 0) *(cmd_ptr++) = ABS;
        else if (stricmp (cmd,  "HLT") == 0) *(cmd_ptr++) = HLT;
        else
        {
            //dump
            printf ("comp error\n");
            return COMP_ERROR;
        }

        if (need_arg == 0) continue;

        arg_t arg = 0;

        if (sscanf (txt -> lines[line] + symbs_read, "%lf", &arg) != 1)
        {
            //dump
            printf ("comp error\n");
            return COMP_ERROR;
        }

        * ((arg_t *) cmd_ptr) = arg;

        //cmd_ptr = (cmd_t *) (((arg_t *) cmd_ptr) + 1);
        cmd_ptr += cmds_in_arg;
    }

    cmds[2] = cmd_ptr - cmds;

    return OK;
}

int WriteCmds (const char *output_file_name, cmd_t *cmds)
{
    if (output_file_name == nullptr) return NULLPTR_ARG;
    if             (cmds == nullptr) return NULLPTR_ARG;

    FILE *out_file = fopen (output_file_name, "wb");
    if (out_file == nullptr) return FOPEN_ERROR;

    if (fwrite ((void *) cmds, CMD_SIZE, cmds [2], out_file) != cmds [2]) return FWRITE_ERROR;

    fclose (out_file);

    return OK;
}