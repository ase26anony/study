/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-increment/decrement optimization
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array for testing */
int global_arr[100] = {0};

/* Test 1: Simple parameter access with mixed addressing */
int test_simple_param(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect - should trigger the uncovered block */
    sum += *base;                     /* Line A: Simple register addressing */
    
    /* Register + constant offset */
    sum += base[5];                   /* Line B: Offset addressing */
    
    /* Loop with pointer increment */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;                /* Line C: Auto-increment pattern */
    }
    
    /* Another simple register access */
    int *simple_ptr = base + 10;
    sum += *simple_ptr;               /* Line D: Another simple register */
    
    return sum;
}

/* Test 2: Global array access via local pointer */
int test_global_access(void) {
    int sum = 0;
    int *p = &global_arr[0];
    
    /* Multiple simple register accesses */
    sum += p[0];                      /* Line E: p[0] is simple register */
    sum += *p;                        /* Line F: *p is also simple register */
    
    /* Mixed with offset */
    sum += p[10];
    
    /* Loop with different stride */
    for (int i = 0; i < 20; i += 2) {
        sum += *p;
        p += 2;
    }
    
    return sum;
}

/* Test 3: Conditional simple access inside loop */
int test_conditional_access(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Conditional simple register access */
        if (i % 3 == 0) {
            sum += *ptr;              /* Line G: Simple in conditional */
        } else {
            sum += ptr[1];            /* Offset access */
        }
        
        /* Always increment */
        ptr++;
    }
    
    /* Final simple access */
    sum += *arr;                      /* Line H: Simple at end */
    
    return sum;
}

/* Test 4: Struct access with pointer */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *d) {
    int sum = 0;
    int *p = d->values;
    
    /* Simple register access to struct member */
    sum += *p;                        /* Line I: Simple struct pointer */
    
    /* Loop through struct array */
    for (int i = 0; i < d->count && i < 20; i++) {
        sum += *p++;
    }
    
    /* Another simple access */
    p = &d->values[5];
    sum += *p;                        /* Line J: Simple after reassignment */
    
    return sum;
}

/* Test 5: Volatile pointer to prevent optimization */
int test_volatile_access(volatile int *base, int n) {
    int sum = 0;
    
    /* Volatile simple access */
    sum += *base;                     /* Line K: Volatile simple register */
    
    /* Mix volatile and non-volatile */
    int *regular_ptr = (int*)base;
    sum += regular_ptr[0];            /* Line L: Cast away volatile */
    
    /* Loop with volatile */
    for (int i = 0; i < n; i++) {
        sum += *base;
        base++;
    }
    
    return sum;
}

/* Test 6: Nested loops with different patterns */
int test_nested_patterns(int *arr, int rows, int cols) {
    int sum = 0;
    
    for (int i = 0; i < rows; i++) {
        int *row_ptr = arr + i * cols;
        
        /* Simple access at start of each row */
        sum += *row_ptr;              /* Line M: Simple per row */
        
        for (int j = 0; j < cols; j++) {
            /* Mix of simple and offset */
            if (j == 0) {
                sum += *row_ptr;      /* Line N: Simple when j==0 */
            } else {
                sum += row_ptr[j];    /* Offset access */
            }
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
    data.count = 20;
    
    /* Run all tests */
    result += test_simple_param(local_arr, 10);
    result += test_global_access();
    result += test_conditional_access(local_arr, 15);
    result += test_struct_access(&data);
    result += test_volatile_access(local_arr, 5);
    result += test_nested_patterns(local_arr, 5, 10);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional forced execution with different parameters */
    for (int iter = 0; iter < 3; iter++) {
        int *dynamic_arr = malloc(50 * sizeof(int));
        for (int i = 0; i < 50; i++) {
            dynamic_arr[i] = i + iter;
        }
        
        result += test_simple_param(dynamic_arr, 8);
        result += test_conditional_access(dynamic_arr, 12);
        
        free(dynamic_arr);
    }
    
    printf("Final result: %d\n", result);
    return result != 0 ? 0 : 1;
}
