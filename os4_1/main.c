
#include "header.h"

int main() {
    int check_error;
    printf("Мой PID: %d\n", getpid());
    show_variable_addresses();

    int* bad_ptr = return_local_address();
    printf("\nАдрес возвращённой локальной переменной: %p, значение: %d\n", (void*)bad_ptr, *bad_ptr);

    check_error = heap_memory_test();
    if (check_error == ERROR){
        return EXIT_FAILURE;
    }
    check_error = environment_test();;
    if (check_error == ERROR){
        return EXIT_FAILURE;
    }

    
    check_error = sleep(60);
    if (check_error != 0){
        char error_str[] = "Незаконченный сон";
        perror(error_str);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
