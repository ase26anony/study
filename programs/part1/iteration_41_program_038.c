/* test_auto_inc_dec.c
 * Designed to trigger specific uncovered lines in GCC's auto-inc-dec.cc
 * Lines 1352-1358: Simple register addressing with zero offset
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array for testing global pointer access */
int global_arr[100];

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

/* Test 2: Local array with pointer */
int test_local_array(int n) {
    int local_arr[50];
    
    /* Initialize array */
    for (int i = 0; i < 50; i++) {
        local_arr[i] = i * 2;
    }
    
    int sum = 0;
    int *p = local_arr;
    
    /* Multiple simple register accesses */
    sum += p[0];                      /* Line 5: p[0] is simple register */
    sum += *p;                        /* Line 6: *p is simple register */
    
    /* Mixed with offset */
    sum += p[10];
    sum += p[20];
    
    /* Loop with conditional simple access */
    for (int i = 0; i < n && i < 50; i++) {
        if (i % 3 == 0) {
            sum += *p;                /* Line 7: Simple in conditional */
        }
        p++;
    }
    
    return sum;
}

/* Test 3: Global array access */
int test_global_array(void) {
    int sum = 0;
    
    /* Pointer to global with simple access */
    int *gp = &global_arr[0];
    sum += gp[0];                     /* Line 8: Global simple access */
    sum += *gp;                       /* Line 9: Another simple access */
    
    /* Different pointer with offset */
    int *gp2 = gp + 30;
    sum += *gp2;                      /* Line 10: Simple from offset ptr */
    
    /* Loop through global */
    for (int i = 0; i < 50; i++) {
        sum += *gp++;
    }
    
    return sum;
}

/* Test 4: Struct access with pointers */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *d) {
    int sum = 0;
    
    /* Simple access to struct member */
    sum += d->count;                  /* Line 11: Struct member access */
    
    /* Pointer to array inside struct */
    int *vp = d->values;
    sum += vp[0];                     /* Line 12: Simple from struct array */
    sum += *vp;                       /* Line 13: Another simple */
    
    /* Mixed access patterns */
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            sum += *vp;               /* Line 14: Simple in loop conditional */
        } else {
            sum += vp[i];             /* Offset in loop */
        }
    }
    
    return sum;
}

/* Test 5: Volatile pointer to prevent optimization */
int test_volatile_access(volatile int *base, int n) {
    int sum = 0;
    
    /* Volatile simple access */
    sum += *base;                     /* Line 15: Volatile simple register */
    
    /* Mix volatile and non-volatile */
    int *normal_ptr = (int*)base;
    sum += normal_ptr[0];             /* Line 16: Cast from volatile */
    
    /* Loop with volatile increment */
    volatile int *vptr = base;
    for (int i = 0; i < n; i++) {
        sum += *vptr++;               /* Auto-inc with volatile */
    }
    
    return sum;
}

/* Test 6: Nested loops with different patterns */
int test_nested_patterns(int *base, int rows, int cols) {
    int sum = 0;
    
    for (int i = 0; i < rows; i++) {
        int *row_ptr = base + i * cols;
        
        /* Simple access at start of each row */
        sum += row_ptr[0];            /* Line 17: Simple per row */
        
        for (int j = 0; j < cols; j++) {
            if (j == 0) {
                sum += *row_ptr;      /* Line 18: Simple when j==0 */
            } else {
                sum += row_ptr[j];    /* Offset access */
            }
        }
    }
    
    return sum;
}

/* Main driver to ensure all code is executed */
int main(void) {
    int total_sum = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_arr[i] = i;
    }
    
    /* Test array for parameter tests */
    int test_array[100];
    for (int i = 0; i < 100; i++) {
        test_array[i] = i * 3;
    }
    
    /* Initialize struct */
    struct Data data;
    for (int i = 0; i < 20; i++) {
        data.values[i] = i * 5;
    }
    data.count = 42;
    
    /* Run all tests */
    total_sum += test_simple_param(test_array, 20);
    total_sum += test_local_array(15);
    total_sum += test_global_array();
    total_sum += test_struct_access(&data);
    total_sum += test_volatile_access(test_array, 10);
    total_sum += test_nested_patterns(test_array, 5, 10);
    
    /* Additional direct simple accesses */
    int *direct_ptr = test_array;
    total_sum += direct_ptr[0];       /* Line 19: Direct in main */
    total_sum += *direct_ptr;         /* Line 20: Another in main */
    
    /* Loop in main with simple access */
    for (int i = 0; i < 5; i++) {
        if (i == 0) {
            total_sum += *direct_ptr; /* Line 21: Simple in main loop */
        }
        direct_ptr++;
    }
    
    printf("Total checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
