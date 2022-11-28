#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#define READ_END 0
#define WRITE_END 1

int main (int argc, char* argv[]) {

    if (argc != 4) {
        printf("usage: tokenring n_processes probability delay(seconds)");
        return EXIT_FAILURE;
    }

    float p = atof(argv[2]); // probability

    char* mem_loc = (char*)malloc(50*sizeof(char)); // allocate memory for 50 characters

    for (int i = 1; i <= atoi(argv[1]); i++) { 
        if (i == atoi(argv[1])) { // if last process
            sprintf(mem_loc, "pipe%dto1", i);
        }
        else { // if not last process
            sprintf(mem_loc, "pipe%dto%d", i, i + 1);
        }

        int pipe = mkfifo(mem_loc, 0666); // create named pipe with read/write permissions for all users

        if (pipe < 0) {
            fprintf(stderr, "%s: error creating named pipe %s\n", argv[0], strerror(errno));
            return EXIT_FAILURE;
        }
    }
    free(mem_loc); // free memory
    

    char* writing = (char*) malloc (50 * sizeof(char));
    char* reading = (char*) malloc (50 * sizeof(char));

    pid_t pid[atoi(argv[1])]; // array of pids
    for (int i = 1; i <= atoi(argv[1]); i++) {
        pid[i - 1] = fork(); // fork a child process for each process

        if (pid[i - 1] < 0) {
            fprintf(stderr, "%s: error forking process %s\\n", argv[0], strerror(errno));
            return EXIT_FAILURE;
        }
        else if (pid[i - 1] == 0) {
            if (i == 1) { // if first process
                sprintf(writing, "pipe%dto%d", i, i + 1); //sprintf because we need to write to a "string"
                sprintf(reading, "pipe%dto1", atoi(argv[1]));
            }
            else if (i == atoi(argv[1])) { // if last process
                sprintf(writing, "pipe%dto1", i); // closes the ring
                sprintf(reading, "pipe%dto%d", i - 1, i);
            }
            else { // if not first or last process
                sprintf(writing, "pipe%dto%d", i, i + 1);
                sprintf(reading, "pipe%dto%d", i - 1, i);
            }
            
            
            // send a token to next process
            int fd[2];
            int token = 0;

            if (i == 1) { 
                fd[WRITE_END] = open(writing, O_WRONLY); // open write end of pipe
                if (fd[WRITE_END] < 0) {
                    fprintf(stderr, "%s: error opening write end of pipe %s\n", argv[0], strerror(errno));
                    return EXIT_FAILURE;
                }

                if (write(fd[WRITE_END], &token, sizeof(int)) < 0) {
                    fprintf(stderr, "%s: error writing to pipe %s\n", argv[0], strerror(errno));
                    return EXIT_FAILURE;
                }
                token++;
                close(fd[WRITE_END]); // close write end of pipe because we are done writing
            }

            srand(i); // seed random number generator
            while (1) {
                //receive token from previous process
                fd[READ_END] = open(reading, O_RDONLY); // open read end of pipe

                if (fd[READ_END] < 0) {
                    fprintf(stderr, "%s: error opening read end of pipe %s\\n", argv[0], strerror(errno));
                    return EXIT_FAILURE;
                }

                if (read(fd[READ_END], &token, sizeof(int)) < 0) { 
                    fprintf(stderr, "%s: error reading from pipe %s\\n", argv[0], strerror(errno));
                    return EXIT_FAILURE;
                }
                close(fd[READ_END]); // close read end of pipe because we are done reading
                token++; // increment because we have received the token
            

                // probability of sending token to next process
                int prob = (rand() % 100); // random number between 0 and 99

                if (prob < p) { 
                    // locking sending token to next process
                    printf("[p%d]lock on token (val = %d)\n", i, token);

                    sleep(atoi(argv[3])); // sleep for argv[3] seconds

                    printf("[p%d]unlock token\n", i);
                }
                
                // send token to next process
                fd[WRITE_END] = open(writing, O_WRONLY); // open write end of pipe
                if (fd[WRITE_END] < 0) {
                    fprintf(stderr, "%s: error opening write end of pipe %s\\n", argv[0], strerror(errno));
                    return EXIT_FAILURE;
                }

                if (write(fd[WRITE_END], &token, sizeof(int)) < 0) {
                    fprintf(stderr, "%s: error writing to pipe %s\n", argv[0], strerror(errno));
                    return EXIT_FAILURE;
                }
                close(fd[WRITE_END]); // close write end of pipe because we are done writing
            }
            return EXIT_SUCCESS;
        }
    }

    // wait for all child processes to finish
    for (int i = 0; i < atoi(argv[1]); i++) {
        int status;
        if (waitpid(pid[i], &status, 0) < 0) {
            fprintf(stderr, "%s: error waiting for child process %s\\n", argv[0], strerror(errno));
            return EXIT_FAILURE;
        }
    }
}