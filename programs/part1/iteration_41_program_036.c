/* test_auto_inc_dec.c
 * Designed to trigger specific uncovered lines in GCC's auto-inc-dec.cc
 * Lines 1352-1358: Simple register addressing with zero offset
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array for testing global pointer access */
int global_arr[100];

/* Test 1: Simple parameter access with mixed addressing */
int test_mixed_addressing(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect - should trigger the uncovered block */
    sum += *base;                    /* Line 1: Simple register addressing */
    
    /* Register + constant offset */
    sum += base[5];                  /* Line 2: Offset addressing */
    
    /* Loop with pointer increment - encourages auto-inc optimization */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;               /* Line 3: Post-increment pattern */
    }
    
    /* Another simple register access after loop */
    sum += *base;                    /* Line 4: Simple register again */
    
    return sum;
}

/* Test 2: Function with only simple register addressing */
int load_simple(int *p) {
    volatile int result;  /* volatile prevents dead code elimination */
    result = *p;          /* Simple register indirect */
    return result;
}

/* Test 3: Global array access via local pointer */
int test_global_simple(void) {
    int *p = &global_arr[0];
    int sum = 0;
    
    /* Multiple simple register accesses */
    sum += p[0];          /* Equivalent to *p */
    sum += *p;            /* Direct simple register */
    
    /* Mix with offset */
    sum += p[10];
    
    return sum;
}

/* Test 4: Conditional simple access inside loop */
int test_conditional_simple(int *arr, int n) {
    int sum = 0;
    int *simple_ptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Complex addressing in loop */
        sum += arr[i * 2];
        
        /* Conditional simple register access */
        if (i % 3 == 0) {
            sum += *simple_ptr;      /* Simple register in conditional */
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

/* Test 5: Struct access with simple pointer */
struct Data {
    int values[20];
    int count;
};

int test_struct_simple(struct Data *d) {
    int sum = 0;
    int *p = d->values;
    
    /* Simple register access to struct member */
    sum += *p;                       /* Simple register */
    sum += p[0];                     /* Also simple register */
    
    /* Offset access */
    sum += p[5];
    
    /* Loop with mixed access */
    for (int i = 0; i < 10; i++) {
        sum += *p++;
    }
    
    return sum;
}

/* Test 6: Multiple simple pointers */
int test_multiple_pointers(int *a, int *b, int *c) {
    int sum = 0;
    
    /* Three different simple register accesses */
    sum += *a;
    sum += *b;
    sum += *c;
    
    /* Mix with offsets */
    sum += a[3];
    sum += b[1];
    sum += c[4];
    
    return sum;
}

/* Test 7: Nested simple accesses */
int test_nested_simple(int **ptr_array, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        int *p = ptr_array[i];
        if (p) {
            sum += *p;               /* Simple register from loaded pointer */
            sum += p[0];             /* Another simple register */
        }
    }
    
    return sum;
}

/* Main driver to ensure all code is executed */
int main(void) {
    int result = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_arr[i] = i * 2;
    }
    
    /* Test array */
    int test_arr[50];
    for (int i = 0; i < 50; i++) {
        test_arr[i] = i + 1;
    }
    
    /* Initialize struct */
    struct Data data;
    for (int i = 0; i < 20; i++) {
        data.values[i] = i * 3;
    }
    data.count = 20;
    
    /* Array of pointers for test 7 */
    int *ptr_array[5];
    for (int i = 0; i < 5; i++) {
        ptr_array[i] = &test_arr[i * 5];
    }
    
    /* Run all tests */
    result += test_mixed_addressing(test_arr, 10);
    result += load_simple(test_arr);
    result += test_global_simple();
    result += test_conditional_simple(test_arr, 15);
    result += test_struct_simple(&data);
    result += test_multiple_pointers(&test_arr[0], &test_arr[10], &test_arr[20]);
    result += test_nested_simple(ptr_array, 5);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
