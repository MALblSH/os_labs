#include "header.h"


void reverse_buffer(char* buffer, int buffer_size) {
    for (int i = 0; i < buffer_size / 2; i++) {
        char temp = buffer[i];
        buffer[i] = buffer[buffer_size - i - 1];
        buffer[buffer_size - i - 1] = temp;
    }
}

void make_flip_dir_path(char* flip_dir_path, char* dest_dir_path, char* reversed_name) {   
    char* helper_ptr = strrchr(dest_dir_path, '/');
    char* base_name = (helper_ptr == NULL) ? dest_dir_path : (helper_ptr + 1);
    strcpy(reversed_name, base_name);
    int len = strlen(flip_dir_path);
    if (len > 0 && flip_dir_path[len - 1] != '/') {
        snprintf(flip_dir_path + strlen(flip_dir_path), sizeof(flip_dir_path), "%s", "/");
    }
    reverse_buffer(reversed_name, strlen(reversed_name));
    snprintf(flip_dir_path + strlen(flip_dir_path), sizeof(flip_dir_path), "%s", reversed_name);
    

}

int get_file_size(FILE* file){
    int check_error;
    check_error = fseek(file, 0, SEEK_END);
    if (check_error == ERROR) {
        char error_str[] = "Could't  set carette on end of file";
        perror(error_str);
        return ERROR;
    }
    long file_size = ftell(file);
    if (file_size == ERROR) {
        char error_str[] = "Could't  get file size";
        perror(error_str);
        return ERROR;
    }
    check_error = fseek(file, 0, SEEK_SET);
    if (check_error == ERROR) {
        char error_str[] = "Could't  return carette on fbegin of file";
        perror(error_str);
        return ERROR;
    }
    return file_size;

}




int process_file(const char *src_path, const char *dest_path){
    int check_error;
    FILE *src = fopen(src_path, "rb");
    if (src == NULL) {
        char error_str[] = "Opening src file: %s";
        char error_message[PATH_MAX + sizeof(error_str)];
        snprintf(error_message, sizeof(error_message), error_str, src_path);
        perror(error_message);
        return ERROR;
    }

    FILE *dest = fopen(dest_path, "wb");
    if (dest == NULL) {
        char error_str[] = "Creating dest file: %s";
        char error_message[PATH_MAX + sizeof(error_str)];
        snprintf(error_message, sizeof(error_message), error_str, dest_path);
        perror(error_message);
        fclose(src);
        return ERROR;
    }

    int file_size = get_file_size(src);
    if (file_size == ERROR){
        fclose(src);
        fclose(dest);
        return ERROR;
    }
    check_error = reverse_file_content(src, dest, file_size, src_path);
    if (check_error == ERROR){
        fclose(src);
        fclose(dest);
        return ERROR;
    }
    return SUCCESS;
}

int reverse_file_content(FILE *src, FILE *dest, int file_size, const char *src_path) {
    int check_error;
    char buffer[BUFFER_SIZE]; 
    long offset = file_size;
    while (offset > 0) {
        size_t chunk_size = (offset > BUFFER_SIZE) ? BUFFER_SIZE : offset;
        long start_pos = offset - chunk_size;
        check_error = fseek(src, start_pos, SEEK_SET);
        if (check_error == -1) {
            char error_str[] = "Error seeking to end of file: %s";
            char error_message[PATH_MAX + sizeof(error_str)];
            snprintf(error_message, sizeof(error_message), error_str, src_path);
            perror(error_message);
            fclose(src);
            fclose(dest);
            return ERROR;
        }
        size_t read_items = fread(buffer, 1, chunk_size, src);
        if (read_items != chunk_size && ferror(src)) {
            char error_str[] = "Failed to read from source file: %s";
            char error_message[PATH_MAX + sizeof(error_str)];
            snprintf(error_message, sizeof(error_message), error_str, src_path);
            perror(error_message);
            fclose(src);
            fclose(dest);
            return ERROR;
        }
        reverse_buffer(buffer, read_items);
        size_t write_items = fwrite(buffer, 1, read_items, dest);
        if (write_items != read_items) {
            char error_str[] = "Failed to write to destination file: %s";
            char error_message[PATH_MAX + sizeof(error_str)];
            snprintf(error_message, sizeof(error_message), error_str, src_path);
            perror(error_message);
            fclose(src);
            fclose(dest);
            return ERROR;
        }

        offset -= read_items;
    }

    fclose(src);
    fclose(dest);
    return SUCCESS;
}


int process_directory_entry(const char *src_dir, const char *dest_dir, struct dirent *entry, char* original_src_dir, char* original_reversed_name) {
    int check_error;
    char src_path[PATH_MAX], dest_path[PATH_MAX], reversed_name[PATH_MAX];
    snprintf(src_path, sizeof(src_path), "%s/%s", src_dir, entry->d_name);
    snprintf(reversed_name, sizeof(reversed_name), "%s", entry->d_name);
    int len_string = strlen(reversed_name);
    reverse_buffer(reversed_name, len_string);
    snprintf(dest_path, sizeof(dest_path)+ sizeof(reversed_name), "%s/%s", dest_dir, reversed_name);
    
    struct stat st;
    check_error = lstat(src_path, &st);
    if (check_error == ERROR) {
        char error_str[] = "Getting file stats: %s";
        char error_message[PATH_MAX + sizeof(error_str)];
        snprintf(error_message, sizeof(error_message), error_str, src_path);
        perror(error_message);
        return ERROR;
    }
    if (S_ISDIR(st.st_mode)) {
        check_error = reverse_copy_directory(src_path, dest_path, original_src_dir, original_reversed_name,st);
        if (check_error == ERROR){
            return ERROR;
        }
    }
    if (S_ISREG(st.st_mode)) {
        check_error = process_file(src_path, dest_path);
        if(check_error == ERROR){
            return ERROR;
        }
    }
    return SUCCESS;
}

int create_reversed_directory(const char *dest_dir, struct stat st) {
    int check_error = mkdir(dest_dir, st.st_mode & 0777);
    if (check_error == ERROR) {
        char error_str[] = "Creating directory: %s";
        char error_message[PATH_MAX+sizeof(error_str)];
        snprintf(error_message, sizeof(error_message), error_str, dest_dir);
        perror(error_message);
        return ERROR;
    }
    return SUCCESS;
}

int reverse_copy_directory(const char *src_dir,  const char *dest_dir, char* original_src_dir, char* original_reversed_name, struct stat st) {
    int check_error;
    


    DIR *dir = opendir(src_dir);
    if (dir == NULL) {
        char error_str[] = "Opening directory: %s";
        char error_message[PATH_MAX+sizeof(error_str)];
        snprintf(error_message, sizeof(error_message), error_str, src_dir);
        perror(error_message);
        return ERROR;
    }
    
    check_error = create_reversed_directory(dest_dir, st);
    if(check_error == ERROR){
        closedir(dir);
        return ERROR;
    }
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL){ 


        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || (strcmp(src_dir, original_src_dir) == 0 && strcmp(entry->d_name, original_reversed_name) == 0)) {
            continue;
        }

        check_error = process_directory_entry(src_dir, dest_dir, entry, original_src_dir, original_reversed_name);
        if(check_error == ERROR){
            closedir(dir);
            return ERROR;
        }
    }
    
    closedir(dir);
    return SUCCESS;
}
