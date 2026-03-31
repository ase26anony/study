/* Simple test program to generate coverage data */
#include <stdio.h>

int main() {
    printf("Generating coverage data for gcov-dump test\n");
    int x = 0;
    
    /* Create some basic code structure for coverage */
    if (x == 0) {
        printf("x is zero\n");
    } else {
        printf("x is not zero\n");
    }
    
    for (int i = 0; i < 3; i++) {
        printf("Loop iteration %d\n", i);
    }
    
    return 0;
}
