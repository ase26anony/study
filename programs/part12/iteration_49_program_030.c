/* test_gcov.c - Minimal program to generate GCOV data */
#include <stdio.h>

int helper(int x) {
    if (x > 0) {
        return x * 2;
    } else {
        return x - 1;
    }
}

int main() {
    int a = 5;
    int b = -3;
    
    printf("Helper(5) = %d\n", helper(a));
    printf("Helper(-3) = %d\n", helper(b));
    
    // Create some branching for coverage
    for (int i = 0; i < 3; i++) {
        if (i % 2 == 0) {
            printf("Even: %d\n", i);
        } else {
            printf("Odd: %d\n", i);
        }
    }
    
    return 0;
}
