/* reload_test.c - Comprehensive test to trigger multiple reload types in GCC */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Force variables to be in memory to increase reload opportunities */
#define NO_INLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Complex structure to force address computations */
struct nested {
    int a[8][8];
    double b[4][4];
    struct nested *next;
};

/* Global variables to prevent optimization */
int global_int USED = 42;
double global_double USED = 3.14159;
__m128i global_vec USED;

/* Function to create register pressure */
NO_INLINE void create_register_pressure() {
    /* Volatile to prevent optimization */
    volatile int i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    volatile double d1 = 1.1, d2 = 2.2, d3 = 3.3;
    volatile __m128i v1, v2;
    
    /* Use all variables to keep them live */
    asm volatile("" : "+r"(i1), "+r"(i2), "+r"(i3), "+r"(i4), "+r"(i5),
                      "+r"(d1), "+r"(d2), "+r"(d3), "+r"(v1), "+r"(v2));
}

int main() {
    /* Diverse variable declarations with different types and storage */
    int int_var = 12345;
    long long ll_var = 9876543210LL;
    float float_var = 2.71828f;
    double double_var = 1.41421356;
    __m128i vec_var = _mm_set_epi32(1, 2, 3, 4);
    __m128 vec_float = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    
    /* Arrays for complex addressing */
    int multi_array[16][16];
    double dbl_array[32];
    struct nested complex_struct[4];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            multi_array[i][j] = i * 100 + j;
        }
    }
    
    for (int i = 0; i < 32; i++) {
        dbl_array[i] = i * 0.5;
    }
    
    /* Create register pressure before each test */
    create_register_pressure();
    
    /******************************************************************
     * BLOCK A: Register Class Conflict Reload
     * Force integer to float register reload
     ******************************************************************/
    {
        int input = 0x40490FDB;  /* Float representation of pi */
        float output;
        
        /* Request float register for integer input - forces reload */
        asm volatile (
            "movd %1, %%xmm0\n\t"        /* Move integer to xmm register */
            "movd %%xmm0, %0\n\t"        /* Move back */
            : "=r"(output)               /* Output in general register */
            : "r"(input)                 /* Input in general register */
            : "%xmm0"                    /* Clobber xmm0 */
        );
        
        float_var = output;
        create_register_pressure();
    }
    
    /******************************************************************
     * BLOCK B: Complex Address Reload with Multi-dimensional Array
     * Force address computation reload
     ******************************************************************/
    {
        int i = 7, j = 11;
        int result;
        
        /* Complex addressing mode that may need reloading */
        asm volatile (
            "movl %1, %0\n\t"
            : "=r"(result)
            : "m"(multi_array[i*2][j*3 + 1])  /* Complex address computation */
            : "memory"
        );
        
        int_var = result;
        create_register_pressure();
    }
    
    /******************************************************************
     * BLOCK C: Early-Clobber Multiple Outputs
     * Force reloads due to early clobber constraints
     ******************************************************************/
    {
        int in1 = 100, in2 = 200, in3 = 300;
        int out1, out2, out3;
        
        /* Early clobber forces separate registers for outputs */
        asm volatile (
            "leal (%1,%2,1), %0\n\t"     /* out1 = in1 + in2 */
            "imull %3, %2\n\t"           /* Modify in3 (clobbered early) */
            "movl %2, %1\n\t"            /* out2 = modified in3 */
            "addl %0, %1\n\t"            /* out3 = out1 + out2 */
            : "=&r"(out1), "=&r"(out2), "=r"(out3)
            : "r"(in1), "r"(in2), "1"(in3)  /* in3 in same reg as out2 */
            : "cc"
        );
        
        ll_var = (long long)out1 + out2 + out3;
        create_register_pressure();
    }
    
    /******************************************************************
     * BLOCK D: Secondary Reload Pattern
     * Force secondary reload through constant/immediate handling
     ******************************************************************/
    {
        __m128i vec_result;
        long long large_const = 0x123456789ABCDEF0LL;
        
        /* Pattern that may require secondary reload for constant */
        asm volatile (
            "movq %1, %%xmm0\n\t"        /* Move 64-bit constant to xmm */
            "pshufd $0xE4, %%xmm0, %0\n\t" /* Shuffle */
            : "=x"(vec_result)
            : "r"(large_const)           /* Constant in general register */
            : "%xmm0"
        );
        
        vec_var = vec_result;
        create_register_pressure();
    }
    
    /******************************************************************
     * BLOCK E: Mixed Mode Reloads
     * Different machine modes in same asm statement
     ******************************************************************/
    {
        int int_input = 255;
        double double_output;
        
        /* Mixed mode operation requiring different register classes */
        asm volatile (
            "cvtsi2sd %1, %%xmm0\n\t"    /* Convert int to double */
            "movsd %%xmm0, %0\n\t"       /* Store result */
            : "=r"(double_output)
            : "r"(int_input)
            : "%xmm0"
        );
        
        double_var = double_output;
        create_register_pressure();
    }
    
    /******************************************************************
     * BLOCK F: Memory Operand with Displacement
     * Force address reload with large displacement
     ******************************************************************/
    {
        double result;
        int index = 24;
        
        /* Large displacement that might not fit in addressing mode */
        asm volatile (
            "movsd %1, %%xmm0\n\t"
            "addsd %%xmm0, %%xmm0\n\t"
            "movsd %%xmm0, %0\n\t"
            : "=r"(result)
            : "m"(dbl_array[index + 8])  /* Large displacement */
            : "%xmm0"
        );
        
        global_double = result;
    }
    
    /******************************************************************
     * BLOCK G: Pointer Chain with Structure Access
     * Complex pointer arithmetic forcing address reload
     ******************************************************************/
    {
        /* Initialize linked structure */
        for (int i = 0; i < 3; i++) {
            complex_struct[i].next = &complex_struct[i + 1];
            for (int j = 0; j < 8; j++)
                for (int k = 0; k < 8; k++)
                    complex_struct[i].a[j][k] = i * 100 + j * 10 + k;
        }
        complex_struct[3].next = NULL;
        
        int chain_result;
        struct nested *ptr = &complex_struct[0];
        
        /* Complex pointer chain access */
        asm volatile (
            "movl %1, %0\n\t"
            : "=r"(chain_result)
            : "m"(ptr->next->next->a[3][4])  /* Multi-level pointer access */
            : "memory"
        );
        
        global_int = chain_result;
    }
    
    /******************************************************************
     * BLOCK H: Vector Register Pressure
     * Force vector register spills and reloads
     ******************************************************************/
    {
        __m128 v1 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
        __m128 v2 = _mm_set_ps(5.0f, 6.0f, 7.0f, 8.0f);
        __m128 v3, v4, v5, v6, v7, v8;
        
        /* Multiple vector operations to create register pressure */
        asm volatile (
            "movaps %1, %0\n\t"
            "addps %2, %0\n\t"
            : "=x"(v3) : "x"(v1), "x"(v2) :);
        
        asm volatile (
            "mulps %1, %0\n\t"
            : "=x"(v4) : "x"(v3), "x"(v1) :);
        
        asm volatile (
            "subps %1, %0\n\t"
            : "=x"(v5) : "x"(v4), "x"(v2) :);
        
        asm volatile (
            "divps %1, %0\n\t"
            : "=x"(v6) : "x"(v5), "x"(v3) :);
        
        /* Chain operations to force spills */
        v7 = _mm_add_ps(v4, v5);
        v8 = _mm_mul_ps(v6, v7);
        
        vec_float = v8;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = int_var + (int)float_var + (int)double_var + 
                   (int)(ll_var & 0xFFFFFFFF) + global_int;
    
    /* Use all variables one more time */
    asm volatile("" : : "r"(int_var), "r"(ll_var), "r"(float_var),
                      "r"(double_var), "r"(vec_var), "r"(vec_float),
                      "r"(checksum));
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;
}
