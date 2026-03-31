/* test1.c - Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Direct modification of condition variable */
int test1_modify_in_then(int x, int y) {
    int result;
    /* Condition variable x gets modified in then block */
    if (x > 0) {
        result = y * 2;
        x = 0;  /* Modifies the condition variable */
    } else {
        result = y / 2;
    }
    return result + x;  /* Use x to prevent optimization */
}

/* Function 2: Multiple modifications in then block */
int test1_complex_modify(int a, int b) {
    int res;
    volatile int cond = a;  /* Prevent constant folding */
    
    if (cond > 10) {
        res = b + 100;
        cond = 5;      /* First modification */
        cond = cond * 2; /* Second modification */
    } else {
        res = b - 100;
    }
    
    return res + cond;  /* Use cond to keep it alive */
}

/* Function 3: Modification with pointer arithmetic */
int test1_pointer_modify(int *ptr, int val) {
    int result;
    int local = *ptr;  /* Read from pointer */
    
    if (local > val) {
        result = val * 3;
        local = val - 1;  /* Modify condition variable */
    } else {
        result = val * 2;
    }
    
    *ptr = local;  /* Store back to prevent optimization */
    return result;
}

int main(int argc, char **argv) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    volatile int v1 = seed;
    volatile int v2 = seed * 2;
    int arr[3] = {seed, seed + 1, seed + 2};
    
    int sum = 0;
    sum += test1_modify_in_then(v1, v2);
    sum += test1_complex_modify(v2, v1);
    sum += test1_pointer_modify(arr, v1);
    
    printf("Test1 result: %d\n", sum);
    return sum % 256;
}
