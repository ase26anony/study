/* test_program.c - Simple program to generate GCOV data */
#include <stdio.h>

int helper_function(int x) {
    if (x > 0) {
        return x * 2;
    } else {
        return x - 1;
    }
}

int main() {
    int result = 0;
    
    // Call helper function multiple times to generate profile data
    for (int i = -2; i <= 2; i++) {
        result += helper_function(i);
    }
    
    printf("Result: %d\n", result);
    return 0;
}
