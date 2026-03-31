/* test.c - Simple program to generate coverage data */
#include <stdio.h>

int main(int argc, char *argv[]) {
    int x = 0;
    
    if (argc > 1) {
        x = atoi(argv[1]);
    }
    
    if (x > 0) {
        printf("Positive: %d\n", x);
    } else if (x < 0) {
        printf("Negative: %d\n", x);
    } else {
        printf("Zero\n");
    }
    
    for (int i = 0; i < x; i++) {
        printf("Iteration %d\n", i);
    }
    
    return 0;
}
