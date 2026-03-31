/* test1.c - Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Direct modification of condition variable */
int test1_modify_in_then(int x, int y) {
    int result;
    /* This should trigger modified_in_p check */
    if (x > 0) {
        result = y * 2;
        x = 5;  /* Modifies condition variable x */
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
        res = b * 3;
        cond = cond + 1;    /* First modification */
        cond = cond * 2;    /* Second modification */
    } else {
        res = b + 5;
    }
    
    return res + (cond % 10);  /* Use cond to keep it alive */
}

int main(int argc, char **argv) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    int x = seed % 20;
    int y = seed % 30 + 1;
    
    int result1 = test1_modify_in_then(x, y);
    int result2 = test1_complex_modify(x, y);
    
    printf("Test1 results: %d, %d\n", result1, result2);
    return result1 + result2;
}
