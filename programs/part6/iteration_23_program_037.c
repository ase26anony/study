/* test1.c - Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Simple integer condition modified in then block */
int test_modify_in_then(int x, int y) {
    int result;
    
    /* Condition variable x gets modified in the then block */
    if (x > 0) {
        result = y * 2;
        x = 5;  /* This modifies the condition variable */
    } else {
        result = y / 2;
    }
    
    return result + x;  /* Use x to prevent dead store elimination */
}

/* Function 2: Multiple modifications in then block */
int test_multiple_modifications(int a, int b) {
    int res;
    
    /* a is used in condition and modified in then block */
    if (a != b) {
        res = b + 10;
        a = b;      /* First modification */
        a += 1;     /* Second modification */
    } else {
        res = b - 10;
    }
    
    return res * a;  /* Use modified a */
}

/* Function 3: Condition variable modified with different operations */
int test_compound_modify(int x, int y, int z) {
    int output;
    
    /* x is condition variable */
    if (x < y + z) {
        output = y * z;
        x++;        /* Modify with increment */
        x *= 2;     /* Modify again */
    } else {
        output = y + z;
    }
    
    return output + x;
}

int main(int argc, char **argv) {
    volatile int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    int result = 0;
    
    /* Call all test functions with input-dependent values */
    result += test_modify_in_then(seed, seed + 1);
    result += test_multiple_modifications(seed * 2, seed + 2);
    result += test_compound_modify(seed - 1, seed, seed + 1);
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
