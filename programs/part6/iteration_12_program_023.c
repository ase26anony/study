/* reload_test.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to force complex addressing */
volatile int global_array[10000];
volatile long long global_big_array[2000];
volatile double global_double_array[1000];

/* Non-inline function with many arguments */
int __attribute__((noinline)) many_args_function(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, void* p1, void* p2)
{
    volatile int result = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    result += (int)f1 + (int)f2;
    result += (int)(intptr_t)p1 + (int)(intptr_t)p2;
    return result;
}

/* Test 1: Extreme register pressure with many live scalars */
int __attribute__((noinline)) test_register_pressure(volatile int a, volatile int b, 
                                                     volatile int c, volatile int d)
{
    /* Force many independent computations to create register pressure */
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
    
    /* Chain computations to keep all values live */
    int sum = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
              t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20 +
              t21 + t22 + t23 + t24;
    
    /* Use volatile to prevent dead code elimination */
    volatile int* vptr = &sum;
    asm volatile("" : "+r" (*vptr));
    
    return sum;
}

/* Test 2: Complex addressing modes */
int __attribute__((noinline)) test_complex_addressing(volatile int idx1, volatile int idx2)
{
    int result = 0;
    
    /* Large immediate offset - may need reload */
    result += global_array[4096];
    result += global_array[8192];
    
    /* Variable index with computation - double register indirect */
    int complex_idx1 = idx1 * 3 + idx2 * 7;
    int complex_idx2 = idx2 * 5 + idx1 * 11;
    
    /* These may require reloads due to complex addressing */
    result += global_array[complex_idx1];
    result += global_array[complex_idx2];
    
    /* Multi-word types with potential alignment issues */
    long long ll1 = global_big_array[complex_idx1 % 100];
    long long ll2 = global_big_array[complex_idx2 % 100];
    result += (int)(ll1 + ll2);
    
    /* Double precision requiring multiple moves */
    double d1 = global_double_array[complex_idx1 % 50];
    double d2 = global_double_array[complex_idx2 % 50];
    result += (int)(d1 + d2);
    
    /* Nested array access with computation */
    result += global_array[global_array[complex_idx1 % 100] % 100];
    
    return result;
}

/* Test 3: Inline assembly with register clobbering */
int __attribute__((noinline)) test_asm_clobber(volatile int x, volatile int y)
{
    int a = x * 3;
    int b = y * 5;
    int c = x + y;
    int d = x - y;
    int e = x ^ y;
    int f = x | y;
    
    /* Clobber many registers - forces spills and reloads */
    asm volatile(
        "# Clobber many registers\n"
        "mov $0, %%rax\n"
        "mov $0, %%rbx\n"
        "mov $0, %%rcx\n"
        "mov $0, %%rdx\n"
        "mov $0, %%rsi\n"
        "mov $0, %%rdi\n"
        "mov $0, %%r8\n"
        "mov $0, %%r9\n"
        "mov $0, %%r10\n"
        "mov $0, %%r11\n"
        "mov $0, %%r12\n"
        "mov $0, %%r13\n"
        "mov $0, %%r14\n"
        "mov $0, %%r15\n"
        :
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
    );
    
    /* Use values after clobber - they must be reloaded */
    int result = a + b + c + d + e + f;
    
    /* Another clobber */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Test 4: Function with many arguments causing register pressure */
int __attribute__((noinline)) test_many_args(volatile int base)
{
    /* Create many argument values */
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
    double f1 = base * 1.5;
    double f2 = base * 2.5;
    void* p1 = (void*)(intptr_t)(base + 100);
    void* p2 = (void*)(intptr_t)(base + 200);
    
    /* Call function with many args - forces register allocation for arguments */
    int result = many_args_function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                    f1, f2, p1, p2);
    
    /* Do more work after call to keep values live */
    int t1 = a1 * 2;
    int t2 = a2 * 3;
    int t3 = a3 * 4;
    int t4 = a4 * 5;
    
    return result + t1 + t2 + t3 + t4;
}

/* Test 5: Mixed types and explicit register variables */
int __attribute__((noinline)) test_mixed_types(volatile int x)
{
    /* Use explicit register variables to constrain allocation */
    register int r1 asm("r10") = x * 2;
    register int r2 asm("r11") = x * 3;
    
    /* Mix different sized types */
    char c1 = x & 0xFF;
    short s1 = x & 0xFFFF;
    int i1 = x;
    long long ll1 = (long long)x * 1000;
    
    /* Force conversions and moves between different register classes */
    double d1 = (double)x;
    float f1 = (float)x;
    
    /* Complex expression mixing types */
    int result = r1 + r2 + c1 + s1 + i1 + (int)ll1 + (int)d1 + (int)f1;
    
    /* Use inline asm with specific register constraints */
    asm volatile(
        "addl %%r10d, %0\n"
        "addl %%r11d, %0\n"
        : "+r" (result)
        :
        : "r10", "r11"
    );
    
    return result;
}

/* Main orchestrator */
int main(int argc, char* argv[])
{
    volatile int seed = argc;
    int total = 0;
    
    /* Initialize global arrays */
    for (int i = 0; i < 10000; i++) {
        global_array[i] = i;
        if (i < 2000) global_big_array[i] = i * 2LL;
        if (i < 1000) global_double_array[i] = i * 1.5;
    }
    
    /* Run all tests to trigger different reload scenarios */
    total += test_register_pressure(seed, seed + 1, seed + 2, seed + 3);
    total += test_complex_addressing(seed % 100, (seed + 50) % 100);
    total += test_asm_clobber(seed * 2, seed * 3);
    total += test_many_args(seed);
    total += test_mixed_types(seed * 5);
    
    /* Use result to prevent optimization */
    printf("Total result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
