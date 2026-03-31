/* reload_test.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to force complex addressing */
volatile int global_array[8192];
volatile long long global_big_array[4096];
volatile double global_double_array[2048];

/* Non-inline function with many arguments */
int __attribute__((noinline)) many_args_func(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, void *p1, void *p2)
{
    volatile int result = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    result += (int)(f1 + f2);
    result += (int)((intptr_t)p1 + (intptr_t)p2);
    return result;
}

/* Test 1: Extreme register pressure with many live scalars */
int __attribute__((noinline)) test_register_pressure(int a, int b, int c, int d, 
                                                     int e, int f, int g, int h)
{
    /* Use volatile to prevent optimization */
    volatile int v1 = a;
    volatile int v2 = b;
    volatile int v3 = c;
    volatile int v4 = d;
    volatile int v5 = e;
    volatile int v6 = f;
    volatile int v7 = g;
    volatile int v8 = h;
    
    /* Create many independent computations to force register pressure */
    int t1 = v1 + v2;
    int t2 = v3 + v4;
    int t3 = v5 + v6;
    int t4 = v7 + v8;
    int t5 = v1 * v2;
    int t6 = v3 * v4;
    int t7 = v5 * v6;
    int t8 = v7 * v8;
    int t9 = t1 + t2;
    int t10 = t3 + t4;
    int t11 = t5 + t6;
    int t12 = t7 + t8;
    int t13 = t9 - t10;
    int t14 = t11 - t12;
    int t15 = t13 * t14;
    int t16 = t1 * t3;
    int t17 = t2 * t4;
    int t18 = t5 * t7;
    int t19 = t6 * t8;
    int t20 = t16 + t17;
    int t21 = t18 + t19;
    int t22 = t20 - t21;
    int t23 = t15 + t22;
    int t24 = t9 * t11;
    int t25 = t10 * t12;
    int t26 = t24 - t25;
    int t27 = t23 + t26;
    
    /* Force all values to be used */
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
           t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 +
           t20 + t21 + t22 + t23 + t24 + t25 + t26 + t27;
}

/* Test 2: Complex addressing modes */
int __attribute__((noinline)) test_complex_addressing(int index1, int index2, 
                                                      int index3, int index4)
{
    volatile int result = 0;
    
    /* Large immediate offset - may need reload */
    result += global_array[4096];
    result += global_array[4097];
    
    /* Variable index with computation - double register indirect */
    int complex_idx1 = index1 * 3 + index2 * 7;
    int complex_idx2 = index3 * 11 + index4 * 13;
    
    result += global_array[complex_idx1 + 2048];
    result += global_array[complex_idx2 + 3072];
    
    /* Multi-word types forcing piecewise moves */
    long long ll1 = global_big_array[complex_idx1 % 100];
    long long ll2 = global_big_array[complex_idx2 % 100];
    result += (int)(ll1 + ll2);
    
    /* Double precision requiring specific handling */
    double d1 = global_double_array[complex_idx1 % 50];
    double d2 = global_double_array[complex_idx2 % 50];
    result += (int)(d1 + d2);
    
    /* Nested complex addressing */
    result += global_array[global_array[complex_idx1 % 100] % 1000];
    
    return result;
}

/* Test 3: Inline assembly with clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int a, int b, int c, int d)
{
    int x1 = a * b;
    int x2 = c * d;
    int x3 = a + b;
    int x4 = c + d;
    int x5 = a - b;
    int x6 = c - d;
    int x7 = a ^ b;
    int x8 = c ^ d;
    
    /* Clobber many registers - forces spills and reloads */
    asm volatile(
        "# Start clobber block\n"
        "nop\n"
        : 
        : 
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
    );
    
    /* More computations after clobber - will need reloads */
    int y1 = x1 * x2;
    int y2 = x3 * x4;
    int y3 = x5 * x6;
    int y4 = x7 * x8;
    
    asm volatile(
        "# Another clobber block\n"
        "nop\n"
        :
        :
        : "rax", "rbx", "rcx", "rdx"
    );
    
    return y1 + y2 + y3 + y4 + x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8;
}

/* Test 4: Function calls with many arguments */
int __attribute__((noinline)) test_many_args(int base)
{
    volatile int v = base;
    
    /* Create many argument values that need registers */
    int a1 = v + 1;
    int a2 = v + 2;
    int a3 = v + 3;
    int a4 = v + 4;
    int a5 = v + 5;
    int a6 = v + 6;
    int a7 = v + 7;
    int a8 = v + 8;
    int a9 = v + 9;
    int a10 = v + 10;
    double f1 = (double)v * 1.1;
    double f2 = (double)v * 2.2;
    void *p1 = (void*)(intptr_t)(v * 100);
    void *p2 = (void*)(intptr_t)(v * 200);
    
    /* Call function with many args - forces register allocation pressure */
    int result = many_args_func(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                f1, f2, p1, p2);
    
    /* More computations to keep values live across call */
    int b1 = a1 * 2;
    int b2 = a2 * 3;
    int b3 = a3 * 4;
    int b4 = a4 * 5;
    
    return result + b1 + b2 + b3 + b4;
}

/* Test 5: Explicit register variables and specific modes */
int __attribute__((noinline)) test_explicit_registers(int a, int b)
{
    /* Try to use specific registers (x86-64) */
    register int r10_val asm("r10") = a * 3;
    register int r11_val asm("r11") = b * 5;
    
    /* Force use of these register variables */
    asm volatile("# Using r10: %0, r11: %1" : : "r"(r10_val), "r"(r11_val));
    
    int c = r10_val + r11_val;
    
    /* Create register pressure around the fixed registers */
    int t1 = a + 1;
    int t2 = a + 2;
    int t3 = a + 3;
    int t4 = a + 4;
    int t5 = a + 5;
    int t6 = a + 6;
    int t7 = a + 7;
    int t8 = a + 8;
    int t9 = a + 9;
    int t10 = a + 10;
    
    /* Use all temporaries */
    return c + t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
}

/* Main orchestrator */
int main(int argc, char *argv[])
{
    /* Use argv to prevent constant propagation */
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    
    int result = 0;
    
    /* Run all tests with values derived from seed */
    int a = seed;
    int b = seed * 2;
    int c = seed * 3;
    int d = seed * 4;
    int e = seed * 5;
    int f = seed * 6;
    int g = seed * 7;
    int h = seed * 8;
    
    printf("Starting reload tests...\n");
    
    result += test_register_pressure(a, b, c, d, e, f, g, h);
    printf("Test 1 complete: %d\n", result);
    
    result += test_complex_addressing(a, b, c, d);
    printf("Test 2 complete: %d\n", result);
    
    result += test_asm_clobber(a, b, c, d);
    printf("Test 3 complete: %d\n", result);
    
    result += test_many_args(seed);
    printf("Test 4 complete: %d\n", result);
    
    result += test_explicit_registers(a, b);
    printf("Test 5 complete: %d\n", result);
    
    printf("Final result: %d\n", result);
    
    /* Use result to prevent dead code elimination */
    return result == 0 ? 1 : 0;
}
