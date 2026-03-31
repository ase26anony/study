/* test_auto_inc_dec.c
 * Designed to trigger uncovered lines 1352-1358 in auto-inc-dec.cc
 * Compile with: gcc -O2 -fno-inline -fno-ipa-pure-const test_auto_inc_dec.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array to ensure memory accesses aren't optimized away */
volatile int global_array[100] = {0};

/* Test 1: Simple parameter access with mixed addressing patterns */
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
    
    /* Another simple register access */
    int *simple_ptr = base + 10;
    sum += *simple_ptr;              /* Line 4: Another simple register */
    
    return sum;
}

/* Test 2: Global array access via local pointer */
int test_global_access(void) {
    int sum = 0;
    int *p = (int*)&global_array[0];
    
    /* Multiple simple register accesses */
    sum += p[0];                     /* Line 5: p[0] is simple register */
    sum += *p;                       /* Line 6: *p is also simple register */
    
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

/* Test 3: Array traversal with multiple pointer variables */
int test_multiple_pointers(int *arr, int size) {
    int sum = 0;
    int *p1 = arr;
    int *p2 = arr + size/2;
    int *p3 = arr + size/3;
    
    /* Simple accesses from different pointers */
    sum += *p1;                      /* Line 8: Simple from p1 */
    sum += *p2;                      /* Line 9: Simple from p2 */
    sum += *p3;                      /* Line 10: Simple from p3 */
    
    /* Loop with mixed addressing */
    for (int i = 0; i < size; i++) {
        if (i % 2 == 0) {
            sum += *p1;              /* Line 11: Simple in loop */
        } else {
            sum += p2[0];            /* Line 12: p2[0] is simple */
        }
        p1++;
        if (i % 3 == 0) {
            p2++;
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
    int *ptr = d->values;
    
    /* Simple access to struct member via pointer */
    sum += *ptr;                     /* Line 13: Simple struct access */
    
    /* Access with offset */
    sum += ptr[5];
    
    /* Loop through struct array */
    for (int i = 0; i < d->count && i < 20; i++) {
        sum += *ptr;                 /* Line 14: Simple in struct loop */
        ptr++;
    }
    
    return sum;
}

/* Test 5: Function with only simple register access */
int pure_simple_access(int *p) {
    /* This should be the simplest case for the uncovered block */
    return *p;                       /* Line 15: Pure simple access */
}

/* Main driver to ensure all code is executed */
int main(void) {
    int result = 0;
    
    /* Initialize test data */
    int test_array[100];
    for (int i = 0; i < 100; i++) {
        test_array[i] = i;
        global_array[i] = i * 2;
    }
    
    struct Data data;
    for (int i = 0; i < 20; i++) {
        data.values[i] = i * 3;
    }
    data.count = 20;
    
    /* Execute all test functions */
    result += test_simple_param(test_array, 10);
    result += test_global_access();
    result += test_multiple_pointers(test_array, 50);
    result += test_struct_access(&data);
    result += pure_simple_access(test_array);
    
    /* Add some volatile operations to prevent optimization */
    volatile int dummy = result;
    
    printf("Result: %d\n", result);
    
    /* Return non-zero to indicate execution */
    return result != 0 ? 0 : 1;
}
