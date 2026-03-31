/* reload_test.c - Test program to trigger various reload scenarios in GCC */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>  /* For vector types */

/* Force variables to be in memory to increase reload opportunities */
#define NO_INLINE __attribute__((noinline))

/* Complex structure to force complex addressing */
struct nested {
    int a[8];
    double b[4];
    struct nested *next;
};

/* Global variables to force memory references */
int global_array[256];
double global_doubles[128];
struct nested global_struct;

/* Function to prevent optimization */
NO_INLINE int use_result(int x) {
    volatile int dummy = x;
    return dummy;
}

int main(void) {
    /* Declare diverse variables with different types and modes */
    int int_var1 = 12345;
    int int_var2 = 67890;
    long long_var = 0x123456789ABCDEFLL;
    float float_var = 3.14159f;
    double double_var = 2.718281828459045;
    __m128i vec_var1, vec_var2;
    
    /* Initialize vector variables */
    vec_var1 = _mm_set_epi32(1, 2, 3, 4);
    vec_var2 = _mm_set_epi32(5, 6, 7, 8);
    
    /* Pointer variables for complex addressing */
    int *ptr1 = &int_var1;
    int *ptr2 = &int_var2;
    double *dptr = &double_var;
    
    /* Array for complex indexing */
    int multi_array[16][32];
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 32; j++) {
            multi_array[i][j] = i * 100 + j;
        }
    }
    
    /* Complex structure chain */
    struct nested local_struct[4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            local_struct[i].a[j] = i * 1000 + j * 100;
        }
        for (int j = 0; j < 4; j++) {
            local_struct[i].b[j] = i * 10.0 + j * 1.0;
        }
        if (i < 3) local_struct[i].next = &local_struct[i + 1];
        else local_struct[i].next = &global_struct;
    }
    
    int result = 0;
    
    /* ======================================================================
       BLOCK A: Register Class Conflict Reload
       Force integer to float register reload
       ====================================================================== */
    {
        double temp_double;
        /* Request float register for integer computation */
        asm volatile (
            /* Convert integer to double through float register */
            "cvtsi2sd %1, %0\n\t"
            : "=f" (temp_double)      /* Output in float register */
            : "r" (int_var1)          /* Input in general register */
            : /* No clobbers */
        );
        /* Use the result to prevent dead code elimination */
        result += (int)temp_double;
    }
    
    /* ======================================================================
       BLOCK B: Complex Address Reload with Multi-dimensional Array
       Force address computation reload
       ====================================================================== */
    {
        int temp_int;
        int i = 5, j = 10;
        
        /* Complex addressing mode that may need reloading */
        asm volatile (
            "movl %1, %0\n\t"
            : "=r" (temp_int)
            : "m" (multi_array[i * 2][j * 3])  /* Complex address computation */
            : /* No clobbers */
        );
        result += temp_int;
    }
    
    /* ======================================================================
       BLOCK C: Early-Clobber Multiple Outputs
       Force reloads due to register conflicts
       ====================================================================== */
    {
        int out1, out2;
        
        /* Early clobber on second output */
        asm volatile (
            "movl %2, %0\n\t"      /* out1 = in1 */
            "addl %3, %0\n\t"      /* out1 += in2 */
            "movl %0, %1\n\t"      /* out2 = out1 */
            "imull %4, %1\n\t"     /* out2 *= in3 */
            : "=&r" (out1), "=&r" (out2)  /* Both early-clobber */
            : "r" (int_var1), "r" (int_var2), "r" (result)
            : /* No clobbers */
        );
        result += out1 + out2;
    }
    
    /* ======================================================================
       BLOCK D: Secondary Reload Pattern
       Complex operation requiring intermediate storage
       ====================================================================== */
    {
        __m128i vec_result;
        
        /* Vector operation that might require secondary reloads */
        asm volatile (
            "paddd %1, %0\n\t"      /* Add vectors */
            "pslld $2, %0\n\t"      /* Shift left */
            : "=x" (vec_result)
            : "x" (vec_var1), "0" (vec_var2)  /* vec_var2 goes into same reg as output */
            : /* No clobbers */
        );
        
        /* Extract result to prevent optimization */
        int temp[4];
        _mm_storeu_si128((__m128i*)temp, vec_result);
        result += temp[0] + temp[1] + temp[2] + temp[3];
    }
    
    /* ======================================================================
       BLOCK E: Memory-to-Memory Reload with Offset
       Complex structure addressing
       ====================================================================== */
    {
        double struct_val;
        int idx1 = 2, idx2 = 1;
        
        /* Very complex addressing through structure chain */
        asm volatile (
            "movsd %1, %0\n\t"
            : "=x" (struct_val)
            : "m" (local_struct[idx1].next->b[idx2 * 2])  /* Nested structure access */
            : /* No clobbers */
        );
        result += (int)struct_val;
    }
    
    /* ======================================================================
       BLOCK F: Multiple Constraint Alternatives
       Force reload by using conflicting constraints
       ====================================================================== */
    {
        long long combined;
        
        /* Try to force reload by using register constraints that conflict */
        asm volatile (
            "lea (%1, %2, 2), %0\n\t"  /* r = a + b*2 */
            : "=r" (combined)
            : "r" (int_var1), "r" (int_var2)
            : /* No clobbers */
        );
        result += (int)combined;
    }
    
    /* ======================================================================
       BLOCK G: Volatile Memory Access with Displacement
       Force address reload with large displacement
       ====================================================================== */
    {
        int far_value;
        /* Access with large displacement that might not fit in addressing mode */
        asm volatile (
            "movl %1, %0\n\t"
            : "=r" (far_value)
            : "m" (global_array[200])  /* Large displacement */
            : /* No clobbers */
        );
        result += far_value;
    }
    
    /* ======================================================================
       BLOCK H: Mixed Register Classes in Single Asm
       Force moves between different register files
       ====================================================================== */
    {
        int int_from_float;
        float float_temp = float_var * 2.0f;
        
        /* Move from float reg to integer reg */
        asm volatile (
            "movd %1, %0\n\t"      /* Move float bits to integer reg */
            : "=r" (int_from_float)
            : "x" (float_temp)
            : /* No clobbers */
        );
        result += int_from_float;
    }
    
    /* Final result computation to prevent optimization */
    result = use_result(result);
    
    printf("Result: %d\n", result);
    return result & 0xFF;  /* Return non-zero to indicate execution */
}
