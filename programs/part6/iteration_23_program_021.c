/* Test case 1: Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

int test1_modify_in_then(int x, int y) {
    int result;
    
    /* Simple if-else pattern that's a candidate for if-conversion */
    if (x > 0) {
        result = y * 2;
        x = 5;  /* Modifies the condition variable x */
    } else {
        result = y + 10;
    }
    
    /* Use both result and modified x to prevent optimization */
    return result + x;
}

int test1_pointer_arithmetic(int *ptr, int threshold) {
    int value;
    
    /* Condition uses pointer dereference */
    if (*ptr > threshold) {
        value = 100;
        (*ptr)++;  /* Modifies the dereferenced value used in condition */
    } else {
        value = 200;
    }
    
    return value + *ptr;
}

int main_test1(int argc, char **argv) {
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 42;
    int x = seed;
    int y = seed * 2;
    
    int arr[2] = {seed, seed + 10};
    int *ptr = arr;
    
    int sum = 0;
    sum += test1_modify_in_then(x, y);
    sum += test1_pointer_arithmetic(ptr, 50);
    
    printf("Test1 result: %d\n", sum);
    return sum;
}
