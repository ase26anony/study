/* test1.c - Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Simple integer condition modified in then block */
int test1_modify_in_then(int x, int y) {
    int result;
    /* Condition variable x gets modified in the then block */
    if (x > 0) {
        result = y * 2;
        x = 5;  /* Modifies the condition variable - should trigger modified_in_p */
    } else {
        result = y / 2;
    }
    return result + x;  /* Use x to prevent dead store elimination */
}

/* Function 2: Condition variable modified in both branches */
int test1_modify_in_both(int a, int b) {
    int res;
    if (a != b) {
        res = a + b;
        a = b;  /* Modify condition variable in then block */
    } else {
        res = a - b;
        a = 0;  /* Also modify in else block */
    }
    return res * a;  /* Use modified a */
}

/* Function 3: Nested modification scenario */
int test1_nested_modification(int x, int y, int z) {
    int val;
    /* Complex condition using x */
    if (x > y && x < z) {
        val = x * y;
        x = (x + y) / 2;  /* Modify x in then block */
    } else {
        val = x + z;
        /* Don't modify x here to test asymmetric case */
    }
    return val - x;  /* Use modified x */
}

volatile int global_seed = 42;

int main(int argc, char **argv) {
    int sum = 0;
    
    /* Use argv to get input-dependent values */
    int base = argc > 1 ? atoi(argv[1]) : global_seed;
    
    /* Call all test functions with varying inputs */
    sum += test1_modify_in_then(base, base + 1);
    sum += test1_modify_in_both(base, base * 2);
    sum += test1_nested_modification(base, base - 1, base + 2);
    
    printf("Test1 result: %d\n", sum);
    return sum;
}
