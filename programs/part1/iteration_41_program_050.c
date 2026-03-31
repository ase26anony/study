/* test_auto_inc_dec.c
 * 
 * This program is designed to trigger the specific uncovered block in
 * GCC's auto-inc-dec.cc (lines 1352-1358) by creating memory operands
 * with simple register addressing (no offset) that will be analyzed
 * by the find_inc_dec optimization pass.
 *
 * Compile with: gcc -O2 -fno-inline -fno-ipa-pure-const test_auto_inc_dec.c -o test
 * Or with: gcc -O3 -fno-inline -funroll-loops test_auto_inc_dec.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array to enable multiple access patterns */
int global_arr[256];

/* Test 1: Simple parameter access with mixed patterns
 * This function includes:
 * - Simple register indirect: *p
 * - Register + constant offset: p[5]
 * - Loop with pointer increment
 */
int test_mixed_patterns(int *p, int n) {
    int sum = 0;
    
    /* Simple register indirect - should trigger the uncovered block */
    sum += *p;
    
    /* Register + constant offset */
    sum += p[5];
    
    /* Loop with pointer increment - encourages auto-inc optimization */
    int *ptr = p;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    
    /* Another simple register indirect after loop */
    sum += *p;
    
    return sum;
}

/* Test 2: Global array access via local pointer
 * Uses a local pointer initialized to global array
 */
int test_global_access(void) {
    int *p = &global_arr[0];
    int sum = 0;
    
    /* Simple register indirect from global */
    sum += *p;
    
    /* Mixed offset access */
    sum += p[10];
    sum += p[20];
    
    /* Loop through part of global array */
    for (int i = 0; i < 50; i++) {
        sum += *p++;
    }
    
    return sum;
}

/* Test 3: Conditional simple access inside loop
 * The conditional creates different code paths
 */
int test_conditional_access(int *base, int n, int threshold) {
    int sum = 0;
    int *simple_ptr = base;
    
    for (int i = 0; i < n; i++) {
        if (i < threshold) {
            /* Simple register indirect inside condition */
            sum += *simple_ptr;
        } else {
            /* Offset access */
            sum += base[i];
        }
    }
    
    /* Another simple access outside loop */
    sum += *base;
    
    return sum;
}

/* Test 4: Struct access with pointer
 * Creates different memory access patterns
 */
struct Data {
    int values[32];
    int count;
};

int test_struct_access(struct Data *data) {
    int sum = 0;
    int *ptr = data->values;
    
    /* Simple register indirect to struct member */
    sum += *ptr;
    
    /* Access with offset */
    sum += ptr[4];
    sum += ptr[8];
    
    /* Loop through struct array */
    for (int i = 0; i < data->count && i < 32; i++) {
        sum += *ptr++;
    }
    
    return sum;
}

/* Test 5: Multiple simple pointers
 * Uses several pointers to create more opportunities
 */
int test_multiple_pointers(int *arr, int n) {
    int *p1 = arr;
    int *p2 = &arr[10];
    int *p3 = &arr[20];
    int sum = 0;
    
    /* Multiple simple register indirects */
    sum += *p1;
    sum += *p2;
    sum += *p3;
    
    /* Mixed patterns */
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            sum += *p1;
            p1++;
        } else {
            sum += p2[i % 5];
        }
    }
    
    return sum;
}

/* Test 6: Volatile pointer to prevent optimization
 * Ensures memory accesses aren't eliminated
 */
int test_volatile_access(volatile int *vp, int n) {
    int sum = 0;
    
    /* Simple register indirect with volatile */
    sum += *vp;
    
    /* Loop with volatile pointer */
    for (int i = 0; i < n; i++) {
        sum += *vp++;
    }
    
    /* Another simple access */
    sum += *vp;
    
    return sum;
}

/* Main driver function
 * Initializes data and calls all test functions
 * Returns a checksum to prevent dead code elimination
 */
int main(void) {
    int result = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_arr[i] = i;
    }
    
    /* Local test arrays */
    int arr1[100];
    int arr2[200];
    struct Data data;
    
    for (int i = 0; i < 100; i++) {
        arr1[i] = i * 2;
    }
    
    for (int i = 0; i < 200; i++) {
        arr2[i] = i * 3;
    }
    
    for (int i = 0; i < 32; i++) {
        data.values[i] = i * 5;
    }
    data.count = 32;
    
    /* Call all test functions with different parameters */
    result += test_mixed_patterns(arr1, 50);
    result += test_global_access();
    result += test_conditional_access(arr2, 100, 30);
    result += test_struct_access(&data);
    result += test_multiple_pointers(arr1, 25);
    result += test_volatile_access(arr2, 10);
    
    /* Print result to prevent optimization */
    printf("Result checksum: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
