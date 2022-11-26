#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>

int main (int argc, char* argv[]) {

    if (argc < 2) {
        printf("usage: txt2epub file");
        return EXIT_FAILURE;
    }

    char* files[argc - 1];  // array of file names
    for (int i = 1; i < argc; i++) {
        files[i - 1] = (char*) malloc ((strlen(argv[i]) + 1) * sizeof(char)); // allocate memory for file name
        strcpy(files[i - 1], argv[i]); // copy file name to array of file names
        files[i - 1][strlen(files[i - 1]) - 4] = '\0'; // remove .txt from file name
        strcat(files[i - 1], ".epub"); // add .epub to file name
    }
    


    pid_t pid[argc - 1];  // array of pids 
    for (int i = 1; i < argc; i++) { // loop through all files
        pid[i - 1] = fork(); // fork a child process for each file

        if (pid[i - 1] < 0) { 
            fprintf(stderr, "%s: error forking process %s\n", argv[0], strerror(errno));
            return EXIT_FAILURE;

        } else if (pid[i - 1] == 0) {
            printf("[pid %d] converting %s ...\n", getpid(), argv[i]);  

            //IMPORTANT: if you don't have pandoc, use sudo apt install pandoc before running
            execlp("pandoc", "pandoc", argv[i], "-o", files[i - 1], "--quiet", NULL); // convert file to epub

            fprintf(stderr, "%s: could not convert file to epub %s\n", argv[0], strerror(errno)); 
            exit(EXIT_FAILURE); 
        }
    }

    // wait for all children to finish
    for (int i = 0; i < argc - 1; i++) {
        int status;
        waitpid(pid[i], &status, 0);

        if (status < 0) { 
            fprintf(stderr, "%s: waitpid errors: %s\n", argv[0], strerror(errno));
            return EXIT_FAILURE;
        }
    }

}