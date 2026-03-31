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
    long long_var1 = 1000LL, long_var2 = 2000LL;
    float float_var1 = 1.0f, float_var2 = 2.0f;
    double double_var1 = 1.0, double_var2 = 2.0;
    __m128i vec_var1, vec_var2;
    int *ptr1 = &int_var1;
    double *ptr2 = &double_var1;
    
    /* Multi-dimensional array for complex addressing */
    int multi_array[4][8][16];
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 8; j++)
            for (int k = 0; k < 16; k++)
                multi_array[i][j][k] = i * 100 + j * 10 + k;
    
    /* Nested structure for pointer chasing */
    struct nested n1, n2;
    struct nested *nptr = &n1;
    n1.next = &n2;
    n2.next = &n1;
    for (int i = 0; i < 8; i++) n1.a[i] = i * 10;
    for (int i = 0; i < 4; i++) n1.b[i] = i * 1.5;
    
    /* ============================================
       BLOCK A: Register Class Conflict Reload
       Force integer to float register reload
       ============================================ */
    {
        int int_input = 12345;
        double float_output;
        
        /* Request float register for integer input - will require reload */
        asm volatile (
            "mov %1, %%eax\n\t"           /* Move integer to eax */
            "cvtsi2sd %%eax, %0\n\t"      /* Convert to double in float reg */
            : "=f" (float_output)         /* Output in floating-point register */
            : "r" (int_input)             /* Input in general-purpose register */
            : "%eax", "memory"
        );
        
        double_var1 += float_output;
    }
    
    /* ============================================
       BLOCK B: Complex Address Reload
       Multi-dimensional array with complex index
       ============================================ */
    {
        int i = 1, j = 2, k = 3;
        int result;
        
        /* Complex addressing mode that may need reloading */
        asm volatile (
            "movl %1, %0\n\t"
            : "=r" (result)
            : "m" (multi_array[i*2][j+1][k*3])  /* Complex address computation */
            : "memory"
        );
        
        int_var1 += result;
    }
    
    /* ============================================
       BLOCK C: Early-Clobber Multiple Outputs
       ============================================ */
    {
        int in1 = 100, in2 = 200, in3 = 300;
        int out1, out2, out3;
        
        /* Early-clobber on out2, multiple uses of inputs */
        asm volatile (
            "mov %3, %0\n\t"      /* out1 = in1 */
            "imul %4, %0\n\t"     /* out1 *= in2 */
            "mov %0, %1\n\t"      /* out2 = out1 (early clobber!) */
            "add %5, %1\n\t"      /* out2 += in3 */
            "mov %1, %2\n\t"      /* out3 = out2 */
            "sub %3, %2\n\t"      /* out3 -= in1 */
            : "=&r" (out1), "=&r" (out2), "=r" (out3)  /* Two early-clobbers */
            : "r" (in1), "r" (in2), "r" (in3)
            : "cc"
        );
        
        int_var2 += out1 + out2 + out3;
    }
    
    /* ============================================
       BLOCK D: Secondary Reload Pattern
       Vector to integer transfer requiring intermediate
       ============================================ */
    {
        __m128i vec = _mm_set_epi32(1, 2, 3, 4);
        int extracted[4];
        
        /* Extract elements from vector - may require secondary reloads */
        asm volatile (
            "movd %1, %0\n\t"
            : "=r" (extracted[0])
            : "x" (vec)                    /* xmm register constraint */
            : "memory"
        );
        
        /* Another pattern: moving between different register files */
        long long large_imm = 0x123456789ABCDEF0LL;
        __m128i vec_result;
        
        asm volatile (
            "movq %1, %%rax\n\t"          /* 64-bit immediate to GP register */
            "movq %%rax, %0\n\t"          /* GP register to vector register */
            : "=x" (vec_result)
            : "r" (large_imm)             /* May need reload for large immediate */
            : "%rax", "memory"
        );
        
        vec_var1 = _mm_add_epi32(vec, vec_result);
    }
    
    /* ============================================
       BLOCK E: Memory-to-Memory with Indexed Addressing
       ============================================ */
    {
        int index = global_int & 7;  /* Non-constant index */
        int value1, value2;
        
        /* Complex memory addressing with structure pointer chain */
        asm volatile (
            "movl (%1, %2, 4), %0\n\t"    /* nptr->a[index] */
            : "=r" (value1)
            : "r" (&nptr->a[0]), "r" (index)
            : "memory"
        );
        
        /* Another with displacement */
        asm volatile (
            "movl 0x10(%1, %2, 8), %0\n\t"  /* multi_array[1][index][index] with offset */
            : "=r" (value2)
            : "r" (multi_array[1][0]), "r" (index)
            : "memory"
        );
        
        int_var3 += value1 + value2;
    }
    
    /* ============================================
       BLOCK F: Mixed Mode Reloads
       Different machine modes in same asm
       ============================================ */
    {
        char char_var = 'A';
        short short_var = 256;
        int int_var = 65536;
        long long ll_var = 0x100000000LL;
        
        /* Mix different sized operations */
        asm volatile (
            "addb %1, %b0\n\t"      /* byte operation */
            "addw %2, %w0\n\t"      /* word operation */
            "addl %3, %k0\n\t"      /* dword operation */
            "addq %4, %0\n\t"       /* qword operation */
            : "+r" (ll_var)
            : "ri" (char_var), "ri" (short_var), "ri" (int_var), "ri" (ll_var)
            : "cc"
        );
        
        long_var1 = ll_var;
    }
    
    /* ============================================
       BLOCK G: High Register Pressure
       Many live variables force spill/reload
       ============================================ */
    {
        /* Use many variables to increase register pressure */
        int t1 = int_var1 * 2;
        int t2 = int_var2 * 3;
        int t3 = int_var3 * 4;
        double t4 = double_var1 * 1.5;
        double t5 = double_var2 * 2.5;
        long t6 = long_var1 + 1000;
        long t7 = long_var2 + 2000;
        
        /* Complex asm using all variables */
        asm volatile (
            "imull %1, %0\n\t"
            "addl %2, %0\n\t"
            "cvtsi2sd %0, %%xmm0\n\t"
            "addsd %3, %%xmm0\n\t"
            "addsd %4, %%xmm0\n\t"
            "cvttsd2si %%xmm0, %0\n\t"
            "addq %5, %0\n\t"
            "addq %6, %0\n\t"
            : "+r" (t1)
            : "r" (t2), "r" (t3), "x" (t4), "x" (t5), "r" (t6), "r" (t7)
            : "%xmm0", "cc", "memory"
        );
        
        global_int = t1;
    }
    
    /* ============================================
       Compute checksum to prevent optimization
       ============================================ */
    int checksum = int_var1 + int_var2 + int_var3;
    checksum += (int)long_var1 + (int)long_var2;
    checksum += (int)float_var1 + (int)float_var2;
    checksum += (int)double_var1 + (int)double_var2;
    
    /* Use computed goto for address reload test */
    void *label_ptr = &&end;
    asm volatile ("" : : "r" (label_ptr) : "memory");
    
end:
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;
}
