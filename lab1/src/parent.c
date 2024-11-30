#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[]){
    if(argc != 2){
        write(STDERR_FILENO, "Error, there should be 2 arguments here\n", 40);
        exit(EXIT_FAILURE);

    }

    FILE *file = fopen(argv[1], "r");
    if(!file){
        write(STDERR_FILENO, "Error with open file\n", 21);
        exit(EXIT_FAILURE);
    }

    int channel[2];
    if(pipe(channel) == -1){
        fclose(file);
        write(STDERR_FILENO, "Error with create pipe\n", 24);
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if(pid == 0){

        close(channel[0]); // close read
        int file_fd = fileno(file); // взяли дескриптор
        if(dup2(file_fd, STDIN_FILENO) == -1){  // переопределили поток входа на файл
            write(STDERR_FILENO, "Error redirecting stdin\n", 24);
            exit(EXIT_FAILURE);
        }


        if(dup2(channel[1], STDOUT_FILENO) == -1){
            write(STDERR_FILENO, "Error redirecting stdout\n", 25);
            exit(EXIT_FAILURE);
        }
        close(channel[1]); // close write

        if(execl("./child", "", NULL) == -1){
            write(STDERR_FILENO, "Error execl\n", 12);
            exit(EXIT_FAILURE);
        }
        fclose(file);
    }
    else if(pid > 0){

        close(channel[1]); // close write
        ssize_t bytes_read;

        char buf[BUFSIZ];
        while((bytes_read = read(channel[0], buf, sizeof(buf))) > 0){
            write(STDOUT_FILENO, buf, bytes_read);

        }
        // Check
        if (bytes_read == -1) {
            write(STDERR_FILENO, "Error reading from pipe\n", 24);
            exit(EXIT_FAILURE);
            write(STDOUT_FILENO, buf, bytes_read);
        }

        close(channel[0]); // close read
        fclose(file);


        // Ждем завершения дочернего процесса
        int status;
        if (wait(&status) == -1) {
            write(STDERR_FILENO, "Error waiting for child process\n", 32);
            exit(EXIT_FAILURE);
        } else if (!(WIFEXITED(status))) {
            write(STDERR_FILENO, "Child process terminated with error\n", 36);

        }
    }
    else{
        fclose(file);
        close(channel[0]);
        close(channel[1]);
        write(STDERR_FILENO, "Error with fork\n", 16);
        exit(EXIT_FAILURE);

    }
    return 0;
}