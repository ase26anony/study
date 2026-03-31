/* Test 1: Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

int test1_modify_condition_in_then(int x, int y) {
    int result;
    /* This creates a conditional block where 'x' is used in condition
       and then modified inside the then block */
    if (x > 0) {
        result = y + 10;
        x = 5;  /* This modifies the condition variable */
    } else {
        result = y - 10;
    }
    return result + x;  /* Use x to prevent dead store elimination */
}

int test1_pointer_arithmetic(int *ptr, int threshold) {
    int result;
    /* Condition uses pointer, then modifies it in then block */
    if (*ptr > threshold) {
        result = 100;
        ptr++;  /* Modifies the pointer used in condition */
    } else {
        result = 200;
    }
    return result + *ptr;  /* Use ptr to prevent optimization */
}

int main(int argc, char **argv) {
    volatile int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    int x = seed;
    int y = seed * 2;
    int result1 = test1_modify_condition_in_then(x, y);
    
    int arr[3] = {seed, seed + 1, seed + 2};
    int *ptr = arr;
    int result2 = test1_pointer_arithmetic(ptr, seed);
    
    printf("Test1 results: %d, %d\n", result1, result2);
    return result1 + result2;
}
