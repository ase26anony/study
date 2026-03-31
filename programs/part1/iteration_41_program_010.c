/* Test program to trigger auto-increment/decrement optimization block
 * Specifically targets lines 1352-1358 in auto-inc-dec.cc
 * Compile with: gcc -O2 -fno-inline -fno-ipa-pure-const -c test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array for testing */
int global_arr[100];

/* Test 1: Simple register indirect access with mixed patterns */
int test_simple_reg_access(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect - should trigger the uncovered block */
    sum += *base;  /* This creates mem_insn.reg1_is_const = true, reg1_val = 0 */
    
    /* Register + constant offset */
    sum += base[5];
    
    /* Loop with pointer increment */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    
    /* Another simple register access */
    int *simple_ptr = base + 10;
    sum += *simple_ptr;  /* Another candidate for simple register addressing */
    
    return sum;
}

/* Test 2: Function parameter used directly */
int load_param(int *p) {
    /* Simple register indirect from parameter */
    volatile int result = *p;  /* volatile prevents elimination */
    return result + p[0];  /* Both should be simple register accesses */
}

/* Test 3: Global array access via local pointer */
int test_global(void) {
    int *p = &global_arr[0];
    int sum = 0;
    
    /* Simple register indirect from global pointer */
    sum += p[0];  /* Should be XEXP(x,0) with zero offset */
    
    /* Mixed with offset */
    sum += p[10];
    
    /* Loop through global array */
    for (int i = 0; i < 50; i++) {
        sum += *p++;
    }
    
    return sum;
}

/* Test 4: Conditional simple access inside loop */
int test_conditional_access(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Complex addressing in loop */
        sum += ptr[i];
        
        /* Conditional simple register access */
        if (i % 3 == 0) {
            int *simple = &ptr[i];
            sum += *simple;  /* Simple register indirect */
        }
    }
    
    /* Final simple access */
    sum += *ptr;
    
    return sum;
}

/* Test 5: Struct access with simple pointer */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *d) {
    int sum = 0;
    int *p = d->values;
    
    /* Simple register indirect from struct member pointer */
    sum += *p;  /* Should trigger the block */
    
    /* Loop through struct array */
    for (int i = 0; i < 10; i++) {
        sum += *p++;
    }
    
    /* Another simple access */
    p = &d->values[5];
    sum += *p;
    
    return sum;
}

/* Test 6: Multiple simple accesses in same function */
int test_multiple_simple(int *a, int *b, int *c) {
    int sum = 0;
    
    /* Three separate simple register indirect accesses */
    sum += *a;
    sum += *b;
    sum += *c;
    
    /* Mix with some offset accesses */
    sum += a[1] + b[2] + c[3];
    
    return sum;
}

/* Test 7: Nested loops with simple access */
int test_nested_loops(int *matrix, int rows, int cols) {
    int sum = 0;
    
    for (int i = 0; i < rows; i++) {
        int *row_ptr = matrix + i * cols;
        
        /* Simple register access at start of each row */
        sum += *row_ptr;  /* Should be analyzed by find_inc_dec */
        
        for (int j = 0; j < cols; j++) {
            sum += row_ptr[j];
        }
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
    
    /* Test arrays */
    int arr1[50];
    int arr2[30];
    int arr3[40];
    
    for (int i = 0; i < 50; i++) arr1[i] = i * 2;
    for (int i = 0; i < 30; i++) arr2[i] = i * 3;
    for (int i = 0; i < 40; i++) arr3[i] = i * 4;
    
    struct Data data;
    for (int i = 0; i < 20; i++) data.values[i] = i * 5;
    data.count = 20;
    
    int matrix[5][5];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Run all tests */
    result += test_simple_reg_access(arr1, 20);
    result += load_param(arr2);
    result += test_global();
    result += test_conditional_access(arr3, 25);
    result += test_struct_access(&data);
    result += test_multiple_simple(arr1, arr2, arr3);
    result += test_nested_loops(&matrix[0][0], 5, 5);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
