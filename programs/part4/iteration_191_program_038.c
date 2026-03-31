/* reload_test.c - Test program to trigger GCC's reload pass uncovered lines */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int global_int = 12345;
volatile long long global_ll = 9876543210LL;
volatile float global_float = 3.14159f;
volatile double global_double = 2.718281828459045;

/* Vector types for SIMD reloads */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function with multiple inline asm statements requiring reloads */
static long long trigger_reloads(int a, long long b, float c, double d, 
                                 v4si vec_int, v4sf vec_float) {
    long long result = 0;
    int out_int;
    long long out_ll;
    float out_float;
    double out_double;
    v4si out_vec_int;
    v4sf out_vec_float;
    
    /* Force register pressure by using many variables */
    register int r1 asm("r8") = a;
    register long long r2 asm("r9") = b;
    register float r3 asm("xmm8") = c;
    register double r4 asm("xmm9") = d;
    
    /* ASM 1: Mixed integer types with specific register constraints */
    /* This should trigger reloads due to register class mismatches */
    asm volatile (
        "mov %[out], %[in]\n\t"
        "add %[add], %[out]\n\t"
        : [out] "=r" (out_int)
        : [in] "r" (a),
          [add] "i" (global_int)  /* Immediate may need secondary reload */
        : "cc"
    );
    result += out_int;
    
    /* ASM 2: Floating point with memory constraints */
    /* Memory address computation may need secondary reload */
    asm volatile (
        "movsd %[in], %[out]\n\t"
        "addsd %[add], %[out]\n\t"
        : [out] "=x" (out_double)
        : [in] "m" (global_double),  /* Memory operand */
          [add] "x" (d)              /* XMM register constraint */
        : 
    );
    result += (long long)out_double;
    
    /* ASM 3: 64-bit operations with specific constraints */
    /* Using 'A' constraint (rax/rdx) which may conflict */
    asm volatile (
        "mov %[in], %%rax\n\t"
        "add %[add], %%rax\n\t"
        "mov %%rax, %[out]\n\t"
        : [out] "=r" (out_ll)
        : [in] "r" (b),
          [add] "r" (global_ll)
        : "rax", "cc"
    );
    result += out_ll;
    
    /* ASM 4: Vector operations with mismatched constraints */
    /* Vector to scalar may require special handling */
    asm volatile (
        "movdqa %[vec], %[out]\n\t"
        "paddd %[add], %[out]\n\t"
        : [out] "=x" (out_vec_int)
        : [vec] "x" (vec_int),
          [add] "m" (vec_int)  /* Same value, different constraint */
        : 
    );
    result += out_vec_int[0];
    
    /* ASM 5: Mixed mode operations (float in integer register) */
    /* This often requires secondary reloads */
    asm volatile (
        "movd %[in], %[out]\n\t"  /* Move float to general reg */
        "add $0x3f800000, %[out]\n\t"  /* Add 1.0f in hex */
        : [out] "=r" (out_int)
        : [in] "x" (c)  /* XMM input, general reg output */
        : 
    );
    result += out_int;
    
    /* ASM 6: Complex addressing mode with multiple constraints */
    /* Force address computation reload */
    asm volatile (
        "lea (%[base], %[index], 4), %[out]\n\t"
        : [out] "=r" (out_int)
        : [base] "r" (&global_int),
          [index] "r" (a)  /* Both in registers */
        : 
    );
    result += out_int;
    
    /* ASM 7: Output in memory with input in register */
    /* Memory output may need special handling */
    int mem_out;
    asm volatile (
        "mov %[in], %[out]\n\t"
        : [out] "=m" (mem_out)
        : [in] "r" (a)
        : 
    );
    result += mem_out;
    
    /* ASM 8: Clobber many registers to force spills */
    asm volatile (
        "mov %1, %0\n\t"
        "add %2, %0\n\t"
        : "=r" (out_int)
        : "r" (r1),  /* Forced register variable */
          "m" (global_int),
          "m" (global_float)  /* Extra memory operand */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15", "cc"
    );
    result += out_int;
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Use argv to create non-constant inputs */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Initialize test variables with non-constant values */
    int a = rand() % 1000;
    long long b = (long long)rand() * rand();
    float c = (float)rand() / RAND_MAX * 100.0f;
    double d = (double)rand() / RAND_MAX * 1000.0;
    
    /* Vector initialization */
    v4si vec_int = { a, a + 1, a + 2, a + 3 };
    v4sf vec_float = { c, c + 1.0f, c + 2.0f, c + 3.0f };
    
    /* Call the function multiple times to increase reload opportunities */
    long long total = 0;
    for (int i = 0; i < 10; i++) {
        total += trigger_reloads(a + i, b + i, c + i, d + i, 
                                vec_int, vec_float);
    }
    
    printf("Result: %lld\n", total);
    return (int)(total % 1000);
}
