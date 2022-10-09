#include <stdlib.h>
#include <string.h>

int main (int argc, char *argv[])
{
    for (int index = 1; index < argc; index++)
    {   
        char cmd[100] = "asm.exe ";
        strcat (cmd, argv[index]);
        strcat (cmd, " code");

        system (cmd);
        system ("proc.exe code");
    }
    return 0;
}