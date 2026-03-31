/* Test program to trigger auto-increment/decrement optimization coverage
   Specifically targets the uncovered block in find_inc_dec function
   that handles simple register addressing with zero offset */

#include <stdio.h>
#include <stdlib.h>

/* Global array for testing global pointer access */
int global_arr[100] = {0};

/* Test 1: Simple parameter load with zero offset */
int test_simple_param_load(int *p) {
    /* This should generate a simple register address: *p */
    volatile int val = *p;
    return val;
}

/* Test 2: Mixed addressing patterns in a single function */
int test_mixed_addressing(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect - should trigger the uncovered block */
    sum += *base;
    
    /* Register + constant offset */
    sum += base[5];
    
    /* Loop with pointer increment - encourages auto-inc optimization */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    
    /* Another simple register access after loop */
    sum += *base;
    
    return sum;
}

/* Test 3: Global array access via local pointer */
int test_global_access(void) {
    int *p = &global_arr[0];
    int sum = 0;
    
    /* Simple register access to global */
    sum += p[0];
    
    /* Offset access */
    sum += p[10];
    
    /* Loop through global array */
    for (int i = 0; i < 50; i++) {
        sum += *p++;
    }
    
    return sum;
}

/* Test 4: Conditional simple access inside loop */
int test_conditional_simple_access(int *arr, int n) {
    int sum = 0;
    int *simple_ptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Complex addressing */
        sum += arr[i * 2];
        
        /* Conditional simple register access */
        if (i % 3 == 0) {
            /* This should be simple register addressing */
            sum += *simple_ptr;
        }
        
        /* Sometimes use offset */
        if (i % 5 == 0) {
            sum += simple_ptr[2];
        }
    }
    
    return sum;
}

/* Test 5: Multiple simple register accesses with different pointers */
int test_multiple_pointers(int *a, int *b, int *c, int n) {
    int sum = 0;
    
    /* Three different simple register accesses */
    sum += *a;
    sum += *b;
    sum += *c;
    
    /* Loop mixing all pointers */
    for (int i = 0; i < n; i++) {
        sum += *a++ + *b++ + *c++;
    }
    
    return sum;
}

/* Test 6: Struct access with simple pointer */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *d) {
    int sum = 0;
    
    /* Simple register access to struct member */
    sum += d->count;
    
    /* Pointer to array member */
    int *p = d->values;
    
    /* Simple register access to array via pointer */
    sum += *p;
    
    /* Loop with increment */
    for (int i = 0; i < 10; i++) {
        sum += *p++;
    }
    
    return sum;
}

/* Test 7: Nested loops with simple access */
int test_nested_loops(int *arr, int rows, int cols) {
    int sum = 0;
    
    for (int i = 0; i < rows; i++) {
        int *row_ptr = arr + i * cols;
        
        /* Simple register access at start of each row */
        sum += *row_ptr;
        
        for (int j = 0; j < cols; j++) {
            /* Mix of simple and indexed access */
            if (j % 2 == 0) {
                sum += *row_ptr;  /* Simple */
            } else {
                sum += row_ptr[j]; /* Indexed */
            }
        }
    }
    
    return sum;
}

/* Main driver to ensure all code is executed */
int main(void) {
    int result = 0;
    
    /* Initialize test data */
    int local_arr[100];
    int arr2[50];
    int arr3[30];
    
    for (int i = 0; i < 100; i++) {
        local_arr[i] = i;
        if (i < 100) global_arr[i] = i * 2;
    }
    for (int i = 0; i < 50; i++) arr2[i] = i * 3;
    for (int i = 0; i < 30; i++) arr3[i] = i * 4;
    
    struct Data data;
    for (int i = 0; i < 20; i++) data.values[i] = i * 5;
    data.count = 42;
    
    /* Run all tests to generate various addressing patterns */
    result += test_simple_param_load(local_arr);
    result += test_mixed_addressing(local_arr, 20);
    result += test_global_access();
    result += test_conditional_simple_access(local_arr, 25);
    result += test_multiple_pointers(local_arr, arr2, arr3, 10);
    result += test_struct_access(&data);
    result += test_nested_loops(local_arr, 5, 10);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
