/* Test 1: Integer condition variable modified in then block */
#include <stdio.h>
#include <stdlib.h>

int test1_modify_condition(int x, int y) {
    int result;
    /* This creates a conditional block where 'x' is used in condition
       and modified inside the then block */
    if (x > 0) {
        result = y * 2;
        x = 5;  /* This modifies the condition variable */
    } else {
        result = y / 2;
    }
    return result + x;  /* Use x to prevent dead store elimination */
}

int test1_pointer_modify(int *p, int threshold) {
    int result;
    /* Pointer comparison with modification in then block */
    if (p != NULL) {
        result = *p * 3;
        p = NULL;  /* Modifies the pointer used in condition */
    } else {
        result = threshold;
    }
    return result + (p != NULL);  /* Use p to prevent optimization */
}

int main(int argc, char **argv) {
    volatile int seed = argc > 1 ? atoi(argv[1]) : 42;
    int x = seed;
    int y = seed * 2;
    int *ptr = &x;
    
    int sum = 0;
    sum += test1_modify_condition(x, y);
    
    x = seed + 1;
    sum += test1_modify_condition(x, y);
    
    sum += test1_pointer_modify(ptr, y);
    
    printf("Result: %d\n", sum);
    return sum > 100 ? 0 : 1;
}
