/* test_auto_inc_dec.c
 * 
 * This program is designed to trigger the specific uncovered block in
 * GCC's auto-inc-dec.cc (lines 1352-1358) by creating memory access
 * patterns that cause the compiler to analyze simple register addressing
 * modes during RTL optimization.
 *
 * The key pattern needed: memory operands where the address is just a
 * register with zero offset (e.g., *p or p[0]).
 *
 * Compile with: gcc -O2 -fno-inline -fno-ipa-pure-const test_auto_inc_dec.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array to enable pointer-to-global patterns */
int global_array[256];

/* Prevent compiler from optimizing away computations */
static volatile int sink;

/* Test 1: Simple parameter access with mixed addressing patterns
 * This creates a simple register access (*base) alongside offset accesses
 * and a loop with pointer increment.
 */
int test_mixed_addressing(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect access - target for uncovered block */
    sum += *base;           /* This should generate mem: (reg) */
    
    /* Register + constant offset */
    sum += base[5];         /* mem: (plus (reg) (const_int)) */
    
    /* Loop with pointer increment - encourages auto-inc optimization */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;      /* Post-increment pattern */
    }
    
    /* Another simple register access after loop */
    sum += *base;           /* Another chance for simple reg access */
    
    return sum;
}

/* Test 2: Local pointer to global with zero offset access
 * Creates a local pointer that points to global, then uses p[0]
 */
int test_global_access(void) {
    int *p = &global_array[0];
    int sum = 0;
    
    /* Multiple simple register accesses */
    sum += p[0];            /* Should be (mem (reg)) with offset 0 */
    sum += *p;              /* Another simple reg access */
    
    /* Mix with offset to ensure find_inc_dec is called */
    sum += p[10];
    sum += p[20];
    
    return sum;
}

/* Test 3: Conditional simple access inside loop
 * The compiler may analyze the simple access differently in a conditional
 */
int test_conditional_access(int *arr, int n, int threshold) {
    int sum = 0;
    int *simple_ptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Complex addressing in loop */
        sum += arr[i * 2];
        
        /* Conditional simple register access */
        if (arr[i] > threshold) {
            sum += *simple_ptr;  /* Simple reg access inside condition */
        }
        
        /* Sometimes use offset */
        if (i % 3 == 0) {
            sum += simple_ptr[0];  /* Another simple access */
        }
    }
    
    return sum;
}

/* Test 4: Pointer arithmetic that results in simple register access
 * Start with offset, then reset to zero offset
 */
int test_pointer_reset(int *base, int n) {
    int sum = 0;
    int *p = base + n;  /* Start with offset */
    
    /* Work backwards to beginning */
    for (int i = n; i > 0; i--) {
        sum += *(--p);  /* Pre-decrement pattern */
    }
    
    /* After loop, p == base, so *p is simple register access */
    sum += *p;          /* Target for uncovered block */
    
    /* Also test p[0] */
    sum += p[0];        /* Another simple access */
    
    return sum;
}

/* Test 5: Multiple simple accesses in sequence
 * Forces multiple analyses of the same pattern
 */
int test_multiple_simple_accesses(int *p1, int *p2, int *p3) {
    int sum = 0;
    
    /* Series of simple register accesses */
    sum += *p1;
    sum += *p2;
    sum += *p3;
    
    /* Zero offset array accesses */
    sum += p1[0];
    sum += p2[0];
    sum += p3[0];
    
    /* Small loop with mixed patterns */
    for (int i = 0; i < 4; i++) {
        sum += p1[i];      /* Offset access */
        if (i == 0) {
            sum += *p2;    /* Simple access inside loop */
        }
    }
    
    return sum;
}

/* Driver function that calls all tests and ensures code isn't eliminated */
int main(void) {
    int result = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    /* Local test arrays */
    int local_array[100];
    int local_array2[50];
    int local_array3[30];
    
    for (int i = 0; i < 100; i++) local_array[i] = i * 2;
    for (int i = 0; i < 50; i++) local_array2[i] = i * 5;
    for (int i = 0; i < 30; i++) local_array3[i] = i * 7;
    
    /* Run all tests with different patterns */
    result += test_mixed_addressing(local_array, 10);
    result += test_global_access();
    result += test_conditional_access(local_array, 20, 15);
    result += test_pointer_reset(local_array2, 25);
    result += test_multiple_simple_accesses(local_array, local_array2, local_array3);
    
    /* Use volatile sink to prevent dead code elimination */
    sink = result;
    
    /* Print result to ensure all code executes */
    printf("Result: %d\n", result);
    
    return 0;
}
