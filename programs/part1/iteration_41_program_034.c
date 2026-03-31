/* test_auto_inc_dec.c
 * 
 * This program is designed to trigger the specific uncovered block in GCC's
 * auto-inc-dec.cc (lines 1352-1358) by creating memory operands with simple
 * register addressing (no offset) in various contexts.
 * 
 * The uncovered block handles the case where a memory address is just a
 * register (XEXP(x,0)) with zero offset, setting up mem_insn and calling
 * find_inc(true).
 * 
 * Compile with: gcc -O2 -fno-inline -fno-ipa-pure-const test_auto_inc_dec.c -o test
 * Or with: gcc -O3 -fno-inline -funroll-loops test_auto_inc_dec.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array to enable different access patterns */
volatile int global_arr[100] = {0};

/* Test 1: Simple parameter access with mixed patterns
 * This function uses:
 * 1. Simple register indirect: *base
 * 2. Register + constant offset: base[5]
 * 3. Loop with pointer increment: *ptr++
 */
int test_mixed_patterns(int *base, int n) {
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
    
    /* Another simple register indirect in the middle */
    sum += *base;
    
    return sum;
}

/* Test 2: Global array access via local pointer
 * Creates a simple pointer to global array and accesses it with zero offset
 */
int test_global_simple(void) {
    int *p = (int*)&global_arr[0];
    int sum = 0;
    
    /* Multiple simple register indirect accesses */
    sum += *p;
    sum += p[0];  /* Same as *p but different syntax */
    
    /* Mix with offset access */
    sum += p[10];
    
    return sum;
}

/* Test 3: Conditional simple access inside loop
 * The conditional creates different control flow paths where
 * simple register addressing might be analyzed
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
    }
    
    /* Another unconditional simple access */
    sum += *base;
    
    return sum;
}

/* Test 4: Pointer arithmetic that results in simple register
 * The compiler might see p = base + offset, then *p as simple register
 */
int test_pointer_arithmetic(int *base, int offset) {
    int *p = base + offset;
    
    /* After arithmetic, this is simple register indirect */
    int val1 = *p;
    
    /* Reset to simple base */
    p = base;
    int val2 = *p;
    
    return val1 + val2;
}

/* Test 5: Nested function with simple parameter access
 * Inlining might create different contexts for the auto-inc-dec pass
 */
static inline int load_simple(int *p) {
    /* Very simple - just return *p */
    return *p;
}

int test_nested_simple(int *base, int n) {
    int sum = 0;
    
    /* Call inline function - might create simple register access after inlining */
    sum += load_simple(base);
    
    for (int i = 0; i < n; i++) {
        sum += base[i];
    }
    
    /* Another simple access */
    sum += load_simple(base + n/2);
    
    return sum;
}

/* Test 6: Struct access with pointer
 * Accessing struct members through pointer can create simple register addressing
 */
struct Data {
    int a;
    int b;
    int c;
    int arr[10];
};

int test_struct_access(struct Data *d) {
    int sum = 0;
    
    /* Access through pointer - simple register for the struct address */
    sum += d->a;
    sum += d->b;
    
    /* Array access within struct */
    int *p = d->arr;
    sum += *p;  /* Simple register indirect */
    
    return sum;
}

/* Test 7: Multiple base pointers
 * Using different pointer variables increases chance of hitting the pattern
 */
int test_multiple_pointers(int *base1, int *base2, int n) {
    int sum = 0;
    
    /* Simple access from first pointer */
    sum += *base1;
    
    /* Simple access from second pointer */
    sum += *base2;
    
    /* Loop mixing both pointers */
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            sum += *base1;
            base1++;
        } else {
            sum += *base2;
            base2++;
        }
    }
    
    return sum;
}

/* Main driver to call all tests and ensure code isn't optimized away */
int main(void) {
    /* Initialize test data */
    int local_arr[100];
    int result = 0;
    
    /* Initialize arrays with some data */
    for (int i = 0; i < 100; i++) {
        local_arr[i] = i;
        global_arr[i] = i * 2;
    }
    
    /* Initialize struct */
    struct Data data = {0};
    data.a = 10;
    data.b = 20;
    data.c = 30;
    for (int i = 0; i < 10; i++) {
        data.arr[i] = i * 3;
    }
    
    /* Run all tests to maximize coverage opportunity */
    result += test_mixed_patterns(local_arr, 10);
    result += test_global_simple();
    result += test_conditional_access(local_arr, 20, 5);
    result += test_pointer_arithmetic(local_arr, 3);
    result += test_nested_simple(local_arr, 15);
    result += test_struct_access(&data);
    result += test_multiple_pointers(local_arr, local_arr + 50, 10);
    
    /* Use volatile to prevent optimization of final result */
    volatile int final_result = result;
    
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
