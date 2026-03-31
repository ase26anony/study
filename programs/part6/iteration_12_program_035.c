/* reload_test.c - Test program to trigger GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to enable complex addressing modes */
volatile long long global_array[8192];
volatile int global_int_array[16384];
volatile double global_double_array[4096];

/* Non-inline function with many arguments to force register pressure */
int __attribute__((noinline)) many_args_function(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, double f3, void *p1, void *p2)
{
    /* Complex computation preventing optimization */
    volatile int result = a1 + a2 - a3 * a4 + a5 / (a6 ? a6 : 1);
    result += (int)(f1 * 100.0) + (int)(f2 * 50.0);
    result += (intptr_t)p1 % 256 + (intptr_t)p2 % 128;
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
    volatile int k = seed + 11;
    volatile int l = seed + 12;
    volatile int m = seed + 13;
    volatile int n = seed + 14;
    volatile int o = seed + 15;
    volatile int p = seed + 16;
    volatile int q = seed + 17;
    volatile int r = seed + 18;
    volatile int s = seed + 19;
    volatile int t = seed + 20;
    
    /* Many independent arithmetic expressions creating live ranges */
    int t1 = a + b * c - d;
    int t2 = e * f + g / (h ? h : 1);
    int t3 = i - j + k * l;
    int t4 = m + n - o * p;
    int t5 = q / (r ? r : 1) + s - t;
    int t6 = t1 * t2 - t3;
    int t7 = t4 + t5 * t1;
    int t8 = t2 / (t3 ? t3 : 1) + t4;
    int t9 = t5 - t6 * t7;
    int t10 = t8 + t9 - t1;
    int t11 = t2 * t3 + t4 - t5;
    int t12 = t6 / (t7 ? t7 : 1) * t8;
    int t13 = t9 + t10 - t11 * t12;
    int t14 = t1 + t2 + t3 + t4 + t5;
    int t15 = t6 - t7 + t8 - t9 + t10;
    int t16 = t11 * t12 / (t13 ? t13 : 1);
    int t17 = t14 + t15 - t16;
    int t18 = t1 * t2 * t3 - t4;
    int t19 = t5 + t6 + t7 + t8 + t9;
    int t20 = t10 - t11 + t12 - t13;
    
    /* Force all values to be used to prevent dead code elimination */
    volatile int result = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
                         t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20;
    
    return result;
}

/* Test 2: Complex addressing modes requiring reloads */
int __attribute__((noinline)) test_complex_addressing(int index)
{
    volatile int idx = index;
    
    /* Large immediate offset requiring reload */
    int val1 = global_int_array[4096] + global_int_array[8192];
    
    /* Variable index with complex computation */
    int complex_idx = (idx * 3 + 7) / 2;
    int val2 = global_int_array[complex_idx] + 
               global_int_array[complex_idx + 256];
    
    /* Multi-word move (long long) on potentially unaligned boundary */
    long long ll_val = global_array[idx % 1000] + 
                       global_array[(idx + 1) % 1000];
    
    /* Double register indirect-like access through pointer arithmetic */
    int *ptr = (int*)global_int_array;
    ptr += (idx * 17) % 500;
    int val3 = *ptr + *(ptr + 128);
    
    /* Mixed size accesses */
    double d_val = global_double_array[idx % 2000] * 2.0;
    
    /* Combine results ensuring all are used */
    return val1 + val2 + (int)ll_val + val3 + (int)d_val;
}

/* Test 3: Inline assembly with many clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int x, int y)
{
    int a = x * 3;
    int b = y * 7;
    int c = a + b;
    int d = a - b;
    int e = a * b;
    int f = b / (a ? a : 1);
    
    /* Inline assembly that clobbers many registers */
    /* For x86_64 - clobber general purpose, segment, and floating point regs */
    asm volatile(
        "# Test assembly block\n"
        "movl %0, %%eax\n"
        "addl %1, %%eax\n"
        : 
        : "r" (c), "r" (d)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "memory", "cc"
    );
    
    /* More computations after clobber - forces reloads */
    int g = e * f + c;
    int h = d - e + f;
    int i = g * h / (c ? c : 1);
    int j = a + b + c + d + e + f + g + h + i;
    
    /* Second assembly block with different clobbers */
    asm volatile(
        "# Another assembly block\n"
        : 
        : 
        : "rax", "rbx", "rcx", "rdx", "st", "st(1)", "st(2)", "st(3)",
          "mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7"
    );
    
    return j;
}

/* Test 4: Function call with many arguments */
int __attribute__((noinline)) test_many_args(int base)
{
    /* Create many values that need to be in registers for call */
    int arg1 = base + 1;
    int arg2 = base * 2;
    int arg3 = base / 3;
    int arg4 = base - 100;
    int arg5 = base * base;
    int arg6 = base + 1000;
    int arg7 = base % 17;
    int arg8 = base * 3 + 7;
    int arg9 = base / 2 + 1;
    int arg10 = base * 5 - 3;
    
    double farg1 = (double)base * 1.5;
    double farg2 = (double)base / 2.5;
    double farg3 = (double)(base * base) * 0.1;
    
    void *parg1 = (void*)(intptr_t)(base + 10000);
    void *parg2 = (void*)(intptr_t)(base + 20000);
    
    /* This call will need many registers for argument passing */
    int result = many_args_function(
        arg1, arg2, arg3, arg4, arg5,
        arg6, arg7, arg8, arg9, arg10,
        farg1, farg2, farg3, parg1, parg2
    );
    
    /* Additional computation to use the result */
    return result * 2 + base;
}

/* Test 5: Mixed types and explicit register variables */
int __attribute__((noinline)) test_mixed_types(int seed)
{
    /* Try to use explicit register variables (GCC extension) */
    register int r1 asm("r10") = seed * 2;
    register int r2 asm("r11") = seed * 3;
    
    /* Use 64-bit values on 32-bit architectures or vice versa */
    long long big1 = (long long)seed * 1000000LL;
    long long big2 = (long long)seed * 2000000LL;
    long long big3 = big1 + big2;
    long long big4 = big1 - big2;
    long long big5 = big1 * (big2 ? big2 : 1LL);
    
    /* Double precision computations */
    double d1 = (double)seed * 3.14159;
    double d2 = (double)seed * 2.71828;
    double d3 = d1 * d2;
    double d4 = d1 / (d2 ? d2 : 1.0);
    
    /* Structure with mixed types */
    struct mixed {
        int a;
        long long b;
        double c;
        int d;
    } m;
    
    m.a = seed;
    m.b = big3;
    m.c = d3;
    m.d = r1 + r2;
    
    /* Access structure members forcing potential reloads */
    int result = m.a + (int)m.b + (int)m.c + m.d;
    
    /* Use the register variables */
    result += r1 * r2;
    
    return result;
}

/* Main function orchestrating all tests */
int main(int argc, char *argv[])
{
    /* Use argv to create non-constant inputs */
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int total = 0;
    
    printf("Starting reload pass tests...\n");
    
    /* Run all tests to trigger different reload scenarios */
    total += test_register_pressure(seed);
    total += test_complex_addressing(seed);
    total += test_asm_clobber(seed, seed * 2);
    total += test_many_args(seed);
    total += test_mixed_types(seed);
    
    /* Use the result to prevent optimization */
    printf("Total result: %d\n", total);
    
    /* Access global arrays to ensure they're used */
    global_int_array[0] = total;
    global_array[0] = total;
    
    return total != 0 ? 0 : 1;
}
