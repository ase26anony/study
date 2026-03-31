/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-increment/decrement optimization pass
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array for testing */
int global_arr[100] = {0};

/* Test 1: Simple parameter access with mixed addressing */
int test_simple_mixed(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect - should trigger the uncovered block */
    sum += *base;                    /* Line 1: Simple register addressing */
    
    /* Register + constant offset */
    sum += base[5];                  /* Line 2: Offset addressing */
    
    /* Loop with pointer increment */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;               /* Line 3: Auto-increment pattern */
    }
    
    /* Another simple register access */
    int *simple_ptr = base + 10;
    sum += *simple_ptr;              /* Line 4: Another simple register */
    
    return sum;
}

/* Test 2: Global array access via local pointer */
int test_global_access(void) {
    int sum = 0;
    int *p = &global_arr[0];
    
    /* Multiple simple register accesses */
    sum += p[0];                     /* Line 5: p[0] is simple register */
    sum += *p;                       /* Line 6: *p is simple register */
    
    /* Mixed with offset */
    sum += p[10];
    
    /* Loop with conditional simple access */
    for (int i = 0; i < 20; i++) {
        if (i % 3 == 0) {
            sum += *p;               /* Line 7: Simple access in loop */
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
    
    /* Access struct member via pointer */
    sum += data->count;              /* Line 8: Simple register for struct member */
    
    /* Array within struct */
    int *vals = data->values;
    sum += vals[0];                  /* Line 9: Simple register */
    
    /* Loop through struct array */
    for (int i = 0; i < 10; i++) {
        sum += *vals;                /* Line 10: Simple in loop */
        vals++;
    }
    
    return sum;
}

/* Test 4: Multiple pointer variables */
int test_multiple_pointers(int *arr1, int *arr2, int n) {
    int sum = 0;
    int *p1 = arr1;
    int *p2 = arr2;
    
    /* Simple accesses from different pointers */
    sum += *p1;                      /* Line 11: Simple from p1 */
    sum += *p2;                      /* Line 12: Simple from p2 */
    
    /* Mixed patterns */
    for (int i = 0; i < n; i++) {
        sum += p1[0];                /* Line 13: p1[0] in loop */
        sum += *p2;                  /* Line 14: *p2 in loop */
        
        if (i % 2 == 0) {
            sum += *p1;              /* Line 15: Conditional simple */
        }
        
        p1++;
        p2 += 2;
    }
    
    return sum;
}

/* Test 5: Volatile pointer to prevent optimization */
int test_volatile_access(volatile int *base) {
    int sum = 0;
    
    /* Volatile simple access */
    sum += *base;                    /* Line 16: Volatile simple register */
    
    /* Multiple volatile accesses */
    volatile int *vptr = base;
    for (int i = 0; i < 5; i++) {
        sum += *vptr;                /* Line 17: Volatile in loop */
        vptr++;
    }
    
    return sum;
}

/* Test 6: Nested loops with simple access */
int test_nested_loops(int *matrix, int rows, int cols) {
    int sum = 0;
    
    for (int i = 0; i < rows; i++) {
        int *row_ptr = matrix + i * cols;
        
        /* Simple access at start of each row */
        sum += *row_ptr;             /* Line 18: Simple per row */
        
        for (int j = 0; j < cols; j++) {
            sum += row_ptr[0];       /* Line 19: Simple in inner loop */
            row_ptr++;
        }
    }
    
    return sum;
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
    data.count = 42;
    
    int arr2[50];
    for (int i = 0; i < 50; i++) {
        arr2[i] = i * 4;
    }
    
    int matrix[25];
    for (int i = 0; i < 25; i++) {
        matrix[i] = i * 5;
    }
    
    /* Execute all tests */
    result += test_simple_mixed(local_arr, 10);
    result += test_global_access();
    result += test_struct_access(&data);
    result += test_multiple_pointers(local_arr, arr2, 5);
    result += test_volatile_access(local_arr);
    result += test_nested_loops(matrix, 5, 5);
    
    /* Print result to prevent dead code elimination */
    printf("Final checksum: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
