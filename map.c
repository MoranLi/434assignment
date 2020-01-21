#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXPAIR 20
#define MAXDATASIZE 40

char *keys[MAXPAIR];
char *values[MAXPAIR];
int use[MAXPAIR]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

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

char* getAll()
{
    char *allInfo = (char*)malloc(MAXDATASIZE * MAXPAIR * 2);
    char key[MAXDATASIZE];
	char value[MAXDATASIZE];
    for (int i = 0; i< MAXPAIR; i++)
    {
        if(use[i] == 1){
            bzero(key, sizeof(key)); 
		    bzero(value, sizeof(value));
            memcpy(key, keys[i], strlen(keys[i]));
            memcpy(value, values[i], strlen(keys[i]));
            strcat(allInfo, key);
            strcat(allInfo,":");
            strcat(allInfo, value);
            strcat(allInfo, "\t");
        }
    }
    return allInfo;
}

int main(void)
{
    printf("%s\n",getAll());
    add("01","EZ");
    printf("%s\n",getAll());
    printf("Get %s : %s\n", "01", get("01"));
    add("02","08");
    printf("%s\n",getAll());
    add("03","06");
    printf("%s\n",getAll());
    removeKey("01");
    printf("%s\n",getAll());
    add("04","04");
    printf("%s\n",getAll());
    char* message = getAll();
			if (message == NULL){
				strcpy(send_message,"get fail");
			}
			else{
				strcpy(send_message,message);
			}
}