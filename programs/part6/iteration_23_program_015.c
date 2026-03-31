/* Test 1: Integer condition modified in then block */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Direct integer modification */
int test_int_modify_then(int x, int y) {
    int result;
    /* This should trigger modified_in_p check */
    if (x > 0) {
        result = y * 2;
        x = 5;  /* Modifies condition variable in then block */
    } else {
        result = y / 2;
    }
    return result + x;  /* Use both to prevent optimization */
}

/* Function 2: Multiple modifications in then block */
int test_multiple_modifications(int a, int b) {
    int res;
    volatile int cond = a;  /* Use volatile to prevent optimization */
    
    if (cond > b) {
        res = a + b;
        cond = b;      /* First modification */
        cond++;        /* Second modification */
    } else {
        res = a - b;
    }
    return res + cond;
}

/* Function 3: Pointer condition with modification */
int test_pointer_modify(int *ptr, int threshold) {
    int value;
    
    if (ptr != NULL) {
        value = *ptr * 3;
        ptr = NULL;  /* Modifies pointer used in condition */
    } else {
        value = threshold * 2;
    }
    return value + (ptr != NULL);
}

/* Function 4: Global variable modification */
static int global_cond = 0;

int test_global_modify(int x) {
    int result;
    
    if (global_cond < x) {
        result = x * 10;
        global_cond = x;  /* Modifies global used in condition */
    } else {
        result = x * 5;
    }
    return result;
}

/* Function 5: Complex expression modification */
int test_complex_expr_modify(int a, int b, int c) {
    int temp = a + b;
    int result;
    
    if (temp > c) {
        result = a * b;
        temp = c - a;  /* Modifies variable in condition */
        result += temp;
    } else {
        result = c * 2;
    }
    return result;
}

/* Main function to drive tests */
int main(int argc, char *argv[]) {
    int seed = 0;
    int result = 0;
    
    /* Use argv to prevent constant folding */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Test 1: Integer modification */
    result += test_int_modify_then(seed, seed + 10);
    
    /* Test 2: Multiple modifications */
    result += test_multiple_modifications(seed, seed - 5);
    
    /* Test 3: Pointer modification */
    int local_var = seed * 2;
    result += test_pointer_modify(&local_var, seed);
    
    /* Test 4: Global modification */
    global_cond = seed / 2;
    result += test_global_modify(seed);
    
    /* Test 5: Complex expression */
    result += test_complex_expr_modify(seed, seed + 1, seed + 2);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
