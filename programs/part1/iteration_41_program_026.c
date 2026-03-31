/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-increment/decrement optimization pass
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array for testing */
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
    volatile int result;  /* Prevent optimization */
    result = *p;          /* Simple register indirect */
    return result;
}

/* Test 3: Global array access via local pointer */
int test_global_access(void) {
    int *p = &global_arr[0];
    int sum = 0;
    
    /* Multiple simple register accesses */
    sum += p[0];          /* Simple register (p + 0) */
    sum += *p;            /* Another simple register indirect */
    sum += p[10];         /* Offset access */
    
    /* Loop with mixed addressing */
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            sum += *p;    /* Conditional simple access */
        }
        p++;
    }
    
    return sum;
}

/* Test 4: Struct access with simple pointer */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *data) {
    int sum = 0;
    int *ptr = data->values;
    
    /* Simple register access to struct member */
    sum += *ptr;                    /* Simple register */
    sum += ptr[0];                  /* Another simple (ptr + 0) */
    
    /* Loop with auto-increment */
    for (int i = 0; i < data->count && i < 20; i++) {
        sum += *ptr++;
    }
    
    /* One more simple access */
    sum += *(data->values);         /* Simple register from struct */
    
    return sum;
}

/* Test 5: Nested loops with conditional simple access */
int test_nested_loops(int *arr, int rows, int cols) {
    int sum = 0;
    
    for (int i = 0; i < rows; i++) {
        int *row_ptr = arr + i * cols;
        
        /* Simple register access at start of each row */
        sum += *row_ptr;            /* Simple register */
        
        for (int j = 0; j < cols; j++) {
            /* Mixed: sometimes simple, sometimes offset */
            if (j == 0) {
                sum += row_ptr[0];  /* Simple (row_ptr + 0) */
            } else {
                sum += row_ptr[j];  /* Offset access */
            }
        }
    }
    
    return sum;
}

/* Test 6: Pointer arithmetic that results in simple register */
int test_pointer_arithmetic(int *base, int offset) {
    int *p = base + offset;
    
    /* After arithmetic, simple register access */
    int val = *p;                   /* Should be simple register */
    
    /* Then use in loop */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += *p++;
    }
    
    return sum + val;
}

/* Main driver to ensure all code is executed */
int main(void) {
    int result = 0;
    
    /* Initialize test data */
    int local_arr[100];
    for (int i = 0; i < 100; i++) {
        local_arr[i] = i;
        global_arr[i] = i * 2;
    }
    
    struct Data data;
    for (int i = 0; i < 20; i++) {
        data.values[i] = i * 3;
    }
    data.count = 20;
    
    int matrix[5][10];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Run all tests */
    result += test_simple_reg_access(local_arr, 10);
    result += load_param(&local_arr[50]);
    result += test_global_access();
    result += test_struct_access(&data);
    result += test_nested_loops(&matrix[0][0], 5, 10);
    result += test_pointer_arithmetic(local_arr, 25);
    
    /* Use volatile to prevent dead code elimination */
    volatile int final_result = result;
    
    printf("Result: %d\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}
