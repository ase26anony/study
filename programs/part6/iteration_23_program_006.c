/* Test 1: Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

int test1_modify_in_then(int x, int y) {
    int result;
    int cond = x > 0;  // Condition variable
    
    if (cond) {
        result = y * 2;
        cond = 0;  // MODIFIES condition variable in then block
    } else {
        result = y / 2;
    }
    
    // Use cond again to prevent optimization
    return result + (cond ? 1 : 0);
}

int test1_modify_in_else(int x, int y) {
    int result;
    int cond = x != 0;  // Condition variable
    
    if (cond) {
        result = y + 10;
    } else {
        result = y - 10;
        cond = 1;  // MODIFIES condition variable in else block
    }
    
    // Use cond again
    return result * (cond ? 2 : 1);
}

int main_test1(int argc, char **argv) {
    volatile int seed = argc > 1 ? atoi(argv[1]) : 42;
    int x = seed % 10;
    int y = seed % 20;
    
    int sum = 0;
    sum += test1_modify_in_then(x, y);
    sum += test1_modify_in_else(x, y);
    
    printf("Test1 result: %d\n", sum);
    return sum;
}
