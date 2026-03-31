/* test1.c - Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

volatile int global_seed = 0;

/* Function 1: Direct modification of condition variable in then block */
int test_modify_in_then(int x, int y) {
    int result;
    
    /* Condition variable x gets modified in the then block */
    if (x > 0) {
        result = y * 2;
        x = 0;  /* This modifies the condition variable! */
    } else {
        result = y / 2;
    }
    
    /* Use x to prevent optimization */
    return result + x;
}

/* Function 2: Modification through pointer in then block */
int test_modify_through_ptr(int x, int y) {
    int *ptr = &x;
    int result;
    
    if (x != 0) {
        result = y + 10;
        *ptr = 0;  /* Modifies x through pointer */
    } else {
        result = y - 10;
    }
    
    return result + x;
}

/* Function 3: Multiple modifications in then block */
int test_multiple_modifications(int x, int y, int z) {
    int result;
    
    if (x > y) {
        result = z * 3;
        x = y;      /* First modification */
        x = x + 1;  /* Second modification */
    } else {
        result = z / 3;
    }
    
    return result + x;
}

int main(int argc, char **argv) {
    int sum = 0;
    
    /* Use argv to get non-constant values */
    int base = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Call all test functions with varying inputs */
    sum += test_modify_in_then(base, base + 1);
    sum += test_modify_through_ptr(base + 2, base + 3);
    sum += test_multiple_modifications(base + 4, base + 5, base + 6);
    
    /* Also test with global variable */
    global_seed = base;
    if (global_seed > 10) {
        sum += 100;
        global_seed = 5;  /* Modify global used in condition */
    }
    
    printf("Result: %d\n", sum);
    return sum;
}
