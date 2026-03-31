/* test_auto_inc_dec.c
 * Designed to trigger uncovered lines 1352-1358 in auto-inc-dec.cc
 * Compile with: gcc -O2 -fno-inline -fno-ipa-pure-const test_auto_inc_dec.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 100

/* Global array to enable multiple access patterns */
int global_array[ARRAY_SIZE];

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

/* Test 2: Global array access via local pointer */
int test_global_access(void) {
    int sum = 0;
    int *p = &global_array[0];       /* Simple pointer to global */
    
    /* Multiple simple register accesses */
    sum += *p;                       /* Line 1: Simple register */
    sum += p[0];                     /* Line 2: p[0] is same as *p */
    
    /* Mixed with offset */
    sum += p[10];                    /* Line 3: Offset */
    
    /* Loop with conditional simple access */
    for (int i = 0; i < 20; i++) {
        if (i % 3 == 0) {
            sum += *p;               /* Line 4: Simple access in loop */
        }
        p++;
    }
    
    return sum;
}

/* Test 3: Array traversal with multiple pointer variables */
int test_multiple_pointers(int *arr, int n) {
    int sum = 0;
    int *p1 = arr;
    int *p2 = arr + n/2;
    
    /* Simple accesses from different pointers */
    sum += *p1;                      /* Line 1: p1 simple access */
    sum += *p2;                      /* Line 2: p2 simple access */
    
    /* Loop with both pointers */
    for (int i = 0; i < n/4; i++) {
        sum += *p1;                  /* Line 3: Simple in loop */
        sum += *p2;                  /* Line 4: Another simple */
        p1++;
        p2--;
    }
    
    /* Final simple access */
    sum += *arr;                     /* Line 5: Direct from parameter */
    
    return sum;
}

/* Test 4: Struct access with pointer */
struct Data {
    int values[10];
    int count;
};

int test_struct_access(struct Data *d) {
    int sum = 0;
    int *p = d->values;
    
    /* Simple access to struct member via pointer */
    sum += *p;                       /* Line 1: Simple register */
    
    /* Access through array notation */
    sum += p[0];                     /* Line 2: Same as above */
    
    /* Loop through struct array */
    for (int i = 0; i < d->count && i < 10; i++) {
        sum += *p;                   /* Line 3: Simple in loop */
        p++;
    }
    
    return sum;
}

/* Test 5: Volatile pointer to prevent optimization */
int test_volatile_access(volatile int *base, int n) {
    int sum = 0;
    
    /* Volatile simple access */
    sum += *base;                    /* Line 1: Simple volatile */
    
    /* Loop with volatile */
    volatile int *vptr = base;
    for (int i = 0; i < n; i++) {
        sum += *vptr;                /* Line 2: Simple in loop */
        vptr++;
    }
    
    return sum;
}

/* Main driver to ensure all code is executed */
int main(void) {
    int result = 0;
    
    /* Initialize global array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        global_array[i] = i * 2 + 1;
    }
    
    /* Local array for testing */
    int local_array[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        local_array[i] = i * 3 + 2;
    }
    
    /* Initialize struct */
    struct Data data;
    for (int i = 0; i < 10; i++) {
        data.values[i] = i * 5 + 1;
    }
    data.count = 10;
    
    /* Run all tests */
    result += test_simple_param(local_array, 20);
    result += test_global_access();
    result += test_multiple_pointers(local_array, 40);
    result += test_struct_access(&data);
    result += test_volatile_access(local_array, 10);
    
    /* Additional test with immediate zero offset */
    int *simple_ptr = &local_array[30];
    result += *simple_ptr;           /* Direct simple access in main */
    
    printf("Result: %d\n", result);
    
    /* Verify result is non-zero to ensure all code ran */
    if (result != 0) {
        printf("All tests executed successfully.\n");
        return 0;
    } else {
        printf("Warning: Result is zero - possible optimization issue.\n");
        return 1;
    }
}
