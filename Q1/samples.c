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
    int rand;

    fseek(file, 0, SEEK_END); // seek to end of file to be able to use ftell
    unsigned size = ftell(file)/sizeof(char); 
    fseek(file, 0, SEEK_SET); // seek back to beginning of file

    int* already_used = (int*) malloc (size * sizeof(int));
    
    if (size < atoi(argv[2]) * atoi(argv[3])) {
        fprintf(stderr, "%s: file %s is too small to be fragmented into %d fragments of size %d\n", argv[0], argv[1], atoi(argv[2]), atoi(argv[3]));
        return EXIT_FAILURE;
    }

    srandom(0); // seed random number generator
    
    for (int i = 0; i < atoi(argv[2]); i++) {
        
        //ensures that the same fragment is not chosen twice
        do { 
            rand = random() % size;
        } 
        while(already_used[rand] == 1);  
        already_used[rand] = 1;

        fseek(file, rand * sizeof(char), SEEK_SET); // seek to random position
        fread (fragment, sizeof(char), atoi(argv[3]), file); // read maxfragsize bytes to fragment

        // Remove non printable characters
        for (int j = 0; j < atoi(argv[3]); j++) {
            if (fragment[j] < 32 || fragment[j] > 126) {
                fragment[j] = ' ';
            }
        }
        
        printf(">%s<\n", fragment);
        fseek(file, 0, SEEK_SET); // reset file pointer to beginning of file

    } 
    free(fragment); 
    fclose(file);
    return EXIT_SUCCESS;
}