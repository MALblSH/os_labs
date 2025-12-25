#ifndef CP_R_H
#define CP_R_H

#include <sys/stat.h>
#include <pthread.h>
#include <limits.h>

#define SUCCESS 0
#define ERROR -1
#define BUFFER_SIZE 8192
#define MAX_RETRIES 100
#define CURRENT_DIR "."
#define PARENT_DIR ".."


typedef struct {
    char src_path[PATH_MAX];
    char dst_path[PATH_MAX];
} task_t;

pthread_mutex_t dir_mutex;

int build_path(char* path, size_t size, const char* dir, const char* name);
int open_with_retry(const char* path, int flags, mode_t mode);
DIR* opendir_with_retry(const char* path);
int create_directory_safe(const char* src_path, const char* dst_path);
void *copy_file_thread(void* arg);
int create_file_task(const char* src_path, const char* dst_path);
int create_directory_task(const char* src_path, const char* dst_path);
int process_single_entry(const char* src_dir, const char* dst_dir, const char* entry_name);
void *work_directory_thread(void* arg);

#endif