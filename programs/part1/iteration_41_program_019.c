/* test_auto_inc_dec.c
 * Designed to trigger specific uncovered lines in GCC's auto-inc-dec.cc
 * Lines 1352-1358: Simple register addressing with zero offset
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array for testing */
int global_arr[100] = {0};

/* Test 1: Simple parameter access with mixed addressing */
int test_mixed_addressing(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect - should trigger the uncovered block */
    sum += *base;                     // Line 1: Simple register addressing
    
    /* Register + constant offset */
    sum += base[5];                   // Line 2: Offset addressing
    
    /* Loop with pointer increment - encourages auto-inc optimization */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;                // Line 3: Auto-increment pattern
    }
    
    /* Another simple register access after loop */
    int *simple_ptr = base + 10;
    sum += *simple_ptr;               // Line 4: Another simple register
    
    return sum;
}

/* Test 2: Direct parameter access */
int load_param_simple(int *p) {
    volatile int *vp = p;             // Prevent optimization
    return *vp;                       // Simple register indirect
}

/* Test 3: Global array access via local pointer */
int test_global_access(void) {
    int *p = &global_arr[0];
    int sum = 0;
    
    /* Multiple simple register accesses */
    sum += p[0];                      // Simple register (p[0] == *p)
    sum += *p;                        // Explicit dereference
    
    /* Mixed with offset */
    sum += p[10];
    
    return sum;
}

/* Test 4: Conditional simple access in loop */
int test_conditional_access(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Complex pattern to force analysis */
        if (i % 3 == 0) {
            sum += *ptr;              // Simple register in condition
        } else if (i % 3 == 1) {
            sum += ptr[2];            // Offset in condition
        } else {
            sum += *ptr++;            // Auto-increment
        }
        
        /* Additional simple access */
        if (i == n/2) {
            int *temp = ptr;
            sum += *temp;             // Another simple register
        }
    }
    
    return sum;
}

/* Test 5: Struct access with simple pointer */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *d) {
    int sum = 0;
    int *p = d->values;
    
    /* Simple register access to struct member */
    sum += *p;                        // Simple register
    
    /* Loop through struct array */
    for (int i = 0; i < 10; i++) {
        sum += *p++;
    }
    
    /* Access via different simple pointer */
    int *q = &d->values[5];
    sum += *q;                        // Another simple register
    
    return sum + d->count;            // Mix with direct struct access
}

/* Test 6: Multiple simple pointers in same function */
int test_multiple_pointers(int *a, int *b, int *c) {
    int sum = 0;
    
    /* Three different simple register accesses */
    sum += *a;
    sum += *b;
    sum += *c;
    
    /* Mixed with offsets */
    sum += a[3];
    sum += b[2];
    
    /* Small loop */
    for (int i = 0; i < 4; i++) {
        sum += *c++;
    }
    
    return sum;
}

/* Test 7: Nested simple accesses */
int test_nested_access(int **ptr_array, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        int *p = ptr_array[i];
        if (p) {
            sum += *p;                // Simple register from array element
            sum += p[0];              // Same access, different syntax
        }
    }
    
    return sum;
}

/* Main driver to ensure all code is executed */
int main(void) {
    int result = 0;
    
    /* Initialize test data */
    int test_arr[50];
    for (int i = 0; i < 50; i++) {
        test_arr[i] = i * 2 + 1;
    }
    
    for (int i = 0; i < 100; i++) {
        global_arr[i] = i;
    }
    
    struct Data data;
    for (int i = 0; i < 20; i++) {
        data.values[i] = i * 3;
    }
    data.count = 42;
    
    int *ptr_array[5];
    for (int i = 0; i < 5; i++) {
        ptr_array[i] = &test_arr[i * 10];
    }
    
    /* Run all tests */
    result += test_mixed_addressing(test_arr, 10);
    result += load_param_simple(test_arr);
    result += test_global_access();
    result += test_conditional_access(test_arr, 15);
    result += test_struct_access(&data);
    
    int *b = &test_arr[10];
    int *c = &test_arr[20];
    result += test_multiple_pointers(test_arr, b, c);
    
    result += test_nested_access(ptr_array, 5);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
