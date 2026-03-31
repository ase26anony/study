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
    sum += *p;                       /* Line 6: *p is also simple register */
    
    /* Mix with offset */
    sum += p[10];
    
    /* Loop through global array */
    for (int i = 0; i < 50; i++) {
        sum += *p++;
    }
    
    return sum;
}

/* Test 3: Array traversal with conditional simple access */
int test_conditional_access(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Complex loop body with mixed accesses */
        if (i % 3 == 0) {
            /* Conditional simple register access */
            sum += *ptr;             /* Line 7: Simple in conditional */
        } else if (i % 3 == 1) {
            /* Offset access */
            sum += ptr[2];
        } else {
            /* Auto-increment */
            sum += *ptr++;
        }
        
        /* Additional simple access outside condition */
        int *temp = ptr;
        sum += *temp;                /* Line 8: Another simple register */
    }
    
    return sum;
}

/* Test 4: Struct access with pointer */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *data) {
    int sum = 0;
    
    /* Simple access to struct member */
    sum += data->count;              /* Line 9: Struct member access */
    
    /* Access array within struct */
    int *vals = data->values;
    sum += vals[0];                  /* Line 10: Simple from struct array */
    
    /* Loop through struct array */
    for (int i = 0; i < 10; i++) {
        sum += *vals++;
    }
    
    return sum;
}

/* Test 5: Multiple base pointers */
int test_multiple_pointers(int *a, int *b, int *c, int n) {
    int sum = 0;
    
    /* Simple accesses from different pointers */
    sum += *a;                       /* Line 11: Simple from a */
    sum += *b;                       /* Line 12: Simple from b */
    sum += *c;                       /* Line 13: Simple from c */
    
    /* Mixed patterns */
    sum += a[5];
    sum += b[3];
    
    /* Loops with different pointers */
    for (int i = 0; i < n; i++) {
        sum += *a++ + *b++ + c[i];
    }
    
    return sum;
}

/* Test 6: Volatile pointer to prevent optimization */
int test_volatile_access(volatile int *base, int n) {
    int sum = 0;
    
    /* Volatile simple access */
    sum += *base;                    /* Line 14: Volatile simple register */
    
    /* Loop with volatile */
    for (int i = 0; i < n; i++) {
        sum += *base++;
    }
    
    return sum;
}

/* Main driver to ensure all code is executed */
int main(void) {
    int result = 0;
    
    /* Initialize test arrays */
    int arr1[100];
    int arr2[50];
    int arr3[30];
    
    for (int i = 0; i < 100; i++) global_arr[i] = i;
    for (int i = 0; i < 100; i++) arr1[i] = i * 2;
    for (int i = 0; i < 50; i++) arr2[i] = i * 3;
    for (int i = 0; i < 30; i++) arr3[i] = i * 4;
    
    struct Data data;
    for (int i = 0; i < 20; i++) data.values[i] = i * 5;
    data.count = 42;
    
    /* Call all test functions */
    result += test_simple_param(arr1, 20);
    result += test_global_access();
    result += test_conditional_access(arr2, 25);
    result += test_struct_access(&data);
    result += test_multiple_pointers(arr1, arr2, arr3, 10);
    result += test_volatile_access(arr1, 5);
    
    /* Use result to prevent dead code elimination */
    printf("Result checksum: %d\n", result);
    
    /* Additional forced execution with different parameters */
    for (int i = 0; i < 10; i++) {
        result += test_simple_param(&global_arr[i], 5);
    }
    
    return result != 0 ? 0 : 1;
}
