/* Test 1: Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

int test1_modify_in_then(int x, int y) {
    int result;
    
    /* Simple if-else pattern that's attractive for if-conversion */
    if (x > 0) {
        result = y * 2;
        x = 5;  /* Modifies the condition variable x */
    } else {
        result = y + 10;
    }
    
    /* Use modified x to prevent dead store elimination */
    return result + x;
}

int test1_pointer_arithmetic(int *ptr, int threshold) {
    int val;
    
    /* Condition uses pointer dereference */
    if (*ptr > threshold) {
        val = 100;
        (*ptr)++;  /* Modifies the condition expression */
    } else {
        val = 200;
    }
    
    return val + *ptr;
}

volatile int global_seed = 0;

int main_test1(int argc, char **argv) {
    int sum = 0;
    
    /* Use argv to get non-constant values */
    int x = argc > 1 ? atoi(argv[1]) : 10;
    int y = argc > 2 ? atoi(argv[2]) : 20;
    
    /* Call test functions multiple times with different values */
    sum += test1_modify_in_then(x, y);
    sum += test1_modify_in_then(-x, y);
    
    int arr[2] = {x, y};
    sum += test1_pointer_arithmetic(&arr[0], 5);
    sum += test1_pointer_arithmetic(&arr[1], 15);
    
    /* Use volatile to prevent optimization */
    sum += global_seed;
    
    return sum;
}
