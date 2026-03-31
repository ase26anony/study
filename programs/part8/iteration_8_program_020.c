/* reload_test.c - Comprehensive test to trigger multiple reload types in GCC */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <xmmintrin.h>  /* For vector types */

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
float global_float_array[16];
int global_int_matrix[8][8];

NOOPT int main(void) {
    /* Diverse variable declarations with different types and modes */
    int int_var = 1;
    long long_var = 2;
    float float_var = 3.0f;
    double double_var = 4.0;
    long long int64_var = 5;
    __m128i vector_var = _mm_set_epi32(6, 7, 8, 9);
    
    /* Arrays for complex addressing */
    int array_1d[256];
    double array_2d[16][16];
    struct nested nested_array[4];
    struct nested *ptr_chain = &nested_array[0];
    
    /* Initialize arrays to prevent optimization */
    for (int i = 0; i < 256; i++) array_1d[i] = i;
    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 16; j++)
            array_2d[i][j] = i * 100.0 + j;
    
    /* Initialize nested struct */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) nested_array[i].a[j] = i * 10 + j;
        for (int j = 0; j < 4; j++) nested_array[i].b[j] = i * 10.0 + j;
        nested_array[i].next = (i < 3) ? &nested_array[i + 1] : NULL;
    }
    
    /* Volatile to prevent optimization across asm blocks */
    volatile int result = 0;
    
    /******************************************************************
     * BLOCK A: Register Class Conflict Reload
     * Force integer to float register reload
     ******************************************************************/
    {
        int int_input = int_var + 100;
        float float_output;
        
        /* Request float register for integer value - forces reload */
        asm volatile (
            "/* Block A: Integer to float register reload */\n\t"
            "mov %1, %%eax\n\t"           /* Load integer to GPR */
            "cvtsi2ss %%eax, %0\n\t"      /* Convert to float in XMM */
            : "=x" (float_output)         /* x = SSE register constraint */
            : "r" (int_input)             /* r = general register */
            : "%eax", "memory"
        );
        
        float_var = float_output;
        result += (int)float_output;
    }
    
    /******************************************************************
     * BLOCK B: Complex Address Reload with Multiple Indexing
     * Force address computation reload
     ******************************************************************/
    {
        int i = int_var % 8;
        int j = long_var % 8;
        int k = (int_var + long_var) % 8;
        double complex_load;
        
        /* Complex addressing: array[i][j] with scaled index */
        asm volatile (
            "/* Block B: Complex address reload */\n\t"
            "movsd (%1), %0\n\t"          /* Load from computed address */
            : "=x" (complex_load)
            : "r" (&array_2d[i][j])       /* Force address into register */
            : "memory"
        );
        
        /* Even more complex: nested struct with pointer chain */
        int chain_index = (i + j) & 3;
        double *complex_addr = &ptr_chain->next->next->b[chain_index];
        
        asm volatile (
            "/* Block B2: Nested pointer chain address */\n\t"
            "movsd (%1), %0\n\t"
            : "=x" (complex_load)
            : "r" (complex_addr)
            : "memory"
        );
        
        double_var = complex_load;
        result += (int)complex_load;
    }
    
    /******************************************************************
     * BLOCK C: Early-Clobber Multiple Output Reloads
     * Force reloads due to register conflicts
     ******************************************************************/
    {
        int in1 = int_var * 2;
        int in2 = long_var * 3;
        int in3 = int64_var;
        int out1, out2, out3;
        
        /* Multiple outputs with early clobber */
        asm volatile (
            "/* Block C: Early-clobber multiple outputs */\n\t"
            "mov %2, %0\n\t"              /* out1 = in1 */
            "add %3, %0\n\t"              /* out1 += in2 */
            "mov %0, %1\n\t"              /* out2 = out1 (early clobber!) */
            "imul %4, %1\n\t"             /* out2 *= in3 */
            "lea (%0,%1,2), %k0\n\t"      /* out1 = out1 + 2*out2 */
            : "=&r" (out1), "=&r" (out2), "=r" (out3)
            : "r" (in1), "r" (in2), "r" (in3)
            : "cc"
        );
        
        result += out1 + out2;
    }
    
    /******************************************************************
     * BLOCK D: Secondary Reload Pattern
     * Force secondary reload through constant/immediate handling
     ******************************************************************/
    {
        __m128i vec_input = vector_var;
        __m128i vec_output;
        
        /* Pattern that often requires secondary reloads:
         * Moving large constant to vector register */
        asm volatile (
            "/* Block D: Secondary reload pattern */\n\t"
            "movdqa %1, %0\n\t"           /* Copy input vector */
            "paddd %0, %0\n\t"            /* Double it */
            "mov $0xDEADBEEF, %%eax\n\t"  /* Large constant - may need reload */
            "movd %%eax, %%xmm1\n\t"      /* Move to vector reg (secondary reload) */
            "pslld $2, %0\n\t"            /* Shift left */
            : "=x" (vec_output)
            : "x" (vec_input)
            : "%eax", "%xmm1", "cc"
        );
        
        /* Extract to check result */
        int vec_result[4];
        _mm_storeu_si128((__m128i*)vec_result, vec_output);
        result += vec_result[0] + vec_result[3];
    }
    
    /******************************************************************
     * BLOCK E: Mixed Mode Reloads
     * Different machine modes in same asm statement
     ******************************************************************/
    {
        char char_var = 'A';
        short short_var = 1234;
        int int_var2 = 5678;
        long long ll_var = 0x123456789ABCDEF0LL;
        
        /* Mixed mode constraints */
        asm volatile (
            "/* Block E: Mixed mode reloads */\n\t"
            "addb %1, %b0\n\t"            /* byte operation */
            "addw %2, %w0\n\t"            /* word operation */
            "addl %3, %k0\n\t"            /* dword operation */
            "addq %4, %0\n\t"             /* qword operation */
            : "+r" (ll_var)
            : "r" ((int)char_var), "r" ((int)short_var), 
              "r" (int_var2), "r" (ll_var)
            : "cc"
        );
        
        result += (int)(ll_var & 0xFFFFFFFF);
    }
    
    /******************************************************************
     * BLOCK F: Memory Spill Reloads
     * Force register spilling with many live variables
     ******************************************************************/
    {
        /* Many live variables to increase register pressure */
        int v1 = result + 1;
        int v2 = result + 2;
        int v3 = result + 3;
        int v4 = result + 4;
        int v5 = result + 5;
        int v6 = result + 6;
        int v7 = result + 7;
        int v8 = result + 8;
        int v9 = result + 9;
        int v10 = result + 10;
        
        /* Complex computation forcing spills */
        asm volatile (
            "/* Block F: Force register spills */\n\t"
            "mov %1, %%eax\n\t"
            "add %2, %%eax\n\t"
            "imul %3, %%eax\n\t"
            "add %4, %%eax\n\t"
            "sub %5, %%eax\n\t"
            "add %6, %%eax\n\t"
            "imul %7, %%eax\n\t"
            "add %8, %%eax\n\t"
            "sub %9, %%eax\n\t"
            "add %10, %%eax\n\t"
            "mov %%eax, %0\n\t"
            : "=r" (result)
            : "r" (v1), "r" (v2), "r" (v3), "r" (v4), "r" (v5),
              "r" (v6), "r" (v7), "r" (v8), "r" (v9), "r" (v10)
            : "%eax", "cc"
        );
    }
    
    /******************************************************************
     * BLOCK G: Input/Output Reload with Same Register
     * Force in-out reload conflicts
     ******************************************************************/
    {
        int inout = result;
        
        /* Input/output same register with modification */
        asm volatile (
            "/* Block G: In-out reload conflict */\n\t"
            "add $777, %0\n\t"
            "ror $13, %0\n\t"
            "not %0\n\t"
            : "+r" (inout)
            :
            : "cc"
        );
        
        result = inout;
    }
    
    /* Final computation to use all variables and prevent dead code elimination */
    double final_sum = double_var + float_var + int_var + long_var + int64_var;
    result += (int)final_sum;
    
    /* Use computed goto for address reload (GCC extension) */
    void *label_ptr = &&final_label;
    
    asm volatile (
        "/* Computed goto for address reload */\n\t"
        "jmp *%0\n\t"
        :
        : "r" (label_ptr)
        : "memory"
    );
    
    /* Unreachable, but forces address computation */
    result *= 2;
    
final_label:
    printf("Result: %d\n", result);
    return result & 255;  /* Return non-zero to indicate execution */
}
