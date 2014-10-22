#include <pthread.h>
#include <sys/time.h>

//Structure for the command node.
typedef struct CommandNode {
    int id;
    char cmd[355];
    struct timeval time;
    struct CommandNode * next;
    struct CommandNode * prev;
} CommandNode;

//Structure for the command list.
typedef struct CommandsList {
    pthread_mutex_t lock;
    int size;
    CommandNode * head;
    CommandNode * tail;
} CommandsList;

//Structure for the argument list for the worker thread.
typedef struct argList {
    CommandsList * cmdsArg;
    pthread_mutex_t * mutArg;
    char * outputArg;
    int accountsArg;
}argList;