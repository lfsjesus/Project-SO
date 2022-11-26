#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
int main(int argc, char* argv[]) {

    if (argc != 4) {
        printf("usage: samples file numberfrags maxfragsize");
        return EXIT_FAILURE;
    }

    FILE* file = fopen(argv[1], "r");

    if (file == NULL) {
        fprintf(stderr, "%s: could not open file %s: %s\n", argv[0], argv[1], strerror(errno));
        return EXIT_FAILURE;
    }

    char* fragment = (char*) malloc ((atoi(argv[3]) + 1) * sizeof(char)); 
    
    fseek(file, 0, SEEK_END); // seek to end of file to be able to use ftell
    unsigned size = ftell(file)/sizeof(char); 
    fseek(file, 0, SEEK_SET); // seek back to beginning of file

    srandom(0); // seed random number generator
    for (unsigned i = 0; i < atoi(argv[2]); i++) {
        int rand = random() % size; // get random number between 0 and size
        fseek(file, rand * sizeof(char), SEEK_SET); // seek to random position
        fread (fragment, sizeof(char), atoi(argv[3]), file); // read maxfragsize bytes to fragment

        for (unsigned j = 0; j < atoi(argv[3]); j++) {
            if (fragment[j] < ' ' || fragment[j] > '~')  // check if fragment contains non-printable characters
                fragment[j] = ' ';     
        }

    printf(">%s<\n", fragment);
    fseek(file, 0, SEEK_SET); // reset file pointer to beginning of file

    } 
    fclose(file);
    return EXIT_SUCCESS;
}