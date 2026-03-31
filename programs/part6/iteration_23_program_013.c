/* test1.c - Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

int test1_modify_in_then(int x, int y) {
    int result;
    // Condition variable 'x' gets modified in the then block
    if (x > 0) {
        result = y * 2;
        x = 5;  // This modifies the condition variable
    } else {
        result = y / 2;
    }
    return result + x;  // Use both to prevent optimization
}

int test1_pointer_arithmetic(int *p, int threshold) {
    int result;
    // Condition involves pointer comparison
    if (p != NULL) {
        result = *p * 3;
        p++;  // Modify the pointer used in condition
    } else {
        result = threshold;
    }
    return result;
}

int main_test1(int argc, char **argv) {
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 42;
    int x = seed;
    int y = seed * 2;
    
    int sum = 0;
    sum += test1_modify_in_then(x, y);
    
    int arr[2] = {seed, seed + 1};
    int *ptr = (seed % 3) ? arr : NULL;
    sum += test1_pointer_arithmetic(ptr, seed);
    
    return sum;
}
