/* Simple test program to generate GCOV data */
#include <stdio.h>

int helper(int x) {
    if (x > 0) {
        return x * 2;
    } else {
        return x - 1;
    }
}

int main() {
    int i;
    printf("Test program for gcov-dump\n");
    
    /* Generate some coverage data */
    for (i = -3; i <= 3; i++) {
        printf("helper(%d) = %d\n", i, helper(i));
    }
    
    /* Conditional branch */
    if (helper(5) > 0) {
        printf("Positive result\n");
    } else {
        printf("Non-positive result\n");
    }
    
    return 0;
}
