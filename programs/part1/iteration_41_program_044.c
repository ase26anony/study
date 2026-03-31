/* test-auto-inc-dec.c
 * 
 * This program is designed to trigger the specific uncovered block in
 * GCC's auto-inc-dec.cc (lines 1352-1358) by creating memory operands
 * with simple register addressing (register + zero offset) that will
 * be analyzed by the find_inc_dec optimization pass.
 * 
 * Compile with: gcc -O2 -fno-inline -fno-ipa-pure-const test-auto-inc-dec.c -o test
 * Or with: gcc -O3 -fno-inline -funroll-loops test-auto-inc-dec.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array to enable pointer-to-global patterns */
int global_array[256];

/* Test 1: Simple parameter access with mixed addressing patterns
 * This function uses:
 * - Simple register indirect: *base
 * - Register + constant offset: base[5]
 * - Loop with pointer increment: *ptr++
 */
int test_mixed_addressing(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect - should trigger the uncovered block */
    sum += *base;
    
    /* Register + constant offset */
    sum += base[5];
    
    /* Loop with pointer increment - encourages auto-inc optimization */
    int *ptr = base;
    for (int i = 0; i < n && i < 100; i++) {
        sum += *ptr++;
    }
    
    /* Another simple register indirect in the middle */
    sum += *base;
    
    return sum;
}

/* Test 2: Global array access via local pointer
 * Creates a simple pointer to global with zero offset access
 */
int test_global_access(void) {
    int *p = &global_array[0];
    int sum = 0;
    
    /* Simple register indirect from global pointer */
    sum += p[0];
    
    /* Mixed with offset access */
    sum += p[10];
    
    /* Loop through global array */
    for (int i = 0; i < 50; i++) {
        sum += *p++;
    }
    
    /* Reset and another simple access */
    p = &global_array[0];
    sum += *p;
    
    return sum;
}

/* Test 3: Conditional simple access inside loop
 * The compiler may analyze the simple access in the conditional path
 */
int test_conditional_access(int *arr, int n, int flag) {
    int sum = 0;
    int *simple_ptr = arr;
    
    for (int i = 0; i < n; i++) {
        if (flag) {
            /* Simple register indirect inside conditional */
            sum += *simple_ptr;
        } else {
            sum += arr[i];
        }
        
        /* Also increment the pointer sometimes */
        if (i % 2 == 0) {
            simple_ptr++;
        }
    }
    
    /* Final simple access */
    sum += *arr;
    
    return sum;
}

/* Test 4: Multiple simple pointers with zero offset
 * Uses several pointer variables that all get simple register addressing
 */
int test_multiple_pointers(int *base1, int *base2, int n) {
    int sum = 0;
    int *p1 = base1;
    int *p2 = base2;
    
    /* Simple accesses from different pointers */
    sum += *p1;
    sum += *p2;
    
    /* Mixed with offset accesses */
    sum += p1[3];
    sum += p2[7];
    
    /* Loops with each pointer */
    for (int i = 0; i < n && i < 50; i++) {
        sum += *p1++;
    }
    
    for (int i = 0; i < n && i < 30; i++) {
        sum += *p2++;
    }
    
    /* Final simple accesses */
    p1 = base1;
    p2 = base2;
    sum += *p1;
    sum += *p2;
    
    return sum;
}

/* Test 5: Struct access with pointer
 * Tests if struct member access through pointer creates simple addressing
 */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *data) {
    int sum = 0;
    int *ptr = data->values;
    
    /* Simple register indirect to struct member array */
    sum += *ptr;
    
    /* Offset access */
    sum += ptr[5];
    
    /* Loop through struct array */
    for (int i = 0; i < 10; i++) {
        sum += *ptr++;
    }
    
    /* Another simple access */
    ptr = data->values;
    sum += *ptr;
    
    return sum;
}

/* Main driver that calls all test functions
 * Ensures code isn't optimized away by using results
 */
int main(void) {
    /* Initialize test data */
    int local_array[256];
    int local_array2[128];
    struct Data my_data;
    
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
        if (i < 256) local_array[i] = i * 2 + 1;
        if (i < 128) local_array2[i] = i * 5 + 2;
    }
    
    for (int i = 0; i < 20; i++) {
        my_data.values[i] = i * 7 + 3;
    }
    my_data.count = 20;
    
    /* Call all test functions with various parameters */
    int result = 0;
    
    result += test_mixed_addressing(local_array, 50);
    result += test_global_access();
    result += test_conditional_access(local_array, 40, 1);
    result += test_multiple_pointers(local_array, local_array2, 25);
    result += test_struct_access(&my_data);
    
    /* Use volatile to prevent optimization of final result */
    volatile int final_result = result;
    
    printf("Result: %d\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}
