/* test-auto-inc-dec.c
 * 
 * This program is designed to trigger the specific uncovered block in
 * GCC's auto-inc-dec.cc (lines 1352-1358) by creating memory access
 * patterns that cause the compiler to analyze simple register addressing
 * modes during RTL optimization.
 *
 * Compile with: gcc -O2 -fno-inline -fno-ipa-pure-const test-auto-inc-dec.c -o test
 * Or with: gcc -O3 -fno-inline -funroll-loops test-auto-inc-dec.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array to provide another context for pointer access */
int global_arr[100] = {0};

/* Function 1: Simple parameter access with mixed addressing patterns
 * This creates a scenario where the compiler sees:
 * - Simple register indirect: *base
 * - Register + constant offset: base[5]
 * - Loop with pointer increment
 */
int test_mixed_addressing(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect access - target for uncovered block */
    sum += *base;
    
    /* Register + constant offset access */
    sum += base[5];
    
    /* Loop with pointer increment - encourages auto-inc optimization */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    
    /* Another simple register access in the middle */
    if (n > 0) {
        sum += *base;
    }
    
    return sum;
}

/* Function 2: Direct parameter load - simplest case
 * Just a simple register indirect access from a parameter
 */
int load_param(int *p) {
    volatile int result = *p;  /* volatile prevents elimination */
    return result;
}

/* Function 3: Global array access via local pointer
 * Creates simple register addressing from a pointer to global
 */
int test_global_access(void) {
    int *p = &global_arr[0];
    int sum = 0;
    
    /* Simple register indirect from global pointer */
    sum += p[0];
    
    /* Mixed with offset access */
    sum += p[10];
    
    /* Loop through global array */
    for (int i = 0; i < 50; i++) {
        sum += *p++;
    }
    
    return sum;
}

/* Function 4: Conditional simple access inside loop
 * The compiler may analyze the simple access in different contexts
 */
int test_conditional_access(int *arr, int n) {
    int sum = 0;
    int *simple_ptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Complex addressing in loop */
        sum += arr[i];
        
        /* Conditional simple register access */
        if (i % 3 == 0) {
            sum += *simple_ptr;  /* Simple register indirect */
        }
        
        /* Sometimes use offset */
        if (i % 5 == 0) {
            sum += simple_ptr[2];
        }
    }
    
    /* Final simple access */
    sum += *arr;
    
    return sum;
}

/* Function 5: Struct access with pointer
 * Different memory access pattern that might trigger the analysis
 */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *d) {
    int sum = 0;
    int *p = d->values;
    
    /* Simple register indirect from struct member pointer */
    sum += *p;
    
    /* Loop through struct array */
    for (int i = 0; i < d->count && i < 20; i++) {
        sum += *p++;
    }
    
    /* Another simple access at the end */
    sum += *(d->values);
    
    return sum;
}

/* Function 6: Multiple simple accesses in sequence
 * Increases chance compiler sees the pattern
 */
int test_multiple_simple(int *a, int *b, int *c) {
    int sum = 0;
    
    /* Three consecutive simple register indirect accesses */
    sum += *a;
    sum += *b;
    sum += *c;
    
    /* Mixed with offset */
    sum += a[1];
    sum += b[2];
    
    return sum;
}

/* Main driver to ensure all functions are called
 * and results are used to prevent optimization
 */
int main(void) {
    int result = 0;
    
    /* Initialize test data */
    int local_arr[100];
    for (int i = 0; i < 100; i++) {
        local_arr[i] = i + 1;
        global_arr[i] = i * 2;
    }
    
    /* Initialize struct */
    struct Data data;
    for (int i = 0; i < 20; i++) {
        data.values[i] = i * 3;
    }
    data.count = 20;
    
    /* Call all test functions with various parameters */
    result += test_mixed_addressing(local_arr, 50);
    result += load_param(&local_arr[10]);
    result += test_global_access();
    result += test_conditional_access(local_arr, 30);
    result += test_struct_access(&data);
    
    /* Multiple simple accesses with different pointers */
    result += test_multiple_simple(&local_arr[0], &local_arr[20], &local_arr[40]);
    
    /* Additional loop to increase optimization opportunities */
    for (int iter = 0; iter < 10; iter++) {
        int *p = local_arr;
        for (int i = 0; i < 10; i++) {
            /* Mix of simple and indexed access in tight loop */
            result += *p;          /* Simple register indirect */
            result += p[i];        /* Register + offset */
            p++;
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
