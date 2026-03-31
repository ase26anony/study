/* reload_coverage.c
 * Designed to trigger uncovered lines in GCC's reload.cc (lines 1381-1399)
 * Compile with: gcc -O1 -fno-inline -fdump-rtl-reload -S reload_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int g_volatile_int = 12345;
volatile long long g_volatile_ll = 9876543210LL;
volatile float g_volatile_float = 3.14159f;
volatile double g_volatile_double = 2.718281828459045;

/* Function with complex inline assembly to trigger reloads */
static long long trigger_reloads(int a, long long b, float c, double d, 
                                 int *ptr1, long long *ptr2) {
    long long accumulator = 0;
    int out_int1, out_int2;
    long long out_ll1, out_ll2;
    float out_float;
    double out_double;
    
    /* ASM 1: Mixed types with specific register constraints
     * This forces reloads due to register class mismatches */
    asm volatile (
        /* Move int to output with 'r' constraint, but we'll use it in memory addressing */
        "movl %[in1], %%eax\n\t"
        "addl %%eax, %[out1]\n\t"
        : [out1] "=r" (out_int1)        /* Output in general register */
        : [in1] "rm" (a),               /* Input: register or memory */
          "m" (*ptr1)                   /* Memory input forcing address computation */
        : "eax", "memory", "cc"         /* Clobber eax, memory, and flags */
    );
    accumulator += out_int1;
    
    /* ASM 2: Floating point with integer constraints
     * This may require secondary reloads on some architectures */
    asm volatile (
        /* Convert float to int representation */
        "movd %[in_float], %%eax\n\t"
        "movl %%eax, %[out_int]\n\t"
        : [out_int] "=rm" (out_int2)    /* Output: register or memory */
        : [in_float] "x" (c)            /* Input in SSE register */
        : "eax", "cc"
    );
    accumulator += out_int2;
    
    /* ASM 3: 64-bit operations with 32-bit constraints
     * This creates mode mismatches */
    asm volatile (
        /* Handle 64-bit value in 32-bit chunks */
        "movl %%ebx, %%eax\n\t"
        "movl %%ecx, %%edx\n\t"
        : "=A" (out_ll1)                /* Output in edx:eax pair */
        : "b" ((int)(b & 0xFFFFFFFF)),  /* Low 32 bits in ebx */
          "c" ((int)(b >> 32)),         /* High 32 bits in ecx */
          "m" (g_volatile_ll)           /* Memory operand for pressure */
        : "eax", "edx", "ebx", "ecx", "memory", "cc"
    );
    accumulator += out_ll1;
    
    /* ASM 4: Complex addressing modes that may require secondary reloads */
    asm volatile (
        /* Load from computed address */
        "leal (%[base], %[index], 4), %%eax\n\t"
        "movl (%%eax), %%ebx\n\t"
        "movl %%ebx, %[result]\n\t"
        : [result] "=r" (out_int1)      /* Output */
        : [base] "r" (ptr1),            /* Base register */
          [index] "r" (a & 3),          /* Index register */
          "m" (*(int (*)[4])ptr1)       /* Memory input with complex type */
        : "eax", "ebx", "memory", "cc"
    );
    accumulator += out_int1;
    
    /* ASM 5: Double precision with specific constraints
     * This tests floating point reloads */
    asm volatile (
        /* Simple double operation */
        "movsd %[in_double], %%xmm0\n\t"
        "addsd %[mem_double], %%xmm0\n\t"
        "movsd %%xmm0, %[out_double]\n\t"
        : [out_double] "=xm" (out_double)  /* Output in XMM or memory */
        : [in_double] "xm" (d),            /* Input in XMM or memory */
          [mem_double] "m" (g_volatile_double)  /* Memory operand */
        : "xmm0", "cc"
    );
    /* Convert double to int for accumulator */
    accumulator += (long long)out_double;
    
    /* ASM 6: Multiple outputs with tied operands
     * This creates complex reload scenarios */
    register int r1 asm("esi");
    register int r2 asm("edi");
    r1 = a * 2;
    r2 = a * 3;
    
    asm volatile (
        /* Swap and operate on registers */
        "xchgl %%esi, %%edi\n\t"
        "addl %%esi, %%edi\n\t"
        "movl %%edi, %[out1]\n\t"
        "movl %%esi, %[out2]\n\t"
        : [out1] "=rm" (out_int1), [out2] "=rm" (out_int2)
        : "S" (r1), "D" (r2)          /* Inputs in specific registers */
        : "cc"
    );
    accumulator += out_int1 + out_int2;
    
    /* ASM 7: Vector-style operation (simulated with integers)
     * This tests multi-register reloads */
    struct two_ints { int a; int b; } vec = {a, a + 1};
    
    asm volatile (
        /* Process two integers together */
        "movl %[vec_a], %%eax\n\t"
        "movl %[vec_b], %%ebx\n\t"
        "imull %%ebx, %%eax\n\t"
        "movl %%eax, %[result]\n\t"
        : [result] "=rm" (out_int1)
        : [vec_a] "rm" (vec.a),       /* Structure member 1 */
          [vec_b] "rm" (vec.b),       /* Structure member 2 */
          "m" (vec)                   /* Whole structure in memory */
        : "eax", "ebx", "memory", "cc"
    );
    accumulator += out_int1;
    
    return accumulator;
}

/* Main function that creates varied input values */
int main(int argc, char *argv[]) {
    /* Use argv to create non-constant inputs */
    int base_val = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Local variables with different types */
    int int_var = base_val * 3;
    long long ll_var = (long long)base_val * 1000000;
    float float_var = (float)base_val / 7.0f;
    double double_var = (double)base_val * 1.23456789;
    
    /* Pointers for memory operands */
    int int_array[4] = {base_val, base_val + 1, base_val + 2, base_val + 3};
    long long ll_array[2] = {ll_var, ll_var + 1};
    
    /* Call the function multiple times with different arguments */
    long long result1 = trigger_reloads(int_var, ll_var, float_var, double_var,
                                       int_array, ll_array);
    
    /* Modify values and call again */
    int_var += argc;
    ll_var += result1;
    float_var *= 1.5f;
    double_var /= 2.0;
    
    long long result2 = trigger_reloads(int_var, ll_var, float_var, double_var,
                                       int_array + 1, ll_array + 1);
    
    /* Use results to prevent optimization */
    printf("Result1: %lld\n", result1);
    printf("Result2: %lld\n", result2);
    printf("Total: %lld\n", result1 + result2);
    
    return (int)((result1 + result2) & 0x7FFFFFFF);
}
