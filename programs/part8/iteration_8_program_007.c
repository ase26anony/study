/* reload_test.c - Comprehensive test to trigger various reload scenarios */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Force no optimization on specific variables */
#define NOOPT __attribute__((optimize("O0")))

/* Complex structure to force address computations */
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
    /* Diverse variable declarations with different types and modes */
    int int_var1 = 1, int_var2 = 2, int_var3 = 3;
    long long ll_var1 = 100, ll_var2 = 200;
    float float_var1 = 1.0f, float_var2 = 2.0f;
    double double_var1 = 1.0, double_var2 = 2.0;
    __m128i vec_var1, vec_var2;
    int *ptr1 = &int_var1;
    double *ptr2 = &double_var1;
    
    /* Multi-dimensional array for complex addressing */
    int md_array[4][8][16];
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 8; j++)
            for (int k = 0; k < 16; k++)
                md_array[i][j][k] = i * 100 + j * 10 + k;
    
    /* Nested structure for pointer chasing */
    struct nested nested1, nested2;
    for (int i = 0; i < 8; i++) nested1.a[i] = i * 10;
    for (int i = 0; i < 4; i++) nested1.b[i] = i * 1.5;
    nested1.next = &nested2;
    for (int i = 0; i < 8; i++) nested2.a[i] = i * 20;
    for (int i = 0; i < 4; i++) nested2.b[i] = i * 2.5;
    nested2.next = &nested1;
    
    /* ===== BLOCK A: Register Class Conflict ===== */
    /* Force integer to float register reload */
    {
        int int_input = 0x40490FDA; /* Float representation of ~3.14 */
        float float_output;
        
        /* Request float register for integer input - will need reload */
        asm volatile (
            "movd %1, %%xmm0\n\t"          /* Move integer to XMM register */
            "movd %%xmm0, %0\n\t"          /* Move back */
            : "=r" (float_output)          /* Output in general reg */
            : "r" (int_input)              /* Input in general reg */
            : "%xmm0"                      /* Clobber XMM0 */
        );
        
        /* Use result to prevent elimination */
        float_var1 += float_output;
    }
    
    /* ===== BLOCK B: Complex Address Reload ===== */
    /* Multi-dimensional array access with complex addressing */
    {
        int i = 1, j = 2, k = 3;
        int result;
        
        /* Complex addressing mode that may need reloading */
        asm volatile (
            "movl %1, %0\n\t"
            : "=r" (result)
            : "m" (md_array[i][j][k + global_int])  /* Complex address */
            : /* No clobbers */
        );
        
        int_var1 += result;
    }
    
    /* Pointer chasing with complex structure access */
    {
        double result;
        int offset = 2;
        
        /* Complex memory operand with structure pointer arithmetic */
        asm volatile (
            "movsd %1, %%xmm0\n\t"
            "movsd %%xmm0, %0\n\t"
            : "=r" (result)
            : "m" (nested1.next->next->b[offset * 2])  /* Very complex address */
            : "%xmm0"
        );
        
        double_var1 += result;
    }
    
    /* ===== BLOCK C: Early-Clobber Multiple Outputs ===== */
    /* Multiple outputs with early clobber */
    {
        int in1 = 100, in2 = 200, in3 = 300;
        int out1, out2, out3;
        
        /* Early clobber on out2 forces reloads */
        asm volatile (
            "movl %3, %0\n\t"      /* out1 = in1 */
            "addl %4, %0\n\t"      /* out1 += in2 */
            "movl %0, %1\n\t"      /* out2 = out1 (early clobber!) */
            "imull %5, %1\n\t"     /* out2 *= in3 */
            "movl %1, %2\n\t"      /* out3 = out2 */
            "addl $1, %2\n\t"      /* out3 += 1 */
            : "=&r" (out1), "=&r" (out2), "=r" (out3)  /* Two early clobbers */
            : "r" (in1), "r" (in2), "r" (in3)
            : /* No clobbers */
        );
        
        int_var2 += out1 + out2 + out3;
    }
    
    /* ===== BLOCK D: Secondary Reload Patterns ===== */
    /* Simulate secondary reload for vector constants */
    {
        __m128i vec_result;
        long long constant = 0x123456789ABCDEF0LL;
        
        /* Moving 64-bit constant may require secondary reload on some arches */
        asm volatile (
            "movq %1, %%xmm0\n\t"      /* Move constant to vector reg */
            "movdqa %%xmm0, %0\n\t"    /* Move to output */
            : "=x" (vec_result)        /* Output in vector register */
            : "r" (constant)           /* Input in general register */
            : "%xmm0"
        );
        
        /* Use vector result */
        int64_t *as_int = (int64_t*)&vec_result;
        ll_var1 += as_int[0];
    }
    
    /* Mixed mode reloads */
    {
        double input = 2.5;
        int int_result;
        float float_result;
        
        /* Different modes for inputs/outputs */
        asm volatile (
            "cvtsd2si %1, %0\n\t"      /* Convert double to int */
            : "=r" (int_result)
            : "x" (input)              /* Input in XMM register */
            : /* No clobbers */
        );
        
        asm volatile (
            "cvtsi2ssl %1, %%xmm0\n\t" /* Convert int to float */
            "movss %%xmm0, %0\n\t"
            : "=r" (float_result)
            : "r" (int_result)         /* Input in general register */
            : "%xmm0"
        );
        
        float_var2 += float_result;
    }
    
    /* ===== Additional Stress Tests ===== */
    /* High register pressure with many live variables */
    {
        int a = int_var1, b = int_var2, c = int_var3;
        int d = ll_var1, e = ll_var2;
        float f = float_var1, g = float_var2;
        double h = double_var1, i = double_var2;
        
        /* Force spilling by using many variables in one asm */
        asm volatile (
            "addl %1, %0\n\t"
            "addl %2, %0\n\t"
            "addl %3, %0\n\t"
            "addl %4, %0\n\t"
            : "+r" (a)
            : "r" (b), "r" (c), "r" (d), "r" (e)
            : /* No clobbers */
        );
        
        /* Use floating point variables too */
        asm volatile (
            "addsd %1, %0\n\t"
            "addsd %2, %0\n\t"
            : "+x" (h)
            : "x" (i), "x" (global_double)
            : /* No clobbers */
        );
        
        int_var1 = a;
        double_var1 = h;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = int_var1 + int_var2 + int_var3;
    checksum += (int)ll_var1 + (int)ll_var2;
    checksum += (int)float_var1 + (int)float_var2;
    checksum += (int)double_var1 + (int)double_var2;
    
    /* Use all variables one more time */
    printf("Checksum: %d\n", checksum);
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
