/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-increment/decrement optimization pass
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array to enable different access patterns */
int global_arr[100] = {0};

/* Test 1: Simple register indirect access with mixed patterns */
int test_simple_reg_access(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect - should trigger the uncovered block */
    sum += *base;                    /* Line 1: Simple register address */
    
    /* Register + constant offset */
    sum += base[5];                  /* Line 2: Offset access */
    
    /* Loop with pointer increment - encourages auto-inc optimization */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;               /* Line 3: Auto-increment pattern */
    }
    
    /* Another simple register access after loop */
    sum += *base;                    /* Line 4: Another simple register */
    
    return sum;
}

/* Test 2: Function parameter used directly */
int load_param(int *p) {
    /* Multiple simple register accesses */
    int val1 = *p;                   /* Simple register indirect */
    int val2 = *p;                   /* Another simple access */
    return val1 + val2;
}

/* Test 3: Global array access via local pointer */
int test_global_access(void) {
    int *p = &global_arr[0];
    int sum = 0;
    
    /* Simple register access to global */
    sum += *p;                       /* Simple register indirect */
    
    /* Offset access */
    sum += p[10];
    
    /* Another simple access */
    sum += *p;
    
    return sum;
}

/* Test 4: Conditional simple access inside loop */
int test_conditional_access(int *base, int n, int threshold) {
    int sum = 0;
    int *simple_ptr = base;
    
    for (int i = 0; i < n; i++) {
        /* Mixed access patterns */
        sum += base[i];              /* Indexed access */
        
        if (i > threshold) {
            /* Conditional simple register access */
            sum += *simple_ptr;      /* Should trigger uncovered block */
        }
        
        /* Pointer arithmetic */
        simple_ptr++;
    }
    
    /* Final simple access */
    sum += *base;
    
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
    sum += *p;                       /* Simple register indirect */
    
    /* Access through pointer with offset */
    sum += p[5];
    
    /* Another simple access */
    sum += *p;
    
    /* Access different struct member */
    sum += d->count;
    
    return sum;
}

/* Test 6: Multiple simple accesses in sequence */
int test_multiple_simple(int *p1, int *p2, int *p3) {
    /* Sequence of simple register accesses */
    int a = *p1;                     /* Simple access 1 */
    int b = *p2;                     /* Simple access 2 */
    int c = *p3;                     /* Simple access 3 */
    
    /* Mix with offset access */
    a += p1[2];
    b += p2[3];
    
    /* More simple accesses */
    c += *p3;
    
    return a + b + c;
}

/* Test 7: Nested loops with simple access */
int test_nested_loops(int *base, int rows, int cols) {
    int sum = 0;
    
    for (int i = 0; i < rows; i++) {
        int *row_ptr = base + i * cols;
        
        for (int j = 0; j < cols; j++) {
            /* Mix of access patterns */
            if (j == 0) {
                sum += *row_ptr;     /* Simple access at start of row */
            } else {
                sum += row_ptr[j];   /* Offset access */
            }
        }
        
        /* Simple access between rows */
        sum += *base;
    }
    
    return sum;
}

/* Test 8: Volatile pointer to prevent optimization */
int test_volatile_access(volatile int *vp) {
    int sum = 0;
    
    /* Simple access through volatile pointer */
    sum += *vp;                      /* Should generate memory operand */
    
    /* Multiple accesses */
    sum += *vp;
    sum += *vp;
    
    return sum;
}

/* Main driver to ensure all code is executed */
int main(void) {
    int result = 0;
    
    /* Initialize test data */
    int local_arr[50];
    for (int i = 0; i < 50; i++) {
        local_arr[i] = i * 2;
    }
    
    for (int i = 0; i < 100; i++) {
        global_arr[i] = i * 3;
    }
    
    struct Data data;
    for (int i = 0; i < 20; i++) {
        data.values[i] = i * 5;
    }
    data.count = 100;
    
    volatile int volatile_val = 42;
    
    /* Execute all tests with different patterns */
    result += test_simple_reg_access(local_arr, 10);
    result += load_param(&local_arr[10]);
    result += test_global_access();
    result += test_conditional_access(local_arr, 15, 5);
    result += test_struct_access(&data);
    
    int *p1 = &local_arr[0];
    int *p2 = &local_arr[10];
    int *p3 = &local_arr[20];
    result += test_multiple_simple(p1, p2, p3);
    
    result += test_nested_loops(local_arr, 3, 5);
    result += test_volatile_access(&volatile_val);
    
    /* Print result to prevent dead code elimination */
    printf("Final checksum: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
