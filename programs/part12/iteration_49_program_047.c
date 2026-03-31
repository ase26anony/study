/* test_gcov.c - Minimal C program for GCOV testing */
#include <stdio.h>

void helper_function(int x) {
    if (x > 0) {
        printf("Positive: %d\n", x);
    } else if (x < 0) {
        printf("Negative: %d\n", x);
    } else {
        printf("Zero\n");
    }
}

int main() {
    int i;
    for (i = -1; i <= 1; i++) {
        helper_function(i);
    }
    return 0;
}
