/* Test 1: Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

int test1_modify_in_then(int x, int y) {
    int result;
    
    /* Simple if-else pattern that if-conversion would like to convert */
    if (x > 0) {
        result = y * 2;
        x = 0;  /* MODIFIES the condition variable x */
    } else {
        result = y / 2;
    }
    
    /* Use modified x to prevent dead store elimination */
    return result + x;
}

int test1_pointer_modify(int *ptr, int threshold) {
    int val;
    
    /* Pointer-based condition */
    if (ptr != NULL) {
        val = *ptr * 3;
        ptr = NULL;  /* MODIFIES the condition variable ptr */
    } else {
        val = threshold * 3;
    }
    
    /* Use ptr to prevent optimization */
    return val + (ptr == NULL ? 0 : 1);
}

int main(int argc, char **argv) {
    volatile int seed = argc > 1 ? atoi(argv[1]) : 42;
    int x = seed;
    int y = seed * 2;
    int *ptr = &x;
    
    int sum = 0;
    sum += test1_modify_in_then(x, y);
    
    x = seed + 1;
    sum += test1_modify_in_then(x, y);
    
    sum += test1_pointer_modify(ptr, y);
    
    ptr = NULL;
    sum += test1_pointer_modify(ptr, y);
    
    printf("Test1 result: %d\n", sum);
    return sum > 100 ? 0 : 1;
}
