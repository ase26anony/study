/* test_auto_inc_dec.c
 * Designed to trigger uncovered lines in GCC's auto-inc-dec.cc
 * Specifically targeting lines 1352-1358 in find_inc_dec function
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array to enable multiple access patterns */
int global_arr[100] = {0};

/* Test 1: Simple register indirect access with mixed patterns */
int test_simple_reg_access(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect - should trigger the uncovered block */
    sum += *base;                    /* Line 1: Simple register address */
    
    /* Register + constant offset */
    sum += base[5];                  /* Line 2: Offset access */
    
    /* Loop with pointer increment */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;               /* Line 3: Auto-increment pattern */
    }
    
    /* Another simple register access after loop */
    sum += *base;                    /* Line 4: Another simple access */
    
    return sum;
}

/* Test 2: Parameter used directly with conditional simple access */
int test_param_direct(int *p, int n) {
    int result = 0;
    
    /* Direct parameter use - simple register */
    result = *p;                     /* Line 5: Simple register from param */
    
    /* Mixed access in loop */
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            result += *p;            /* Line 6: Conditional simple access */
        } else {
            result += p[i];          /* Offset access */
        }
    }
    
    return result;
}

/* Test 3: Global array accessed via local pointer */
int test_global_access(void) {
    int sum = 0;
    int *p = &global_arr[0];         /* Local pointer to global */
    
    /* Multiple simple register accesses */
    sum += p[0];                     /* Line 7: p[0] is simple register */
    sum += *p;                       /* Line 8: *p is simple register */
    
    /* Mixed with offset */
    sum += p[10];
    
    /* Loop with simple access */
    for (int i = 0; i < 10; i++) {
        sum += *p;                   /* Line 9: Repeated simple access */
        p = &global_arr[i];          /* Change pointer */
    }
    
    return sum;
}

/* Test 4: Struct access with simple pointer */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *d) {
    int total = 0;
    int *ptr = d->values;
    
    /* Simple register access to struct member */
    total = *ptr;                    /* Line 10: Simple struct member access */
    
    /* Access through different expressions */
    total += ptr[0];                 /* Line 11: Another simple access */
    total += d->values[0];           /* Different pattern */
    
    /* Loop mixing patterns */
    for (int i = 0; i < d->count && i < 20; i++) {
        if (i == 0) {
            total += *ptr;           /* Line 12: Simple in loop */
        }
        total += ptr[i];
    }
    
    return total;
}

/* Test 5: Volatile pointer to prevent optimization */
int test_volatile_access(volatile int *vp, int n) {
    int sum = 0;
    
    /* Volatile simple access */
    sum += *vp;                      /* Line 13: Volatile simple register */
    
    /* Mix with non-volatile pattern */
    int *p = (int *)vp;
    sum += p[0];                     /* Line 14: Cast then simple access */
    
    /* Loop with volatile */
    for (int i = 0; i < n; i++) {
        sum += *vp;                  /* Line 15: Repeated volatile simple */
        sum += vp[i];                /* Offset access */
    }
    
    return sum;
}

/* Test 6: Nested function with multiple simple accesses */
static int helper_simple_load(int *a, int *b) {
    /* Two simple register accesses */
    int x = *a;                      /* Line 16: First param simple */
    int y = *b;                      /* Line 17: Second param simple */
    return x + y;
}

int test_nested_simple(int *arr, int n) {
    int result = 0;
    
    /* Call helper with simple pointers */
    result += helper_simple_load(&arr[0], &arr[1]);
    
    /* Additional simple accesses */
    int *p = arr;
    result += *p;                    /* Line 18: Local pointer simple */
    
    for (int i = 0; i < n; i++) {
        if (i == 0) {
            result += *arr;          /* Line 19: Simple in loop condition */
        }
        result += p[i];
    }
    
    return result;
}

/* Main driver to ensure all code is executed */
int main(void) {
    int test_data[50];
    int volatile volatile_data[10];
    struct Data my_data = {{0}, 10};
    int final_sum = 0;
    
    /* Initialize data */
    for (int i = 0; i < 50; i++) {
        test_data[i] = i + 1;
    }
    
    for (int i = 0; i < 100; i++) {
        global_arr[i] = i * 2;
    }
    
    for (int i = 0; i < 10; i++) {
        volatile_data[i] = i * 3;
    }
    
    for (int i = 0; i < 20; i++) {
        my_data.values[i] = i * 4;
    }
    my_data.count = 10;
    
    /* Run all tests */
    final_sum += test_simple_reg_access(test_data, 10);
    final_sum += test_param_direct(&test_data[5], 8);
    final_sum += test_global_access();
    final_sum += test_struct_access(&my_data);
    final_sum += test_volatile_access(volatile_data, 5);
    final_sum += test_nested_simple(test_data, 6);
    
    /* Additional direct tests */
    int *simple_ptr = &test_data[0];
    final_sum += *simple_ptr;        /* Line 20: Direct simple access in main */
    final_sum += simple_ptr[0];      /* Line 21: Another simple access */
    
    /* Loop with mixed patterns in main */
    for (int i = 0; i < 5; i++) {
        if (i == 0) {
            final_sum += *simple_ptr; /* Line 22: Simple in main's loop */
        }
        final_sum += simple_ptr[i];
        simple_ptr = &test_data[i];
    }
    
    printf("Final checksum: %d\n", final_sum);
    return final_sum != 0 ? 0 : 1;
}
