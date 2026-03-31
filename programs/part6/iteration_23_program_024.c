/* test1.c - Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Direct modification of condition variable */
int test1_modify_condition(int x, int y) {
    int result;
    volatile int cond = x;  /* Prevent optimization */
    
    /* This should trigger modified_in_p check */
    if (cond > 0) {
        result = y * 2;
        cond = 0;  /* MODIFIES CONDITION VARIABLE - should trigger uncovered code */
    } else {
        result = y / 2;
    }
    
    return result + cond;  /* Use cond to prevent dead store elimination */
}

/* Function 2: Multiple modifications in then block */
int test1_multiple_mods(int x, int y) {
    int result;
    int cond = x;
    
    if (cond != 0) {
        result = y + 10;
        cond++;      /* First modification */
        cond *= 2;   /* Second modification - both should be detected */
    } else {
        result = y - 10;
    }
    
    return result * cond;
}

int main(int argc, char **argv) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    int result = 0;
    
    /* Call with different patterns to ensure coverage */
    result += test1_modify_condition(seed, seed * 2);
    result += test1_multiple_mods(seed % 3, seed);
    
    printf("Test1 result: %d\n", result);
    return result != 0 ? 0 : 1;
}
