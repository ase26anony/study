/* test_auto_inc_dec.c
 * Designed to trigger uncovered lines in GCC's auto-inc-dec.cc optimization pass
 * Specifically targets lines 1352-1358 where mem_insn is set up for simple register addressing
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array to enable different access patterns */
volatile int global_arr[100] = {0};

/* Test 1: Simple register indirect access with mixed patterns */
int test_simple_reg_access(int *base, int n) {
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
    
    /* Another simple register access after loop */
    sum += *base;                     /* Line D: Another simple register */
    
    return sum;
}

/* Test 2: Parameter used directly with conditional simple access */
int test_param_direct(int *p, int n) {
    int sum = 0;
    
    /* Direct parameter use - simple register */
    sum = *p;                         /* Simple register addressing */
    
    /* Loop with conditional simple access */
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            sum += *p;                /* Simple register in condition */
        }
        sum += p[i];                  /* Offset addressing */
    }
    
    return sum;
}

/* Test 3: Global array access via local pointer */
int test_global_access(void) {
    int sum = 0;
    int *p = &global_arr[0];          /* Local pointer to global */
    
    /* Multiple simple register accesses */
    sum += *p;                        /* Simple register */
    sum += p[0];                      /* Also simple register (offset 0) */
    
    /* Mixed with offset */
    sum += p[10];
    
    /* Pointer arithmetic but still simple register at point of use */
    int *q = p + 5;
    sum += *q;                        /* Simple register via different pointer */
    
    return sum;
}

/* Test 4: Nested loops with varying access patterns */
int test_nested_patterns(int *arr, int rows, int cols) {
    int sum = 0;
    
    /* Outer loop with simple register access */
    for (int i = 0; i < rows; i++) {
        int *row_ptr = arr + i * cols;
        
        /* Simple register access at start of each row */
        sum += *row_ptr;              /* Simple register */
        
        /* Inner loop with auto-increment */
        for (int j = 0; j < cols; j++) {
            sum += *row_ptr++;
        }
        
        /* Reset and simple access again */
        row_ptr = arr + i * cols;
        sum += *row_ptr;              /* Simple register */
    }
    
    return sum;
}

/* Test 5: Structure access with pointer */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *d) {
    int sum = 0;
    
    /* Access structure member via pointer - simple register */
    sum += d->count;                  /* May compile to simple register */
    
    /* Access array within struct */
    int *p = d->values;
    sum += *p;                        /* Simple register */
    sum += p[0];                      /* Also simple register */
    
    /* Loop through struct array */
    for (int i = 0; i < 10; i++) {
        sum += *p++;
    }
    
    return sum;
}

/* Test 6: Multiple base pointers */
int test_multiple_bases(int *a, int *b, int *c, int n) {
    int sum = 0;
    
    /* Simple accesses from different pointers */
    sum += *a;                        /* Simple register */
    sum += *b;                        /* Simple register */
    sum += *c;                        /* Simple register */
    
    /* Mixed patterns */
    for (int i = 0; i < n; i++) {
        sum += a[i];                  /* Offset */
        sum += *b++;                  /* Auto-increment */
        if (i % 3 == 0) {
            sum += *c;                /* Simple register in condition */
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
    
    /* Run all tests to generate various RTL patterns */
    result += test_simple_reg_access(local_arr, 10);
    result += test_param_direct(local_arr + 20, 5);
    result += test_global_access();
    result += test_nested_patterns(local_arr, 5, 10);
    result += test_struct_access(&data);
    
    int *arr2 = local_arr + 30;
    int *arr3 = local_arr + 50;
    result += test_multiple_bases(local_arr, arr2, arr3, 7);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    /* Additional volatile access to ensure compiler can't optimize away */
    volatile int *volatile_ptr = local_arr;
    printf("First element: %d\n", *volatile_ptr);
    
    return result != 0 ? 0 : 1;
}
