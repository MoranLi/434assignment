#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXDATASIZE 256

int id = 0;
int size;
int current_size;

int add(int keys[], char *values[] , int use[], char* value)
{
  if(current_size == size){
    printf("Queue full\n");
    return -1;
  }
    for (int i = 0; i< size; i++)
    {
        if (use[i] == 0)
        {
            keys[i] = id;
            values[i] = (char*)malloc(strlen(value));
            memcpy(values[i], value, strlen(value));
            use[i] = 1;
            current_size += 1;
            id += 1;
            return 0;
        }
    }
    return -1;
}

int removeKey(int keys[], int use[], int key)
{
    for (int i = 0; i< size; i++)
    {
        if (keys[i] == key)
        {
            use[i] = 0;
            current_size -= 1;
            return 0;
        }
    }
    return -1;
}

char* getall(int keys[], char *values[], int use[])
{
    char *allInfo = (char*)malloc(MAXDATASIZE * size * 2);
	  char key[10];
    char value[MAXDATASIZE];
    for (int i = 0; i< size; i++)
    {
        if(use[i] == 1){
          bzero(key, sizeof(key));
          bzero(value, sizeof(value));
          memcpy(value, values[i], strlen(values[i]));
          //itoa(keys[i], key, 2);
          snprintf(key, sizeof(key), "%d", keys[i]);
          strcat(allInfo, key);
          strcat(allInfo,":");
          strcat(allInfo, value);
          strcat(allInfo, "; ");
        }
    }
    return allInfo;
}

int main(void)
{
  char inputSize[256];
  int n = 0;

  printf("Please enter queue size");
  while ((inputSize[n++] = getchar()) != '\n')
		; 

  size = atoi(inputSize);
  
  int keys[size];
  char *values[size];
  int use[size];

  for (int i=0;i<size;i++){
    use[i] = 0;
  }

  char* message1 = "hello world";
  add(keys, values, use, message1);

  char* message2 = "hi world";
  add(keys, values, use, message2);

  char message3[256];
  n = 0;
  printf("enter message3:");
  while ((message3[n++] = getchar()) != '\n')
		;

  add(keys, values, use, message3);

  printf(getall(keys, values, use));

  removeKey(keys, use, 1);

  printf(getall(keys, values, use));

  removeKey(keys, use, 0);

  printf(getall(keys, values, use));

}