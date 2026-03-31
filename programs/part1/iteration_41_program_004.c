/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-increment/decrement optimization
 * Specifically targets the uncovered block in auto-inc-dec.cc lines 1352-1358
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array for testing */
int global_arr[100] = {0};

/* Function 1: Simple parameter access with mixed addressing */
int test_simple_param(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect access - should trigger the uncovered block */
    sum += *base;                    /* mem_insn.reg1_val = 0 case */
    
    /* Register + constant offset */
    sum += base[5];                  /* Different addressing mode */
    
    /* Loop with pointer increment */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;               /* Auto-increment candidate */
    }
    
    /* Another simple register access */
    int *simple_ptr = base + 10;
    sum += *simple_ptr;              /* Another mem_insn.reg1_val = 0 case */
    
    return sum;
}

/* Function 2: Global array access via local pointer */
int test_global_access(void) {
    int sum = 0;
    int *p = &global_arr[0];
    
    /* Multiple simple register accesses */
    sum += p[0];                     /* Should trigger: mem_insn.reg1_is_const = true */
    sum += *p;                       /* Direct dereference */
    
    /* Mixed with offset */
    sum += p[20];
    sum += p[30];
    
    /* Loop with different pattern */
    for (int i = 0; i < 10; i++) {
        sum += *p;                   /* Repeated simple access */
        p += 2;                      /* Step by 2 */
    }
    
    return sum;
}

/* Function 3: Array traversal with conditional simple access */
int test_conditional_access(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Complex addressing in loop */
        sum += ptr[i];
        
        /* Conditional simple register access */
        if (i % 3 == 0) {
            int *temp = ptr + i;
            sum += *temp;            /* Simple register access inside condition */
        }
        
        /* Another pattern */
        if (i == n/2) {
            sum += *ptr;             /* Simple access at midpoint */
        }
    }
    
    /* Final simple access */
    sum += *arr;                     /* One more simple register case */
    
    return sum;
}

/* Function 4: Struct access with pointer */
struct Data {
    int values[50];
    int count;
};

int test_struct_access(struct Data *d) {
    int sum = 0;
    int *p = d->values;
    
    /* Simple access to struct member via pointer */
    sum += *p;                       /* Should trigger the block */
    
    /* Access with offset */
    sum += p[10];
    sum += p[20];
    
    /* Loop through struct array */
    for (int i = 0; i < d->count && i < 50; i++) {
        sum += *p++;
    }
    
    return sum;
}

/* Function 5: Volatile pointer to prevent optimization */
int test_volatile_access(volatile int *base, int n) {
    int sum = 0;
    
    /* Volatile simple access */
    sum += *base;                    /* mem_insn.reg1_val = 0 with volatile */
    
    /* Non-volatile pointer derived from volatile */
    int *ptr = (int*)base;
    sum += ptr[0];                   /* Another simple access */
    
    /* Loop with mixed accesses */
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
        if (i % 4 == 0) {
            sum += *base;            /* Periodic simple access to base */
        }
    }
    
    return sum;
}

/* Function 6: Nested loops with different addressing */
int test_nested_loops(int *arr, int rows, int cols) {
    int sum = 0;
    
    /* Simple access before loop */
    sum += *arr;                     /* Target uncovered block */
    
    for (int i = 0; i < rows; i++) {
        int *row_ptr = arr + i * cols;
        
        /* Simple access at start of row */
        sum += *row_ptr;             /* Another simple register access */
        
        for (int j = 0; j < cols; j++) {
            /* Different addressing patterns */
            if (j == 0) {
                sum += row_ptr[0];   /* Simple access at column 0 */
            } else {
                sum += *(row_ptr + j); /* Register + variable offset */
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
    for (int i = 0; i < 50; i++) {
        data.values[i] = i * 3;
    }
    data.count = 50;
    
    /* Call all test functions with various parameters */
    result += test_simple_param(local_arr, 20);
    result += test_global_access();
    result += test_conditional_access(local_arr, 30);
    result += test_struct_access(&data);
    result += test_volatile_access(local_arr, 15);
    result += test_nested_loops(local_arr, 5, 10);
    
    /* Use result to prevent dead code elimination */
    printf("Result checksum: %d\n", result);
    
    /* Also return it to ensure compiler keeps all computations */
    return result == 0 ? 0 : 1;
}
