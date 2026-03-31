/* reload_test.c - Test program to trigger GCC reload pass initialization */
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
    double f1, double f2, double f3, void *p1, void *p2)
{
    /* Complex computation preventing optimization */
    volatile int result = a1 + a2 - a3 * a4 + a5 / (a6 ? a6 : 1);
    result += a7 ^ a8 | a9 & a10;
    result += (int)(f1 + f2 * f3);
    result += (int)((intptr_t)p1 + (intptr_t)p2);
    return result;
}

/* Test 1: Extreme register pressure with many live scalars */
int __attribute__((noinline)) test_register_pressure(int seed)
{
    /* Use volatile inputs to prevent constant propagation */
    volatile int a = seed + 1;
    volatile int b = seed + 2;
    volatile int c = seed + 3;
    volatile int d = seed + 4;
    volatile int e = seed + 5;
    volatile int f = seed + 6;
    volatile int g = seed + 7;
    volatile int h = seed + 8;
    volatile int i = seed + 9;
    volatile int j = seed + 10;
    
    /* Many independent computations creating many live temporaries */
    int t1 = a + b;
    int t2 = c - d;
    int t3 = e * f;
    int t4 = g / (h ? h : 1);
    int t5 = a ^ b;
    int t6 = c | d;
    int t7 = e & f;
    int t8 = g << 2;
    int t9 = h >> 1;
    int t10 = i + j;
    int t11 = a - c;
    int t12 = b * d;
    int t13 = e / (f ? f : 1);
    int t14 = g ^ h;
    int t15 = i | j;
    int t16 = a & b;
    int t17 = c << 3;
    int t18 = d >> 2;
    int t19 = e + g;
    int t20 = f * h;
    
    /* Force all temporaries to be used in final computation */
    int result = t1 + t2 - t3 * t4 + t5 ^ t6 | t7 & t8;
    result += t9 - t10 + t11 * t12 - t13 ^ t14 | t15 & t16;
    result += t17 + t18 - t19 * t20;
    
    /* More computations to increase pressure */
    int t21 = result + a;
    int t22 = result - b;
    int t23 = result * c;
    int t24 = result / (d ? d : 1);
    int t25 = t21 ^ t22;
    int t26 = t23 | t24;
    
    return t25 + t26 + result;
}

/* Test 2: Complex addressing modes requiring reloads */
int __attribute__((noinline)) test_complex_addressing(int index)
{
    volatile int result = 0;
    
    /* Large immediate offset requiring reload */
    result += global_array[4096];  /* Large offset may not fit in addressing mode */
    result += global_array[8192];  /* Another large offset */
    
    /* Complex index computation */
    int complex_index = (index * 3 + 7) / 2;
    
    /* Double register indirect-like access */
    result += global_array[complex_index + test_register_pressure(index) % 100];
    
    /* Misaligned 64-bit access on potentially unaligned boundary */
    long long ll_result = 0;
    ll_result += global_big_array[complex_index % 500];
    ll_result += global_big_array[(complex_index * 2) % 500];
    
    /* Double type requiring possible multi-register moves */
    double d_result = 0.0;
    d_result += global_double_array[complex_index % 250];
    d_result += global_double_array[(complex_index + 1) % 250];
    
    /* Mixed size accesses */
    result += (int)ll_result;
    result += (int)d_result;
    
    /* Array access with complex base calculation */
    int *ptr = (int*)global_array;
    ptr += complex_index * 3;
    result += ptr[0] + ptr[1] + ptr[2];
    
    return result;
}

/* Test 3: Inline assembly with many clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int x, int y)
{
    int a = x * 3;
    int b = y * 7;
    int c = a + b;
    int d = a - b;
    
    /* Inline assembly that clobbers many registers */
    /* For x86_64, clobber general purpose registers */
    asm volatile(
        "# Start of clobber assembly\n"
        "mov %0, %%eax\n"
        "mov %1, %%ebx\n"
        "# Doing some dummy operations\n"
        "add $1, %%eax\n"
        "sub $1, %%ebx\n"
        : /* no outputs */
        : "r"(c), "r"(d)
        : "eax", "ebx", "ecx", "edx", "esi", "edi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "memory", "cc"
    );
    
    /* More computations after clobber - forces reloads */
    int e = a * b;
    int f = c * d;
    int g = e + f;
    
    /* Another assembly block with different clobbers */
    asm volatile(
        "# Second clobber block\n"
        : /* no outputs */
        : /* no inputs */
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15"
    );
    
    return g + a + b;
}

/* Test 4: Function call with many arguments forcing register/memory moves */
int __attribute__((noinline)) test_many_args(int base)
{
    /* Create many different values for arguments */
    int i1 = base + 1;
    int i2 = base + 2;
    int i3 = base + 3;
    int i4 = base + 4;
    int i5 = base + 5;
    int i6 = base + 6;
    int i7 = base + 7;
    int i8 = base + 8;
    int i9 = base + 9;
    int i10 = base + 10;
    
    double d1 = base * 1.1;
    double d2 = base * 2.2;
    double d3 = base * 3.3;
    
    void *p1 = (void*)&global_array[0];
    void *p2 = (void*)&global_big_array[0];
    
    /* Call function with many arguments - will need to use stack or reload */
    int result = many_args_function(
        i1, i2, i3, i4, i5, i6, i7, i8, i9, i10,
        d1, d2, d3, p1, p2
    );
    
    /* Call it again with different values to prevent optimization */
    result += many_args_function(
        i10, i9, i8, i7, i6, i5, i4, i3, i2, i1,
        d3, d2, d1, p2, p1
    );
    
    return result;
}

/* Test 5: Mixed types and explicit register variables */
int __attribute__((noinline)) test_mixed_types(int seed)
{
    /* Use explicit register variables to force specific register allocation */
    register int r1 asm("r10") = seed * 2;
    register int r2 asm("r11") = seed * 3;
    
    /* Mix 32-bit and 64-bit operations */
    long long ll1 = (long long)seed * 1000000LL;
    long long ll2 = (long long)seed * 2000000LL;
    
    /* Force 64-bit operations that may need multiple registers */
    long long ll3 = ll1 + ll2;
    long long ll4 = ll1 - ll2;
    long long ll5 = ll1 * (ll2 ? ll2 : 1);
    
    /* Double operations that may use different register class */
    double d1 = seed * 1.234;
    double d2 = seed * 5.678;
    double d3 = d1 + d2;
    double d4 = d1 * d2;
    
    /* Use all values in computation */
    int result = (int)(ll3 >> 32) + (int)ll4 + (int)(ll5 & 0xFFFFFFFF);
    result += (int)d3 + (int)d4;
    result += r1 + r2;
    
    /* Complex memory access pattern */
    for (int i = 0; i < 10; i++) {
        /* Access with stride that prevents optimization */
        result += global_array[(seed + i * 257) % 10000];
    }
    
    return result;
}

int main(int argc, char *argv[])
{
    /* Use argv to create non-constant inputs */
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
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
    
    printf("Starting reload tests with seed=%d\n", seed);
    
    /* Run all tests to trigger different reload scenarios */
    result += test_register_pressure(seed);
    printf("Test 1 complete: %d\n", result);
    
    result += test_complex_addressing(seed);
    printf("Test 2 complete: %d\n", result);
    
    result += test_asm_clobber(seed, seed * 2);
    printf("Test 3 complete: %d\n", result);
    
    result += test_many_args(seed);
    printf("Test 4 complete: %d\n", result);
    
    result += test_mixed_types(seed);
    printf("Test 5 complete: %d\n", result);
    
    printf("Final result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
