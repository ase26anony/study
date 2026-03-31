/* reload_test.c - Test program to trigger multiple reload scenarios in GCC */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Force no optimization on specific variables */
#define VOLATILE_VAR(var) volatile var

/* Complex structure to force address computations */
struct nested {
    int a[8];
    double b[4];
    struct nested *next;
};

/* Multi-dimensional array for complex addressing */
int multi_array[16][32][8];

/* Global variables to ensure liveness */
volatile int global_counter = 0;
volatile double global_double = 3.14159;

int main(void) {
    /* Diverse variable declarations with different types and modes */
    int int_var1 = 12345;
    long long_var1 = 9876543210LL;
    float float_var1 = 2.71828f;
    double double_var1 = 1.41421356;
    int64_t int64_var = 0x123456789ABCDEFLL;
    __m128i vec_var1, vec_var2;
    __m128 float_vec1, float_vec2;
    
    /* Array and pointer variables for address reloads */
    int array1[256];
    double array2[128];
    struct nested nested_array[64];
    struct nested *nested_ptr = &nested_array[0];
    volatile int *volatile_ptr = array1;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) array1[i] = i * 3;
    for (int i = 0; i < 128; i++) array2[i] = i * 1.5;
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 8; j++) nested_array[i].a[j] = i + j;
        for (int j = 0; j < 4; j++) nested_array[i].b[j] = i * 0.5 + j;
        nested_array[i].next = (i < 63) ? &nested_array[i + 1] : NULL;
    }
    
    /* Initialize vector variables */
    vec_var1 = _mm_set_epi32(1, 2, 3, 4);
    vec_var2 = _mm_set_epi32(5, 6, 7, 8);
    float_vec1 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    float_vec2 = _mm_set_ps(5.0f, 6.0f, 7.0f, 8.0f);
    
    /* Prevent optimization */
    global_counter++;
    
    /********************************************************************
     * BLOCK A: Register Class Conflict Reload
     * Force integer to float register reload
     ********************************************************************/
    {
        int int_input = 42;
        double double_output;
        
        /* This asm tries to use an integer in a floating-point constraint */
        asm volatile (
            /* Input in integer register, but constraint asks for float reg */
            "mov %1, %%eax\n\t"           /* Move integer to eax */
            "cvtsi2sd %%eax, %0\n\t"      /* Convert to double, needs xmm reg */
            : "=f" (double_output)        /* Output constraint: float register */
            : "r" (int_input)             /* Input constraint: general register */
            : "%eax", "memory"
        );
        
        double_var1 += double_output;
        global_counter++;
    }
    
    /********************************************************************
     * BLOCK B: Complex Address Reload with Multiple Indexing
     * Force address computation reload with non-trivial addressing mode
     ********************************************************************/
    {
        int i = global_counter % 16;
        int j = (global_counter * 7) % 32;
        int k = (global_counter * 3) % 8;
        int result;
        
        /* Complex array addressing that may not fit in one addressing mode */
        asm volatile (
            "movl %[array], %[out]\n\t"
            : [out] "=r" (result)
            : [array] "m" (multi_array[i][j][k])  /* Complex address computation */
            : "memory"
        );
        
        int_var1 += result;
        global_counter++;
    }
    
    /********************************************************************
     * BLOCK C: Early-Clobber Multiple Outputs
     * Force reloads due to early clobber and multiple outputs
     ********************************************************************/
    {
        int in1 = int_var1;
        int in2 = int_var1 * 2;
        int in3 = int_var1 * 3;
        int out1, out2, out3;
        
        /* Multiple outputs with early clobber on one */
        asm volatile (
            "movl %[in1], %[out1]\n\t"    /* out1 gets in1 */
            "addl %[in2], %[out1]\n\t"    /* modify out1 (early clobbered reg) */
            "movl %[out1], %[out2]\n\t"   /* out2 gets out1 */
            "imull %[in3], %[out2]\n\t"   /* out2 *= in3 */
            "movl %[out2], %[out3]\n\t"   /* out3 gets out2 */
            : [out1] "=&r" (out1),        /* Early clobber - written before all inputs read */
              [out2] "=r" (out2),
              [out3] "=r" (out3)
            : [in1] "r" (in1),
              [in2] "r" (in2),
              [in3] "r" (in3)
            : "cc"
        );
        
        int_var1 = out1 + out2 + out3;
        global_counter++;
    }
    
    /********************************************************************
     * BLOCK D: Secondary Reload Pattern
     * Force secondary reload through complex constraints
     ********************************************************************/
    {
        double complex_input = double_var1;
        __m128d vec_output;
        
        /* Pattern that might require secondary reload on some architectures */
        asm volatile (
            "movq %[in], %%xmm0\n\t"      /* Move scalar to xmm register */
            "unpcklpd %%xmm0, %%xmm0\n\t" /* Duplicate to both lanes */
            "movapd %%xmm0, %[out]\n\t"   /* Output to variable */
            : [out] "=x" (vec_output)     /* SSE2 register constraint */
            : [in] "fm" (complex_input)   /* Floating point or memory - may need reload */
            : "%xmm0"
        );
        
        /* Use the result */
        double_var1 += ((double*)&vec_output)[0];
        global_counter++;
    }
    
    /********************************************************************
     * BLOCK E: Mixed Mode Reloads (Different Data Sizes)
     * Trigger reloads with different machine modes
     ********************************************************************/
    {
        char char_var = 127;
        short short_var = 32767;
        int int_var = 65535;
        long long ll_var = 0xFFFFFFFF;
        float float_var = 3.14f;
        double double_var = 2.71828;
        
        /* Mixed mode operations in inline asm */
        asm volatile (
            /* Different size operations forcing different reload modes */
            "addb %[c], %%al\n\t"
            "addw %[s], %%ax\n\t"
            "addl %[i], %%eax\n\t"
            "movq %[ll], %%rcx\n\t"
            "addq %%rcx, %%rax\n\t"
            "cvtsi2ss %[i], %%xmm0\n\t"
            "addss %[f], %%xmm0\n\t"
            "cvtsi2sd %[ll], %%xmm1\n\t"
            "addsd %[d], %%xmm1\n\t"
            : /* no outputs, just side effects */
            : [c] "r" (char_var),
              [s] "r" (short_var),
              [i] "r" (int_var),
              [ll] "r" (ll_var),
              [f] "x" (float_var),
              [d] "x" (double_var)
            : "%rax", "%rcx", "%xmm0", "%xmm1", "cc"
        );
        
        global_counter++;
    }
    
    /********************************************************************
     * BLOCK F: Pointer Chain with Offset Computation
     * Force address reload through structure pointer chasing
     ********************************************************************/
    {
        int offset = global_counter % 8;
        double result;
        
        /* Complex pointer arithmetic that may need reload */
        asm volatile (
            "movq (%[ptr], %[off], 8), %[out]\n\t"  /* ptr->b[offset] */
            : [out] "=r" (result)
            : [ptr] "r" (nested_ptr),
              [off] "r" (offset)
            : "memory"
        );
        
        double_var1 += result;
        global_counter++;
    }
    
    /********************************************************************
     * BLOCK G: Vector Register Pressure
     * Force spills and reloads of vector registers
     ********************************************************************/
    {
        __m128i temp1, temp2, temp3, temp4;
        
        /* Multiple vector operations to increase register pressure */
        asm volatile (
            "movdqa %[v1], %[t1]\n\t"
            "paddd %[v2], %[t1]\n\t"
            "movdqa %[t1], %[t2]\n\t"
            "pslld $2, %[t2]\n\t"
            "movdqa %[t2], %[t3]\n\t"
            "por %[v1], %[t3]\n\t"
            "movdqa %[t3], %[t4]\n\t"
            "psrld $1, %[t4]\n\t"
            : [t1] "=&x" (temp1),
              [t2] "=&x" (temp2),
              [t3] "=&x" (temp3),
              [t4] "=x" (temp4)
            : [v1] "x" (vec_var1),
              [v2] "x" (vec_var2)
            : /* clobbers intentionally omitted to increase pressure */
        );
        
        /* Use results to prevent elimination */
        vec_var1 = _mm_add_epi32(temp1, temp4);
        global_counter++;
    }
    
    /********************************************************************
     * BLOCK H: Memory Barrier with Complex Constraints
     * Force reloads around memory barriers
     ********************************************************************/
    {
        int sync_var1 = 100, sync_var2 = 200;
        
        asm volatile (
            "mfence\n\t"                  /* Memory barrier */
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "mfence\n\t"
            : [out1] "=rm" (sync_var1)    /* Register or memory - may force reload */
            : [in1] "rm" (sync_var1),     /* Same constraint for inputs */
              [in2] "rm" (sync_var2)
            : "%eax", "memory"
        );
        
        int_var1 += sync_var1;
        global_counter++;
    }
    
    /* Final computation to use all variables and prevent dead code elimination */
    long long checksum = (long long)int_var1 + 
                        long_var1 + 
                        (long long)(float_var1 * 1000) + 
                        (long long)(double_var1 * 1000) + 
                        int64_var +
                        global_counter;
    
    /* Use vector results */
    int vec_sum[4];
    _mm_storeu_si128((__m128i*)vec_sum, vec_var1);
    checksum += vec_sum[0] + vec_sum[1] + vec_sum[2] + vec_sum[3];
    
    /* Use array results */
    checksum += array1[global_counter % 256];
    checksum += (long long)array2[global_counter % 128];
    
    printf("Checksum: %lld\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
