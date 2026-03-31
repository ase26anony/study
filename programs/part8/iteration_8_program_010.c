/* reload_test.c - Comprehensive test to trigger multiple reload types in GCC */

#include <stdint.h>
#include <stdio.h>
#include <xmmintrin.h>  /* For vector types */

/* Force no optimization on specific variables */
#define NOOPT __attribute__((optimize("O0")))

/* Complex structure to force address computations */
struct nested {
    int a[8][8];
    double b[4][4];
    struct nested *next;
};

/* Global variables to increase register pressure */
int global_array[256];
double global_doubles[128];
__m128i global_vectors[64];

NOOPT int main(void) {
    /* ========== VARIABLE DECLARATIONS ========== */
    /* Diverse types to trigger different machine modes */
    int int_var = 0x12345678;
    long long_var = 0x9ABCDEF012345678LL;
    float float_var = 3.14159f;
    double double_var = 2.718281828459045;
    __m128i vector_var = _mm_set_epi32(1, 2, 3, 4);
    
    /* Arrays for complex addressing */
    int multi_array[16][16];
    double dbl_multi[8][8];
    
    /* Pointers for pointer chains */
    struct nested nested1, nested2, nested3;
    struct nested *ptr_chain = &nested1;
    nested1.next = &nested2;
    nested2.next = &nested3;
    nested3.next = NULL;
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            multi_array[i][j] = i * 16 + j;
        }
    }
    
    /* ========== BLOCK A: REGISTER CLASS CONFLICT ========== */
    /* Force integer into floating-point register */
    {
        int int_source = 0xDEADBEEF;
        double fp_result;
        
        /* This asm requests a floating-point register for integer input */
        asm volatile (
            /* Convert integer to double through FPU */
            "mov %1, %%eax\n\t"
            "cvtsi2sd %%eax, %0\n\t"
            : "=f" (fp_result)      /* Output in FP register */
            : "r" (int_source)      /* Input in general register */
            : "%eax", "memory"
        );
        
        double_var += fp_result;
    }
    
    /* ========== BLOCK B: COMPLEX ADDRESS RELOAD ========== */
    /* Complex array addressing that may not fit addressing mode */
    {
        int index1 = 5, index2 = 7, index3 = 3;
        int result;
        
        /* Complex address computation with multiple indices */
        asm volatile (
            "movl %c[array](%[i1],%[i2],4), %0\n\t"  /* array[i1][i2] */
            : "=r" (result)
            : [array] "i" (multi_array), 
              [i1] "r" (index1), 
              [i2] "r" (index2),
              "m" (multi_array[0][0])  /* Memory operand */
            : "memory"
        );
        
        /* Even more complex: nested structure with pointer chain */
        int chain_result;
        asm volatile (
            "movl (%[ptr],%[idx],4), %0\n\t"
            : "=r" (chain_result)
            : [ptr] "r" (&ptr_chain->next->a[0][0]),
              [idx] "r" (index3),
              "m" (ptr_chain->next->a[0][0])
            : "memory"
        );
        
        int_var += result + chain_result;
    }
    
    /* ========== BLOCK C: EARLY-CLOBBER MULTIPLE OUTPUTS ========== */
    /* Multiple outputs with early clobber */
    {
        int in1 = 100, in2 = 200, in3 = 300;
        int out1, out2, out3;
        
        /* Early clobber on out2 means it's written before all inputs read */
        asm volatile (
            "movl %2, %0\n\t"      /* out1 = in1 */
            "addl %3, %0\n\t"      /* out1 += in2 */
            "movl %0, %1\n\t"      /* out2 = out1 (early clobber!) */
            "subl %4, %1\n\t"      /* out2 -= in3 */
            "leal (%0,%1,2), %k3\n\t" /* Use both outputs as inputs */
            : "=&r" (out1), "=&r" (out2), "=r" (out3)
            : "r" (in1), "r" (in2), "r" (in3), "2" (0)
            : "cc"
        );
        
        long_var += out1 + out2 + out3;
    }
    
    /* ========== BLOCK D: SECONDARY RELOAD PATTERNS ========== */
    /* Patterns that often require secondary reloads */
    {
        /* Large immediate to vector register (common on AArch64, 
           may require GPR intermediate on some x86 patterns) */
        __m128i vec_result;
        long long large_imm = 0x123456789ABCDEF0LL;
        
        asm volatile (
            "movq %1, %0\n\t"      /* Move 64-bit immediate */
            "punpcklqdq %0, %0\n\t" /* Duplicate to 128-bit */
            : "=x" (vec_result)
            : "r" (large_imm)      /* May need reload to get into register */
            : "memory"
        );
        
        /* Memory operand with complex address to XMM register */
        double dbl_result;
        int idx = 7;
        
        asm volatile (
            "movsd (%[base],%[idx],8), %0\n\t"
            : "=x" (dbl_result)
            : [base] "r" (global_doubles),
              [idx] "r" (idx),
              "m" (global_doubles[0])
            : "memory"
        );
        
        /* Mix vector and scalar */
        vector_var = _mm_add_epi32(vector_var, vec_result);
        double_var += dbl_result;
    }
    
    /* ========== BLOCK E: MIXED MODE RELOADS ========== */
    /* Different machine modes in same asm */
    {
        char char_var = 'A';
        short short_var = 0x1234;
        int int_result;
        float float_result;
        
        /* Mixed width operations */
        asm volatile (
            "movsbl %1, %0\n\t"    /* Sign extend byte to int */
            "addw %2, %w0\n\t"     /* Add short to low word */
            "cvtsi2ss %0, %3\n\t"  /* Convert to float */
            : "=r" (int_result), "=X" (float_result)
            : "r" (char_var), "r" (short_var), "0" (0), "3" (0.0f)
            : "cc"
        );
        
        float_var += float_result;
    }
    
    /* ========== BLOCK F: HIGH REGISTER PRESSURE ========== */
    /* Many operands to force spill/reload */
    {
        int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
        int r1, r2, r3, r4, r5, r6, r7, r8;
        
        /* Many input/output operands to increase register pressure */
        asm volatile (
            "movl %9, %0\n\t"
            "addl %10, %1\n\t"
            "subl %11, %2\n\t"
            "imull %12, %3\n\t"
            "andl %13, %4\n\t"
            "orl %14, %5\n\t"
            "xorl %15, %6\n\t"
            "leal (%0,%1,1), %7\n\t"
            : "=r" (r1), "=r" (r2), "=r" (r3), "=r" (r4),
              "=r" (r5), "=r" (r6), "=r" (r7), "=r" (r8)
            : "0" (a), "1" (b), "2" (c), "3" (d),
              "4" (e), "5" (f), "6" (g), "7" (h)
            : "cc"
        );
        
        int_var += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
    }
    
    /* ========== FINAL CHECKSUM ========== */
    /* Compute checksum to prevent dead code elimination */
    int checksum = int_var;
    checksum += (int)long_var;
    checksum += (int)float_var;
    checksum += (int)double_var;
    checksum += _mm_extract_epi32(vector_var, 0);
    
    /* Use checksum in output */
    printf("Reload test checksum: %d\n", checksum);
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
