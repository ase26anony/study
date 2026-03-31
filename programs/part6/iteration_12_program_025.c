/* reload_test.c - Test program to trigger GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to force complex addressing */
volatile int global_array[10000];
volatile long long global_big_array[2000];
volatile double global_double_array[1000];

/* Non-inline function with many arguments to force register pressure */
int __attribute__((noinline)) many_args_func(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, double f3, void *p1, void *p2)
{
    /* Force use of all arguments to prevent elimination */
    volatile int result = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    result += (int)(f1 + f2 + f3);
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
    
    /* Many independent arithmetic expressions to create many live temporaries */
    int t1 = a + b;
    int t2 = c + d;
    int t3 = e + f;
    int t4 = g + h;
    int t5 = i + j;
    int t6 = k + l;
    int t7 = m + n;
    int t8 = o + p;
    int t9 = q + r;
    int t10 = s + t;
    
    int t11 = t1 * t2;
    int t12 = t3 * t4;
    int t13 = t5 * t6;
    int t14 = t7 * t8;
    int t15 = t9 * t10;
    
    int t16 = t11 + t12;
    int t17 = t13 + t14;
    int t18 = t15 + t16;
    int t19 = t17 + t18;
    int t20 = t19 * 2;
    
    /* More computations to exceed register file */
    int t21 = t20 + a;
    int t22 = t21 + b;
    int t23 = t22 + c;
    int t24 = t23 + d;
    int t25 = t24 + e;
    int t26 = t25 + f;
    int t27 = t26 + g;
    int t28 = t27 + h;
    int t29 = t28 + i;
    int t30 = t29 + j;
    
    /* Force all values to be used */
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
           t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20 +
           t21 + t22 + t23 + t24 + t25 + t26 + t27 + t28 + t29 + t30;
}

/* Test 2: Complex addressing modes requiring reloads */
int __attribute__((noinline)) test_complex_addressing(int index)
{
    volatile int result = 0;
    
    /* 1. Double register indirect with complex index */
    int complex_index = index * 3 + 7;
    result += global_array[complex_index + index];
    
    /* 2. Large immediate offset (4096 is often too large for some archs) */
    result += global_array[4096];
    
    /* 3. Multi-word moves with misalignment concerns */
    volatile long long ll1 = global_big_array[index];
    volatile long long ll2 = global_big_array[index + 1];
    result += (int)(ll1 + ll2);
    
    /* 4. Double with alignment requirements */
    volatile double d1 = global_double_array[index % 100];
    volatile double d2 = global_double_array[(index + 1) % 100];
    result += (int)(d1 + d2);
    
    /* 5. Nested array indexing with variable offsets */
    int idx1 = index * 2;
    int idx2 = index * 3;
    result += global_array[global_array[idx1] + idx2];
    
    return result;
}

/* Test 3: Inline assembly with many clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int x, int y)
{
    int a = x * 3;
    int b = y * 5;
    int c = a + b;
    int d = a * b;
    int e = c + d;
    int f = e * 2;
    
    /* Clobber many registers to force spills and reloads */
    __asm__ volatile (
        "# Clobber many registers\n"
        "nop\n"
        : 
        : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f)
        : 
#if defined(__x86_64__)
        "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
        "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
        "xmm12", "xmm13", "xmm14", "xmm15"
#elif defined(__i386__)
        "eax", "ebx", "ecx", "edx", "esi", "edi",
        "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
#endif
    );
    
    /* Use values after asm to ensure they need reloading */
    return a + b + c + d + e + f;
}

/* Test 4: Function call with many arguments */
int __attribute__((noinline)) test_many_args(int base)
{
    /* Create many different values for arguments */
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
    void *p1 = (void*)&global_array[0];
    void *p2 = (void*)&global_big_array[0];
    
    /* Call function with many arguments - each may need register reload */
    int result = many_args_func(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                f1, f2, f3, p1, p2);
    
    /* Additional computations to increase register pressure around call */
    int t1 = a1 * a2;
    int t2 = a3 * a4;
    int t3 = a5 * a6;
    int t4 = a7 * a8;
    int t5 = a9 * a10;
    
    return result + t1 + t2 + t3 + t4 + t5;
}

/* Test 5: Machine-specific register constraints (x86_64 specific) */
#if defined(__x86_64__)
int __attribute__((noinline)) test_machine_specific(int x)
{
    /* Use explicit register variables to force specific register allocation */
    register int r10_var asm("r10") = x * 2;
    register int r11_var asm("r11") = x * 3;
    register int r12_var asm("r12") = x * 4;
    register int r13_var asm("r13") = x * 5;
    
    /* Create many other variables to pressure the register allocator */
    int a = x + 1;
    int b = x + 2;
    int c = x + 3;
    int d = x + 4;
    int e = x + 5;
    int f = x + 6;
    int g = x + 7;
    int h = x + 8;
    int i = x + 9;
    int j = x + 10;
    
    /* Force use of all variables */
    int result = r10_var + r11_var + r12_var + r13_var;
    result += a + b + c + d + e + f + g + h + i + j;
    
    /* Use 64-bit operations that might need multiple registers */
    long long ll1 = (long long)r10_var * r11_var;
    long long ll2 = (long long)r12_var * r13_var;
    result += (int)(ll1 + ll2);
    
    return result;
}
#endif

/* Main function orchestrates all tests */
int main(int argc, char *argv[])
{
    /* Use argv to create non-constant inputs */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    int index = argc > 2 ? atoi(argv[2]) : 100;
    
    int total = 0;
    
    /* Run each test to trigger different reload scenarios */
    total += test_register_pressure(seed);
    total += test_complex_addressing(index);
    total += test_asm_clobber(seed, index);
    total += test_many_args(seed);
    
#if defined(__x86_64__)
    total += test_machine_specific(seed);
#endif
    
    /* Additional register pressure in main */
    volatile int extra1 = seed * 3;
    volatile int extra2 = seed * 5;
    volatile int extra3 = seed * 7;
    volatile int extra4 = seed * 11;
    volatile int extra5 = seed * 13;
    
    total += extra1 + extra2 + extra3 + extra4 + extra5;
    
    /* Access global with complex addressing from main too */
    total += global_array[seed % 1000];
    total += (int)global_big_array[index % 500];
    total += (int)global_double_array[(seed + index) % 100];
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
