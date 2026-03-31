/* test_auto_inc_dec.c
 * Designed to trigger uncovered lines 1352-1358 in auto-inc-dec.cc
 * Compile with: gcc -O2 -fno-inline -fno-ipa-pure-const test_auto_inc_dec.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array to ensure memory accesses are real */
volatile int global_array[256];

/* Test 1: Simple parameter access with mixed addressing */
int test_simple_param_access(int *base, int n) {
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
    sum += p[20];
    
    /* Loop with conditional simple access */
    for (int i = 0; i < 50; i++) {
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
    int *p3 = arr + size/4;
    
    /* Simple accesses from different pointers */
    sum += *p1;                      /* Line 8 */
    sum += *p2;                      /* Line 9 */
    sum += *p3;                      /* Line 10 */
    
    /* p1 auto-increment loop */
    for (int i = 0; i < size/2; i++) {
        sum += *p1++;
    }
    
    /* Simple access after loop */
    sum += *p2;                      /* Line 11 */
    
    return sum;
}

/* Test 4: Struct access with pointer */
struct Data {
    int values[16];
    int count;
};

int test_struct_access(struct Data *data) {
    int sum = 0;
    int *ptr = data->values;
    
    /* Simple register access to struct member */
    sum += *ptr;                     /* Line 12 */
    
    /* Access with offset */
    sum += ptr[4];
    sum += ptr[8];
    
    /* Loop through struct array */
    for (int i = 0; i < data->count && i < 16; i++) {
        sum += *ptr;                 /* Line 13: Simple in loop */
        ptr++;
    }
    
    return sum;
}

/* Test 5: Function with only simple register access */
int load_simple(int *p) {
    return *p;                       /* Line 14: Pure simple register */
}

/* Test 6: Complex scenario with nested loops */
int test_complex_pattern(int *base, int rows, int cols) {
    int sum = 0;
    
    for (int r = 0; r < rows; r++) {
        int *row_ptr = base + r * cols;
        
        /* Simple access at start of each row */
        sum += *row_ptr;             /* Line 15 */
        
        /* Process row with auto-increment */
        for (int c = 0; c < cols; c++) {
            sum += *row_ptr++;
        }
        
        /* Simple access at end */
        row_ptr = base + r * cols + cols/2;
        sum += *row_ptr;             /* Line 16 */
    }
    
    return sum;
}

/* Main driver to ensure all code is executed */
int main(void) {
    int result = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    /* Local test arrays */
    int local_array[100];
    int local_array2[200];
    
    for (int i = 0; i < 100; i++) {
        local_array[i] = i * 2;
    }
    
    for (int i = 0; i < 200; i++) {
        local_array2[i] = i * 5;
    }
    
    /* Initialize struct */
    struct Data data;
    for (int i = 0; i < 16; i++) {
        data.values[i] = i * 7;
    }
    data.count = 16;
    
    /* Run all tests to generate various addressing patterns */
    result += test_simple_param_access(local_array, 20);
    result += test_global_access();
    result += test_multiple_pointers(local_array2, 100);
    result += test_struct_access(&data);
    result += load_simple(local_array);
    result += test_complex_pattern(local_array, 5, 10);
    
    /* Add some volatile operations to prevent optimization */
    volatile int dummy = result;
    
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
