#include "header.h"


int global_init = 5;
int global_uninit;
const int global_const = 31;

void show_variable_addresses() {
    int local = 3;
    static int static_var = 6;
    const int const_local = 8;

    printf("Локальная переменная: %p\n", (void*)&local);
    printf("Статическая переменная: %p\n", (void*)&static_var);
    printf("Константа в функции: %p\n", (void*)&const_local);
    printf("Глобальная инициализированная: %p\n", (void*)&global_init);
    printf("Глобальная неинициализированная: %p\n", (void*)&global_uninit);
    printf("Глобальная константа: %p\n", (void*)&global_const);
}

int* return_local_address() {
    int local_var = 52;
    void* p = &local_var;
    return p;
}

int heap_memory_test() {
    char* buffer = (char*)malloc(100);
    if (buffer == NULL) {
        perror("Ошибка malloc");
        return ERROR;
    }
    strcpy(buffer, "hello world");
    printf("\nБуфер после malloc: %s\n", buffer);
    free(buffer);
    printf("Буфер после free: %s\n", buffer);

    char* new_buffer = (char*)malloc(100);
    if (new_buffer == NULL) {
        perror("Ошибка malloc");
        return ERROR;
    }
    strcpy(new_buffer, "hello world");
    
    printf("Новый буфер: %s\n", new_buffer);
    
    
    //char* mid_ptr = new_buffer + 50;
    //free(mid_ptr);
    //printf("Буфер после неправильного free: %s\n", new_buffer);
    return SUCCESS;
}

int environment_test() {

    char* var = getenv("VAR");
    if (var == NULL){
        printf("Переменная VAR не найдена\n");
    }
    printf("Исходное значение: %s\n", var ? var : "не найдена");

    int check_error = setenv("VAR", "new_value", 1);
    if (check_error == ERROR){
        perror("Ошибка sentev");
        return ERROR;
    }
    var = getenv("VAR");
    if (var == NULL){
        perror("Переменная VAR не найдена\n");
    }
    printf("Новое значение VAR: %s\n", var);
    return SUCCESS;
}
