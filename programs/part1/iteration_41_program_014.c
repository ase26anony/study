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
    sum += *base;                    /* Line A: Simple register addressing */
    
    /* Register + constant offset */
    sum += base[5];                  /* Line B: Offset addressing */
    
    /* Loop with pointer increment */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;               /* Line C: Auto-increment pattern */
    }
    
    /* Another simple register access */
    int *simple_ptr = base + 10;
    sum += *simple_ptr;              /* Line D: Another simple register */
    
    return sum;
}

/* Test 2: Global array access via local pointer */
int test_global_access(void) {
    int sum = 0;
    int *p = &global_arr[0];
    
    /* Multiple simple register accesses */
    sum += p[0];                     /* Simple register (p[0] == *(p + 0)) */
    sum += *p;                       /* Direct dereference */
    
    /* Mixed with offset */
    sum += p[10];
    sum += p[20];
    
    /* Loop through global array */
    for (int i = 0; i < 50; i++) {
        sum += *p++;
    }
    
    return sum;
}

/* Test 3: Conditional simple access inside loop */
int test_conditional_access(int *base, int n) {
    int sum = 0;
    int *ptr = base;
    
    for (int i = 0; i < n; i++) {
        /* Complex loop body with conditional */
        if (i % 3 == 0) {
            /* Simple register access in conditional path */
            int *temp = ptr;
            sum += *temp;            /* Simple register in conditional */
        } else if (i % 3 == 1) {
            sum += ptr[2];           /* Offset access */
        } else {
            sum += *ptr++;           /* Auto-increment */
        }
    }
    
    /* Final simple access */
    sum += *base;                    /* Another simple register */
    
    return sum;
}

/* Test 4: Struct access with simple pointer */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *data) {
    int sum = 0;
    
    /* Access struct member via simple pointer */
    int *values_ptr = data->values;
    
    /* Simple register access to struct member */
    sum += *values_ptr;              /* Simple register */
    
    /* Loop through struct array */
    for (int i = 0; i < data->count && i < 20; i++) {
        sum += *values_ptr++;
    }
    
    /* Another simple access */
    values_ptr = &data->values[5];
    sum += *values_ptr;              /* Simple register */
    
    return sum;
}

/* Test 5: Multiple simple accesses in sequence */
int test_multiple_simple(int *base) {
    /* Sequence of simple register accesses */
    int a = *base;                   /* Simple register */
    int b = *(base + 1);             /* Offset (but might be seen as simple) */
    int *p1 = base + 2;
    int c = *p1;                     /* Simple register via different pointer */
    int *p2 = base + 3;
    int d = *p2;                     /* Another simple register */
    
    return a + b + c + d;
}

/* Test 6: Volatile pointer to prevent optimization */
int test_volatile_access(volatile int *base) {
    int sum = 0;
    
    /* Volatile simple access */
    sum += *base;                    /* Simple register with volatile */
    
    /* Non-volatile pointer derived from volatile */
    int *derived = (int *)base;
    sum += *derived;                 /* Simple register */
    
    return sum;
}

/* Main driver to ensure all code is executed */
int main(void) {
    int result = 0;
    
    /* Initialize test arrays */
    int test_array[100];
    for (int i = 0; i < 100; i++) {
        test_array[i] = i;
        global_arr[i] = i * 2;
    }
    
    /* Initialize struct */
    struct Data data;
    for (int i = 0; i < 20; i++) {
        data.values[i] = i * 3;
    }
    data.count = 20;
    
    /* Run all tests */
    result += test_simple_param(test_array, 10);
    result += test_global_access();
    result += test_conditional_access(test_array, 15);
    result += test_struct_access(&data);
    result += test_multiple_simple(test_array);
    result += test_volatile_access(test_array);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Also use result in a way compiler can't predict */
    if (result > 1000) {
        printf("Large result detected\n");
    }
    
    return result % 256;
}
