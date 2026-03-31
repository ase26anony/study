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
    int result = 0;
    
    /* Generate some branching for coverage data */
    for (int i = 0; i < 5; i++) {
        if (i % 2 == 0) {
            result += helper(i);
        } else {
            result -= helper(i);
        }
    }
    
    printf("Result: %d\n", result);
    return 0;
}
