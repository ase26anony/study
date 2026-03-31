/* reload_test.c - Test program to trigger GCC's reload pass initialization */
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
    double f1, double f2, void *p1, void *p2, long long ll1)
{
    /* Force use of all arguments */
    volatile int result = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    result += (int)f1 + (int)f2;
    result += (int)(intptr_t)p1 + (int)(intptr_t)p2;
    result += (int)ll1;
    return result;
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
    int t14 = b << 3;
    int t15 = c << 1;
    int t16 = d << 4;
    int t17 = t1 + t2;
    int t18 = t3 + t4;
    int t19 = t5 + t6;
    int t20 = t7 + t8;
    int t21 = t9 + t10;
    int t22 = t11 + t12;
    int t23 = t13 + t14;
    int t24 = t15 + t16;
    
    /* Force all values to be used in final computation */
    int result = t17 + t18 + t19 + t20 + t21 + t22 + t23 + t24;
    
    /* Additional pressure with floating point */
    double ft1 = (double)t1 * 1.1;
    double ft2 = (double)t2 * 2.2;
    double ft3 = (double)t3 * 3.3;
    double ft4 = (double)t4 * 4.4;
    
    result += (int)(ft1 + ft2 + ft3 + ft4);
    
    return result;
}

/* Test 2: Complex addressing modes requiring reloads */
int __attribute__((noinline)) test_complex_addressing(volatile int idx1, volatile int idx2)
{
    int result = 0;
    
    /* Large immediate offset - may require reload */
    result += global_array[4096];
    result += global_array[8192];
    
    /* Variable index with computation - double register indirect */
    result += global_array[idx1 * 2 + idx2];
    
    /* Complex base computation */
    int complex_base = (idx1 * idx2) + (idx1 >> 2) - (idx2 << 1);
    result += global_array[complex_base + 100];
    
    /* Multi-word access forcing piecewise moves */
    long long ll_val = global_big_array[idx1];
    result += (int)ll_val;
    
    /* Misaligned-type access on architectures with alignment requirements */
    struct __attribute__((packed)) misaligned {
        char c;
        int i;
        long long ll;
    } m;
    
    /* Force reload by taking address of misaligned member */
    int *misaligned_ptr = &m.i;
    result += *misaligned_ptr;
    
    /* Double with potential alignment issues */
    double d_val = global_double_array[idx2];
    result += (int)d_val;
    
    return result;
}

/* Test 3: Inline assembly with many clobbered registers */
int __attribute__((noinline)) test_asm_clobber(volatile int x, volatile int y)
{
    int a = x * 2;
    int b = y * 3;
    int c = x + y;
    int d = x - y;
    
    /* Clobber many registers - forces spills around asm */
    __asm__ volatile (
        "# Dummy assembly that clobbers registers\n"
        "nop\n"
        : 
        : 
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
    );
    
    /* Use values after asm - they must be reloaded */
    int result = a + b + c + d;
    
    /* More computation with clobber */
    __asm__ volatile (
        "# More clobbering\n"
        "nop\n"
        : 
        : 
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi"
    );
    
    result *= 2;
    return result;
}

/* Test 4: Explicit register variables to force specific register classes */
int __attribute__((noinline)) test_register_variables(volatile int x)
{
    /* Force use of specific registers */
    register int r10_val asm("r10") = x * 2;
    register int r11_val asm("r11") = x * 3;
    
    /* Use them in computation */
    int result = 0;
    
    /* Inline asm to force specific register usage */
    __asm__ volatile (
        "addl %%r10d, %0\n"
        "addl %%r11d, %0\n"
        : "+r" (result)
        : 
        : "r10", "r11"
    );
    
    /* Force x87 register usage if compiled without SSE */
    volatile long double ld1 = 3.14159265358979323846L;
    volatile long double ld2 = 2.71828182845904523536L;
    volatile long double ld3 = ld1 * ld2;
    
    result += (int)ld3;
    
    return result;
}

/* Test 5: Mixed types and calling convention pressure */
int __attribute__((noinline)) test_mixed_types(volatile int base)
{
    /* Create many values of different types */
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
    
    double d1 = (double)base * 1.1;
    double d2 = (double)base * 2.2;
    
    void *p1 = (void*)&global_array[0];
    void *p2 = (void*)&global_array[100];
    
    long long ll1 = (long long)base * 1000LL;
    
    /* Call function with many arguments - forces register/stack moves */
    int result = many_args_function(
        i1, i2, i3, i4, i5, i6, i7, i8, i9, i10,
        d1, d2, p1, p2, ll1
    );
    
    /* More computation after call */
    result += test_register_pressure(i1, i2, i3, i4);
    
    return result;
}

/* Main orchestrator */
int main(int argc, char *argv[])
{
    /* Use argv to prevent constant propagation */
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    
    int total = 0;
    
    /* Run all tests to trigger different reload scenarios */
    total += test_register_pressure(seed, seed+1, seed+2, seed+3);
    total += test_complex_addressing(seed % 100, (seed * 3) % 100);
    total += test_asm_clobber(seed, seed * 2);
    total += test_register_variables(seed);
    total += test_mixed_types(seed);
    
    /* Also test in a loop with varying values */
    for (int i = 0; i < 10; i++) {
        total += test_register_pressure(seed + i, seed + i + 1, 
                                       seed + i + 2, seed + i + 3);
    }
    
    /* Access global with large offset - might trigger reload */
    total += global_array[5000];
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
