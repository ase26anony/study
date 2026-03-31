/* Test case 1: Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

int test_modify_in_then(int x, int y) {
    int result;
    
    /* Simple if-else pattern that if-conversion would like to convert */
    if (x > 0) {
        result = y * 2;
        x = 0;  /* MODIFIES the condition variable x */
    } else {
        result = y + 5;
    }
    
    /* Use modified x to prevent dead store elimination */
    return result + x;
}

int test_modify_in_else(int x, int y) {
    int result;
    
    /* Another pattern where condition is modified in else block */
    if (x <= 10) {
        result = y * 3;
    } else {
        result = y - 2;
        x = 5;  /* MODIFIES condition variable in else block */
    }
    
    return result + x;
}

int main(int argc, char **argv) {
    volatile int seed = argc > 1 ? atoi(argv[1]) : 42;
    int sum = 0;
    
    /* Call functions with volatile/input-dependent values */
    sum += test_modify_in_then(seed, seed + 1);
    sum += test_modify_in_else(seed + 2, seed + 3);
    
    printf("Result: %d\n", sum);
    return sum;
}
