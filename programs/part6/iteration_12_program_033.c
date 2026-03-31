/* reload_test.c - Comprehensive test to trigger GCC reload pass initialization */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to prevent constant propagation */
volatile int global_seed = 42;
volatile long global_offset = 4096;
int global_array[8192];
double global_doubles[1024];

/* Non-inline function with many arguments to force register pressure */
int __attribute__((noinline)) 
dummy_function(int a1, int a2, int a3, int a4, int a5,
               int a6, int a7, int a8, int a9, int a10,
               double f1, double f2, double f3, double f4)
{
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10
           + (int)(f1 + f2 + f3 + f4);
}

/* Test 1: Extreme register pressure with many live scalars */
int __attribute__((noinline))
test_register_pressure(int a, int b, int c, int d, int e, int f)
{
    /* Force many independent computations to create live ranges */
    int t1 = a + b + global_seed;
    int t2 = c + d + global_seed;
    int t3 = e + f + global_seed;
    int t4 = a * b - global_seed;
    int t5 = c * d - global_seed;
    int t6 = e * f - global_seed;
    int t7 = a ^ b ^ global_seed;
    int t8 = c ^ d ^ global_seed;
    int t9 = e ^ f ^ global_seed;
    int t10 = a & b & global_seed;
    int t11 = c & d & global_seed;
    int t12 = e & f & global_seed;
    int t13 = a | b | global_seed;
    int t14 = c | d | global_seed;
    int t15 = e | f | global_seed;
    int t16 = ~a + ~b + global_seed;
    int t17 = ~c + ~d + global_seed;
    int t18 = ~e + ~f + global_seed;
    int t19 = (a << 2) + (b >> 1);
    int t20 = (c << 3) + (d >> 2);
    int t21 = (e << 4) + (f >> 3);
    
    /* Use all temporaries in a complex expression */
    int result = t1 + t2 - t3 * t4 / (t5 + 1) + t6 - t7 + t8 - t9
                 + t10 * t11 - t12 + t13 - t14 + t15 - t16
                 + t17 * t18 - t19 + t20 - t21;
    
    /* Force another round of computations */
    int u1 = result + a;
    int u2 = result + b;
    int u3 = result + c;
    int u4 = result + d;
    int u5 = result + e;
    int u6 = result + f;
    int u7 = u1 * u2;
    int u8 = u3 * u4;
    int u9 = u5 * u6;
    int u10 = u7 + u8 + u9;
    
    return result + u10;
}

/* Test 2: Complex addressing modes */
int __attribute__((noinline))
test_complex_addressing(int index)
{
    int result = 0;
    
    /* Large immediate offset forcing reload */
    result += global_array[4096];  /* Large offset */
    result += global_array[global_offset];  /* Volatile offset */
    
    /* Complex index calculation */
    int complex_index = (index * 3 + 7) / 2;
    result += global_array[complex_index + 1024];
    
    /* Double register indirect-like pattern */
    int base = global_seed;
    result += global_array[base + complex_index * 2];
    
    /* Misaligned access with long long */
    long long *llptr = (long long*)&global_array[index & ~1];
    long long llval = *llptr;  /* May require multiple moves */
    result += (int)(llval >> 32) + (int)llval;
    
    /* Double with potential alignment issues */
    double dval = global_doubles[index % 1024];
    result += (int)dval;
    
    return result;
}

/* Test 3: Inline assembly with clobbered registers */
int __attribute__((noinline))
test_asm_clobber(int x, int y)
{
    int a = x * 3;
    int b = y * 7;
    int c = a + b;
    int d = a - b;
    
    /* Clobber many registers to force spills */
    __asm__ volatile (
        "/* Clobber many registers */"
        :
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15"
    );
    
    /* Use variables after clobber - they must be reloaded */
    int e = c * d;
    int f = a * c + b * d;
    
    __asm__ volatile (
        "/* Another clobber */"
        :
        :
        : "rax", "rbx", "rcx", "rdx"
    );
    
    return e + f;
}

/* Test 4: Function with many arguments */
int __attribute__((noinline))
test_many_args(void)
{
    /* Create many argument values with complex computations */
    int a1 = global_seed + 1;
    int a2 = global_seed + 2;
    int a3 = global_seed + 3;
    int a4 = global_seed + 4;
    int a5 = global_seed + 5;
    int a6 = global_seed + 6;
    int a7 = global_seed + 7;
    int a8 = global_seed + 8;
    int a9 = global_seed + 9;
    int a10 = global_seed + 10;
    
    double f1 = global_seed * 1.1;
    double f2 = global_seed * 1.2;
    double f3 = global_seed * 1.3;
    double f4 = global_seed * 1.4;
    
    /* Call forces register pressure for argument passing */
    return dummy_function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                         f1, f2, f3, f4);
}

/* Test 5: Mixed types and explicit register variables */
int __attribute__((noinline))
test_mixed_types(int x)
{
    /* Use explicit register variables to constrain allocation */
    register int r1 asm("r10") = x + 1;
    register int r2 asm("r11") = x + 2;
    
    /* Mix integer and floating point computations */
    int i1 = r1 * 2;
    int i2 = r2 * 3;
    double d1 = (double)i1 * 1.5;
    double d2 = (double)i2 * 2.5;
    
    /* Force conversion between types */
    long long ll1 = (long long)i1 * i2;
    long long ll2 = (long long)(d1 * d2);
    
    /* Access with potential alignment issues */
    struct {
        int a;
        char b;
        double c;
    } __attribute__((packed)) s;
    
    s.a = i1;
    s.b = (char)i2;
    s.c = d1;
    
    /* Force reloads by using all values */
    return i1 + i2 + (int)d1 + (int)d2 + (int)ll1 + (int)ll2 + s.a;
}

/* Main orchestrator */
int main(int argc, char *argv[])
{
    /* Initialize globals */
    for (int i = 0; i < 8192; i++) {
        global_array[i] = i;
    }
    for (int i = 0; i < 1024; i++) {
        global_doubles[i] = i * 1.5;
    }
    
    /* Use argv to prevent constant propagation */
    int base = argc > 1 ? atoi(argv[1]) : 100;
    
    int result = 0;
    
    /* Run all tests to trigger different reload scenarios */
    result += test_register_pressure(base, base+1, base+2, 
                                    base+3, base+4, base+5);
    
    result += test_complex_addressing(base % 1000);
    
    result += test_asm_clobber(base, base * 2);
    
    result += test_many_args();
    
    result += test_mixed_types(base);
    
    /* Complex addressing with volatile */
    volatile int *volatile_ptr = &global_array[0];
    for (int i = 0; i < 100; i++) {
        result += *(volatile_ptr + i + global_offset);
    }
    
    /* Final computation mixing everything */
    long long big_result = (long long)result * global_seed;
    big_result += (long long)global_array[4096] * global_array[8191];
    
    printf("Result: %lld\n", big_result);
    return (int)(big_result % 1000);
}
