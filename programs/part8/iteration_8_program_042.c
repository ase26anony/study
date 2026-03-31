/* reload_test.c - Comprehensive test to trigger multiple reload types */
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
__m128i global_vec;

NOOPT int main(void) {
    /* Diverse variable declarations */
    int i = 1, j = 2, k = 3, result = 0;
    long long ll1 = 0x123456789ABCDEF0LL, ll2 = 0xFEDCBA9876543210LL;
    float f1 = 1.5f, f2 = 2.5f, f3 = 3.5f;
    double d1 = 1.23456789, d2 = 9.87654321;
    __m128i v1 = _mm_set_epi32(1, 2, 3, 4);
    __m128i v2 = _mm_set_epi32(5, 6, 7, 8);
    
    /* Multi-dimensional arrays for complex addressing */
    int matrix[10][10];
    double cube[5][5][5];
    struct nested nodes[4];
    struct nested *ptr = &nodes[0];
    
    /* Initialize arrays */
    for (int x = 0; x < 10; x++)
        for (int y = 0; y < 10; y++)
            matrix[x][y] = x * 10 + y;
    
    for (int x = 0; x < 4; x++) {
        for (int y = 0; y < 8; y++)
            nodes[x].a[y] = x * 100 + y;
        nodes[x].next = &nodes[(x + 1) % 4];
    }
    
    /* ============================================
       BLOCK A: Register Class Conflict Reload
       ============================================ */
    {
        int int_var = 12345;
        double double_var;
        
        /* Force integer to be reloaded into floating-point register */
        asm volatile (
            /* Request f constraint for integer value */
            "mov %1, %%eax\n\t"
            "cvtsi2sd %%eax, %0\n\t"
            : "=f" (double_var)      /* Output in FP register */
            : "r" (int_var)          /* Input in general register */
            : "%eax", "memory"
        );
        
        /* Use the result */
        d1 += double_var;
    }
    
    /* ============================================
       BLOCK B: Complex Address Reload
       ============================================ */
    {
        int idx1 = i * 2 + j;
        int idx2 = k * 3 - 1;
        int idx3 = (i + j) * (k - 1);
        int loaded_val;
        
        /* Complex addressing mode that may need reloading */
        asm volatile (
            "movl %1, %0\n\t"
            : "=r" (loaded_val)
            : "m" (cube[idx1 % 5][idx2 % 5][idx3 % 5])  /* Complex address */
            : "memory"
        );
        
        result += loaded_val;
    }
    
    /* ============================================
       BLOCK C: Early-Clobber Multiple Outputs
       ============================================ */
    {
        int in1 = 100, in2 = 200, in3 = 300;
        int out1, out2, out3;
        
        /* Early-clobber forces separate registers for outputs */
        asm volatile (
            "leal (%1, %2), %0\n\t"      /* out1 = in1 + in2 */
            "imull %3, %0\n\t"           /* out1 *= in3 - modifies out1 early */
            "movl %0, %2\n\t"            /* Use out1 to compute out2 */
            "addl $42, %2\n\t"
            : "=&r" (out1), "=&r" (out2), "=r" (out3)  /* & = early clobber */
            : "r" (in1), "r" (in2), "r" (in3)
            : "cc"
        );
        
        result += out1 + out2 + out3;
    }
    
    /* ============================================
       BLOCK D: Secondary Reload Pattern
       ============================================ */
    {
        /* Large immediate that may need secondary reload on some arches */
        long long large_const = 0x1234567890ABCDEFLL;
        long long temp1, temp2;
        
        /* Pattern that often requires temporary register */
        asm volatile (
            "movq %2, %0\n\t"
            "xorq %1, %1\n\t"
            "addq $0x7FFFFFFFFFFFFFFF, %0\n\t"  /* Large constant */
            "subq %2, %1\n\t"
            : "=&r" (temp1), "=&r" (temp2)      /* Both early-clobbered */
            : "r" (large_const)
            : "cc"
        );
        
        ll1 ^= temp1;
        ll2 ^= temp2;
    }
    
    /* ============================================
       BLOCK E: Mixed Mode Reloads
       ============================================ */
    {
        /* Different machine modes in same asm */
        int int_result;
        float float_result;
        double double_result;
        
        asm volatile (
            "cvtsi2ssl %3, %%xmm0\n\t"
            "cvtsi2ssl %4, %%xmm1\n\t"
            "addss %%xmm1, %%xmm0\n\t"
            "movss %%xmm0, %0\n\t"
            "cvtsi2sdq %5, %%xmm2\n\t"
            "movsd %%xmm2, %1\n\t"
            "movl %6, %2\n\t"
            : "=m" (float_result), "=m" (double_result), "=r" (int_result)
            : "r" (i), "r" (j), "r" (ll1), "r" (k)
            : "%xmm0", "%xmm1", "%xmm2", "memory"
        );
        
        result += int_result + (int)float_result;
    }
    
    /* ============================================
       BLOCK F: High Register Pressure
       ============================================ */
    {
        /* Many live variables to force spills and reloads */
        int a = result, b = i, c = j, d = k;
        int e = a * b, f = c * d, g = e + f, h = g - a;
        
        /* Complex expression with many intermediate values */
        asm volatile (
            "imull %1, %0\n\t"
            "addl %2, %0\n\t"
            "subl %3, %0\n\t"
            "xorl %4, %0\n\t"
            "orl  %5, %0\n\t"
            "andl %6, %0\n\t"
            : "+r" (a)        /* Read-write operand */
            : "r" (b), "r" (c), "r" (d), "r" (e), "r" (f), "r" (g)
            : "cc"
        );
        
        result = a + h;
    }
    
    /* ============================================
       BLOCK G: Memory-to-Memory via Register
       ============================================ */
    {
        /* Force memory operand reload into register */
        double src[4] = {1.1, 2.2, 3.3, 4.4};
        double dst[4];
        
        /* Memory-to-memory operation requiring register intermediate */
        for (int idx = 0; idx < 4; idx++) {
            asm volatile (
                "movsd %1, %%xmm0\n\t"
                "addsd %2, %%xmm0\n\t"
                "movsd %%xmm0, %0\n\t"
                : "=m" (dst[idx])
                : "m" (src[idx]), "m" (d1)  /* Two memory inputs */
                : "%xmm0", "memory"
            );
        }
        
        d2 = dst[0] + dst[1] + dst[2] + dst[3];
    }
    
    /* ============================================
       BLOCK H: Vector Register Spill/Reload
       ============================================ */
    {
        __m128i v3, v4, v5;
        
        /* Multiple vector operations to pressure register allocator */
        asm volatile (
            "paddd %1, %0\n\t"
            "pxor  %2, %0\n\t"
            "pslld $2, %0\n\t"
            : "+x" (v1)        /* Read-write XMM register */
            : "x" (v2), "xm" (global_vec)  /* XMM or memory */
            : "cc"
        );
        
        /* Chain of dependencies forcing multiple reloads */
        v3 = _mm_add_epi32(v1, v2);
        v4 = _mm_sub_epi32(v3, v1);
        v5 = _mm_mullo_epi32(v4, v2);
        
        /* Extract to scalar to force spill */
        int vec_result;
        asm volatile (
            "pextrd $0, %1, %0\n\t"
            : "=r" (vec_result)
            : "x" (v5)
        );
        
        result += vec_result;
    }
    
    /* Final computation to prevent dead code elimination */
    result += (int)d1 + (int)d2 + (int)f1 + (int)f2 + (int)f3;
    result += (int)(ll1 & 0xFFFFFFFF) + (int)(ll2 >> 32);
    
    /* Use computed goto for address reload test */
    void* labels[] = { &&label1, &&label2, &&label3 };
    goto *labels[result % 3];
    
label1:
    result += 1;
    goto end;
label2:
    result += 2;
    goto end;
label3:
    result += 3;
    goto end;
    
end:
    /* Print to prevent optimization */
    printf("Result: %d\n", result);
    
    return result & 0xFF;  /* Return non-zero to indicate execution */
}
