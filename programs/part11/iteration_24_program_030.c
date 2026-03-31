/* Simple test program to generate coverage data files */
#include <stdio.h>

int main() {
    printf("Generating coverage data for gcov-dump test\n");
    int x = 5;
    if (x > 0) {
        printf("x is positive\n");
    } else {
        printf("x is non-positive\n");
    }
    return 0;
}
