/* reload_test.c - Test program to trigger GCC reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to prevent constant propagation */
volatile int global_seed = 42;
volatile long global_offset = 4096;
int global_array[8192];

/* Function with many arguments to create register pressure */
int __attribute__((noinline)) many_args_function(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, double f3, void *p1, void *p2)
{
    /* Complex computation using all arguments */
    int sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    double product = f1 * f2 * f3;
    long addr_diff = (long)p2 - (long)p1;
    
    return sum + (int)product + (int)addr_diff;
}

/* Test 1: Extreme register pressure with many live scalars */
int __attribute__((noinline)) test_register_pressure(int a, int b, int c, int d, int e, int f)
{
    /* Force many independent computations to create register pressure */
    int t1 = a + b + global_seed;
    int t2 = c - d * global_seed;
    int t3 = e ^ f ^ global_seed;
    int t4 = a * c + global_seed;
    int t5 = b * d - global_seed;
    int t6 = e * f ^ global_seed;
    int t7 = t1 + t2 + global_seed;
    int t8 = t3 - t4 * global_seed;
    int t9 = t5 ^ t6 ^ global_seed;
    int t10 = t7 * t8 + global_seed;
    int t11 = t8 * t9 - global_seed;
    int t12 = t9 ^ t10 ^ global_seed;
    int t13 = t10 + t11 + global_seed;
    int t14 = t11 - t12 * global_seed;
    int t15 = t12 ^ t13 ^ global_seed;
    int t16 = t13 * t14 + global_seed;
    int t17 = t14 * t15 - global_seed;
    int t18 = t15 ^ t16 ^ global_seed;
    int t19 = t16 + t17 + global_seed;
    int t20 = t17 - t18 * global_seed;
    
    /* Use all temporaries to keep them live */
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
           t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20;
}

/* Test 2: Complex addressing modes */
int __attribute__((noinline)) test_complex_addressing(int index)
{
    /* Large immediate offset forcing reload */
    int val1 = global_array[4096];  /* Large immediate offset */
    int val2 = global_array[global_offset];  /* Volatile offset */
    
    /* Complex index calculation */
    int complex_index = (index * global_seed) / 7 + 256;
    int val3 = global_array[complex_index];
    
    /* Double indirection with computation */
    int *ptr = &global_array[1024];
    int val4 = ptr[index * 2 + global_seed % 16];
    
    /* Multi-word type forcing piecewise moves */
    long long big_val = ((long long)val1 << 32) | val2;
    struct { int a; int b; int c; } three_word = {val1, val2, val3};
    
    return (int)big_val + val3 + val4 + three_word.a + three_word.b + three_word.c;
}

/* Test 3: Inline assembly with clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int x, int y)
{
    int result;
    
    /* Computation before assembly */
    int a = x * 3 + global_seed;
    int b = y * 7 - global_seed;
    int c = a ^ b;
    int d = (a + b) * 2;
    
    /* Inline assembly clobbering many registers */
    __asm__ volatile (
        "# Dummy assembly\n"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (c), "r" (d)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
    );
    
    /* More computations after assembly */
    int e = result * 11 + global_seed;
    int f = e ^ d;
    int g = f * 13 - global_seed;
    
    return result + e + f + g;
}

/* Test 4: Explicit register variables */
int __attribute__((noinline)) test_explicit_registers(int x, int y, int z)
{
    /* Try to allocate specific registers (x86-64 specific) */
    register int r10_var asm("r10") = x + global_seed;
    register int r11_var asm("r11") = y * global_seed;
    register int r12_var asm("r12") = z ^ global_seed;
    
    /* Force use of these registers in computation */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        r10_var = r10_var * 3 + i;
        r11_var = r11_var / 2 + i;
        r12_var = r12_var ^ i;
        sum += r10_var + r11_var + r12_var;
    }
    
    return sum;
}

/* Test 5: Mixed types and calling convention pressure */
double __attribute__((noinline)) test_mixed_types(int a, double b, long c, float d)
{
    /* Many live values of different types */
    int i1 = a * 2;
    int i2 = i1 + global_seed;
    int i3 = i2 ^ (int)c;
    
    double d1 = b * 3.14159;
    double d2 = d1 + (double)global_seed;
    double d3 = d2 / 2.0;
    
    float f1 = d * 2.0f;
    float f2 = f1 + (float)global_seed;
    float f3 = f2 * 3.0f;
    
    long l1 = c << 2;
    long l2 = l1 + global_offset;
    long l3 = l2 ^ 0x12345678;
    
    /* Function call with many arguments */
    int result = many_args_function(
        i1, i2, i3, (int)l1, (int)l2,
        (int)d1, (int)d2, (int)f1, (int)f2, (int)f3,
        d1, d2, d3, &global_array[0], &global_array[1024]
    );
    
    return (double)result + d1 + d2 + d3 + f1 + f2 + f3;
}

/* Main orchestrator */
int main(int argc, char *argv[])
{
    /* Use argv to prevent constant propagation */
    int base = argc > 1 ? atoi(argv[1]) : 100;
    
    /* Initialize global array */
    for (int i = 0; i < 8192; i++) {
        global_array[i] = i * 3 + global_seed;
    }
    
    int total = 0;
    
    /* Run all tests to trigger different reload scenarios */
    total += test_register_pressure(base, base+1, base+2, base+3, base+4, base+5);
    
    total += test_complex_addressing(base % 1000);
    
    total += test_asm_clobber(base, base*2);
    
    total += test_explicit_registers(base, base+10, base+20);
    
    double dresult = test_mixed_types(base, base*1.5, base*100L, base*0.75f);
    total += (int)dresult;
    
    /* Use volatile to prevent dead code elimination */
    volatile int final_result = total;
    
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
