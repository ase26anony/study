/* caller-save-test.c
 * Designed to trigger specific instruction reordering logic in GCC's caller-save pass
 * Lines 905-913 of caller-save.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Non-inline helper functions to force actual call instructions */
__attribute__((noinline)) int helper1(int a, int b) {
    return a + b;
}

__attribute__((noinline)) int helper2(int a, int b, int c) {
    return a * b - c;
}

__attribute__((noinline)) int helper3(int a, int b, int c, int d) {
    return (a ^ b) | (c & d);
}

__attribute__((noinline)) int helper4(int a, int b, int c, int d, int e) {
    return (a * b) + (c * d) - e;
}

/* Volatile function pointer to create unpredictable call site */
static int (*volatile volatile_func)(int, int, int, int, int) = helper4;

/* Main test function with complex register usage across calls */
__attribute__((noinline)) int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed + 1;
    int b = seed * 2;
    int c = seed ^ 0x55AA55AA;
    int d = seed - 100;
    int e = seed * seed;
    int f = seed | 0x12345678;
    
    /* Complex arithmetic to keep values live in registers */
    a = b * c + d;
    e = a ^ f;
    d = (b << 3) | (c >> 2);
    
    /* First call - uses some registers, clobbers others */
    int r1 = helper1(a, b);
    c = r1 * d + e;
    
    /* More arithmetic between calls */
    f = (a & b) | (c ^ d);
    b = e * 2 - f;
    
    /* Second call with more arguments */
    int r2 = helper2(a, b, c);
    e = r2 ^ d;
    a = b + c * 2;
    
    /* Third call with even more arguments */
    int r3 = helper3(a, b, c, d);
    f = r3 & e;
    d = a * b - c;
    
    /* Volatile function pointer call - compiler can't optimize this away */
    int r4 = volatile_func(a, b, c, d, e);
    
    /* More arithmetic after the volatile call */
    c = r4 + f;
    b = d ^ e;
    a = c * 2 - b;
    
    /* Fourth call - result used in final computation */
    int r5 = helper4(a, b, c, d, e);
    
    /* Final complex computation using all values */
    int result = (a + b) * (c - d) ^ (e & f) | r5;
    
    /* Ensure all variables are used to prevent elimination */
    result += (r1 ^ r2) | (r3 & r4);
    
    return result;
}

/* Another test variant to create different patterns */
__attribute__((noinline)) int test_caller_save2(int seed) {
    int x = seed * 3;
    int y = seed + 777;
    int z = seed ^ 0x87654321;
    int w = seed / 2;
    int v = seed % 100;
    
    /* Chain of calls with arithmetic in between */
    x = helper1(x, y);
    y = x * z + w;
    
    z = helper2(y, z, w);
    w = z ^ v;
    
    v = helper3(w, v, x, y);
    x = v | z;
    
    /* Volatile call near the end */
    y = volatile_func(x, y, z, w, v);
    
    /* Final computation that uses result from volatile call */
    z = helper4(y, x, w, v, z);
    
    return x + y - z * w ^ v;
}

int main() {
    int total = 0;
    
    /* Call test functions in a loop with different seeds
     * to prevent constant propagation and create varying
     * register allocation patterns
     */
    for (int i = 0; i < 100; i++) {
        total ^= test_caller_save(i);
        total += test_caller_save2(i * 7);
        
        /* Occasionally change the volatile function pointer
         * to create different call patterns
         */
        if (i % 17 == 0) {
            volatile_func = (int (*)(int, int, int, int, int))helper3;
        } else if (i % 23 == 0) {
            volatile_func = (int (*)(int, int, int, int, int))helper2;
        } else {
            volatile_func = helper4;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
