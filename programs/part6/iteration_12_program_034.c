/* reload_test.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to force complex addressing */
volatile int global_array[10000];
volatile long long global_big_array[2000];
volatile double global_double_array[1000];

/* Non-inline function with many arguments to force register pressure */
int __attribute__((noinline)) many_args_function(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, double f3, void* p1, void* p2)
{
    /* Force use of all arguments */
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
    int t2 = c + d;
    int t3 = a * b;
    int t4 = c * d;
    int t5 = a - b;
    int t6 = c - d;
    int t7 = a ^ b;
    int t8 = c ^ d;
    int t9 = a | b;
    int t10 = c | d;
    int t11 = a & b;
    int t12 = c & d;
    int t13 = a << 2;
    int t14 = c << 3;
    int t15 = b >> 1;
    int t16 = d >> 2;
    int t17 = t1 + t2;
    int t18 = t3 + t4;
    int t19 = t5 + t6;
    int t20 = t7 + t8;
    int t21 = t9 + t10;
    int t22 = t11 + t12;
    int t23 = t13 + t14;
    int t24 = t15 + t16;
    
    /* Force all values to be used together */
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
           t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20 +
           t21 + t22 + t23 + t24;
}

/* Test 2: Complex addressing modes requiring reloads */
int __attribute__((noinline)) test_complex_addressing(volatile int idx1, volatile int idx2,
                                                     volatile int idx3, volatile int idx4)
{
    int result = 0;
    
    /* Large immediate offset - may require reload */
    result += global_array[4096];
    result += global_array[8192];
    
    /* Variable index with complex computation */
    result += global_array[idx1 * idx2 + 100];
    result += global_array[idx3 * 7 + idx4 * 13];
    
    /* Multi-word type with potential alignment issues */
    long long ll1 = global_big_array[idx1];
    long long ll2 = global_big_array[idx2 + 256];
    long long ll3 = global_big_array[idx3 + 512];
    long long ll4 = global_big_array[idx4 + 768];
    
    /* Double type accesses */
    double d1 = global_double_array[idx1];
    double d2 = global_double_array[idx2 + 128];
    double d3 = global_double_array[idx3 + 256];
    double d4 = global_double_array[idx4 + 384];
    
    /* Combine results in complex way */
    result += (int)ll1 + (int)ll2 + (int)ll3 + (int)ll4;
    result += (int)d1 + (int)d2 + (int)d3 + (int)d4;
    
    /* Nested array access with variable offset */
    result += global_array[global_array[idx1] & 0xFF];
    
    return result;
}

/* Test 3: Inline assembly with many clobbered registers */
int __attribute__((noinline)) test_asm_clobber(volatile int x, volatile int y)
{
    int a = x * 3;
    int b = y * 7;
    int c = x + y;
    int d = x - y;
    int e = x ^ y;
    int f = x | y;
    
    /* Clobber many registers - forces spills and reloads */
    __asm__ volatile (
        "# Clobber many registers\n"
        "nop\n"
        : 
        : 
        : "rax", "rbx", "rcx", "rdx", "rdi", "rsi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
    );
    
    /* Use values after clobber - they must be reloaded */
    return a + b + c + d + e + f;
}

/* Test 4: Function calls with many arguments causing register pressure */
int __attribute__((noinline)) test_many_args(volatile int base)
{
    /* Create many values that need to be passed */
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
    void* p1 = (void*)(intptr_t)(base + 100);
    void* p2 = (void*)(intptr_t)(base + 200);
    
    /* Call function with many arguments - forces register allocation pressure */
    int result = many_args_function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                   f1, f2, f3, p1, p2);
    
    /* Do more work after call to keep values live */
    int b1 = result + a1;
    int b2 = result + a2;
    int b3 = result + a3;
    int b4 = result + a4;
    
    return b1 + b2 + b3 + b4 + result;
}

/* Test 5: Mixed types and explicit register variables */
int __attribute__((noinline)) test_mixed_types(volatile int x, volatile double y)
{
    /* Use explicit register variables to constrain register allocation */
    register int r1 asm("r10") = x * 2;
    register int r2 asm("r11") = x * 3;
    
    /* Mixed integer sizes */
    short s1 = x & 0xFFFF;
    short s2 = (x >> 16) & 0xFFFF;
    char c1 = x & 0xFF;
    char c2 = (x >> 8) & 0xFF;
    
    /* Floating point operations */
    double d1 = y * 2.0;
    double d2 = y * 3.0;
    double d3 = y * 4.0;
    double d4 = y * 5.0;
    
    /* Force use of all values */
    int int_sum = r1 + r2 + s1 + s2 + c1 + c2;
    double double_sum = d1 + d2 + d3 + d4;
    
    return int_sum + (int)double_sum;
}

/* Main function orchestrates all tests */
int main(int argc, char *argv[])
{
    /* Use argv to prevent constant propagation */
    volatile int seed = argc > 1 ? atoi(argv[1]) : 12345;
    
    /* Initialize global arrays */
    for (int i = 0; i < 10000; i++) {
        global_array[i] = i + seed;
    }
    for (int i = 0; i < 2000; i++) {
        global_big_array[i] = i * seed;
    }
    for (int i = 0; i < 1000; i++) {
        global_double_array[i] = i * seed * 0.5;
    }
    
    int total = 0;
    
    /* Run all tests to trigger different reload scenarios */
    total += test_register_pressure(seed, seed+1, seed+2, seed+3);
    total += test_complex_addressing(seed%100, (seed+1)%100, (seed+2)%100, (seed+3)%100);
    total += test_asm_clobber(seed, seed*2);
    total += test_many_args(seed);
    total += test_mixed_types(seed, seed * 0.75);
    
    /* Call many_args_function directly as well */
    total += many_args_function(
        seed, seed+1, seed+2, seed+3, seed+4,
        seed+5, seed+6, seed+7, seed+8, seed+9,
        seed*1.5, seed*2.5, seed*3.5,
        (void*)(intptr_t)seed, (void*)(intptr_t)(seed+100)
    );
    
    printf("Total result: %d\n", total);
    return total != 0 ? 0 : 1;
}
