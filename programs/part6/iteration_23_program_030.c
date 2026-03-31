/* test1.c - Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Simple integer condition modified in then block */
int test1_modify_condition_in_then(int x, int y) {
    int result;
    /* Condition variable x gets modified in then block */
    if (x > 0) {
        result = y * 2;
        x = 5;  /* Modifies the condition variable */
    } else {
        result = y / 2;
    }
    return result + x;  /* Use x to prevent optimization */
}

/* Function 2: Multiple modifications in then block */
int test1_complex_modification(int a, int b) {
    int res;
    /* a is used in condition and modified in then block */
    if (a != b) {
        res = b * 3;
        a = b;      /* First modification */
        a += 1;     /* Second modification */
    } else {
        res = a * 2;
    }
    return res + a;  /* Use modified a */
}

/* Function 3: Condition variable modified via pointer in then block */
int test1_pointer_modification(int x, int y) {
    int *ptr = &x;
    int result;
    
    if (x < 10) {
        result = y + 100;
        *ptr = 20;  /* Modifies x through pointer */
    } else {
        result = y - 50;
    }
    return result + x;
}

int main(int argc, char **argv) {
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 42;
    int sum = 0;
    
    /* Use volatile to prevent constant folding */
    volatile int v1 = seed;
    volatile int v2 = seed * 2;
    
    sum += test1_modify_condition_in_then(v1, v2);
    sum += test1_complex_modification(v1 + 1, v2 - 1);
    sum += test1_pointer_modification(v1 * 2, v2 / 2);
    
    printf("Test1 result: %d\n", sum);
    return sum;
}
