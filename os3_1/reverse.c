#include "header.h"


int main(int argc, char *argv[]) {
    int check_error_int;
    char* check_error_char;
    if (argc != 2) {
        char error_str[] = "incorrect number of arguments %s";
        char error_message[PATH_MAX];

        snprintf(error_message, sizeof(error_message), error_str, argv[0]);
        perror(error_message);
        return EXIT_FAILURE;
    }
    char cwd[PATH_MAX];
    char *check_errors = getcwd(cwd, sizeof(cwd));
    if (check_errors == NULL) {
        char error_str[] = "getcwd error";
        perror(error_str);
        return ERROR;
    }
    char absolute_path[PATH_MAX];
    check_error_char = realpath(argv[1], absolute_path);
    if (check_error_char == NULL) {
        char error_str[] = "Path conversion error";
        perror(error_str);
        return ERROR;
    }

    char flip_dir_path[PATH_MAX];
    strcpy(flip_dir_path, cwd);

    char reversed_name[PATH_MAX];

    char original_src_dir[PATH_MAX];
    strcpy(original_src_dir, absolute_path);

    
    make_flip_dir_path(flip_dir_path, absolute_path, reversed_name);
    struct stat st;
    check_error_int = lstat(argv[1], &st);
    if (check_error_int == ERROR) {
        char error_str[] = "Getting file stats: %s";
        char error_message[PATH_MAX + sizeof(error_str)];
        snprintf(error_message, sizeof(error_message), error_str, argv[1]);
        perror(error_message);
        return EXIT_FAILURE;
    }
    check_error_int = reverse_copy_directory(absolute_path, flip_dir_path, cwd, reversed_name, st);
    if (check_error_int == ERROR){
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
