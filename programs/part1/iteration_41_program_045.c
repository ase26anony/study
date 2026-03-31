/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-increment/decrement optimization
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array for testing global pointer access */
int global_arr[100] = {0};

/* Test 1: Simple parameter access with mixed addressing */
int test_simple_param(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect access - should trigger the uncovered block */
    sum += *base;                     /* Line 1: Simple register addressing */
    
    /* Register + constant offset */
    sum += base[5];                   /* Line 2: Offset addressing */
    
    /* Loop with pointer increment */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;                /* Line 3: Auto-increment pattern */
    }
    
    /* Another simple register access */
    int *simple_ptr = base + 10;
    sum += *simple_ptr;               /* Line 4: Another simple register */
    
    return sum;
}

/* Test 2: Global array access via local pointer */
int test_global_access(void) {
    int sum = 0;
    int *p = &global_arr[0];
    
    /* Multiple simple register accesses */
    sum += p[0];                      /* Line 5: p[0] is simple register */
    sum += *p;                        /* Line 6: *p is also simple register */
    
    /* Mixed with offset */
    sum += p[10];
    sum += p[20];
    
    /* Loop with conditional simple access */
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            sum += *p;                /* Line 7: Simple access in loop */
        }
        p++;
    }
    
    return sum;
}

/* Test 3: Struct access with pointer */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *data) {
    int sum = 0;
    int *ptr = data->values;
    
    /* Simple register access to struct member */
    sum += ptr[0];                    /* Line 8: Simple struct member access */
    
    /* Access through different pointer */
    int *base = ptr;
    sum += *base;                     /* Line 9: Another simple register */
    
    /* Loop with mixed addressing */
    for (int i = 0; i < data->count && i < 20; i++) {
        if (i == 0) {
            sum += *ptr;              /* Line 10: Simple access at loop start */
        } else {
            sum += ptr[i];            /* Offset access */
        }
    }
    
    return sum;
}

/* Test 4: Volatile pointer to prevent optimization */
int test_volatile_access(int *arr, int n) {
    volatile int *volatile_ptr = arr;
    int sum = 0;
    
    /* Simple access through volatile pointer */
    sum += *volatile_ptr;             /* Line 11: Volatile simple access */
    
    /* Normal pointer derived from volatile */
    int *normal_ptr = (int *)volatile_ptr;
    sum += normal_ptr[0];             /* Line 12: Simple access */
    
    /* Loop with increment */
    for (int i = 0; i < n; i++) {
        sum += *normal_ptr++;
    }
    
    return sum;
}

/* Test 5: Multiple simple accesses in sequence */
int test_multiple_simple(int *p1, int *p2, int *p3) {
    int sum = 0;
    
    /* Three consecutive simple register accesses */
    sum += *p1;                       /* Line 13: First simple */
    sum += *p2;                       /* Line 14: Second simple */
    sum += *p3;                       /* Line 15: Third simple */
    
    /* Add some offsets to create variety */
    sum += p1[5];
    sum += p2[3];
    sum += p3[7];
    
    return sum;
}

/* Test 6: Nested loops with simple access */
int test_nested_loops(int *base, int rows, int cols) {
    int sum = 0;
    
    for (int i = 0; i < rows; i++) {
        int *row_ptr = base + i * cols;
        
        /* Simple access at start of each row */
        sum += *row_ptr;              /* Line 16: Simple per row */
        
        for (int j = 0; j < cols; j++) {
            if (j == 0) {
                sum += row_ptr[0];    /* Line 17: Simple in inner loop */
            }
            sum += row_ptr[j];
        }
    }
    
    return sum;
}

/* Main driver to ensure all code is executed */
int main(void) {
    int result = 0;
    
    /* Initialize test data */
    int test_arr[100];
    for (int i = 0; i < 100; i++) {
        test_arr[i] = i + 1;
        global_arr[i] = i * 2;
    }
    
    struct Data data;
    for (int i = 0; i < 20; i++) {
        data.values[i] = i * 3;
    }
    data.count = 20;
    
    /* Run all tests */
    result += test_simple_param(test_arr, 10);
    result += test_global_access();
    result += test_struct_access(&data);
    result += test_volatile_access(test_arr, 5);
    
    int *p1 = test_arr;
    int *p2 = test_arr + 20;
    int *p3 = test_arr + 40;
    result += test_multiple_simple(p1, p2, p3);
    
    int matrix[5][5];
    int counter = 0;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            matrix[i][j] = counter++;
        }
    }
    result += test_nested_loops(&matrix[0][0], 5, 5);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Also use result in a way that can't be optimized out */
    if (result > 0) {
        return 0;
    } else {
        return 1;
    }
}
