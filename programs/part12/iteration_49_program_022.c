/* test.c - Minimal program to generate GCOV data */
#include <stdio.h>

int helper(int x) {
    if (x > 0) {
        return x * 2;
    } else {
        return x - 1;
    }
}

int main() {
    int result = helper(5);
    printf("Result: %d\n", result);
    
    // Another call with different value
    result = helper(-3);
    printf("Result: %d\n", result);
    
    return 0;
}
