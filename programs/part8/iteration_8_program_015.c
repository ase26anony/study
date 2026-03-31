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
    /* Diverse variable declarations with different types and alignments */
    volatile int int_var = 1;
    volatile long long ll_var = 2;
    volatile float float_var = 3.0f;
    volatile double double_var = 4.0;
    volatile __m128i vec_var = _mm_set_epi32(1, 2, 3, 4);
    volatile __m128 vec_float = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    
    /* Arrays for complex addressing */
    int array_2d[16][16];
    double darray[256];
    struct nested nested_array[8];
    struct nested *nested_ptr = &nested_array[0];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            array_2d[i][j] = i * 16 + j;
        }
    }
    
    for (int i = 0; i < 256; i++) {
        darray[i] = i * 0.1;
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            nested_array[i].a[j] = i * 8 + j;
        }
        for (int j = 0; j < 4; j++) {
            nested_array[i].b[j] = i * 0.5 + j;
        }
        nested_array[i].next = &nested_array[(i + 1) % 8];
    }
    
    /* Intermediate variables to force spills */
    int temp1, temp2, temp3;
    double dtemp1, dtemp2;
    __m128i vtemp;
    
    /* ======================================================================
       BLOCK A: Register Class Conflict Reload
       Force integer to float register reload
       ====================================================================== */
    {
        int int_input = int_var;
        double double_output;
        
        /* This asm requires an integer in a floating-point register,
           forcing a reload from general-purpose to floating-point register */
        asm volatile (
            "movq %1, %%xmm0\n\t"          /* Move integer to XMM register */
            "cvtsi2sd %2, %%xmm1\n\t"      /* Convert integer to double */
            "addsd %%xmm0, %%xmm1\n\t"
            "movsd %%xmm1, %0"
            : "=m" (double_output)
            : "r" (int_input), "r" (ll_var)
            : "xmm0", "xmm1", "memory"
        );
        
        dtemp1 = double_output;
    }
    
    /* ======================================================================
       BLOCK B: Complex Address Reload with Multiple Indexing
       Force address computation reload with complex addressing mode
       ====================================================================== */
    {
        int idx1 = int_var % 16;
        int idx2 = (int_var * 7) % 16;
        int idx3 = (int_var * 13) % 16;
        int result;
        
        /* Complex array access that may not fit in a single addressing mode */
        asm volatile (
            "movl %[idx1], %%eax\n\t"
            "imull $16, %%eax\n\t"
            "addl %[idx2], %%eax\n\t"
            "movl %[array](,%%eax,4), %%ebx\n\t"
            "addl %[idx3], %%ebx\n\t"
            "movl %%ebx, %[result]"
            : [result] "=r" (result)
            : [array] "m" (array_2d[0][0]),
              [idx1] "r" (idx1),
              [idx2] "r" (idx2),
              [idx3] "r" (idx3)
            : "eax", "ebx", "memory"
        );
        
        temp1 = result;
        
        /* Another complex address with structure pointer chain */
        int chain_result;
        asm volatile (
            "movq %[ptr], %%rax\n\t"
            "movq 64(%%rax), %%rax\n\t"    /* Access next pointer */
            "movl 32(%%rax), %%ebx\n\t"    /* Access a[4] through pointer */
            "movl %%ebx, %[result]"
            : [result] "=r" (chain_result)
            : [ptr] "r" (nested_ptr)
            : "rax", "rbx", "memory"
        );
        
        temp2 = chain_result;
    }
    
    /* ======================================================================
       BLOCK C: Early-Clobber Multiple Output Reloads
       Force reloads due to early-clobber constraints
       ====================================================================== */
    {
        int in1 = int_var + 1;
        int in2 = int_var + 2;
        int in3 = int_var + 3;
        int out1, out2, out3;
        
        /* Multiple outputs with early-clobber on one output */
        asm volatile (
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"       /* out2 is early-clobber, may conflict */
            "movl %%eax, %[out2]\n\t"
            "movl %[in3], %%ebx\n\t"
            "subl %%eax, %%ebx\n\t"
            "movl %%ebx, %[out1]\n\t"
            "imull %%ebx, %%eax\n\t"
            "movl %%eax, %[out3]"
            : [out1] "=&r" (out1),  /* Early-clobber - written before all inputs read */
              [out2] "=r" (out2),
              [out3] "=r" (out3)
            : [in1] "r" (in1),
              [in2] "r" (in2),
              [in3] "r" (in3)
            : "eax", "ebx", "memory"
        );
        
        temp3 = out1 + out2 + out3;
        
        /* Another early-clobber with floating point */
        double din1 = double_var;
        double din2 = double_var * 2;
        double dout1, dout2;
        
        asm volatile (
            "movsd %[din1], %%xmm0\n\t"
            "mulsd %[din2], %%xmm0\n\t"    /* dout1 is early-clobber */
            "movsd %%xmm0, %[dout1]\n\t"
            "addsd %[din1], %%xmm0\n\t"
            "movsd %%xmm0, %[dout2]"
            : [dout1] "=&f" (dout1),  /* Early-clobber floating point register */
              [dout2] "=f" (dout2)
            : [din1] "f" (din1),
              [din2] "f" (din2)
            : "xmm0"
        );
        
        dtemp2 = dout1 + dout2;
    }
    
    /* ======================================================================
       BLOCK D: Secondary Reload Patterns
       Force secondary reloads through complex constraints
       ====================================================================== */
    {
        /* Pattern that often requires secondary reloads:
           Moving between different register classes with no direct instruction */
        __m128i vec_input = vec_var;
        __m128i vec_output;
        
        /* This pattern may require a temporary register on some architectures
           when moving between vector and general-purpose registers */
        asm volatile (
            "movdqa %1, %%xmm0\n\t"
            "movdqa %%xmm0, %%xmm1\n\t"
            "psrld $16, %%xmm1\n\t"
            "paddd %%xmm1, %%xmm0\n\t"
            "movdqa %%xmm0, %0"
            : "=v" (vec_output)    /* Vector register constraint */
            : "v" (vec_input)      /* Vector register constraint */
            : "xmm0", "xmm1"
        );
        
        vtemp = vec_output;
        
        /* Another secondary reload pattern: large immediate to vector register */
        int64_t large_imm = 0x123456789ABCDEF0LL;
        __m128i vec_from_imm;
        
        asm volatile (
            "movq %1, %%rax\n\t"           /* May require reload to get immediate */
            "movq %%rax, %%xmm0\n\t"       /* Then move to vector reg */
            "punpcklqdq %%xmm0, %%xmm0\n\t"
            "movdqa %%xmm0, %0"
            : "=v" (vec_from_imm)
            : "r" (large_imm)             /* Force through general-purpose reg */
            : "rax", "xmm0"
        );
        
        vtemp = _mm_add_epi32(vtemp, vec_from_imm);
    }
    
    /* ======================================================================
       BLOCK E: Mixed Mode Reloads (Different machine modes)
       ====================================================================== */
    {
        /* Mix different machine modes in same asm */
        char char_var = 'A';
        short short_var = 1000;
        int int_var2 = 10000;
        long long ll_var2 = 1000000LL;
        
        long long mixed_result;
        
        asm volatile (
            "movsbl %[char], %%eax\n\t"    /* Sign extend char to int */
            "movswl %[short], %%ebx\n\t"   /* Sign extend short to int */
            "addl %%ebx, %%eax\n\t"
            "movl %[int], %%ecx\n\t"
            "addl %%ecx, %%eax\n\t"
            "cltq\n\t"                     /* Sign extend eax to rax */
            "addq %[ll], %%rax\n\t"
            "movq %%rax, %[result]"
            : [result] "=r" (mixed_result)
            : [char] "r" (char_var),
              [short] "r" (short_var),
              [int] "r" (int_var2),
              [ll] "r" (ll_var2)
            : "rax", "rbx", "rcx", "memory"
        );
        
        temp1 = (int)mixed_result;
    }
    
    /* ======================================================================
       BLOCK F: High Register Pressure to Force Spills and Reloads
       ====================================================================== */
    {
        /* Use many variables in one asm to increase register pressure */
        int a = temp1, b = temp2, c = temp3;
        double x = dtemp1, y = dtemp2;
        int r1, r2, r3, r4, r5;
        
        asm volatile (
            "movl %[a], %%eax\n\t"
            "addl %[b], %%eax\n\t"
            "movl %%eax, %[r1]\n\t"
            "movl %[c], %%ebx\n\t"
            "subl %%eax, %%ebx\n\t"
            "movl %%ebx, %[r2]\n\t"
            "imull %%ebx, %%eax\n\t"
            "movl %%eax, %[r3]\n\t"
            "movsd %[x], %%xmm0\n\t"
            "addsd %[y], %%xmm0\n\t"
            "cvttsd2si %%xmm0, %%ecx\n\t"
            "movl %%ecx, %[r4]\n\t"
            "addl %%ecx, %%eax\n\t"
            "movl %%eax, %[r5]"
            : [r1] "=&r" (r1), [r2] "=&r" (r2),
              [r3] "=&r" (r3), [r4] "=&r" (r4),
              [r5] "=r" (r5)
            : [a] "r" (a), [b] "r" (b), [c] "r" (c),
              [x] "f" (x), [y] "f" (y)
            : "eax", "ebx", "ecx", "xmm0", "memory"
        );
        
        /* Use results to prevent optimization */
        int_var += r1 + r2 + r3 + r4 + r5;
    }
    
    /* Final computation to use all variables and prevent dead code elimination */
    int checksum = int_var + (int)ll_var + (int)float_var + (int)double_var + 
                   temp1 + temp2 + temp3 + (int)dtemp1 + (int)dtemp2;
    
    /* Access vector elements for checksum */
    int vec_data[4];
    _mm_storeu_si128((__m128i*)vec_data, vtemp);
    checksum += vec_data[0] + vec_data[1] + vec_data[2] + vec_data[3];
    
    /* Use array elements */
    checksum += array_2d[0][0] + (int)darray[0];
    
    printf("Checksum: %d\n", checksum);
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
