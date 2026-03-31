/* reload_test.c - Comprehensive test to trigger various reload scenarios */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Force variables to be in memory to increase reload opportunities */
volatile int global_counter = 0;

int main(void) {
    /* ========== VARIABLE DECLARATIONS ========== */
    /* Diverse types to trigger different machine modes */
    int int_var1 = 0x12345678;
    int int_var2 = 0x9ABCDEF0;
    long long ll_var1 = 0x1122334455667788LL;
    long long ll_var2 = 0x8877665544332211LL;
    float float_var1 = 3.14159f;
    float float_var2 = 2.71828f;
    double double_var1 = 3.141592653589793;
    double double_var2 = 2.718281828459045;
    
    /* Vector types for SIMD reloads */
    __m128i vec_var1 = _mm_set_epi32(0xF0E1D2C3, 0xB4A59687, 0x78695A4B, 0x3C2D1E0F);
    __m128i vec_var2 = _mm_set_epi32(0x0F1E2D3C, 0x4B5A6978, 0x8796A5B4, 0xC3D2E1F0);
    
    /* Arrays for complex addressing modes */
    int multi_array[8][16] = {0};
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            multi_array[i][j] = i * 100 + j;
        }
    }
    
    /* Pointers for address computations */
    int *ptr1 = &multi_array[0][0];
    int *ptr2 = &multi_array[7][15];
    
    /* Variables for output from asm blocks */
    int out1, out2, out3;
    long long out_ll1, out_ll2;
    float out_float;
    double out_double;
    __m128i out_vec;
    
    /* ========== BLOCK A: REGISTER CLASS CONFLICT ========== */
    /* Force integer to float register reload */
    asm volatile (
        /* Request float register for integer variable */
        "movss %1, %%xmm0\n\t"
        "cvtsi2ss %2, %%xmm1\n\t"
        "addss %%xmm1, %%xmm0\n\t"
        "movss %%xmm0, %0"
        : "=f" (out_float)          /* Output in float register */
        : "f" (float_var1),         /* Input in float register */
          "r" (int_var1)            /* Input in general register - will need reload */
        : "xmm0", "xmm1", "memory"
    );
    
    /* ========== BLOCK B: COMPLEX ADDRESS RELOAD ========== */
    /* Complex array addressing that may not fit addressing mode */
    int index1 = 3, index2 = 7;
    asm volatile (
        /* Load from complex address computation */
        "movl (%[base], %[idx1], 4), %%eax\n\t"    /* base + idx1*4 */
        "addl (%[base], %[idx2], 8), %%eax\n\t"    /* base + idx2*8 */
        "movl %%eax, %[result]"
        : [result] "=r" (out1)
        : [base] "r" (ptr1),        /* Base pointer - may need reload */
          [idx1] "r" (index1),      /* Scaled index 1 */
          [idx2] "r" (index2)       /* Scaled index 2 */
        : "eax", "memory"
    );
    
    /* Even more complex addressing with displacement */
    struct nested {
        int a[4];
        struct {
            int x;
            int y[3];
        } inner[2];
    } complex_struct = {0};
    
    int *struct_ptr = &complex_struct.inner[1].y[2];
    asm volatile (
        "movl $0xDEADBEEF, (%[ptr])"
        : 
        : [ptr] "r" (struct_ptr)    /* Complex pointer expression */
        : "memory"
    );
    
    /* ========== BLOCK C: EARLY-CLOBBER MULTIPLE OUTPUTS ========== */
    /* Early-clobber with multiple outputs and reused inputs */
    asm volatile (
        /* out1 gets input1 * 2, out2 gets input2 + input1 */
        /* Early-clobber on out2 means it's written before all inputs read */
        "leal (%[in1], %[in1]), %[out1]\n\t"    /* out1 = in1 * 2 */
        "movl %[in2], %[out2]\n\t"              /* Early clobber! */
        "addl %[in1], %[out2]\n\t"              /* out2 = in2 + in1 */
        "addl %[out1], %[out2]\n\t"             /* Use out1 as input */
        "movl %[out2], %[out3]"                 /* Chain dependencies */
        : [out1] "=r" (out1),                   /* Output 1 */
          [out2] "=&r" (out2),                  /* Early-clobber output */
          [out3] "=r" (out3)                    /* Output 3 */
        : [in1] "r" (int_var1),                 /* Input used twice */
          [in2] "r" (int_var2)                  /* Another input */
        : "cc"
    );
    
    /* ========== BLOCK D: SECONDARY RELOAD PATTERNS ========== */
    /* Pattern that often requires secondary reloads */
    /* Large immediate to vector register (common on AArch64, x86 with constraints) */
    asm volatile (
        /* Simulate instruction needing constant reload */
        "movdqa %1, %%xmm0\n\t"
        "paddd %2, %%xmm0\n\t"
        "movdqa %%xmm0, %0"
        : "=x" (out_vec)
        : "x" (vec_var1),
          "m" (vec_var2)    /* Memory operand - may need register reload */
        : "xmm0"
    );
    
    /* Another secondary reload pattern: register to register with mode change */
    asm volatile (
        /* Convert and move between register classes */
        "movq %1, %%xmm0\n\t"       /* Move 64-bit integer to XMM */
        "movq %%xmm0, %0"           /* Move back to general register */
        : "=r" (out_ll1)
        : "r" (ll_var1)             /* Will likely need reload through memory */
        : "xmm0"
    );
    
    /* ========== BLOCK E: MIXED MODE RELOADS ========== */
    /* Mixed integer sizes forcing partial register reloads */
    asm volatile (
        /* Operate on different parts of registers */
        "movl %1, %%eax\n\t"
        "movl %2, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movq %3, %%rcx\n\t"
        "addq %%rcx, %%rax\n\t"
        "movl %%eax, %0\n\t"
        "shrq $32, %%rax\n\t"
        "movl %%eax, %4"
        : "=r" (out1), "=r" (out2)
        : "r" (int_var1), "r" (int_var2), "r" (ll_var1)
        : "rax", "rbx", "rcx", "cc"
    );
    
    /* ========== BLOCK F: HIGH REGISTER PRESSURE ========== */
    /* Many operands to force register spilling and reloading */
    register int r0 asm("r8") = int_var1;
    register int r1 asm("r9") = int_var2;
    register int r2 asm("r10") = out1;
    register int r3 asm("r11") = out2;
    register int r4 asm("r12") = out3;
    
    asm volatile (
        /* Many register operations to increase pressure */
        "addl %1, %0\n\t"
        "subl %2, %0\n\t"
        "imull %3, %0\n\t"
        "addl %4, %0\n\t"
        "movl %0, %5"
        : "+r" (r0), "+r" (r1), "+r" (r2)
        : "r" (r3), "r" (r4), "m" (global_counter)
        : "cc"
    );
    
    /* ========== PREVENT DEAD CODE ELIMINATION ========== */
    /* Compute checksum from all modified variables */
    int checksum = 0;
    checksum ^= out1;
    checksum ^= out2;
    checksum ^= out3;
    checksum ^= (int)out_float;
    checksum ^= (int)out_double;
    checksum ^= out_ll1 ^ (out_ll1 >> 32);
    
    /* Mix in vector variable */
    int vec_elems[4];
    _mm_storeu_si128((__m128i*)vec_elems, out_vec);
    for (int i = 0; i < 4; i++) {
        checksum ^= vec_elems[i];
    }
    
    /* Use all array elements to prevent optimization */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            checksum ^= multi_array[i][j];
        }
    }
    
    printf("Checksum: 0x%08X\n", checksum);
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
