/* reload_test.c - Test program to trigger GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global arrays to force complex addressing */
volatile int global_array[8192];
volatile long long global_big_array[4096];
volatile double global_double_array[2048];

/* Non-inline function with many arguments to force register pressure */
int __attribute__((noinline)) many_args_function(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, double f3, void *p1, void *p2)
{
    /* Force use of all arguments */
    volatile int sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    volatile double fsum = f1 + f2 + f3;
    return (int)(sum + fsum + (intptr_t)p1 + (intptr_t)p2);
}

/* Test 1: Extreme register pressure with many live scalars */
int __attribute__((noinline)) test_register_pressure(int a, int b, int c, int d, int e, int f)
{
    /* Use volatile inputs to prevent optimization */
    volatile int v1 = a;
    volatile int v2 = b;
    volatile int v3 = c;
    volatile int v4 = d;
    volatile int v5 = e;
    volatile int v6 = f;
    
    /* Create many independent computations to force register allocation */
    int t1 = v1 + v2;
    int t2 = v3 * v4;
    int t3 = v5 ^ v6;
    int t4 = v1 - v2;
    int t5 = v3 / (v4 ? v4 : 1);
    int t6 = v5 | v6;
    int t7 = v1 & v2;
    int t8 = v3 << v4;
    int t9 = v5 >> (v6 & 31);
    int t10 = t1 + t2;
    int t11 = t3 - t4;
    int t12 = t5 * t6;
    int t13 = t7 ^ t8;
    int t14 = t9 & t10;
    int t15 = t11 | t12;
    int t16 = t13 + t14;
    int t17 = t15 - t16;
    int t18 = t1 * t17;
    int t19 = t2 / (t3 ? t3 : 1);
    int t20 = t4 ^ t18;
    int t21 = t5 & t19;
    int t22 = t6 | t20;
    int t23 = t7 + t21;
    int t24 = t8 - t22;
    int t25 = t9 * t23;
    
    /* Force all values to be live simultaneously */
    asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4), "r"(t5),
                     "r"(t6), "r"(t7), "r"(t8), "r"(t9), "r"(t10),
                     "r"(t11), "r"(t12), "r"(t13), "r"(t14), "r"(t15),
                     "r"(t16), "r"(t17), "r"(t18), "r"(t19), "r"(t20),
                     "r"(t21), "r"(t22), "r"(t23), "r"(t24), "r"(t25));
    
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
           t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20 +
           t21 + t22 + t23 + t24 + t25;
}

/* Test 2: Complex addressing modes requiring reloads */
int __attribute__((noinline)) test_complex_addressing(int index, int offset)
{
    volatile int idx = index;
    volatile int off = offset;
    
    /* Large immediate offset - may require reload */
    int val1 = global_array[4096];  /* Large constant offset */
    
    /* Variable index with computation */
    int val2 = global_array[idx * 2 + 256];
    
    /* Complex addressing with multiple computations */
    int val3 = global_array[(idx * off) / 2 + 1024];
    
    /* Multi-word access with long long */
    long long val4 = global_big_array[idx];
    
    /* Double type with potential alignment issues */
    double val5 = global_double_array[off];
    
    /* Nested array access with computation */
    int val6 = global_array[global_array[idx] & 0xFFF];
    
    /* Force use of all loaded values */
    asm volatile("" : : "r"(val1), "r"(val2), "r"(val3), "r"(val4), "r"(val5), "r"(val6));
    
    return val1 + val2 + val3 + (int)val4 + (int)val5 + val6;
}

/* Test 3: Inline assembly with many clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int a, int b, int c, int d)
{
    volatile int x = a;
    volatile int y = b;
    volatile int z = c;
    volatile int w = d;
    
    /* Do some computation that uses registers */
    int result = x * y + z / (w ? w : 1);
    
    /* Clobber many registers - forces spills and reloads */
    asm volatile(
        "# Start clobber block\n"
        "movl $0, %%eax\n"
        "movl $0, %%ebx\n"
        "movl $0, %%ecx\n"
        "movl $0, %%edx\n"
        "movl $0, %%esi\n"
        "movl $0, %%edi\n"
        "# End clobber block\n"
        : /* no outputs */
        : /* no inputs */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15",
          "cc", "memory"
    );
    
    /* More computation after clobber - requires reloads */
    result += x - y * z;
    
    return result;
}

/* Test 4: Function call with many arguments */
int __attribute__((noinline)) test_many_args(int base)
{
    volatile int v = base;
    
    /* Prepare many arguments with computations */
    int a1 = v + 1;
    int a2 = v * 2;
    int a3 = v - 3;
    int a4 = v / 4;
    int a5 = v ^ 5;
    int a6 = v & 6;
    int a7 = v | 7;
    int a8 = v << 2;
    int a9 = v >> 1;
    int a10 = v + 10;
    
    double f1 = (double)v * 1.1;
    double f2 = (double)v * 2.2;
    double f3 = (double)v * 3.3;
    
    void *p1 = (void*)&global_array[v & 0xFF];
    void *p2 = (void*)&global_big_array[(v * 2) & 0x1FF];
    
    /* This call will need to move many values to argument registers */
    int result = many_args_function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                    f1, f2, f3, p1, p2);
    
    return result + v;
}

/* Test 5: Mixed types and register classes */
int __attribute__((noinline)) test_mixed_types(int a, int b)
{
    /* Use explicit register variables to force specific register allocation */
    register int r10_var asm("r10") = a * 2;
    register int r11_var asm("r11") = b * 3;
    
    /* Mix different data types */
    long long ll1 = (long long)a * b;
    long long ll2 = (long long)r10_var * r11_var;
    
    double d1 = (double)a / (b ? b : 1);
    double d2 = (double)r10_var / (r11_var ? r11_var : 1);
    
    /* Structure with mixed types */
    struct mixed {
        int i;
        long long ll;
        double d;
    } m;
    
    m.i = a + b;
    m.ll = ll1 + ll2;
    m.d = d1 + d2;
    
    /* Access structure members with complex addressing */
    int result = m.i + (int)(m.ll >> 32) + (int)m.d;
    
    /* Force use of register variables */
    asm volatile("" : : "r"(r10_var), "r"(r11_var));
    
    return result;
}

/* Main function orchestrates all tests */
int main(int argc, char *argv[])
{
    /* Use command line arguments to prevent constant propagation */
    volatile int seed = argc;
    volatile int base = (argv[0] ? (intptr_t)argv[0] : 12345) & 0xFF;
    
    int total = 0;
    
    /* Run all tests to trigger different reload scenarios */
    total += test_register_pressure(base + 1, base + 2, base + 3, 
                                   base + 4, base + 5, base + 6);
    
    total += test_complex_addressing(base, base * 2);
    
    total += test_asm_clobber(base + 10, base + 20, base + 30, base + 40);
    
    total += test_many_args(base * 3);
    
    total += test_mixed_types(base + 7, base + 8);
    
    /* Use the result to prevent dead code elimination */
    printf("Total result: %d\n", total);
    
    return total > 0 ? 0 : 1;
}
