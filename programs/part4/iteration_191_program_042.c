/* reload_test.c - Test program to trigger GCC reload pass uncovered lines */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global volatile variables to prevent optimization */
volatile int global_int = 12345;
volatile long long global_ll = 9876543210LL;
volatile float global_float = 3.14159f;
volatile double global_double = 2.718281828459045;

/* Vector types for SIMD reloads */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function with complex inline assembly to trigger reloads */
static long long trigger_reloads(int a, long long b, float c, double d,
                                 v4si vec_int, v4sf vec_float) {
    long long result = 0;
    int out1, out2;
    long long out3;
    float out4;
    double out5;
    v4si out_vec_int;
    v4sf out_vec_float;
    
    /* Force register pressure by using many variables */
    register int r1 asm("r8") = a;
    register int r2 asm("r9") = global_int;
    register long long r3 asm("r10") = b;
    
    /* ASM 1: Mixed modes with specific register constraints */
    /* This should trigger reloads with secondary info */
    asm volatile (
        /* Move with different sized operands */
        "movl %[in1], %%eax\n\t"
        "movq %[in2], %%rbx\n\t"
        /* Force a reload by using conflicting constraints */
        "addl %%eax, %[out1]\n\t"
        "addq %%rbx, %[out3]\n\t"
        : [out1] "=r" (out1), [out3] "=r" (out3)
        : [in1] "rm" (r1), [in2] "rm" (r3),
          "0" (0), "1" (0)  /* Start outputs at 0 */
        : "eax", "ebx", "memory", "cc"
    );
    result += out1 + out3;
    
    /* ASM 2: Floating point with integer conversion */
    /* Mixing modes often requires secondary reloads */
    asm volatile (
        /* Convert float to int and back */
        "cvttss2si %[fin], %%eax\n\t"
        "cvtsi2ssl %%eax, %[fout]\n\t"
        /* Also use double precision */
        "cvtsi2sdq %[llin], %[dout]\n\t"
        : [fout] "=x" (out4), [dout] "=x" (out5)
        : [fin] "xm" (c), [llin] "rm" (b),
          "0" (0.0f), "1" (0.0)  /* Initialize outputs */
        : "eax", "memory", "cc"
    );
    result += (long long)(out4 * 1000) + (long long)out5;
    
    /* ASM 3: Vector operations with memory constraints */
    /* Vector reloads often need special handling */
    asm volatile (
        /* Load vector, do operation, store */
        "movdqu %[vin], %%xmm0\n\t"
        "paddd %%xmm0, %%xmm0\n\t"  /* Multiply by 2 */
        "movdqu %%xmm0, %[vout]\n\t"
        : [vout] "=xm" (out_vec_int)
        : [vin] "xm" (vec_int)
        : "xmm0", "memory"
    );
    
    /* Use vector result */
    for (int i = 0; i < 4; i++) {
        result += out_vec_int[i];
    }
    
    /* ASM 4: Complex addressing modes that need reloads */
    /* Force address computation into registers */
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i * 2;
    }
    
    asm volatile (
        /* Complex address calculation */
        "leaq %[array], %%rax\n\t"
        "movl %[index], %%ecx\n\t"
        "shlq $2, %%rcx\n\t"  /* Multiply index by 4 */
        "addq %%rcx, %%rax\n\t"
        "movl (%%rax), %[out2]\n\t"
        : [out2] "=r" (out2)
        : [array] "m" (array), [index] "rm" (a & 0x7F)
        : "rax", "rcx", "ecx", "memory"
    );
    result += out2;
    
    /* ASM 5: Multiple outputs with different constraints */
    /* This should create multiple reload entries */
    asm volatile (
        /* Multiple independent operations */
        "mov %[in_a], %[out_a]\n\t"
        "mov %[in_b], %[out_b]\n\t"
        "mov %[in_c], %[out_c]\n\t"
        : [out_a] "=r" (out1),
          [out_b] "=r" (out2),
          [out_c] "=r" (r1)  /* Reuse input register */
        : [in_a] "rm" (global_int),
          [in_b] "rm" (a),
          [in_c] "rm" (r2)
        : "memory"
    );
    result += out1 + out2 + r1;
    
    /* ASM 6: In/out operand with '+' constraint */
    /* These often need special reload handling */
    int inout = a * 2;
    asm volatile (
        "addl $100, %[io]\n\t"
        "imull $3, %[io], %[io]\n\t"
        : [io] "+r" (inout)
        :
        : "cc"
    );
    result += inout;
    
    /* ASM 7: Force use of specific register classes */
    /* Try to force floating point register reloads */
    double d1 = d;
    double d2 = global_double;
    asm volatile (
        /* Floating point operation */
        "addsd %[d2], %[d1]\n\t"
        "mulsd %[d1], %[d1]\n\t"
        : [d1] "+x" (d1)
        : [d2] "xm" (d2)
        : "cc"
    );
    result += (long long)d1;
    
    return result;
}

/* Wrapper to ensure function isn't inlined */
__attribute__((noinline))
static long long reload_wrapper(int a, long long b, float c, double d) {
    /* Create vector values */
    v4si vec_int = {a, a+1, a+2, a+3};
    v4sf vec_float = {c, c+1.0f, c+2.0f, c+3.0f};
    
    return trigger_reloads(a, b, c, d, vec_int, vec_float);
}

int main(int argc, char *argv[]) {
    /* Use argv to create variable inputs to prevent constant propagation */
    int base = argc > 1 ? atoi(argv[1]) : 42;
    
    int int_val = base * 2;
    long long ll_val = (long long)base * 1000000;
    float float_val = (float)base / 3.0f;
    double double_val = (double)base / 7.0;
    
    /* Call the reload-intensive function multiple times */
    long long total = 0;
    for (int i = 0; i < 10; i++) {
        total += reload_wrapper(int_val + i, 
                               ll_val + i * 1000,
                               float_val + i * 0.1f,
                               double_val + i * 0.01);
    }
    
    printf("Result: %lld\n", total);
    
    /* Use the result to prevent dead code elimination */
    if (total > 1000000) {
        printf("Large result detected\n");
    }
    
    return (int)(total % 1000);
}
