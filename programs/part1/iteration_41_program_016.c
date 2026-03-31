/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-increment/decrement optimization
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array for testing global pointer access */
int global_arr[100];

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
    
    /* Another simple register access after loop */
    sum += *base;                     /* Line 4: Another simple register access */
    
    return sum;
}

/* Test 2: Local array with pointer */
int test_local_array(int n) {
    int local_arr[50];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 50; i++) {
        local_arr[i] = i * 2;
    }
    
    int *p = &local_arr[0];
    
    /* Mixed access patterns */
    sum += p[0];                      /* Simple register + zero offset */
    sum += p[10];                     /* Register + constant offset */
    
    /* Loop with conditional simple access */
    for (int i = 0; i < n && i < 50; i++) {
        if (i % 3 == 0) {
            sum += *p;                /* Conditional simple register access */
        }
        p++;
    }
    
    return sum;
}

/* Test 3: Global array access */
int test_global_access(void) {
    int *p = &global_arr[0];
    int sum = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_arr[i] = i * 3;
    }
    
    /* Multiple simple register accesses */
    sum += *p;                        /* Simple register indirect */
    sum += p[0];                      /* Another way to write the same */
    
    /* Mixed with offset accesses */
    sum += p[25];
    sum += p[50];
    
    /* Loop with pointer arithmetic */
    for (int i = 0; i < 20; i++) {
        sum += *p;                    /* Repeated simple access */
        p += 2;                       /* Step by 2 */
    }
    
    return sum;
}

/* Test 4: Struct access with pointers */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *data) {
    int sum = 0;
    int *ptr = data->values;
    
    /* Simple access through struct pointer */
    sum += *ptr;                      /* Simple register addressing */
    
    /* Access through array with zero index */
    sum += ptr[0];                    /* Another simple register access */
    
    /* Loop through struct array */
    for (int i = 0; i < data->count && i < 20; i++) {
        sum += *ptr;                  /* Simple access in loop */
        ptr++;
    }
    
    return sum;
}

/* Test 5: Volatile pointer to prevent optimization */
int test_volatile_access(volatile int *base, int n) {
    int sum = 0;
    
    /* Volatile ensures the access happens */
    sum += *base;                     /* Simple volatile register access */
    
    /* Mix volatile and non-volatile */
    int *normal_ptr = (int *)base;
    sum += normal_ptr[0];             /* Cast away volatile for simple access */
    
    /* Loop with mixed accesses */
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            sum += *base;             /* Volatile simple access */
        } else {
            sum += normal_ptr[i];     /* Offset access */
        }
    }
    
    return sum;
}

/* Test 6: Function with only simple register access */
int load_param(int *p) {
    return *p;                        /* Pure simple register access */
}

/* Test 7: Nested loops with different patterns */
int test_nested_patterns(int *base, int rows, int cols) {
    int sum = 0;
    
    for (int i = 0; i < rows; i++) {
        int *row_ptr = base + i * cols;
        
        /* Simple access at start of each row */
        sum += *row_ptr;              /* Simple register access per row */
        
        for (int j = 0; j < cols; j++) {
            /* Mix of simple and offset accesses */
            if (j == 0) {
                sum += row_ptr[0];    /* Simple access with zero offset */
            } else {
                sum += row_ptr[j];    /* Offset access */
            }
        }
    }
    
    return sum;
}

/* Main driver function */
int main(void) {
    int test_array[100];
    int total_sum = 0;
    
    /* Initialize test data */
    for (int i = 0; i < 100; i++) {
        test_array[i] = i + 1;
    }
    
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_arr[i] = i * 2;
    }
    
    /* Initialize struct data */
    struct Data data;
    for (int i = 0; i < 20; i++) {
        data.values[i] = i * 5;
    }
    data.count = 20;
    
    /* Run all tests to generate various addressing patterns */
    total_sum += test_simple_param(test_array, 10);
    total_sum += test_local_array(15);
    total_sum += test_global_access();
    total_sum += test_struct_access(&data);
    total_sum += test_volatile_access(test_array, 5);
    total_sum += load_param(test_array);
    total_sum += test_nested_patterns(test_array, 5, 10);
    
    /* Add more calls with different parameters */
    for (int i = 0; i < 5; i++) {
        total_sum += test_simple_param(&test_array[i * 10], 3);
    }
    
    printf("Total checksum: %d\n", total_sum);
    
    /* Use the result to prevent dead code elimination */
    if (total_sum > 0) {
        return 0;
    } else {
        return 1;
    }
}
