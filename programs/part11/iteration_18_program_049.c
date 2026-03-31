/* Simple test program to generate coverage data */
#include <stdio.h>

int main(int argc, char *argv[]) {
    int x = 0;
    
    if (argc > 1) {
        x = atoi(argv[1]);
    }
    
    for (int i = 0; i < x; i++) {
        printf("Iteration %d\n", i);
    }
    
    if (x > 10) {
        printf("Large value\n");
    } else {
        printf("Small value\n");
    }
    
    return 0;
}
