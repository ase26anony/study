/* test_auto_inc_dec.c
 * 
 * This program is designed to trigger the specific uncovered block in GCC's
 * auto-inc-dec.cc optimization pass. The block handles memory operands with
 * simple register addressing (no offset) and attempts to combine them with
 * auto-increment/decrement opportunities.
 *
 * Compile with: gcc -O2 -fno-inline -fno-ipa-pure-const test_auto_inc_dec.c -o test
 * Or with:       gcc -O3 -fno-inline -funroll-loops test_auto_inc_dec.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array to enable pointer-to-global patterns */
int global_array[256];

/* Test 1: Simple parameter access with mixed addressing patterns
 * This function includes:
 * - Simple register indirect: *base
 * - Register + constant offset: base[5]
 * - Loop with pointer increment
 */
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
    
    /* Another simple register indirect after loop */
    sum += *base;
    
    return sum;
}

/* Test 2: Global array access via local pointer
 * Creates a simple pointer to global array and accesses it
 */
int test_global_access(void) {
    int *p = &global_array[0];
    int sum = 0;
    
    /* Simple register indirect from global pointer */
    sum += *p;
    
    /* Mixed with offset access */
    sum += p[10];
    
    /* Loop through part of global array */
    for (int i = 0; i < 50; i++) {
        sum += *p++;
    }
    
    return sum;
}

/* Test 3: Conditional simple access inside loop
 * The conditional creates different code paths where simple
 * register addressing might be analyzed
 */
int test_conditional_access(int *base, int n, int threshold) {
    int sum = 0;
    int *simple_ptr = base;
    
    for (int i = 0; i < n; i++) {
        if (i > threshold) {
            /* Simple register indirect inside conditional */
            sum += *simple_ptr;
        } else {
            /* Offset access */
            sum += base[i];
        }
        
        /* Pointer increment in some iterations */
        if (i % 2 == 0) {
            simple_ptr++;
        }
    }
    
    /* Final simple access */
    sum += *base;
    
    return sum;
}

/* Test 4: Struct access with pointer
 * Tests if struct member access via pointer triggers the pattern
 */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *data) {
    int sum = 0;
    int *ptr = data->values;
    
    /* Simple register indirect to struct member */
    sum += *ptr;
    
    /* Loop through struct array */
    for (int i = 0; i < data->count && i < 20; i++) {
        sum += *ptr++;
    }
    
    /* Another simple access */
    sum += *(data->values);
    
    return sum;
}

/* Test 5: Multiple simple accesses in sequence
 * Creates several simple register indirect accesses in a row
 */
int test_multiple_simple_accesses(int *base) {
    int *p1 = base;
    int *p2 = base + 10;
    int *p3 = base + 20;
    
    int sum = 0;
    
    /* Sequence of simple register indirect accesses */
    sum += *p1;
    sum += *p2;
    sum += *p3;
    
    /* Mix with some offset accesses */
    sum += p1[1];
    sum += p2[2];
    sum += p3[3];
    
    /* More simple accesses */
    sum += *p1;
    sum += *p2;
    
    return sum;
}

/* Test 6: Volatile pointer to prevent optimization
 * Uses volatile to ensure memory accesses aren't eliminated
 */
int test_volatile_access(volatile int *base, int n) {
    int sum = 0;
    volatile int *ptr = base;
    
    /* Simple register indirect with volatile */
    sum += *ptr;
    
    /* Loop with volatile pointer */
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    
    /* Final simple volatile access */
    sum += *base;
    
    return sum;
}

/* Main driver function
 * Initializes test data and calls all test functions
 * Returns a checksum that depends on all memory accesses
 */
int main(void) {
    int result = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    /* Local test arrays */
    int local_array[100];
    for (int i = 0; i < 100; i++) {
        local_array[i] = i * 2 + 5;
    }
    
    int medium_array[50];
    for (int i = 0; i < 50; i++) {
        medium_array[i] = i * 7 - 3;
    }
    
    /* Initialize struct */
    struct Data data;
    for (int i = 0; i < 20; i++) {
        data.values[i] = i * 11 + 7;
    }
    data.count = 20;
    
    /* Call all test functions with different patterns */
    result += test_mixed_addressing(local_array, 20);
    result += test_global_access();
    result += test_conditional_access(medium_array, 30, 10);
    result += test_struct_access(&data);
    result += test_multiple_simple_accesses(local_array);
    result += test_volatile_access(local_array, 15);
    
    /* Also test with NULL pointer check pattern */
    int *nullable_ptr = local_array;
    if (nullable_ptr != NULL) {
        result += *nullable_ptr;  /* Simple register indirect */
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result checksum: %d\n", result);
    
    return (result > 0) ? 0 : 1;
}
