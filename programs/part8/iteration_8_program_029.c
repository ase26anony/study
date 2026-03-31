/* reload_test.c - Comprehensive test to trigger multiple reload types in GCC */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <xmmintrin.h>  /* For vector types */

/* Force no optimization on specific variables */
#define NOOPT __attribute__((optimize("O0")))

/* Complex structure to force address computations */
struct nested {
    int data[8][8];
    double matrix[4][4];
    struct nested *next;
};

/* Global variables to increase register pressure */
int global_int = 42;
double global_double = 3.14159;
float global_float_array[32];
__m128i global_vector;

NOOPT int main(void) {
    /* Diverse variable declarations with different types and sizes */
    int int_var = 1;
    long long_var = 2;
    float float_var = 3.0f;
    double double_var = 4.0;
    long long longlong_var = 5;
    __m128i vector_var = _mm_setzero_si128();
    int *int_ptr = &int_var;
    double *double_ptr = &double_var;
    
    /* Arrays for complex addressing */
    int multi_array[16][16];
    double complex_array[8][8][8];
    struct nested nested_array[4];
    struct nested *nested_ptr = &nested_array[0];
    
    /* Initialize arrays to prevent optimization */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            multi_array[i][j] = i * 16 + j;
        }
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                complex_array[i][j][k] = i * 64.0 + j * 8.0 + k;
            }
        }
    }
    
    /* BLOCK A: Register Class Conflict Reload */
    /* Force integer into floating-point register */
    {
        int int_input = 100;
        double fp_output;
        
        /* This asm requires moving an integer value to FP register */
        asm volatile (
            "mov %1, %%eax\n\t"          /* Load integer to eax */
            "cvtsi2sd %%eax, %0\n\t"     /* Convert to double, needs FP reg */
            : "=f" (fp_output)           /* Output in FP register */
            : "r" (int_input)            /* Input in general register */
            : "%eax", "memory"
        );
        
        double_var += fp_output;
    }
    
    /* BLOCK B: Complex Address Reload with Non-trivial Addressing */
    {
        int i = 3, j = 5, k = 7;
        double result;
        
        /* Complex addressing mode that may need reloading */
        asm volatile (
            "movsd (%1), %0\n\t"         /* Load from complex address */
            : "=x" (result)              /* Output in SSE register */
            : "r" (&complex_array[i][j][k] + (i * j * k) / 8)  /* Complex address */
            : "memory"
        );
        
        /* Even more complex addressing with structure */
        int struct_offset;
        asm volatile (
            "mov (%1, %2, 4), %0\n\t"    /* base + index*4 addressing */
            : "=r" (struct_offset)
            : "r" (nested_ptr), "r" (i * 16 + j)  /* Complex index computation */
            : "memory"
        );
        
        double_var += result + struct_offset;
    }
    
    /* BLOCK C: Early-Clobber Multiple Output Reloads */
    {
        int in1 = 10, in2 = 20, in3 = 30;
        int out1, out2, out3;
        
        /* Early-clobber on out2 forces reloads */
        asm volatile (
            "mov %2, %0\n\t"             /* out1 = in1 */
            "imul %3, %0\n\t"            /* out1 *= in2 (uses out1 as input!) */
            "mov %0, %1\n\t"             /* out2 = out1 (early clobbered) */
            "add %4, %1\n\t"             /* out2 += in3 */
            "lea (%0, %1, 2), %2\n\t"    /* out3 = out1 + out2*2 */
            : "=&r" (out1), "=&r" (out2), "=r" (out3)  /* Two early-clobber outputs */
            : "r" (in1), "r" (in2), "r" (in3)
            : "cc"
        );
        
        int_var += out1 + out2 + out3;
    }
    
    /* BLOCK D: Secondary Reload Pattern */
    /* Simulate pattern needing temporary register */
    {
        uint64_t large_constant = 0x123456789ABCDEF0ULL;
        uint64_t result64;
        
        /* Pattern that might need secondary reload on some arches */
        asm volatile (
            "mov %1, %%rax\n\t"          /* Load 64-bit constant */
            "ror $32, %%rax\n\t"         /* Rotate - needs same register */
            "mov %%rax, %0\n\t"          /* Store result */
            : "=r" (result64)
            : "ri" (large_constant)      /* 'i' constraint for constant */
            : "%rax", "cc"
        );
        
        longlong_var += result64;
    }
    
    /* BLOCK E: Mixed Mode Reloads (Different Data Sizes) */
    {
        char char_var = 'A';
        short short_var = 256;
        int int_result;
        float float_result;
        
        /* Mixed size operations forcing mode conversions */
        asm volatile (
            "movsx %1, %%eax\n\t"        /* Sign extend char to int */
            "add %2, %%eax\n\t"          /* Add short */
            "mov %%eax, %0\n\t"          /* Store int result */
            : "=r" (int_result)
            : "r" (char_var), "r" (short_var)
            : "%eax"
        );
        
        /* Float with different precision */
        asm volatile (
            "cvtss2sd %1, %0\n\t"        /* Convert float to double */
            : "=x" (float_result)
            : "x" (float_var)
        );
        
        int_var += int_result;
        double_var += float_result;
    }
    
    /* BLOCK F: High Register Pressure with Many Clobbers */
    {
        int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
        int r1, r2, r3, r4, r5, r6;
        
        /* Many operands clobbering multiple registers */
        asm volatile (
            "mov %6, %0\n\t"
            "add %7, %0\n\t"
            "mov %0, %1\n\t"
            "imul %8, %1\n\t"
            "mov %1, %2\n\t"
            "sub %9, %2\n\t"
            "mov %2, %3\n\t"
            "xor %10, %3\n\t"
            "mov %3, %4\n\t"
            "or %11, %4\n\t"
            "mov %4, %5\n\t"
            "and $0xFF, %5"
            : "=&r" (r1), "=&r" (r2), "=&r" (r3), 
              "=&r" (r4), "=&r" (r5), "=r" (r6)
            : "r" (a), "r" (b), "r" (c), 
              "r" (d), "r" (e), "r" (f)
            : "cc"
        );
        
        int_var += r1 + r2 + r3 + r4 + r5 + r6;
    }
    
    /* BLOCK G: Memory-to-Memory with Intermediate Register */
    {
        double src[4] = {1.1, 2.2, 3.3, 4.4};
        double dst[4];
        
        /* Memory-to-memory move requiring temporary register */
        for (int i = 0; i < 4; i++) {
            asm volatile (
                "movsd (%1), %%xmm0\n\t"
                "movsd %%xmm0, (%0)\n\t"
                : 
                : "r" (&dst[i]), "r" (&src[i])
                : "xmm0", "memory"
            );
        }
        
        double_var += dst[0] + dst[3];
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = int_var + long_var + (int)double_var + 
                   (int)float_var + (int)(longlong_var & 0xFFFFFFFF) +
                   multi_array[0][0] + (int)complex_array[0][0][0];
    
    printf("Checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}
