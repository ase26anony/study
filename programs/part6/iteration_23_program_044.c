/* test1.c - Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

int test1(int x, int y) {
    int result;
    int cond = x > 0;  // Condition variable
    
    if (cond) {
        result = y * 2;
        cond = 0;  // MODIFIES condition variable - should trigger modified_in_p
    } else {
        result = y / 2;
    }
    
    // Use cond again to prevent dead store elimination
    return result + (cond ? 10 : 0);
}

int test1_variant(int x, int y) {
    int result;
    volatile int cond = x > 0;  // volatile to prevent optimization
    
    if (cond) {
        result = y + 100;
        cond = x < 0;  // Different modification
    } else {
        result = y - 100;
    }
    
    return result;
}

int main_test1(int argc, char **argv) {
    int x = (argc > 1) ? atoi(argv[1]) : 5;
    int y = (argc > 2) ? atoi(argv[2]) : 10;
    
    int sum = 0;
    sum += test1(x, y);
    sum += test1_variant(x, y);
    
    // Use volatile to prevent optimization
    volatile int output = sum;
    return output;
}
