#include <signal.h>

void zero_Error();

int main() {
    
    signal(SIGFPE, zero_Error);
    printf("Entering infinite loop\n");
    
    int a = 4;
    a = a/0;
    
}

    void zero_Error() {
        printf("Caught a SIGFPE\n");
        exit(1);
    }