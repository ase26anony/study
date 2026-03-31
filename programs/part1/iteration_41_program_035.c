/* test_auto_inc_dec.c
 * Designed to trigger specific uncovered lines in GCC's auto-inc-dec.cc
 * Lines 1352-1358: Simple register addressing with zero offset
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array for testing */
int global_arr[100] = {0};

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
    
    /* Another simple register access after loop */
    sum += *base;                     /* Line 4: Another simple register */
    
    return sum;
}

/* Test 2: Global array access via local pointer */
int test_global_access(void) {
    int sum = 0;
    int *p = &global_arr[0];
    
    /* Multiple simple register accesses */
    sum += p[0];                      /* Line 1: p[0] is simple register */
    sum += *p;                        /* Line 2: *p is simple register */
    
    /* Mixed with offset */
    sum += p[10];                     /* Line 3: Offset addressing */
    
    /* Loop with conditional simple access */
    for (int i = 0; i < 20; i++) {
        if (i % 3 == 0) {
            sum += *p;                /* Line 4: Conditional simple access */
        }
        p++;
    }
    
    return sum;
}

/* Test 3: Local array with volatile pointer */
int test_volatile_local(int n) {
    int local_arr[50];
    volatile int *vol_ptr = local_arr;
    
    /* Initialize array */
    for (int i = 0; i < 50; i++) {
        local_arr[i] = i * 2;
    }
    
    int sum = 0;
    
    /* Simple volatile register access */
    sum += *vol_ptr;                  /* Line 1: Volatile simple register */
    
    /* Loop with mixed access patterns */
    int *ptr = local_arr;
    for (int i = 0; i < n && i < 50; i++) {
        /* Alternate between simple and offset */
        if (i % 2 == 0) {
            sum += *ptr;              /* Line 2: Simple in loop */
        } else {
            sum += ptr[3];            /* Line 3: Offset in loop */
        }
        ptr++;
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
    int *p = data->values;
    
    /* Simple register access to struct member */
    sum += *p;                        /* Line 1: Simple struct pointer */
    
    /* Access through different expressions */
    sum += data->values[0];           /* Line 2: Another simple access */
    
    /* Loop through struct array */
    for (int i = 0; i < data->count && i < 20; i++) {
        sum += *p++;                  /* Line 3: Auto-increment */
    }
    
    /* Final simple access */
    sum += *(data->values);           /* Line 4: Dereference of array name */
    
    return sum;
}

/* Test 5: Multiple base pointers */
int test_multiple_pointers(int *a, int *b, int n) {
    int sum = 0;
    
    /* Simple accesses to different pointers */
    sum += *a;                        /* Line 1: Simple a */
    sum += *b;                        /* Line 2: Simple b */
    
    /* Mixed patterns */
    sum += a[0];                      /* Line 3: a[0] - simple */
    sum += b[5];                      /* Line 4: b[5] - offset */
    
    /* Loops with different pointers */
    for (int i = 0; i < n; i++) {
        sum += *a + *b;               /* Line 5: Two simple accesses in loop */
        a++;
        b++;
    }
    
    return sum;
}

/* Main driver to ensure all code is executed */
int main(void) {
    int result = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_arr[i] = i;
    }
    
    /* Test array */
    int test_arr[50];
    for (int i = 0; i < 50; i++) {
        test_arr[i] = i * 3;
    }
    
    /* Struct for testing */
    struct Data data;
    for (int i = 0; i < 20; i++) {
        data.values[i] = i * 4;
    }
    data.count = 20;
    
    /* Run all tests */
    result += test_simple_param(test_arr, 10);
    result += test_global_access();
    result += test_volatile_local(15);
    result += test_struct_access(&data);
    
    /* Test with two different arrays */
    int arr1[20], arr2[20];
    for (int i = 0; i < 20; i++) {
        arr1[i] = i * 5;
        arr2[i] = i * 6;
    }
    result += test_multiple_pointers(arr1, arr2, 10);
    
    /* Additional edge cases */
    
    /* Test with pointer that doesn't change in loop */
    int *fixed_ptr = test_arr;
    for (int i = 0; i < 5; i++) {
        result += *fixed_ptr;         /* Repeated simple access */
    }
    
    /* Test with array parameter */
    result += test_simple_param(global_arr, 5);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
