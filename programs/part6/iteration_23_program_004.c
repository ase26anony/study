/* Test suite for if-conversion coverage targeting modified_in_p detection */
#include <stdio.h>
#include <stdlib.h>

/* Global variable for condition modification tests */
volatile int global_cond = 0;

/* Variant 1: Integer condition modified in then block */
int test_int_condition_modified(int x, int y) {
    int result;
    
    /* Condition variable x gets modified in then block */
    if (x > 0) {
        result = y * 2;
        x = 5;  /* This modifies the condition variable */
    } else {
        result = y / 2;
    }
    
    /* Use modified x to prevent dead store elimination */
    return result + x;
}

/* Variant 2: Pointer condition modified in then block */
int test_pointer_condition_modified(int *ptr, int threshold) {
    int result;
    
    /* Condition based on pointer comparison */
    if (ptr != NULL) {
        result = *ptr * 3;
        ptr++;  /* Modify the pointer used in condition */
    } else {
        result = threshold * 4;
    }
    
    /* Use ptr to prevent optimization */
    return result + (ptr != NULL ? 1 : 0);
}

/* Variant 3: Global variable condition modified in then block */
int test_global_condition_modified(int value) {
    int result;
    
    /* Condition uses global variable */
    if (global_cond > 10) {
        result = value * 5;
        global_cond = 0;  /* Modify global condition variable */
    } else {
        result = value + 10;
    }
    
    return result;
}

/* Variant 4: Multiple modifications in then block */
int test_multiple_modifications(int a, int b) {
    int result;
    
    /* a is used in condition and modified multiple times */
    if (a < b) {
        result = b - a;
        a = a + 1;      /* First modification */
        a = a * 2;      /* Second modification */
    } else {
        result = a - b;
    }
    
    /* Use modified a */
    return result + a;
}

/* Variant 5: Condition variable modified through pointer */
int test_indirect_modification(int x, int *px) {
    int result;
    
    /* x is used in condition, *px aliases x */
    if (x > 100) {
        result = 50;
        *px = 0;  /* This might modify x if px points to x */
    } else {
        result = 100;
    }
    
    return result + x;
}

/* Variant 6: Nested condition with modification */
int test_nested_modification(int x, int y, int z) {
    int result;
    
    if (x > y) {
        if (y > z) {
            result = x + y + z;
            x = z;  /* Modify outer condition variable */
        } else {
            result = x - y - z;
        }
    } else {
        result = y - x;
    }
    
    return result;
}

/* Main driver that uses all test functions */
int main(int argc, char *argv[]) {
    int seed = 0;
    int total = 0;
    
    /* Use command line argument or default for variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    } else {
        /* Use volatile to prevent constant folding */
        volatile int v = 42;
        seed = v;
    }
    
    /* Test 1: Simple integer modification */
    int x1 = seed % 100;
    int y1 = (seed + 1) % 100;
    total += test_int_condition_modified(x1, y1);
    
    /* Test 2: Pointer modification */
    int arr[2] = {seed, seed + 1};
    int *ptr = (seed % 3 == 0) ? NULL : arr;
    total += test_pointer_condition_modified(ptr, seed);
    
    /* Test 3: Global variable modification */
    global_cond = seed % 20;
    total += test_global_condition_modified(seed);
    
    /* Test 4: Multiple modifications */
    int a4 = seed % 50;
    int b4 = (seed + 10) % 50;
    total += test_multiple_modifications(a4, b4);
    
    /* Test 5: Indirect modification */
    int x5 = seed % 200;
    total += test_indirect_modification(x5, &x5);  /* Self-pointer */
    
    /* Test 6: Nested modification */
    int x6 = seed % 30;
    int y6 = (seed + 5) % 30;
    int z6 = (seed + 10) % 30;
    total += test_nested_modification(x6, y6, z6);
    
    /* Print result to prevent optimization */
    printf("Total: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
