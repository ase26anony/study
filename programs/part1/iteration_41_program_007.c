/* test_auto_inc_dec.c
 * Designed to trigger specific uncovered lines in GCC's auto-inc-dec.cc
 * Lines 1352-1358: Simple register addressing with zero offset
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array for testing */
int global_arr[100] = {0};

/* Test 1: Simple parameter access with mixed addressing */
int test_simple_param(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect - should trigger the uncovered block */
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
    int values[50];
    int count;
};

int test_struct_access(struct Data *data) {
    int sum = 0;
    int *ptr = data->values;
    
    /* Simple register access to struct member */
    sum += data->count;               /* Line 8: Might be simple register */
    
    /* Access via pointer */
    sum += *ptr;                      /* Line 9: Simple register indirect */
    
    /* Loop through array */
    for (int i = 0; i < 25; i++) {
        sum += *ptr++;
    }
    
    /* Final simple access */
    ptr = &data->values[10];
    sum += *ptr;                      /* Line 10: Another simple register */
    
    return sum;
}

/* Test 4: Complex function with multiple patterns */
int test_complex_patterns(int *arr, int size) {
    int sum = 0;
    volatile int *volatile_ptr = arr;  /* Prevent optimization */
    
    /* Pattern 1: Series of simple accesses */
    sum += *volatile_ptr;
    sum += *volatile_ptr;
    sum += *volatile_ptr;              /* Repeated simple access */
    
    /* Pattern 2: Offset accesses */
    for (int i = 0; i < 5; i++) {
        sum += volatile_ptr[i];
    }
    
    /* Pattern 3: Pointer arithmetic in loop */
    int *p = arr;
    for (int i = 0; i < size; i++) {
        if (i == 0) {
            sum += *p;                 /* Simple access at loop start */
        }
        sum += *p++;
    }
    
    /* Pattern 4: Multiple base pointers */
    int *p1 = arr;
    int *p2 = arr + 20;
    sum += *p1 + *p2;                  /* Two simple register accesses */
    
    return sum;
}

/* Test 5: Nested loops with different addressing */
int test_nested_loops(int *matrix, int rows, int cols) {
    int sum = 0;
    
    for (int i = 0; i < rows; i++) {
        int *row_ptr = matrix + i * cols;
        
        /* Simple access to first element */
        sum += *row_ptr;               /* Simple register access */
        
        /* Loop through row */
        for (int j = 0; j < cols; j++) {
            sum += *row_ptr++;
        }
        
        /* Reset and access middle element */
        row_ptr = matrix + i * cols + cols/2;
        sum += *row_ptr;               /* Another simple access */
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
    for (int i = 0; i < 50; i++) {
        data.values[i] = i * 3;
    }
    data.count = 50;
    
    /* Run all tests */
    result += test_simple_param(test_arr, 20);
    result += test_global_access();
    result += test_struct_access(&data);
    result += test_complex_patterns(test_arr, 30);
    
    /* Create a small matrix */
    int matrix[3][4];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            matrix[i][j] = i * 4 + j;
        }
    }
    
    result += test_nested_loops(&matrix[0][0], 3, 4);
    
    /* Print result to prevent dead code elimination */
    printf("Final checksum: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
