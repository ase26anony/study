/* caller-save-test.c
 * Designed to trigger specific instruction reordering logic in GCC's
 * caller-save optimization (lines 905-913 of caller-save.cc)
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Non-inlineable helper functions to force actual call instructions */
__attribute__((noinline)) int helper1(int a, int b) {
    volatile int result = a + b;
    return result;
}

__attribute__((noinline)) int helper2(int a, int b, int c) {
    volatile int result = a * b - c;
    return result;
}

__attribute__((noinline)) int helper3(int a, int b, int c, int d) {
    volatile int result = (a ^ b) | (c & d);
    return result;
}

__attribute__((noinline)) int helper4(int a, int b) {
    volatile int result = (a << 3) | (b >> 2);
    return result;
}

/* Volatile function pointer to create unpredictable call site */
typedef int (*func_ptr_t)(int, int);
volatile func_ptr_t volatile_func = helper1;

/* Main test function with register-intensive computations between calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed + 1;
    int b = seed * 2;
    int c = seed ^ 0x55AA55AA;
    int d = seed - 100;
    int e = seed | 0x00FF00FF;
    int f = seed & 0xFF00FF00;
    
    /* First computation creating dependencies */
    a = b * c + d;
    b = a ^ f;
    c = d | e;
    
    /* First call - clobbers call-clobbered registers */
    int r1 = helper1(a, b);
    
    /* More computations keeping values live across calls */
    d = r1 * e + 12345;
    e = (c ^ d) * 17;
    f = a + b + c + d;
    
    /* Second call with different arguments */
    int r2 = helper2(b, c, d);
    
    /* Complex computation chain */
    a = (r2 << 4) | (e >> 2);
    b = (d * 3) ^ (f + 1);
    c = helper3(a, b, r1, r2);
    
    /* Volatile function pointer call - compiler can't optimize this away */
    int r3 = volatile_func(c, d);
    
    /* More computations after volatile call */
    d = e * f - r3;
    e = (a & b) | (c ^ d);
    
    /* Third call */
    int r4 = helper4(d, e);
    
    /* Final computation using all values to prevent dead code elimination */
    int result = (a + b) ^ (c - d) | (e & f) + (r1 * r2) - (r3 | r4);
    
    /* Mix in all variables to ensure they're used */
    result += (a & 0xFF) + (b & 0xFF00) + (c & 0xFF0000) + 
              (d & 0xFF000000) + (e & 0xF) + (f & 0xF0);
    
    return result;
}

int main() {
    int total = 0;
    
    /* Call test function multiple times with different seeds
     * to prevent constant propagation */
    for (int i = 0; i < 100; i++) {
        int result = test_caller_save(i);
        total += result;
        
        /* Use volatile to prevent loop optimization */
        volatile int dummy = result;
        (void)dummy;
    }
    
    printf("Total: %d\n", total);
    
    /* Additional test with more complex patterns */
    for (int i = 0; i < 50; i++) {
        /* Create varying call patterns */
        volatile_func = (i % 3 == 0) ? helper1 : 
                       (i % 3 == 1) ? helper2 : 
                       (int (*)(int, int))helper4;
        
        int result = test_caller_save(i * 7 + 123);
        total ^= result;
    }
    
    printf("Final result: %d\n", total);
    return 0;
}
