#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

enum Errors{
    OK,
    ERROR
};

enum Errors Str_to_int(char *str, int *answer, int line){
    char *endptr;
    long number = strtol(str, &endptr, 10);

    if (number == LONG_MAX || number == LONG_MIN){
        char bufferic[BUFSIZ];
        int length = snprintf(bufferic, sizeof(bufferic), "Error, incorrect value in %d line\n", line);
        write(STDOUT_FILENO, bufferic, length);
        return ERROR;
    }
    else if (*endptr != '\0' || number > INT_MAX || number < INT_MIN) {
        char bufferic[BUFSIZ];
        int length = snprintf(bufferic, sizeof(bufferic), "Error, incorrect value in %d line\n", line);
        write(STDOUT_FILENO, bufferic, length);
        return ERROR;
    }
    *answer = number;
    return OK;

}
int main(){

    char buffer[BUFSIZ];
    int line_sum = 0, line = 1;

    while(fgets(buffer, sizeof(buffer), stdin) != NULL){


        int valid_line = 1;
        buffer[strcspn(buffer, "\n")] = '\0';
        buffer[strcspn(buffer, "\r")] = '\0';

        char *token = strtok(buffer, " ");


        while (token != NULL) {
            int num;
            if (Str_to_int(token, &num, line) != OK) {
                valid_line = 0;

                line++;
                line_sum = 0;
                break;

            } else {
                line_sum += num;
            }
            token = strtok(NULL, " ");
        }

        if (valid_line) {
            char buf[BUFSIZ];
            int length = snprintf(buf, sizeof(buf), "Sum in %d line = %d\n", line, line_sum);
            ssize_t bytes_written = write(STDOUT_FILENO, buf, length);

            // Check
            if (bytes_written == -1) {
                write(STDERR_FILENO, "Error writing to stdout\n", 24);
            } else if (bytes_written != length) {
                write(STDERR_FILENO, "Warning: partial write\n", 23);
            }
            line_sum = 0;
            line++;
        }

    }
    if (ferror(stdin)) {
        write(STDERR_FILENO, "Error reading input\n", 20);
        exit(EXIT_FAILURE);
    }


}