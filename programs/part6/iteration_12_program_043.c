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
    double f1, double f2, double f3, void *p1, void *p2)
{
    /* Complex computation to prevent elimination */
    int sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    double fsum = f1 + f2 + f3;
    return sum + (int)fsum + (int)((intptr_t)p1 ^ (intptr_t)p2);
}

/* Test 1: Extreme register pressure with many live scalars */
int __attribute__((noinline)) test_register_pressure(int seed)
{
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
    int t2 = c + d;
    int t3 = e + f;
    int t4 = g + h;
    int t5 = i + j;
    int t6 = t1 * t2;
    int t7 = t3 * t4;
    int t8 = t5 * t1;
    int t9 = t2 * t3;
    int t10 = t4 * t5;
    int t11 = t6 + t7;
    int t12 = t8 + t9;
    int t13 = t10 + t6;
    int t14 = t7 + t8;
    int t15 = t9 + t10;
    int t16 = t11 * t12;
    int t17 = t13 * t14;
    int t18 = t15 * t11;
    int t19 = t12 * t13;
    int t20 = t14 * t15;
    
    /* Force all values to be used to prevent dead code elimination */
    return t16 + t17 + t18 + t19 + t20 + 
           t1 + t2 + t3 + t4 + t5 + 
           t6 + t7 + t8 + t9 + t10;
}

/* Test 2: Complex addressing modes requiring reloads */
int __attribute__((noinline)) test_complex_addressing(int index)
{
    volatile int idx = index;
    
    /* Large immediate offset - may require reload on some arches */
    int val1 = global_array[4096];
    int val2 = global_array[4096 + idx];
    
    /* Complex index calculation */
    int complex_idx = (idx * 37 + 12345) % 1000;
    
    /* Multi-word type requiring potential piecewise moves */
    long long ll_val = global_big_array[complex_idx];
    
    /* Double with alignment requirements */
    double d_val = global_double_array[complex_idx % 500];
    
    /* Nested array access with computation */
    int val3 = global_array[global_array[idx % 100] % 1000];
    
    /* Mixed type computation to use all values */
    return val1 + val2 + (int)ll_val + (int)d_val + val3;
}

/* Test 3: Inline assembly with many clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int x, int y)
{
    int result;
    
    /* Do some computation before assembly */
    int a = x * 3;
    int b = y * 7;
    int c = a + b;
    int d = a * b;
    int e = c ^ d;
    
    /* Inline assembly that clobbers many registers */
    /* For x86_64, clobber commonly used registers */
    asm volatile (
        "# Dummy assembly to clobber registers\n"
        "mov %0, %%eax\n"
        "add %1, %%eax\n"
        : 
        : "r" (c), "r" (d)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "memory", "cc"
    );
    
    /* More computation after assembly - values in clobbered regs need reloading */
    int f = e * 2;
    int g = a + f;
    int h = b * g;
    
    /* Another assembly block with different clobbers */
    asm volatile (
        "# More clobbering\n"
        : 
        : 
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
    );
    
    return f + g + h;
}

/* Test 4: Function call with many arguments causing register pressure */
int __attribute__((noinline)) test_many_args(int base)
{
    /* Create many argument values that need to be in registers */
    int a1 = base + 1;
    int a2 = base + 2;
    int a3 = base + 3;
    int a4 = base + 4;
    int a5 = base + 5;
    int a6 = base + 6;
    int a7 = base + 7;
    int a8 = base + 8;
    int a9 = base + 9;
    int a10 = base + 10;
    double f1 = base * 1.1;
    double f2 = base * 2.2;
    double f3 = base * 3.3;
    void *p1 = (void*)&global_array;
    void *p2 = (void*)&global_big_array;
    
    /* Call function with many args - forces register allocation pressure */
    int result = many_args_function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                    f1, f2, f3, p1, p2);
    
    /* Use result in further computation to prevent elimination */
    return result * 2 + a1 + a2;
}

/* Test 5: Mixed types and explicit register variables */
int __attribute__((noinline)) test_mixed_types(int seed)
{
    /* Use explicit register variables to force specific register allocation */
    register int r1 asm("r10") = seed * 2;
    register int r2 asm("r11") = seed * 3;
    
    /* Mix different sized types */
    char c1 = seed & 0xFF;
    short s1 = seed * 2;
    int i1 = seed * 3;
    long long ll1 = (long long)seed * 1000;
    
    /* Force conversions and type mixing */
    int t1 = c1 + s1;
    int t2 = i1 + (int)ll1;
    int t3 = r1 + r2;
    
    /* Unaligned access simulation */
    struct {
        char c;
        int i;
    } __attribute__((packed)) misaligned;
    
    misaligned.c = c1;
    misaligned.i = i1;
    
    /* Use packed struct member - may require unaligned load */
    int t4 = misaligned.i;
    
    return t1 + t2 + t3 + t4;
}

int main(int argc, char *argv[])
{
    /* Use argv to create non-constant inputs */
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int result = 0;
    
    printf("Starting reload tests with seed=%d\n", seed);
    
    /* Run all tests to trigger different reload scenarios */
    result += test_register_pressure(seed);
    printf("test_register_pressure: %d\n", result);
    
    result += test_complex_addressing(seed);
    printf("test_complex_addressing: %d\n", result);
    
    result += test_asm_clobber(seed, seed * 2);
    printf("test_asm_clobber: %d\n", result);
    
    result += test_many_args(seed);
    printf("test_many_args: %d\n", result);
    
    result += test_mixed_types(seed);
    printf("test_mixed_types: %d\n", result);
    
    printf("Final result: %d\n", result);
    
    return result == 0 ? 0 : 1;
}
