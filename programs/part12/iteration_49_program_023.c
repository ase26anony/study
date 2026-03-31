/* test_gcov.c - Minimal program to generate GCOV data */
#include <stdio.h>

void helper_function(int x) {
    if (x > 0) {
        printf("Positive: %d\n", x);
    } else {
        printf("Non-positive: %d\n", x);
    }
}

int main() {
    int i;
    for (i = -1; i <= 2; i++) {
        helper_function(i);
    }
    return 0;
}
