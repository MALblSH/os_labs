#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <fcntl.h>
#include <linux/binfmts.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#define ERROR -1
#define SUCCESS 1
#define BUFFER_SIZE 2048

void reverse_buffer(char* buffer, int buffer_size);
int open_file(const char *path, int flags, mode_t mode, const char *error_message);
int reverse_copy_directory(const char *src_dir, const char *dest_dir, char* original_src_dir, char* original_reversed_name, struct stat st);
void make_flip_dir_path(char* flip_dir_path, char* dest_dir_path, char* reversed_name);
int process_file(const char *src_path, const char *dest_path);
int reverse_file_content(FILE *src, FILE *dest, int file_size, const char *src_path);

int process_directory_entry(const char *src_dir, const char *dest_dir, struct dirent *entry, char* original_src_dir, char* original_reversed_name);
int create_reversed_directory(const char *dest_dir, struct stat st);

#endif // HEADER_H
