/* reload_test.c - Test program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to enable complex addressing modes */
volatile int global_array[10000];
volatile long long global_big_array[2000];
volatile double global_double_array[1000];

/* Non-inline function with many arguments to force register pressure */
int __attribute__((noinline)) many_args_function(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, double f3, void* p1, void* p2)
{
    /* Use all arguments to prevent optimization */
    volatile int sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    volatile double fsum = f1 + f2 + f3;
    return (int)(sum + fsum + (intptr_t)p1 + (intptr_t)p2);
}

/* Test 1: Extreme register pressure with many live scalars */
int __attribute__((noinline)) test_register_pressure(volatile int a, volatile int b, 
                                                    volatile int c, volatile int d)
{
    /* Force many independent computations that must stay live */
    int t1 = a + b;
    int t2 = c - d;
    int t3 = a * b;
    int t4 = c / (d ? d : 1);
    int t5 = t1 ^ t2;
    int t6 = t3 | t4;
    int t7 = t5 & t6;
    int t8 = t1 << 2;
    int t9 = t2 >> 1;
    int t10 = t3 + t4;
    int t11 = t5 - t6;
    int t12 = t7 * t8;
    int t13 = t9 / (t10 ? t10 : 1);
    int t14 = t11 ^ t12;
    int t15 = t13 | t14;
    int t16 = t1 + t15;
    int t17 = t2 - t16;
    int t18 = t3 * t17;
    int t19 = t4 / (t18 ? t18 : 1);
    int t20 = t5 ^ t19;
    
    /* Use all temporaries in final computation */
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
           t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20;
}

/* Test 2: Complex addressing modes requiring reloads */
int __attribute__((noinline)) test_complex_addressing(volatile int idx1, volatile int idx2,
                                                     volatile int idx3)
{
    /* Large immediate offset - may require reload */
    int val1 = global_array[4096];
    
    /* Variable index with computation - double register indirect */
    int val2 = global_array[idx1 * 3 + idx2];
    
    /* Complex nested computation in index */
    int val3 = global_array[(idx1 + idx2) * (idx3 ? idx3 : 1) - 17];
    
    /* Multi-word type with potential alignment issues */
    long long ll_val = global_big_array[idx1];
    double d_val = global_double_array[idx2];
    
    /* Mixed type computations forcing register class moves */
    return val1 + val2 + val3 + (int)ll_val + (int)d_val;
}

/* Test 3: Inline assembly with many clobbered registers */
int __attribute__((noinline)) test_asm_clobber(volatile int x, volatile int y)
{
    int result;
    
    /* Computation before asm - values must be preserved */
    int a = x * 3;
    int b = y / 2;
    int c = a + b;
    int d = a - b;
    int e = c * d;
    
    /* Inline asm that clobbers many registers */
    __asm__ volatile (
        "# Dummy assembly\n\t"
        "nop"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "cc", "memory"
    );
    
    /* More computations after asm - forcing reloads */
    int f = result + a;
    int g = f * b;
    int h = g - c;
    
    return h + d + e;
}

/* Test 4: Function call with many arguments */
int __attribute__((noinline)) test_many_args(volatile int base)
{
    /* Create many argument values with dependencies */
    int a1 = base + 1;
    int a2 = base * 2;
    int a3 = base - 3;
    int a4 = base / 4;
    int a5 = base ^ 5;
    int a6 = base | 6;
    int a7 = base & 7;
    int a8 = base << 1;
    int a9 = base >> 2;
    int a10 = base + 10;
    
    double f1 = (double)base * 1.1;
    double f2 = (double)base * 2.2;
    double f3 = (double)base * 3.3;
    
    void* p1 = (void*)(intptr_t)(base + 100);
    void* p2 = (void*)(intptr_t)(base + 200);
    
    /* This call will need to shuffle many values into argument registers */
    return many_args_function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                             f1, f2, f3, p1, p2);
}

/* Test 5: Mixed types and register classes */
int __attribute__((noinline)) test_mixed_types(volatile int i, volatile double d)
{
    /* Force moves between different register classes */
    int i1 = (int)d;
    double d1 = (double)i;
    
    /* Use explicit register variables to create conflicts */
    register int r1 asm("r10") = i * 2;
    register int r2 asm("r11") = i * 3;
    
    /* Long long operations that might need multiple registers */
    long long ll1 = (long long)i * 1000LL;
    long long ll2 = (long long)r1 * 2000LL;
    
    /* Double computations */
    double d2 = d * 2.0;
    double d3 = d1 * 3.0;
    
    /* Mix everything together */
    return r1 + r2 + (int)ll1 + (int)ll2 + (int)d1 + (int)d2 + (int)d3;
}

int main(int argc, char *argv[])
{
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    int result = 0;
    
    /* Initialize global arrays */
    for (int i = 0; i < 10000; i++) {
        global_array[i] = i * 3;
    }
    for (int i = 0; i < 2000; i++) {
        global_big_array[i] = i * 5LL;
    }
    for (int i = 0; i < 1000; i++) {
        global_double_array[i] = i * 1.5;
    }
    
    /* Run all tests to trigger different reload scenarios */
    result += test_register_pressure(seed, seed+1, seed+2, seed+3);
    result += test_complex_addressing(seed%100, (seed+1)%100, (seed+2)%100);
    result += test_asm_clobber(seed*2, seed*3);
    result += test_many_args(seed);
    result += test_mixed_types(seed, (double)seed * 0.5);
    
    printf("Result: %d\n", result);
    return result != 0;
}
