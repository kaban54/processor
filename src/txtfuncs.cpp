#include "txtfuncs.h"


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

size_t GetSize (FILE *inp_file)
{
    if (inp_file == nullptr) return 0;
    struct stat stat_buf = {};

    fstat (fileno (inp_file), &stat_buf);
    return stat_buf.st_size;
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
