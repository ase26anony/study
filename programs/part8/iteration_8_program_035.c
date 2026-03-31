/* reload_test.c - Comprehensive test to trigger multiple reload types in GCC */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Force no optimization on specific variables */
#define NOOPT __attribute__((optimize("O0")))

/* Complex struct to force address computations */
struct nested {
    int a[8];
    double b[4];
    struct nested *next;
};

/* Global variables to increase register pressure */
int global_int = 42;
double global_double = 3.14159;
__m128i global_vec = {0};

NOOPT int main(void) {
    /* Diverse variable declarations with different types and sizes */
    int int_var1 = 1, int_var2 = 2, int_var3 = 3;
    long long_var1 = 100, long_var2 = 200;
    float float_var1 = 1.0f, float_var2 = 2.0f;
    double double_var1 = 1.0, double_var2 = 2.0;
    __m128i vec_var1, vec_var2;
    int *ptr1 = &int_var1;
    double *ptr2 = &double_var1;
    
    /* Multi-dimensional array for complex addressing */
    int md_array[4][8][16] = {0};
    md_array[1][2][3] = 123;
    md_array[2][4][8] = 456;
    
    /* Nested structure with pointer chain */
    struct nested n1, n2, n3;
    struct nested *nptr = &n1;
    n1.next = &n2;
    n2.next = &n3;
    n3.next = NULL;
    
    /* Initialize vector variables */
    vec_var1 = _mm_set_epi32(1, 2, 3, 4);
    vec_var2 = _mm_set_epi32(5, 6, 7, 8);
    
    /* Volatile to prevent optimization */
    volatile int vol_int = 999;
    volatile double vol_double = 2.71828;
    
    /* ===== BLOCK A: Register Class Conflict ===== */
    /* Force integer to float register reload */
    {
        int temp_int = int_var1 + int_var2;
        double temp_double;
        
        /* Inline asm requiring float register for integer-derived value */
        asm volatile (
            /* Input in integer register, output in float register */
            "mov %1, %%eax\n\t"
            "cvtsi2sd %%eax, %0\n\t"
            : "=f" (temp_double)    /* Output in floating-point register */
            : "r" (temp_int)        /* Input in general-purpose register */
            : "%eax", "memory"
        );
        
        double_var1 += temp_double;
        printf("Block A result: %f\n", temp_double);
    }
    
    /* ===== BLOCK B: Complex Address Reload ===== */
    /* Force address computation reload with multi-dimensional array */
    {
        int i = 1, j = 2, k = 3;
        int result;
        
        /* Complex addressing mode that may need reloading */
        asm volatile (
            "movl %[array], %[out]\n\t"
            : [out] "=r" (result)
            : [array] "m" (md_array[i][j][k])  /* Complex address computation */
            : "memory"
        );
        
        int_var1 += result;
        printf("Block B result: %d\n", result);
    }
    
    /* ===== BLOCK C: Early-Clobber Multiple Outputs ===== */
    /* Force reloads due to early-clobber constraints */
    {
        int out1, out2;
        int in1 = int_var1, in2 = int_var2, in3 = int_var3;
        
        /* Multiple outputs with early-clobber on second output */
        asm volatile (
            "mov %2, %0\n\t"        /* out1 = in1 */
            "add %3, %0\n\t"        /* out1 += in2 */
            "mov %0, %1\n\t"        /* out2 = out1 (clobbers early) */
            "imul %4, %1\n\t"       /* out2 *= in3 */
            : "=&r" (out1), "=&r" (out2)  /* Both early-clobber */
            : "r" (in1), "r" (in2), "r" (in3)
            : "cc"
        );
        
        int_var1 = out1;
        int_var2 = out2;
        printf("Block C results: %d, %d\n", out1, out2);
    }
    
    /* ===== BLOCK D: Secondary Reload Pattern ===== */
    /* Force secondary reload through complex constraints */
    {
        __m128i vec_result;
        long long large_const = 0x123456789ABCDEF0LL;
        
        /* Pattern that may require secondary reload on some architectures */
        asm volatile (
            "movq %1, %%xmm0\n\t"   /* Move 64-bit constant to vector reg */
            "pshufd $0x44, %%xmm0, %0\n\t"  /* Duplicate to 128-bit */
            : "=x" (vec_result)
            : "r" (large_const)     /* Integer constant in GP register */
            : "%xmm0"
        );
        
        vec_var1 = _mm_add_epi32(vec_var1, vec_result);
        printf("Block D executed\n");
    }
    
    /* ===== BLOCK E: Memory Operand with Displacement ===== */
    /* Force address reload with structure pointer chain */
    {
        int struct_result;
        
        /* Complex structure pointer chain access */
        asm volatile (
            "movl (%[ptr]), %[out]\n\t"
            "addl 4(%[ptr]), %[out]\n\t"
            : [out] "=r" (struct_result)
            : [ptr] "r" (nptr->next->next),  /* Pointer computation */
              "m" (*(int (*)[8])nptr->next->next->a)  /* Memory constraint */
            : "memory"
        );
        
        int_var3 += struct_result;
        printf("Block E result: %d\n", struct_result);
    }
    
    /* ===== BLOCK F: Mixed Mode Reloads ===== */
    /* Force reloads with different machine modes */
    {
        float float_result;
        double double_result;
        int int_result;
        
        /* Mixed precision operations requiring mode conversions */
        asm volatile (
            "cvtsi2ss %2, %%xmm0\n\t"    /* int to float */
            "cvtss2sd %%xmm0, %%xmm1\n\t" /* float to double */
            "cvtsd2si %%xmm1, %0\n\t"    /* double to int */
            "movss %%xmm0, %1\n\t"       /* store float */
            "movsd %%xmm1, %3\n\t"       /* store double */
            : "=r" (int_result), "=m" (float_result), 
              "+m" (double_result), "=m" (vol_double)
            : "r" (int_var1)
            : "%xmm0", "%xmm1", "memory"
        );
        
        printf("Block F results: int=%d, float=%f, double=%f\n", 
               int_result, float_result, double_result);
    }
    
    /* ===== BLOCK G: High Register Pressure ===== */
    /* Force spilling and reloading due to register pressure */
    {
        /* Use many variables in one asm to increase pressure */
        asm volatile (
            "mov %0, %%eax\n\t"
            "add %1, %%eax\n\t"
            "add %2, %%eax\n\t"
            "add %3, %%eax\n\t"
            "add %4, %%eax\n\t"
            "mov %%eax, %0\n\t"
            : "+r" (int_var1)
            : "r" (int_var2), "r" (int_var3), 
              "r" (long_var1), "r" (vol_int)
            : "%eax", "cc"
        );
        
        printf("Block G result: %d\n", int_var1);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = int_var1 + int_var2 + int_var3 + 
                   (int)float_var1 + (int)float_var2 +
                   (int)double_var1 + (int)double_var2 +
                   (int)long_var1 + (int)long_var2 +
                   vol_int;
    
    /* Use vector variable to prevent optimization */
    int vec_elems[4];
    _mm_storeu_si128((__m128i*)vec_elems, vec_var1);
    checksum += vec_elems[0] + vec_elems[1] + vec_elems[2] + vec_elems[3];
    
    printf("Final checksum: %d\n", checksum);
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
