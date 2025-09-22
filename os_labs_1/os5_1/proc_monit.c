#include "header.h"

int global_var = 10;

void child_print(int* local_var) {
    printf("\nChild process:\n");
    printf("pid: %d\n", getpid());
    printf("parent pid: %d\n", getppid());
        
    printf("global var: address=%p, value=%d\n", (void*)&global_var, global_var);
    printf("local var: address=%p, value=%d\n", (void*)local_var, *local_var);
        
    global_var = 100;
    *local_var = 200;
    printf("changed global var to %d\n", global_var);
    printf("changed local var to %d\n", *local_var);
        
    int exitStatus = 5;
    sleep(30);
    _exit(exitStatus);
}

int parent_print(int* local_var) {
    
    sleep(45);
    printf("\nParent process after fork:\n");
    printf("global var: address=%p, value=%d\n", (void*)&global_var, global_var);
    printf("local var: address=%p, value=%d\n", (void*)local_var, *local_var);
        
    int status;
    pid_t check_error = wait(&status);
    if (check_error == ERROR) {
        perror("Wait in parentProcessTask");
        return ERROR;
    }
    int destroyed_signal = WIFSIGNALED(status);
    int destroyed_exit = WIFEXITED(status);
    if (destroyed_exit) {
        printf("child exited with status: %d\n", WEXITSTATUS(status));
    }
    if (destroyed_signal) {
        printf("child killed by signal: %d\n", WTERMSIG(status));
    }
    return SUCCESS;    
    
}

int proc_monitoring() {
    int check_error;
    int local_var = 20;
    
    printf("global var: address=%p, value=%d\n", (void*)&global_var, global_var);
    printf("local var: address=%p, value=%d\n", (void*)&local_var, local_var);
    
    pid_t pid = fork();
    if (pid == ERROR){
        perror("fork failed");
        return ERROR;
    }
    if (pid == FORK_VAL_FOR_CHILD) {
        child_print(&local_var);
    }
    check_error = parent_print(&local_var);
    if (check_error == ERROR){
        return ERROR;
    }
    
    return SUCCESS;
}