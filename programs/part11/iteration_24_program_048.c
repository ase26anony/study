/* Simple test program to generate coverage data files */
#include <stdio.h>

int main() {
    printf("Generating coverage data for gcov-dump test\n");
    int x = 0;
    
    /* Add some basic logic to generate coverage data */
    for (int i = 0; i < 10; i++) {
        x += i;
    }
    
    if (x > 0) {
        printf("Sum is positive: %d\n", x);
    } else {
        printf("Sum is non-positive\n");
    }
    
    return 0;
}
