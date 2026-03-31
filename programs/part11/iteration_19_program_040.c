/* caller-save-test.c
 * Designed to trigger GCC's caller-save instruction reordering logic
 * Specifically targets lines 905-913 in caller-save.cc
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

/* Test function with complex register usage across multiple calls */
__attribute__((noinline, optimize("no-crossjumping", "no-sibling-calls")))
int test_caller_save(int seed) {
    /* Declare multiple local variables to create register pressure */
    int a = seed + 1;
    int b = seed * 2;
    int c = seed ^ 0x55AA55AA;
    int d = seed - 12345;
    int e = seed * seed;
    int f = seed | 0x00FF00FF;
    
    /* Complex arithmetic creating dependencies, forcing values to stay in registers */
    a = b * c + d;          /* Uses b, c, d -> result in a */
    e = a ^ f;              /* Uses a, f -> result in e */
    b = (c << 3) | (d >> 2); /* Uses c, d -> result in b */
    f = (e * a) + (b ^ c);  /* Uses e, a, b, c -> result in f */
    
    /* First call - uses some registers, clobbers call-clobbered regs */
    int r1 = helper1(a, b);
    c = r1 * d;             /* Result used immediately, must be in register */
    
    /* More arithmetic between calls */
    d = (c ^ e) + (f & a);
    e = (b * 7) - (d / 3);
    
    /* Second call with more arguments */
    int r2 = helper2(c, d, e);
    a = r2 + f;             /* Result used immediately */
    
    /* Complex computation creating more register dependencies */
    b = (a << 2) | (c >> 1);
    f = (d * e) ^ (a & b);
    
    /* Third call with even more arguments */
    int r3 = helper3(a, b, c, d);
    e = r3 * f;             /* Result used immediately */
    
    /* More arithmetic to keep values live */
    c = (b + e) * (f - a);
    d = (e ^ f) | (a & b);
    
    /* Volatile function pointer call - compiler can't optimize this away */
    int r4 = volatile_func(a, b, c, d, e);
    
    /* Final computation using all values - ensures nothing is dead */
    int result = (a * b) + (c * d) - (e * f) + r1 - r2 + r3 * r4;
    
    /* Additional arithmetic after the volatile call */
    /* This creates a scenario where save/restore might be needed AFTER the call */
    int post_call = (result & 0xFF) + (r4 >> 8);
    result = result ^ post_call;
    
    return result;
}

int main() {
    int total = 0;
    
    /* Call test function multiple times with different seeds
     * to prevent constant propagation and ensure actual execution */
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(i);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
