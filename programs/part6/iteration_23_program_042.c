/* Test case 1: Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

int test1_modify_in_then(int x, int y) {
    int result;
    /* Simple if-else pattern that if-conversion would consider */
    if (x > 0) {
        result = y * 2;
        x = 0;  /* Modifies the condition variable x */
    } else {
        result = y / 2;
    }
    return result + x;  /* Use x to prevent dead store elimination */
}

int test1_pointer_modify(int *ptr, int threshold) {
    int val;
    /* Another pattern: pointer comparison with modification */
    if (ptr != NULL) {
        val = *ptr * 3;
        ptr = NULL;  /* Modifies the condition variable ptr */
    } else {
        val = threshold * 5;
    }
    /* Use ptr in computation to prevent optimization */
    return val + (ptr != NULL ? 1 : 0);
}

int main_test1(int argc, char **argv) {
    volatile int seed = argc > 1 ? atoi(argv[1]) : 42;
    int x = seed % 10;
    int y = seed % 20 + 1;
    
    int sum = 0;
    sum += test1_modify_in_then(x, y);
    
    int arr[5] = {1, 2, 3, 4, 5};
    int *ptr = (seed % 3) ? arr : NULL;
    sum += test1_pointer_modify(ptr, y);
    
    return sum;
}
