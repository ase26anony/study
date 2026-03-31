/* test_auto_inc_dec.c
 * Designed to trigger uncovered lines 1352-1358 in auto-inc-dec.cc
 * Compile with: gcc -O2 -fno-inline -fno-ipa-pure-const test_auto_inc_dec.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array for testing */
int global_arr[100] = {0};

/* Test 1: Simple parameter access with mixed addressing patterns */
int test_simple_param(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect access - should trigger the uncovered block */
    sum += *base;                     // Line 1: Simple register addressing
    
    /* Register + constant offset */
    sum += base[5];                   // Line 2: Offset addressing
    
    /* Loop with pointer increment */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;                // Line 3: Auto-increment pattern
    }
    
    /* Another simple register access after loop */
    sum += *base;                     // Line 4: Another simple register
    
    return sum;
}

/* Test 2: Local pointer to global array */
int test_global_access(void) {
    int *p = &global_arr[0];
    int sum = 0;
    
    /* Multiple simple register accesses */
    sum += p[0];                      // Line 1: p[0] is simple register
    sum += *p;                        // Line 2: *p is simple register
    
    /* Mixed with offset */
    sum += p[10];                     // Line 3: Offset
    
    /* Loop with conditional simple access */
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            sum += *p;                // Line 4: Conditional simple access
        }
        p++;
    }
    
    return sum;
}

/* Test 3: Array traversal with multiple pointer variables */
int test_multiple_pointers(int *arr, int n) {
    int *p1 = arr;
    int *p2 = &arr[n/2];
    int sum = 0;
    
    /* Simple accesses with different pointers */
    sum += *p1;                       // Line 1: Simple via p1
    sum += *p2;                       // Line 2: Simple via p2
    
    /* Loop with both pointers */
    for (int i = 0; i < n/2; i++) {
        sum += *p1++;                 // Line 3: Auto-increment p1
        sum += *p2++;                 // Line 4: Auto-increment p2
    }
    
    /* Final simple access */
    sum += *arr;                      // Line 5: Simple via parameter
    
    return sum;
}

/* Test 4: Struct access with pointer */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *data) {
    int *ptr = data->values;
    int sum = 0;
    
    /* Simple struct member access */
    sum += *ptr;                      // Line 1: Simple via ptr
    
    /* Loop through struct array */
    for (int i = 0; i < data->count && i < 20; i++) {
        sum += *ptr;                  // Line 2: Simple in loop
        ptr++;
    }
    
    /* Access via different expression */
    sum += data->values[0];           // Line 3: Another simple
    
    return sum;
}

/* Test 5: Volatile pointer to prevent optimization */
int test_volatile_access(volatile int *base, int n) {
    int sum = 0;
    volatile int *p = base;
    
    /* Volatile simple access */
    sum += *p;                        // Line 1: Volatile simple
    
    /* Mix volatile and non-volatile */
    int *reg_ptr = (int *)p;
    sum += *reg_ptr;                  // Line 2: Non-volatile simple
    
    /* Loop with volatile */
    for (int i = 0; i < n; i++) {
        sum += *p;                    // Line 3: Volatile in loop
        p++;
    }
    
    return sum;
}

/* Test 6: Nested loops with simple access */
int test_nested_loops(int *arr, int rows, int cols) {
    int sum = 0;
    
    for (int i = 0; i < rows; i++) {
        int *row_ptr = arr + i * cols;
        
        /* Simple access at start of row */
        sum += *row_ptr;              // Line 1: Simple per row
        
        for (int j = 0; j < cols; j++) {
            sum += *row_ptr;          // Line 2: Simple in inner loop
            row_ptr++;
        }
    }
    
    return sum;
}

/* Main driver to execute all tests */
int main(void) {
    int result = 0;
    
    /* Initialize test data */
    int test_arr[100];
    for (int i = 0; i < 100; i++) {
        test_arr[i] = i + 1;
        global_arr[i] = i * 2;
    }
    
    struct Data data;
    for (int i = 0; i < 20; i++) {
        data.values[i] = i * 3;
    }
    data.count = 20;
    
    /* Run all tests */
    result += test_simple_param(test_arr, 10);
    result += test_global_access();
    result += test_multiple_pointers(test_arr, 20);
    result += test_struct_access(&data);
    result += test_volatile_access(test_arr, 5);
    result += test_nested_loops(test_arr, 5, 10);
    
    printf("Final result: %d\n", result);
    
    /* Verify result is non-zero */
    if (result != 0) {
        printf("Tests completed successfully.\n");
        return 0;
    } else {
        printf("Unexpected zero result.\n");
        return 1;
    }
}
