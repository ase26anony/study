/* test_auto_inc_dec.c
 * Designed to trigger specific uncovered lines in GCC's auto-inc-dec.cc
 * Lines 1352-1358: Simple register addressing with zero offset
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
    sum += p[20];
    
    /* Loop with conditional simple access */
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            sum += *p;               /* Line 7: Conditional simple access */
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

int test_struct_access(struct Data *d) {
    int sum = 0;
    int *ptr = d->values;
    
    /* Simple register access to struct member */
    sum += ptr[0];                   /* Line 8: Simple register */
    
    /* Access through different pointer */
    int *base = ptr;
    sum += *base;                    /* Line 9: Simple register */
    
    /* Loop with mixed addressing */
    for (int i = 0; i < d->count && i < 20; i++) {
        if (i == 0) {
            sum += *ptr;             /* Line 10: Simple in loop */
        } else {
            sum += ptr[i];
        }
    }
    
    return sum;
}

/* Test 4: Volatile pointer to prevent optimization */
int test_volatile_access(volatile int *base, int n) {
    int sum = 0;
    
    /* Volatile simple access */
    sum += *base;                    /* Line 11: Volatile simple register */
    
    /* Non-volatile pointer derived from volatile */
    int *ptr = (int *)base;
    sum += ptr[0];                   /* Line 12: Simple register */
    
    /* Loop */
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr++;
    }
    
    return sum;
}

/* Test 5: Nested pointer access */
int test_nested_pointers(int **pp, int n) {
    int sum = 0;
    
    if (pp && *pp) {
        int *p = *pp;
        
        /* Simple access through dereferenced pointer */
        sum += p[0];                 /* Line 13: Simple register */
        sum += *p;                   /* Line 14: Simple register */
        
        /* Loop with increment */
        for (int i = 0; i < n; i++) {
            sum += *p++;
        }
    }
    
    return sum;
}

/* Test 6: Array parameter with simple access */
int test_array_param(int arr[], int n) {
    int sum = 0;
    
    /* Simple access to array parameter */
    sum += arr[0];                   /* Line 15: Simple register */
    
    /* Pointer to array element */
    int *p = &arr[10];
    sum += *p;                       /* Line 16: Simple register */
    
    /* Mixed loop */
    for (int i = 0; i < n; i++) {
        if (i == 0) {
            sum += *p;               /* Line 17: Simple in loop */
        }
        sum += arr[i];
    }
    
    return sum;
}

/* Main driver to ensure all code is executed */
int main(void) {
    int total_sum = 0;
    
    /* Initialize test data */
    int local_arr[50];
    for (int i = 0; i < 50; i++) {
        local_arr[i] = i + 1;
    }
    
    for (int i = 0; i < 100; i++) {
        global_arr[i] = i * 2;
    }
    
    struct Data data;
    for (int i = 0; i < 20; i++) {
        data.values[i] = i * 3;
    }
    data.count = 20;
    
    /* Run all tests */
    total_sum += test_simple_mixed(local_arr, 10);
    total_sum += test_global_access();
    total_sum += test_struct_access(&data);
    total_sum += test_volatile_access(local_arr, 5);
    
    int *ptr_arr[2];
    ptr_arr[0] = local_arr;
    ptr_arr[1] = NULL;
    total_sum += test_nested_pointers(ptr_arr, 8);
    
    total_sum += test_array_param(local_arr, 12);
    
    /* Print result to prevent dead code elimination */
    printf("Total checksum: %d\n", total_sum);
    
    return total_sum != 0 ? 0 : 1;
}
