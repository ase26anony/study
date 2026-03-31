/* reload_test.c - Test program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to enable complex addressing modes */
volatile long global_array[8192];
volatile int global_index = 2048;
volatile double global_double[4096];

/* Non-inline function with many arguments to force register pressure */
int __attribute__((noinline)) many_args_function(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, double f3, void *p1, void *p2)
{
    /* Complex computation preventing optimization */
    volatile int result = a1 + a2 - a3 * a4 + a5 / (a6 ? a6 : 1);
    result += a7 ^ a8 | a9 & a10;
    result += (int)(f1 * 100.0) + (int)(f2 * 200.0) + (int)(f3 * 300.0);
    result += (intptr_t)p1 % 1000 + (intptr_t)p2 % 500;
    return result;
}

/* Test 1: Extreme register pressure with many live scalars */
int __attribute__((noinline)) test_register_pressure(int seed)
{
    /* Use volatile inputs to prevent optimization */
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
    
    /* Many independent computations creating many live temporaries */
    int t1 = a + b * c;
    int t2 = d - e / (f ? f : 1);
    int t3 = g ^ h | i & j;
    int t4 = k << 2 + l >> 3;
    int t5 = m * n + o - p;
    int t6 = q % (r ? r : 1) + s * t;
    int t7 = t1 ^ t2 + t3;
    int t8 = t4 | t5 & t6;
    int t9 = t7 * t8 - a + b;
    int t10 = c * d + e * f - g;
    int t11 = h / (i ? i : 1) + j * k;
    int t12 = l ^ m | n & o;
    int t13 = p << 3 + q >> 2;
    int t14 = r * s + t - a;
    int t15 = b % (c ? c : 1) + d * e;
    int t16 = f ^ g | h & i;
    int t17 = j * k + l - m;
    int t18 = n / (o ? o : 1) + p * q;
    int t19 = r ^ s | t & a;
    int t20 = b << 4 + c >> 1;
    
    /* Force all values to be used to prevent dead code elimination */
    volatile int result = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
                         t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20;
    
    return result;
}

/* Test 2: Complex addressing modes requiring reloads */
long __attribute__((noinline)) test_complex_addressing(int index)
{
    volatile long total = 0;
    
    /* 1. Double register indirect with complex computation */
    int complex_idx = (index * 3 + 7) / 2;
    total += global_array[complex_idx + global_index];
    
    /* 2. Large immediate offset (may require reload on some archs) */
    total += global_array[4096];  /* Large constant offset */
    
    /* 3. Multi-step addressing with multiple operations */
    total += global_array[index * 2 + 100];
    total += global_array[index * 3 + 200];
    total += global_array[index * 4 + 300];
    
    /* 4. Misaligned access simulation with byte manipulation */
    char *byte_ptr = (char *)global_array;
    total += *(long *)(byte_ptr + index * 8 + 3);  /* Potentially unaligned */
    
    /* 5. Complex global array access with variable offset */
    total += global_array[global_index + index * index / 2];
    
    return total;
}

/* Test 3: Inline assembly with many clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int x, int y)
{
    int result = x * y;
    
    /* First do some computation that uses registers */
    int a = x + y;
    int b = x - y;
    int c = x * y;
    int d = x ^ y;
    int e = x | y;
    int f = x & y;
    
    /* Inline assembly that clobbers many registers */
    /* For x86_64, clobber both caller-saved and some callee-saved */
    __asm__ volatile (
        "# Clobber many registers to force spills\n"
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
        : /* no outputs */
        : /* no inputs */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "memory"
    );
    
    /* Use the values after assembly to force reloads */
    result += a + b - c + d | e & f;
    
    /* Another assembly block with different clobbers */
    __asm__ volatile (
        "# Second clobber block\n"
        "mov $1, %%r12\n"
        "mov $1, %%r13\n"
        "mov $1, %%r14\n"
        "mov $1, %%r15\n"
        : /* no outputs */
        : /* no inputs */
        : "r12", "r13", "r14", "r15", "memory"
    );
    
    return result;
}

/* Test 4: Function with many arguments causing register pressure */
int __attribute__((noinline)) test_many_args(int base)
{
    /* Create many different values as arguments */
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
    void *p1 = (void *)(intptr_t)(base + 100);
    void *p2 = (void *)(intptr_t)(base + 200);
    
    /* Call function with many arguments - may need reloads for argument setup */
    int result = many_args_function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                   f1, f2, f3, p1, p2);
    
    /* Call it again with different values to prevent tail call optimization */
    result += many_args_function(a10, a9, a8, a7, a6, a5, a4, a3, a2, a1,
                                f3, f2, f1, p2, p1);
    
    return result;
}

/* Test 5: Mixed types and explicit register variables */
#ifdef __x86_64__
int __attribute__((noinline)) test_mixed_types(int x)
{
    /* Use explicit register variables to constrain register allocation */
    register int r10_var asm("r10") = x * 2;
    register int r11_var asm("r11") = x * 3;
    
    /* Mix different sized types */
    long long ll1 = (long long)x * 1000000000LL;
    long long ll2 = (long long)x * 2000000000LL;
    long long ll3 = (long long)x * 3000000000LL;
    
    double d1 = x * 1.234567;
    double d2 = x * 2.345678;
    double d3 = x * 3.456789;
    
    /* Force use of all values */
    int result = (int)(ll1 % 1000) + (int)(ll2 % 1000) + (int)(ll3 % 1000);
    result += (int)(d1 * 100.0) + (int)(d2 * 100.0) + (int)(d3 * 100.0);
    result += r10_var + r11_var;
    
    /* Access with potentially unaligned 64-bit values */
    struct { char c; long long ll; } __attribute__((packed)) misaligned;
    misaligned.c = 'A';
    misaligned.ll = ll1;
    
    result += (int)misaligned.ll;
    
    return result;
}
#endif

/* Main function orchestrating all tests */
int main(int argc, char *argv[])
{
    /* Use command line arguments to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 12345;
    int result = 0;
    
    printf("Starting reload pass tests...\n");
    
    /* Run all tests to trigger different reload scenarios */
    result += test_register_pressure(seed);
    printf("Test 1 complete: register pressure\n");
    
    result += test_complex_addressing(seed % 100);
    printf("Test 2 complete: complex addressing\n");
    
    result += test_asm_clobber(seed, seed * 2);
    printf("Test 3 complete: asm clobber\n");
    
    result += test_many_args(seed);
    printf("Test 4 complete: many arguments\n");
    
#ifdef __x86_64__
    result += test_mixed_types(seed);
    printf("Test 5 complete: mixed types\n");
#endif
    
    /* Also create some register pressure in main itself */
    volatile int v1 = seed * 3;
    volatile int v2 = seed * 5;
    volatile int v3 = seed * 7;
    volatile int v4 = seed * 11;
    volatile int v5 = seed * 13;
    volatile int v6 = seed * 17;
    volatile int v7 = seed * 19;
    volatile int v8 = seed * 23;
    
    int main_tmp1 = v1 + v2 * v3;
    int main_tmp2 = v4 - v5 / (v6 ? v6 : 1);
    int main_tmp3 = v7 ^ v8 | v1 & v2;
    int main_tmp4 = v3 << 2 + v4 >> 3;
    
    result += main_tmp1 + main_tmp2 + main_tmp3 + main_tmp4;
    
    printf("Final result: %d\n", result);
    return result != 0 ? 0 : 1;
}
