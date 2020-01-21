#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXPAIR 3
#define MAXDATASIZE 40

char *keys[MAXPAIR];
char *values[MAXPAIR];
int use[MAXPAIR]= {0,0,0};

int add(char* key, char* value)
{
    for (int i = 0; i< MAXPAIR; i++)
    {
        if (use[i] == 0)
        {
            keys[i] = (char*)malloc(strlen(key));
            values[i] = (char*)malloc(strlen(value));
            memcpy(keys[i], key, strlen(key));
            memcpy(values[i], value, strlen(value));
            use[i] = 1;
            return 0;
        }
    }
    return -1;
}

char* get(char* key)
{
    for (int i = 0; i< MAXPAIR; i++)
    {
        if (strncmp(key, keys[i], strlen(key)) == 0)
        {
            return values[i];
        }
    }
    return NULL;
}

int removeKey(char* key)
{
    for (int i = 0; i< MAXPAIR; i++)
    {
        if (strncmp(key, keys[i], strlen(key)) == 0)
        {
            use[i] = 0;
            return 0;
        }
    }
    return -1;
}

void printMap()
{
    printf("PrintMap: \n");
    for (int i = 0; i< MAXPAIR; i++)
    {
        if(use[i] == 1){
            printf("%s:%s\n", keys[i], values[i]);
        }
    }
}

int main(void)
{
    printMap();
    add("01","EZ");
    printMap();
    printf("Get %s : %s\n", "01", get("01"));
    add("02","08");
    printMap();
    add("03","06");
    printMap();
    removeKey("01");
    printMap();
    add("04","04");
    printMap();

}