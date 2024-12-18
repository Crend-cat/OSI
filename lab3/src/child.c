#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>
#include <limits.h>

#define SHM_NAME "/shared_memory"
#define SEM_WRITE "/sem_write"
#define SEM_READ "/sem_read"
#define BUF_SIZE 1024

enum Errors {
    OK,
    ERROR
};

enum Errors Str_to_int(char *str, int *answer, int line) {
    char *endptr;
    long number = strtol(str, &endptr, 10);

    if (number > INT_MAX || number < INT_MIN || *endptr != '\0') {
        fprintf(stderr, "Error, incorrect value in %d line\n", line);
        return ERROR;
    }
    *answer = (int) number;
    return OK;
}

int main() {
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Error opening shared memory");
        exit(EXIT_FAILURE);
    }

    char *shm_ptr = mmap(0, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("Error mapping shared memory");
        exit(EXIT_FAILURE);
    }

    sem_t *sem_write = sem_open(SEM_WRITE, 0);
    sem_t *sem_read = sem_open(SEM_READ, 0);
    if (sem_write == SEM_FAILED || sem_read == SEM_FAILED) {
        perror("Error opening semaphores");
        exit(EXIT_FAILURE);
    }

    int line = 1;
    while (1) {
        sem_wait(sem_read);
        if (shm_ptr[0] == '\0') {
            break;
        }

        char *token = strtok(shm_ptr, " ");
        int line_sum = 0;
        int valid_line = 1;

        while (token != NULL) {
            int num;
            if (Str_to_int(token, &num, line) != OK) {
                valid_line = 0;
                break;
            }
            line_sum += num;
            token = strtok(NULL, " ");
        }

        if (valid_line) {
            printf("Sum in %d line = %d\n", line, line_sum);
        } else {
            fprintf(stderr, "Error processing line %d\n", line);
        }

        line++;
        sem_post(sem_write);
    }

    munmap(shm_ptr, BUF_SIZE);
    sem_close(sem_write);
    sem_close(sem_read);
    return 0;
}
