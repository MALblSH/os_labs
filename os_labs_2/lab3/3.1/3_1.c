#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/stat.h>
#include <limits.h>
#include "3_1.h"

pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  count_cond  = PTHREAD_COND_INITIALIZER;
int active_threads = 0;
int fatal_error = 0;

int thread_inc(void) {
    int err;
    err = pthread_mutex_lock(&count_mutex);
    if (err != SUCCESS) {
        printf("thread_inc: pthread_mutex_lock() failed: %s\n", strerror(err));
        return ERROR;
    }
    active_threads++;
    err = pthread_mutex_unlock(&count_mutex);
    if (err != SUCCESS) {
        printf("thread_inc: pthread_mutex_unlock() failed: %s\n", strerror(err));
        return ERROR;
    }
    return SUCCESS;
}

int thread_dec(void) {
    int err;
    err = pthread_mutex_lock(&count_mutex);
    if (err != SUCCESS) {
        printf("thread_dec: pthread_mutex_lock() failed: %s\n", strerror(err));
        return ERROR;
    }
    active_threads--;
    if (active_threads == 0) {
        pthread_cond_signal(&count_cond);
    }
    err = pthread_mutex_unlock(&count_mutex);
    if (err != SUCCESS) {
        printf("thread_dec: pthread_mutex_unlock() failed: %s\n", strerror(err));
        return ERROR;
    }  
    return SUCCESS; 
}

int report_fatal_error(const char *msg) {
    int err;
    err = pthread_mutex_lock(&count_mutex);
    if (err != SUCCESS) {
        printf("report_fatal_error: pthread_mutex_lock() failed: %s\n", strerror(err));
        return ERROR;
    }

    fatal_error = 1;

    fprintf(stderr, "FATAL ERROR: %s: %s\n", msg, strerror(errno));

    pthread_cond_signal(&count_cond);

    err = pthread_mutex_unlock(&count_mutex);
    if (err != SUCCESS) {
        printf("report_fatal_error: pthread_mutex_unlock() failed: %s\n", strerror(err));
        return ERROR;
    } 
    return SUCCESS;
}

int build_path(char* path, size_t size, const char* dir, const char* name) {
    int len_path = snprintf(path, size, "%s/%s", dir, name);
    if (len_path < 0 || (size_t)len_path >= size) {
        return ERROR;
    }
    return SUCCESS;
}

int check_src_dst(const char* src, const char* dst) {
    char src_real[PATH_MAX];
    char *check_error_char = realpath(src, src_real);
    if (check_error_char == NULL) {
        perror("realpath(src)");
        return ERROR;
    }
    char dst_copy[PATH_MAX];
    strncpy(dst_copy, dst, PATH_MAX - 1);
    dst_copy[PATH_MAX - 1] = '\0';

    char* last_slash = strrchr(dst_copy, '/');
    if (last_slash != NULL) {
        *last_slash = '\0';
    } else {
        strcpy(dst_copy, ".");
    }

    char dst_parent_real[PATH_MAX];
    check_error_char = realpath(dst_copy, dst_parent_real);
    if (check_error_char == NULL) {
        perror("realpath(dst parent)");
        return ERROR;
    }

    size_t src_len = strlen(src_real);
    if (strncmp(dst_parent_real, src_real, src_len) == 0 &&
        (dst_parent_real[src_len] == '/' || dst_parent_real[src_len] == '\0')) {
        perror("Error: destination is inside source");
        return ERROR;
    }

    return SUCCESS;
}

int open_with_retry(const char* path, int flags, mode_t mode) {
    int fd;
    int retries = 0;
    while (retries < MAX_RETRIES) {
        fd = open(path, flags, mode);
        if (fd != ERROR) {
            return fd;
        }
        if (errno != EMFILE) {
            printf("open_with_retry: open() failed for %s: %s\n", path, strerror(errno));
            return ERROR;
        }
        retries++;
        sleep(1);
    }
    return ERROR;
}

DIR* opendir_with_retry(const char* path) {
    DIR* dir;
    int retries = 0;
    while (retries < MAX_RETRIES) {
        dir = opendir(path);
        if (dir != NULL) {
            return dir;
        }
        if (errno != EMFILE) {
            printf("opendir_with_retry: opendir() failed for %s: %s\n", path, strerror(errno));
            return NULL;
        }
        retries++;
        sleep(1);
    }
    return NULL;
}

void *copy_file_thread(void* arg) {
    task_t* task = (task_t*)arg;
    char buffer[BUFFER_SIZE];
    int err;
    ssize_t bytes_read, bytes_written;
    struct stat src_stat;
    int copy_error = 0;

    err = thread_inc();
    if (err != SUCCESS){
        free(task);
        err = report_fatal_error("copy_file_thread: thread_inc failed");
        if (err != SUCCESS){
            fprintf(stderr, "FATAL ERROR: report_fatal_error failed\n");
        }
        return NULL;
    }

    err = lstat(task->src_path, &src_stat);
    if (err != SUCCESS) {
        free(task);
        err = report_fatal_error("copy_file_thread: lstat failed");
        if (err != SUCCESS){
            fprintf(stderr, "FATAL ERROR: report_fatal_error failed\n");
        }
        return NULL;
    }

    int src_fd = open_with_retry(task->src_path, O_RDONLY, 0);
    if (src_fd == ERROR) {
        free(task);
        err = report_fatal_error("copy_file_thread: failed to open source file");
        if (err != SUCCESS){
            fprintf(stderr, "FATAL ERROR: report_fatal_error failed\n");
        }
        return NULL;
    }

    int dst_fd = open_with_retry(task->dst_path, O_WRONLY | O_CREAT | O_TRUNC, src_stat.st_mode);
    if (dst_fd == ERROR) {
        close(src_fd);
        free(task);
        err = report_fatal_error("copy_file_thread: failed to open destination file");
        if (err != SUCCESS){
            fprintf(stderr, "FATAL ERROR: report_fatal_error failed\n");
        }
        return NULL;
    }

    while (1) {
        bytes_read = read(src_fd, buffer, BUFFER_SIZE);
        if (bytes_read == ERROR) {
            copy_error = 1;
            break;
        }
        if (bytes_read == 0) {
            break;
        }

        ssize_t total_written = 0;
        char* ptr = buffer;
        while (total_written < bytes_read) {
            bytes_written = write(dst_fd, ptr + total_written, bytes_read - total_written);
            if (bytes_written == ERROR) {
                copy_error = ERROR;
                break;
            }
            total_written += bytes_written;
        }
        if (copy_error) {
            break;  
        }   
    }

    err = close(src_fd);
    if (err != SUCCESS) {
        err = report_fatal_error("copy_file_thread: close() failed for source fd:"); 
        if (err != SUCCESS){
            fprintf(stderr, "FATAL ERROR: report_fatal_error failed\n");
        }
    }    
    err = close(dst_fd);
    if (err != SUCCESS) {
        err = report_fatal_error("copy_file_thread: close() failed for source fd:");
        if (err != SUCCESS){
            fprintf(stderr, "FATAL ERROR: report_fatal_error failed\n");
        }
    }
    free(task);

    if (copy_error == ERROR){
        err = report_fatal_error("thread_dec failed");
        if (err != SUCCESS){
            fprintf(stderr, "FATAL ERROR: report_fatal_error failed\n");
        }
        return NULL;
    }
    err = thread_dec();
    if (err != SUCCESS){
        err = report_fatal_error("thread_dec failed");
        if (err != SUCCESS){
            fprintf(stderr, "FATAL ERROR: report_fatal_error failed\n");
        }
        return NULL;
    }
    return NULL;
}

int create_file_task(const char* src_path, const char* dst_path) {
    int err;
    pthread_t thread;
    task_t* task = malloc(sizeof(task_t));
    if (task == NULL) {
        printf("create_file_task: memory allocation failed\n");
        return ERROR;
    }
    strcpy(task->src_path, src_path);
    strcpy(task->dst_path, dst_path);    
    
    err = pthread_create(&thread, NULL, copy_file_thread, task);
	if (err != SUCCESS) {
		printf("create_file_task: pthread_create() failed: %s\n", strerror(err));
        free(task);
		return ERROR;
	}
    err = pthread_detach(thread);
    if (err != SUCCESS) {
        printf("create_file_task: pthread_detach() failed: %s\n", strerror(err));
        free(task);
		return ERROR;
    }    
    return SUCCESS;
}

int create_directory_task(const char* src_path, const char* dst_path) {
    int err;
    pthread_t thread;
    task_t* task = malloc(sizeof(task_t));
    if (task == NULL) {
        printf("create_directory_task: memory allocation failed\n");
        return ERROR;
    }  
    strcpy(task->src_path, src_path);
    strcpy(task->dst_path, dst_path);        
    
    err = pthread_create(&thread, NULL, work_directory_thread, task);
	if (err != SUCCESS) {
		printf("create_directory_task: pthread_create() failed: %s\n", strerror(err));
        free(task);
		return ERROR;
	}
    err = pthread_detach(thread);
    if (err != SUCCESS) {
        printf("create_directory_task: pthread_detach() failed: %s\n", strerror(err));
        free(task);
		return ERROR;
    }    
    return SUCCESS;
}

int process_single_entry(const char* src_dir, const char* dst_dir, const char* entry_name) {
    int err;
    char src_path[PATH_MAX];
    char dst_path[PATH_MAX];
    struct stat stat_buf;
    err = build_path(src_path, sizeof(src_path), src_dir, entry_name);
    if (err != SUCCESS) {
        printf("process_single_entry: source path too long: %s/%s\n", src_dir, entry_name);
        return ERROR;
    }    
    err = build_path(dst_path, sizeof(dst_path), dst_dir, entry_name);
    if (err != SUCCESS) {
        printf("process_single_entry: destination path too long: %s/%s\n", dst_dir, entry_name);
        return ERROR;
    }

    err = lstat(src_path, &stat_buf);
    if (err != SUCCESS) {
        printf("process_single_entry: lstat() failed for %s: %s\n", src_path, strerror(errno));
        return ERROR;
    }
    if (S_ISDIR(stat_buf.st_mode)) {
        return create_directory_task(src_path, dst_path);
    }
    if (S_ISREG(stat_buf.st_mode)) {
        return create_file_task(src_path, dst_path);
    }    
    return SUCCESS;
}

void *work_directory_thread(void* arg) {
    task_t* task = (task_t*)arg;
    struct stat src_stat;
    int err;
    err = thread_inc();
    if (err != SUCCESS){
        free(task);
        err = report_fatal_error("thread_inc failed");
        if (err != SUCCESS){
            fprintf(stderr, "FATAL ERROR: report_fatal_error failed\n");
        }
        return NULL;
    }

    err = lstat(task->src_path, &src_stat);
    if (err != SUCCESS) {
        free(task);
        err = report_fatal_error("thread_inc failed");
        if (err != SUCCESS){
            fprintf(stderr, "FATAL ERROR: report_fatal_error failed\n");
        }
        return NULL;
    }

    err = mkdir(task->dst_path, src_stat.st_mode);
    if (err != SUCCESS && errno != EEXIST) {
        free(task);
        err = report_fatal_error("thread_inc failed");
        if (err != SUCCESS){
            fprintf(stderr, "FATAL ERROR: report_fatal_error failed\n");
        }
        return NULL;
    }

    DIR* dir = opendir_with_retry(task->src_path);
    if (dir == NULL) {
        free(task);
        err = report_fatal_error("thread_inc failed");
        if (err != SUCCESS){
            fprintf(stderr, "FATAL ERROR: report_fatal_error failed\n");
        }
        return NULL;
    }

    struct dirent* entry;
    while (1) {
        errno = SUCCESS;
        entry = readdir(dir);
        if (entry == NULL && errno != SUCCESS) {
            printf("work_directory_thread: readdir error: %s\n", strerror(errno));
            closedir(dir);
            free(task);
            err = report_fatal_error("thread_inc failed");
            if (err != SUCCESS){
                fprintf(stderr, "FATAL ERROR: report_fatal_error failed\n");
            }
            return NULL;
        }   
        if (entry == NULL) {
            err = SUCCESS;
            break; 
        }
        if (strcmp(entry->d_name, CURRENT_DIR) == 0 || strcmp(entry->d_name, PARENT_DIR) == 0) {
            continue;
        }
        err = process_single_entry(task->src_path, task->dst_path, entry->d_name);
        if (err != SUCCESS){
            closedir(dir);
            free(task);
            err = report_fatal_error("process_single_entry failed");
            if (err != SUCCESS){
                fprintf(stderr, "FATAL ERROR: report_fatal_error failed\n");
            }
            return NULL;
        }
    }

    err = closedir(dir);
    if (err != SUCCESS) {
        err = report_fatal_error("work_directory_thread: closedir() failed");
        if (err != SUCCESS){
            fprintf(stderr, "FATAL ERROR: report_fatal_error failed\n");
        }
    } 
    free(task);
    err = thread_dec();
    if (err != SUCCESS){
        err = report_fatal_error("thread_inc failed");
        if (err != SUCCESS){
            fprintf(stderr, "FATAL ERROR: report_fatal_error failed\n");
        }
        return NULL;
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    struct stat stat_buf;
    pthread_t main_thread;
    int err;

    if (argc != 3) {
        printf("Usage: %s src dst\n", argv[0]);
        return ERROR;
    }

    err = lstat(argv[1], &stat_buf);
    if (err != SUCCESS) {
        printf("main: lstat() failed: %s\n", strerror(errno));
        return ERROR;
    }
    if (S_ISDIR(stat_buf.st_mode) != true) {
        printf("main: Source path %s is not a directory\n", argv[1]);
        return ERROR;
    }

    err = check_src_dst(argv[1], argv[2]);
    if (err != SUCCESS) {
        return ERROR;
    }

    task_t* task = malloc(sizeof(task_t));
    if (task == NULL) {
        printf("main: memory allocation failed\n");
        return ERROR;
    }

    strcpy(task->src_path, argv[1]);
    strcpy(task->dst_path, argv[2]);

    err = pthread_create(&main_thread, NULL, work_directory_thread, task);
	if (err != SUCCESS) {
        printf("main: pthread_create() failed: %s\n", strerror(err));
        free(task);
		return ERROR;
    }

    err = pthread_join(main_thread, NULL);
	if (err != SUCCESS) {
		printf("main: pthread_join() failed: %s\n", strerror(err));
		return ERROR;
	}

    err = pthread_mutex_lock(&count_mutex);
    if (err != SUCCESS) {
        printf("main: pthread_mutex_lock() failed: %s\n", strerror(err));
        return ERROR;
    }

    while (active_threads > 0 && !fatal_error) {
        pthread_cond_wait(&count_cond, &count_mutex);
    }

    if (fatal_error) {
        err = pthread_mutex_unlock(&count_mutex);
            if (err != SUCCESS) {
            printf("main: pthread_mutex_unlock() failed: %s\n", strerror(err));
            return ERROR;
        }
        fprintf(stderr, "main: terminated due to fatal error\n");
        return ERROR;
    }
    err = pthread_mutex_unlock(&count_mutex);
    if (err != SUCCESS) {
        printf("main: pthread_mutex_unlock() failed: %s\n", strerror(err));
        return ERROR;
    }

    printf("main: all tasks completed\n");
    return SUCCESS;
}
