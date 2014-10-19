#include <pthread.h>
#include <sys/time.h>

typedef struct CommandNode {
    int id;
    char **cmds;
    //struct timeval time;
    struct CommandNode * next;
    struct CommandNode * prev;
} CommandNode;

typedef struct CommandsList {
    pthread_mutex_t lock;
    int size;
    CommandNode * head;
    CommandNode * foot;
} CommandsList;

typedef struct argList {
    CommandsList * cmdsArg;
    pthread_mutex_t * mutArg;
    char * outputArg;
    int * accountsArg;
}argList;