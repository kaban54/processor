typedef int Elem_t;
#include "stack.h"



int StackConstructor (struct Stack_t *stk, size_t capacity, const char *name, const char *func_name, const char *file_name, int line)
{
    if (stk == nullptr) return ACCESS_ERROR;

    if (stk -> status != NEW)
    {
        stk -> error = STATUS_ERROR;
        Dump (stk, LOG_FILE_NAME);
        return stk -> error;
    }

    stk -> info.     name =      name;
    stk -> info.func_name = func_name;
    stk -> info.file_name = file_name;
    stk -> info.     line =      line;

    stk -> capacity = capacity;
    stk -> size = 0;

    #ifdef CANARY_PROT
    
    stk -> data = (Elem_t*) ((char *) calloc (1, capacity * sizeof (Elem_t) + 2 * sizeof (Canary_t)) + sizeof (Canary_t));
    if (stk -> data - sizeof (Canary_t) == NULL) return DATA_ERROR;

    *((Canary_t *) (stk -> data) - 1       ) = CANARY;
    *((Canary_t *) (stk -> data + capacity)) = CANARY;

    stk ->  leftcan = CANARY;
    stk -> rightcan = CANARY;

    #else

    stk -> data = (Elem_t*) (calloc (capacity, sizeof (Elem_t)));
    if (stk -> data == NULL) return DATA_ERROR;
    
    #endif

    for (size_t index = 0; index < capacity; index++)
        stk -> data[index] = POISON_ELEM;

    stk -> status = CONSTRUCTED;

    SetHash (stk);

    AssertOK(stk);

    return 0;
}


int StackDtor (struct Stack_t *stk)
{
    AssertOK(stk);

    #ifdef CANARY_PROT
    free ((void *) ((Canary_t *) (stk -> data) - 1));
    #else
    free ((void*) (stk -> data));
    #endif

    stk -> capacity = POISON_SIZE;
    stk -> size     = POISON_SIZE;

    stk -> data = (Elem_t*) POISON_PTR;

    #ifdef HAHS_PROT
    stk -> struct_hash = 0;
    stk ->   data_hash = 0;
    #endif

    stk -> status = DECONSTRUCTED;

    return 0;
}

int StackPush (struct Stack_t *stk, Elem_t value)
{
    AssertOK(stk);

    int err = 0;

    if (stk -> size >= stk -> capacity) 
    {
        if (stk -> capacity == 0) err = StackResize (stk, BASE_CAPACITY      , NOT_SET_HASH);
        else                      err = StackResize (stk, stk -> capacity * 2, NOT_SET_HASH);
    }

    if (!err) (stk -> data)[(stk -> size)++] = value;

    SetHash (stk);

    AssertOK(stk);

    return 0;
}


int StackPop (struct Stack_t *stk, Elem_t *value)
{
    AssertOK(stk);

    if (stk -> size <= 0) return SIZE_ERROR;

    stk -> size--;
    *value = (stk -> data) [stk -> size];
    stk -> data [stk -> size] = POISON_ELEM;

    if (stk -> size * 4 <= (stk -> capacity) && stk -> size > 0) StackResize (stk, stk -> size * 2, NOT_SET_HASH);

    SetHash (stk);

    AssertOK(stk);

    return 0;
}

int StackResize (struct Stack_t *stk, size_t capacity, int param = SET_HASH)
{
    if (param == SET_HASH) AssertOK(stk);

    #ifdef CANARY_PROT

    stk -> data = (Elem_t *) ((char *) Recalloc (
                                                 (void *) ((Canary_t *) (stk -> data) - 1),
                                                        capacity * sizeof (stk -> data [0]) + 2 * sizeof (CANARY), 1,
                                                 stk -> capacity * sizeof (stk -> data [0]) + 2 * sizeof (CANARY)
                                                )
                              + sizeof (CANARY));

    *((Canary_t *) (stk -> data + capacity)) = CANARY;

    #else

    stk -> data = (Elem_t *) (Recalloc ((void *) (stk -> data), capacity, sizeof (stk -> data [0]), stk -> capacity);

    #endif

    if (capacity > stk -> capacity)
    {
        for (size_t index = stk -> capacity; index < capacity; index++)
            stk -> data[index] = POISON_ELEM;
    }

    stk -> capacity = capacity; 

    if (param == SET_HASH)
    {
        SetHash (stk);
        AssertOK(stk);
    }

    return 0;
}

int StackError (struct Stack_t *stk)
{
    if (stk == NULL) return ACCESS_ERROR;

    if (stk -> status != CONSTRUCTED && stk -> status != NEW)
    {
        stk -> error |=     STATUS_ERROR;
        return stk -> error;
    }
    
    #ifdef CAN
    if (stk -> leftcan != CANARY || stk -> rightcan != CANARY)
        stk -> error |=     STRUCT_ERROR;
    #endif

    #ifdef HASH // <------
    if (!CheckStructHash (stk))
        stk -> error |=     STRUCT_ERROR;
    #endif

    if (stk -> data == NULL || stk -> data == POISON_PTR)
        stk -> error |=     ACCESS_ERROR;

    if (stk -> size > stk -> capacity || stk -> size == POISON_SIZE || stk -> capacity == POISON_SIZE)
        stk -> error |=       SIZE_ERROR;

    #ifdef CAN
    if (!(stk -> error & (ACCESS_ERROR | SIZE_ERROR)))
        if (*((Canary_t *) (stk -> data) - 1) != CANARY || *((Canary_t *) (stk -> data + stk -> capacity)) != CANARY)
            stk -> error |=   DATA_ERROR;
    #endif

    #ifdef HASH
    if (!CheckDataHash (stk))
        stk -> error |=       DATA_ERROR;
    #endif

    if (stk -> info.     name == NULL || stk -> info.     name == POISON_PTR ||
        stk -> info.func_name == NULL || stk -> info.func_name == POISON_PTR ||
        stk -> info.file_name == NULL || stk -> info.file_name == POISON_PTR ||
        stk -> info.     line <= 0)
        stk -> error |=       INFO_ERROR;

    return stk -> error;
}

void StackDump (struct Stack_t *stk, const char *filename, const char *func_name, const char *file_name, int line)
{
    #ifndef NDEBUG

    FILE *log = fopen (filename, "a");
    if (log == NULL) return;

    if (func_name == NULL) func_name = "NULL";
    if (file_name == NULL) file_name = "NULL";

    fprintf (log, "%s at %s(%d):\n", func_name, file_name, line);

    if (stk == NULL)
    {
        fprintf (log, "stack [NULL] <-- ACCESS ERROR\n");
        return;
    }

    fprintf (log, "ERROR: %d\n", stk -> error);

    if (stk -> error & INFO_ERROR)
    {
        fprintf (log, "unknown stack [%p] <-- INFO ERROR:", stk);
        return;
    }

    if (stk -> error & STATUS_ERROR)
    {
        if      (stk -> status == NEW)
            fprintf (log, "\tSTATUS ERROR (STACK HAS NOT BEEN CONSTRUCTED)\n\n");
        else if (stk -> status == DECONSTRUCTED)
            fprintf (log, "\tSTATUS ERROR (STACK HAS BEEN DECONSTRUCTED)\n\n");
        else 
            fprintf (log, "\tSTATUS ERROR (UNDEFINED STATUS)\n\n");
        return;
    }

    fprintf (log, "stack [%p] \"%s\" at %s at %s(%d):\n{\n", stk,
                                                            stk -> info.     name,
                                                            stk -> info.func_name,
                                                            stk -> info.file_name,
                                                            stk -> info.     line);

    if (stk -> error & STRUCT_ERROR)
    {
        fprintf (log, "\tSTRUCT ERROR\n}\n\n");
        return;
    }

    if (stk -> size == POISON_SIZE)             fprintf (log, "\tsize     = POISON"               );
    else                                        fprintf (log, "\tsize     = %Iu", stk -> size      );

    if (stk -> error & SIZE_ERROR)              fprintf (log, " <-- SIZE ERROR");
    fprintf (log, "\n");

    if (stk -> size == POISON_SIZE)             fprintf (log, "\tcapacity = POISON\n"             );
    else                                        fprintf (log, "\tcapacity = %Iu\n", stk -> capacity);

    if (stk -> error & ACCESS_ERROR)
    {
        if (stk -> data == POISON_PTR)          fprintf (log, "\tdata [POISON] <-- ACCESS ERROR\n");
        else                                    fprintf (log, "\tdata [%p] <-- ACCESS ERROR\n", stk -> data);
        fprintf (log, "}\n\n");
        return;
    }
    else                                        fprintf (log, "\tdata [%p]\n\t{\n", stk -> data);

    if (stk -> error & DATA_ERROR)
    {
        fprintf (log, "\t\tDATA ERROR\n\t}\n}\n\n");
        return;
    }

    for (size_t index = 0; index < stk -> capacity; index++)
    {
                                                fprintf (log, index < stk -> size ? "\t\t*" : "\t\t ");
        if (stk -> data [index] == POISON_ELEM) fprintf (log, "[%Iu] = POISON\n", index);
        else                                    fprintf (log, "[%Iu] = %d\n", index, stk -> data [index]); // ??        
    }

                                                fprintf (log, "\t}\n}\n\n");
    fclose (log);
    #endif
}

void *Recalloc (void *memptr, size_t num, size_t size, size_t old_num)
{
    memptr = realloc (memptr, num * size);
    if (memptr == NULL) return NULL;

    if (num > old_num) memset ((void *) ((char *) memptr + old_num * size), 0, (num - old_num) * size);

    return memptr;
}

int GetHash (char *ptr, size_t len)
{
    int hash = 5381;
    
    for (size_t index = 0; index < len; index++)
        hash = (hash << 5) + hash + *(ptr + index);
    
    return hash;
}

void Sethash (struct Stack_t *stk)
{
    #ifdef HASH_PROT
    stk -> struct_hash = 0;
    
    stk ->   data_hash = GetHash ((char *) stk -> data - sizeof (Canary_t), (stk -> capacity) * sizeof (stk -> data [0]) + 2 * sizeof (Canary_t));
    stk -> struct_hash = GetHash ((char *) stk                            , sizeof (*stk));
    #endif
}

int CheckStructHash (struct Stack_t *stk)
{
    #ifndef HASH_PROT 
    return 1;
    #endif

    int hashbuf = stk -> struct_hash;
    
    stk -> struct_hash = 0;

    if (hashbuf != GetHash ((char *) stk, sizeof (*stk))) return 0;

    stk -> struct_hash = hashbuf;

    return 1;
}

int CheckDataHash (struct Stack_t *stk)
{
    #ifndef HASH_PROT
    return 1;
    #endif

    #ifdef CANARY_PROT
    if (stk -> data_hash != GetHash ((char *) (stk -> data) - sizeof (Canary_t), (stk -> capacity) * sizeof (stk -> data [0]) + 2 * sizeof (Canary_t))) return 0;
    #else
    if (stk -> data_hash != GetHash ((char *) (stk -> data), (stk -> capacity) * sizeof (stk -> data [0]))) return 0;
    #endif
    
    return 1;
}
