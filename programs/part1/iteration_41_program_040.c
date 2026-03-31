/* test_auto_inc_dec.c
 * 
 * This program is designed to trigger the uncovered block in GCC's
 * auto-inc-dec.cc (lines 1352-1358) by creating memory access patterns
 * that cause the compiler to analyze simple register addressing modes.
 * 
 * The uncovered block handles memory operands where the address is
 * just a register with zero offset (e.g., *p or p[0]).
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array to enable pointer-based access patterns */
int global_arr[100] = {0};

/* Test 1: Simple parameter access with mixed addressing */
int test_simple_param(int *p, int n) {
    int sum = 0;
    
    /* This should generate simple register addressing: *p */
    sum += *p;
    
    /* This adds register+constant offset: p[5] */
    sum += p[5];
    
    /* Loop with pointer increment to encourage auto-inc optimization */
    int *ptr = p;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    
    /* Another simple register access inside loop */
    for (int i = 0; i < 3; i++) {
        sum += *p;  /* Simple register, no offset */
    }
    
    return sum;
}

/* Test 2: Global array access via local pointer */
int test_global_access(void) {
    int sum = 0;
    int *p = &global_arr[0];
    
    /* Simple register indirect: *p */
    sum = *p;
    
    /* Register+constant offset */
    sum += p[10];
    sum += p[20];
    
    /* Loop with mixed addressing */
    for (int i = 0; i < 50; i++) {
        if (i % 10 == 0) {
            /* Conditional simple register access */
            sum += *p;
        }
        sum += *p++;
    }
    
    return sum;
}

/* Test 3: Array traversal with pointer arithmetic */
int test_array_traversal(int *arr, int size) {
    if (size <= 0) return 0;
    
    int sum = 0;
    int *start = arr;
    
    /* Initial simple register access */
    sum = *start;
    
    /* Middle element with simple register (when start == arr + size/2) */
    int *mid = arr + size/2;
    sum += *mid;
    
    /* Loop through array */
    for (int *ptr = arr; ptr < arr + size; ptr++) {
        /* Occasionally use simple register access */
        if ((ptr - arr) % 7 == 0) {
            sum += *ptr;  /* Simple register */
        } else {
            sum += ptr[0] + ptr[1];  /* Mixed addressing */
        }
    }
    
    return sum;
}

/* Test 4: Struct access with pointer */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *d) {
    int sum = 0;
    
    /* Access struct member with simple pointer */
    sum = d->count;
    
    /* Access array within struct */
    int *p = d->values;
    
    /* Simple register access to array */
    sum += *p;
    
    /* Loop through array values */
    for (int i = 0; i < 10; i++) {
        sum += *p++;
    }
    
    /* Another simple access after loop */
    sum += *p;
    
    return sum;
}

/* Test 5: Volatile pointer to prevent optimization */
int test_volatile_access(volatile int *vp, int n) {
    int sum = 0;
    
    /* Volatile simple register access */
    sum = *vp;
    
    /* Non-volatile pointer derived from volatile */
    int *p = (int *)vp;
    
    /* Mixed accesses */
    sum += p[0];  /* Simple register via p */
    sum += p[5];
    
    for (int i = 0; i < n; i++) {
        sum += *p++;
    }
    
    return sum;
}

/* Main driver to ensure all code paths are executed */
int main(void) {
    int result = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_arr[i] = i;
    }
    
    /* Local arrays for testing */
    int local_arr[50];
    for (int i = 0; i < 50; i++) {
        local_arr[i] = i * 2;
    }
    
    struct Data data;
    for (int i = 0; i < 20; i++) {
        data.values[i] = i * 3;
    }
    data.count = 20;
    
    /* Run all tests with various parameters */
    result += test_simple_param(local_arr, 10);
    result += test_global_access();
    result += test_array_traversal(local_arr, 50);
    result += test_struct_access(&data);
    
    /* Use volatile to ensure memory accesses aren't optimized away */
    volatile int volatile_arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    result += test_volatile_access(volatile_arr, 5);
    
    /* Additional test: nested function calls */
    int *ptr = local_arr;
    for (int i = 0; i < 3; i++) {
        /* Multiple simple register accesses */
        result += *ptr;
        ptr++;
    }
    
    printf("Result checksum: %d\n", result);
    
    /* Return non-zero to indicate success */
    return result != 0 ? 0 : 1;
}
