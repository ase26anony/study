/* reload_test.c - Comprehensive test to trigger multiple reload types in GCC */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>  /* For SSE intrinsics */

/* Force no optimization on specific variables */
#define VOL(var) *(volatile typeof(var)*)&(var)

/* Complex struct to force address computations */
struct nested {
    int a[8];
    double b[4];
    struct nested *next;
};

/* Global variables to increase register pressure */
int global_array[256];
double global_doubles[128];
__m128i global_vecs[64];

int main(void) {
    /* ========== DECLARE DIVERSE VARIABLES ========== */
    /* Scalars of different types and sizes */
    int i = 42, j = 73, k = 19;
    long long ll1 = 0x123456789ABCDEF0LL, ll2 = 0xFEDCBA9876543210LL;
    float f1 = 3.14159f, f2 = 2.71828f;
    double d1 = 1.41421356, d2 = 1.73205080;
    
    /* Arrays for complex addressing */
    int array2d[16][16];
    struct nested nested_array[8];
    struct nested *nptr = &nested_array[0];
    
    /* Vector types */
    __m128i v1 = _mm_set_epi32(1, 2, 3, 4);
    __m128i v2 = _mm_set_epi32(5, 6, 7, 8);
    
    /* Pointers for address computations */
    int *ptr1 = &global_array[0];
    int *ptr2 = &global_array[128];
    
    /* Initialize arrays */
    for (int idx = 0; idx < 16; idx++) {
        for (int jdx = 0; jdx < 16; jdx++) {
            array2d[idx][jdx] = idx * 16 + jdx;
        }
    }
    
    for (int idx = 0; idx < 8; idx++) {
        for (int jdx = 0; jdx < 8; jdx++) {
            nested_array[idx].a[jdx] = idx * 8 + jdx;
        }
        nested_array[idx].next = &nested_array[(idx + 1) % 8];
    }
    
    /* ========== BLOCK A: REGISTER CLASS CONFLICT ========== */
    /* Force integer to float register reload */
    {
        int int_val = 0x40490FDB;  /* ~3.14159 in IEEE 754 */
        float float_result;
        
        /* Request float register for integer value - will need reload */
        asm volatile (
            "movd %1, %%xmm0\n\t"          /* Move integer to XMM register */
            "movd %%xmm0, %0\n\t"          /* Move back */
            : "=r" (float_result)          /* Output in general reg, but we use XMM */
            : "r" (int_val)                /* Input in general reg */
            : "%xmm0"                      /* Clobber XMM0 */
        );
        
        VOL(float_result) = float_result;
        f1 = float_result;
    }
    
    /* ========== BLOCK B: COMPLEX ADDRESS RELOAD ========== */
    /* Multi-dimensional array with complex index computation */
    {
        int idx1 = i % 16;
        int idx2 = (j * k) % 16;
        int result;
        
        /* Complex addressing mode that may need reloading */
        asm volatile (
            "movl %1, %0\n\t"
            : "=r" (result)
            : "m" (array2d[idx1][idx2 * 2 + 3])  /* Complex address computation */
            : /* No clobbers */
        );
        
        VOL(result) = result;
        i = result;
    }
    
    /* Pointer chain with complex computation */
    {
        int chain_result;
        struct nested *current = nptr;
        
        /* Very complex address computation */
        asm volatile (
            "movl %1, %0\n\t"
            : "=r" (chain_result)
            : "m" (current->next->next->a[current->a[0] % 8])  /* Nested pointer chain */
            : /* No clobbers */
        );
        
        VOL(chain_result) = chain_result;
        j = chain_result;
    }
    
    /* ========== BLOCK C: EARLY-CLOBBER MULTIPLE OUTPUTS ========== */
    {
        int out1, out2;
        int in1 = i, in2 = j, in3 = k;
        
        /* Early-clobber on second output - forces reloads */
        asm volatile (
            "movl %3, %0\n\t"      /* out1 = in1 */
            "addl %4, %0\n\t"      /* out1 += in2 */
            "movl %0, %1\n\t"      /* out2 = out1 (early clobber!) */
            "imull %5, %1\n\t"     /* out2 *= in3 */
            : "=&r" (out1), "=&r" (out2)  /* Both early-clobber */
            : "0" (0), "r" (in1), "r" (in2), "r" (in3)  /* Inputs */
            : /* No clobbers */
        );
        
        VOL(out1) = out1;
        VOL(out2) = out2;
        k = out1 + out2;
    }
    
    /* ========== BLOCK D: SECONDARY RELOAD PATTERNS ========== */
    /* Pattern that often requires secondary reloads on various architectures */
    {
        double dresult;
        long long large_const = 0x123456789ABCDEF0LL;
        
        /* Large constant may need secondary reload on some arches */
        asm volatile (
            "movq %1, %%rax\n\t"           /* First reload: constant to register */
            "movq %%rax, %0\n\t"           /* Second reload: register to output */
            : "=r" (dresult)               /* Output */
            : "i" (0x123456789ABCDEF0LL)   /* Large immediate */
            : "%rax"                       /* Clobber RAX */
        );
        
        VOL(dresult) = dresult;
        d1 = dresult;
    }
    
    /* Vector reload with different modes */
    {
        __m128i vresult;
        
        /* Vector operation that might need secondary reload */
        asm volatile (
            "movdqa %1, %0\n\t"            /* Aligned move */
            "pslld $4, %0\n\t"             /* Shift left */
            : "=x" (vresult)               /* XMM register constraint */
            : "xm" (v1)                    /* XMM or memory */
            : /* No clobbers */
        );
        
        /* Force use of result */
        v2 = _mm_add_epi32(v2, vresult);
    }
    
    /* ========== ADDITIONAL RELOAD SCENARIOS ========== */
    /* High register pressure to force spill/reload */
    {
        int r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
        
        asm volatile (
            "movl %10, %0\n\t"
            "addl %11, %1\n\t"
            "subl %12, %2\n\t"
            "imull %13, %3\n\t"
            "andl %14, %4\n\t"
            "orl  %15, %5\n\t"
            "xorl %16, %6\n\t"
            "shll $2, %7\n\t"
            "shrl $1, %8\n\t"
            "leal (%9,%10,2), %9\n\t"
            : "=&r" (r1), "=&r" (r2), "=&r" (r3), "=&r" (r4),
              "=&r" (r5), "=&r" (r6), "=&r" (r7), "=&r" (r8),
              "=&r" (r9), "=&r" (r10)
            : "r" (i), "r" (j), "r" (k), "r" (ll1 & 0xFFFFFFFF),
              "r" (ll2 & 0xFFFFFFFF), "r" (r1), "r" (r2), "r" (r3),
              "r" (r4), "r" (r5)
            : /* No clobbers */
        );
        
        /* Use results to prevent elimination */
        i = r1 + r2 + r3 + r4 + r5;
    }
    
    /* Memory operand with displacement too large */
    {
        int far_result;
        
        /* Access far element - may need address reload */
        asm volatile (
            "movl %1, %0\n\t"
            : "=r" (far_result)
            : "m" (global_array[200])      /* Large displacement */
            : /* No clobbers */
        );
        
        VOL(far_result) = far_result;
        j += far_result;
    }
    
    /* ========== COMPUTE CHECKSUM ========== */
    /* Prevent dead code elimination */
    int checksum = i + j + k + (int)ll1 + (int)ll2 + 
                   (int)f1 + (int)f2 + (int)d1 + (int)d2;
    
    /* Mix in array contents */
    for (int idx = 0; idx < 8; idx++) {
        checksum += nested_array[idx].a[0];
    }
    
    /* Use vector results */
    int vec_elems[4];
    _mm_storeu_si128((__m128i*)vec_elems, v2);
    checksum += vec_elems[0] + vec_elems[1] + vec_elems[2] + vec_elems[3];
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;
}
