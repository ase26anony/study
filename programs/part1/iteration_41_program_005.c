/* test_auto_inc_dec.c
 * Designed to trigger GCC's auto-increment/decrement optimization pass
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
    sum += *base;                    /* Line 1: Simple register addressing */
    
    /* Register + constant offset */
    sum += base[5];                  /* Line 2: Offset addressing */
    
    /* Loop with pointer increment */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;               /* Line 3: Auto-increment pattern */
    }
    
    /* Another simple register access after loop */
    sum += *base;                    /* Line 4: Simple register again */
    
    return sum;
}

/* Test 2: Local array with pointer */
int test_local_array(int n) {
    int local_arr[50];
    int *p = local_arr;
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 50; i++) {
        local_arr[i] = i;
    }
    
    /* Multiple simple register accesses */
    sum += p[0];                     /* Line 5: p[0] is simple register */
    sum += *p;                       /* Line 6: *p is simple register */
    
    /* Mixed with offset */
    sum += p[10];
    sum += p[20];
    
    /* Loop with conditional simple access */
    for (int i = 0; i < n; i++) {
        if (i % 3 == 0) {
            sum += *p;               /* Line 7: Simple access in loop */
        }
        p++;
    }
    
    return sum;
}

/* Test 3: Global array access */
int test_global_access(void) {
    int *gp = &global_arr[0];
    int sum = 0;
    
    /* Simple register access to global */
    sum += gp[0];                    /* Line 8: Simple register */
    sum += *gp;                      /* Line 9: Simple register */
    
    /* Offset access */
    sum += gp[25];
    sum += gp[50];
    
    /* Reset pointer and do simple access */
    gp = &global_arr[10];
    sum += *gp;                      /* Line 10: Simple register */
    
    return sum;
}

/* Test 4: Struct access with pointer */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *d) {
    int sum = 0;
    int *vp = d->values;
    
    /* Simple register access to struct member */
    sum += vp[0];                    /* Line 11: Simple register */
    sum += *vp;                      /* Line 12: Simple register */
    
    /* Access with offset */
    sum += vp[5];
    sum += vp[10];
    
    /* Loop mixing simple and offset */
    for (int i = 0; i < d->count; i++) {
        if (i == 0) {
            sum += *vp;              /* Line 13: Simple in loop */
        } else {
            sum += vp[i];
        }
    }
    
    return sum;
}

/* Test 5: Multiple simple accesses in sequence */
int test_multiple_simple(int *arr, int n) {
    int *p1 = arr;
    int *p2 = arr + 10;
    int *p3 = arr + 20;
    int sum = 0;
    
    /* Sequence of simple register accesses */
    sum += *p1;                      /* Line 14: Simple */
    sum += *p2;                      /* Line 15: Simple */
    sum += *p3;                      /* Line 16: Simple */
    
    /* Interleaved with offsets */
    sum += p1[5];
    sum += p2[3];
    sum += p3[1];
    
    /* Back to simple */
    sum += *p1;                      /* Line 17: Simple */
    
    return sum;
}

/* Test 6: Volatile pointer to prevent optimization */
int test_volatile_access(volatile int *vp, int n) {
    int sum = 0;
    
    /* Simple access through volatile pointer */
    sum += *vp;                      /* Line 18: Simple register */
    
    /* Loop with mixed access */
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            sum += *vp;              /* Line 19: Simple in loop */
        } else {
            sum += vp[i];
        }
    }
    
    return sum;
}

/* Main driver to ensure all code is executed */
int main(void) {
    int result = 0;
    
    /* Initialize test data */
    int test_array[100];
    for (int i = 0; i < 100; i++) {
        test_array[i] = i;
        global_arr[i] = i * 2;
    }
    
    struct Data data;
    for (int i = 0; i < 20; i++) {
        data.values[i] = i * 3;
    }
    data.count = 10;
    
    volatile int *volatile_ptr = test_array;
    
    /* Run all tests */
    result += test_simple_param(test_array, 10);
    result += test_local_array(15);
    result += test_global_access();
    result += test_struct_access(&data);
    result += test_multiple_simple(test_array, 5);
    result += test_volatile_access(volatile_ptr, 8);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
