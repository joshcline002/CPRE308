#include <pthread.h>
void* thread1();
void* thread2();

int main(){
    //This is the pthread_t variables i1 and i2.
    pthread_t i1;
    pthread_t i2;
    
    //Here it creates the two new threads that run the functions thread1 and thread2 respectivly.
    pthread_create(&i1, NULL, thread1, NULL);
    pthread_create(&i2, NULL, thread2, NULL);
    
    //Here is pthread_join which is where the program waits for the threads to finish and return before continuing.
    pthread_join(i1, NULL);
    pthread_join(i2, NULL);
    
    //Print statement for main.
    printf("Hello, I am main process\n");
}

//Thread 1 function.
void *thread1(){
    sleep(5);
    printf("Hello, I am thread 1\n");
}

//Thread 2 function.
void *thread2(){
    sleep(5);
    printf("Hello, I am thread 2\n");
}