/* test1.c - Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Direct modification of condition variable */
int test1_modify_in_then(int x, int y) {
    int result;
    /* This should trigger modified_in_p check */
    if (x > 0) {
        result = y * 2;
        x = 5;  /* Modifies the condition variable */
    } else {
        result = y / 2;
    }
    return result + x; /* Use x to prevent optimization */
}

/* Function 2: Multiple modifications */
int test1_multiple_mods(int a, int b) {
    int res;
    volatile int cond = a; /* Prevent constant folding */
    
    if (cond > 10) {
        res = b + 100;
        cond = cond - 5;  /* First modification */
        cond = cond * 2;  /* Second modification */
    } else {
        res = b - 100;
    }
    
    return res + cond; /* Use cond to keep it alive */
}

/* Function 3: Modification with side effect */
int test1_side_effect_mod(int x, int y) {
    int *ptr = &x;
    int result;
    
    if (x != 0) {
        result = y * 3;
        (*ptr)++;  /* Modifies x through pointer */
    } else {
        result = y / 3;
    }
    
    return result + x;
}

int main_test1(int argc, char **argv) {
    int sum = 0;
    
    /* Use argv to get non-constant values */
    int base = argc > 1 ? atoi(argv[1]) : 42;
    
    sum += test1_modify_in_then(base, base + 1);
    sum += test1_multiple_mods(base + 2, base + 3);
    sum += test1_side_effect_mod(base + 4, base + 5);
    
    printf("Test1 sum: %d\n", sum);
    return sum;
}
