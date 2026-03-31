/* reload_test.c - Comprehensive test to trigger multiple reload types */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>  /* For SSE intrinsics */

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
    long long int_var2 = 9876543210LL;
    float float_var1 = 2.71828f;
    double double_var1 = 1.41421356;
    VOLATILE_VAR(int) volatile_int = 999;
    
    /* Array and pointer variables for address reloads */
    int array1[256];
    double array2[128];
    struct nested nested_array[64];
    struct nested *nested_ptr = &nested_array[0];
    
    /* Vector/SIMD types for vector register reloads */
    __m128i vec_var1, vec_var2;
    __m128 float_vec1, float_vec2;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) array1[i] = i * 3;
    for (int i = 0; i < 128; i++) array2[i] = i * 1.5;
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 8; j++) nested_array[i].a[j] = i + j;
        for (int j = 0; j < 4; j++) nested_array[i].b[j] = (i + j) * 0.5;
        nested_array[i].next = (i < 63) ? &nested_array[i + 1] : NULL;
    }
    
    /* Initialize vector variables */
    vec_var1 = _mm_set_epi32(1, 2, 3, 4);
    float_vec1 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    
    int result = 0;
    
    /* ============================================
       BLOCK A: Register Class Conflict Reload
       ============================================ */
    {
        /* Force integer to be moved to floating-point register */
        double temp_double;
        asm volatile (
            /* Request float register for integer-derived value */
            "mov %1, %%eax\n\t"
            "cvtsi2sd %%eax, %0\n\t"
            : "=f" (temp_double)      /* Output in floating-point register */
            : "r" (int_var1)          /* Input in general-purpose register */
            : "%eax", "memory"
        );
        double_var1 += temp_double;
        global_counter++;
    }
    
    /* ============================================
       BLOCK B: Complex Address Reload
       ============================================ */
    {
        /* Complex array indexing that may not fit in addressing mode */
        int idx1 = volatile_int & 0xF;
        int idx2 = (volatile_int >> 4) & 0x1F;
        int idx3 = (volatile_int >> 9) & 0x7;
        
        int loaded_value;
        asm volatile (
            /* Complex addressing that may need reloading */
            "movl %1, %0\n\t"
            : "=r" (loaded_value)
            : "m" (multi_array[idx1][idx2 * 2][idx3 + 1])  /* Complex address */
            : "memory"
        );
        
        /* Even more complex addressing with structure */
        double struct_val;
        asm volatile (
            "movsd %1, %0\n\t"
            : "=x" (struct_val)  /* SSE register */
            : "m" (nested_ptr->next->next->b[2])  /* Deep structure access */
            : "memory"
        );
        
        result += loaded_value;
        double_var1 += struct_val;
        global_counter += 2;
    }
    
    /* ============================================
       BLOCK C: Early-Clobber Multiple Outputs
       ============================================ */
    {
        int out1, out2;
        int in1 = int_var1;
        int in2 = int_var1 + 1000;
        int in3 = volatile_int;
        
        asm volatile (
            /* Multiple outputs with early clobber */
            "movl %3, %0\n\t"      /* out1 gets in1 */
            "addl %4, %0\n\t"      /* out1 += in2 */
            "movl %0, %1\n\t"      /* out2 = out1 */
            "imull %5, %1\n\t"     /* out2 *= in3 - early clobber conflict */
            : "=&r" (out1), "=&r" (out2)  /* Both early-clobber */
            : "0" (0), "r" (in1), "r" (in2), "r" (in3)
            : "cc"
        );
        
        result += out1 + out2;
        global_counter += 3;
    }
    
    /* ============================================
       BLOCK D: Secondary Reload Patterns
       ============================================ */
    {
        /* Pattern that often requires secondary reloads:
           Moving between different register files with constraints */
        __m128i vec_temp;
        long long large_constant = 0x123456789ABCDEF0LL;
        
        asm volatile (
            /* This pattern may require secondary reloads on some arches */
            "movq %1, %%rax\n\t"      /* First reload to GPR */
            "movq %%rax, %0\n\t"      /* Then to vector reg */
            : "=x" (vec_temp)         /* SSE/vector register */
            : "r" (large_constant)    /* General purpose register */
            : "%rax", "memory"
        );
        
        /* Another secondary reload pattern: memory to vector with offset */
        double mem_to_vec;
        asm volatile (
            "movsd %c1(%2), %0\n\t"   /* Load with constant offset */
            : "=x" (mem_to_vec)
            : "i" (16), "r" (array2)  /* Constant offset + base register */
            : "memory"
        );
        
        vec_var2 = vec_temp;
        float_var1 += (float)mem_to_vec;
        global_counter += 4;
    }
    
    /* ============================================
       BLOCK E: Mixed Mode Reloads
       ============================================ */
    {
        /* Mixed integer/floating point operations forcing mode conversions */
        float float_result;
        double double_result;
        
        /* Integer to float with different modes */
        asm volatile (
            "cvtsi2ssl %1, %0\n\t"
            : "=x" (float_result)
            : "r" (int_var1)
            : 
        );
        
        /* Float to double conversion */
        asm volatile (
            "cvtss2sd %1, %0\n\t"
            : "=x" (double_result)
            : "x" (float_result)
            : 
        );
        
        /* Different size accesses */
        short short_val;
        asm volatile (
            "movw %1, %0\n\t"
            : "=r" (short_val)
            : "m" (array1[10])
            : "memory"
        );
        
        double_var1 += double_result + short_val;
        global_counter += 5;
    }
    
    /* ============================================
       BLOCK F: High Register Pressure
       ============================================ */
    {
        /* Many live variables to increase register pressure */
        int r1, r2, r3, r4, r5, r6, r7, r8;
        asm volatile (
            "movl $1, %0\n\t"
            "movl $2, %1\n\t"
            "movl $3, %2\n\t"
            "movl $4, %3\n\t"
            "movl $5, %4\n\t"
            "movl $6, %5\n\t"
            "movl $7, %6\n\t"
            "movl $8, %7\n\t"
            : "=&r" (r1), "=&r" (r2), "=&r" (r3), "=&r" (r4),
              "=&r" (r5), "=&r" (r6), "=&r" (r7), "=&r" (r8)
            : 
            : "cc"
        );
        
        /* Use all results to prevent optimization */
        result += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
        
        /* Force spills and reloads with many operands */
        asm volatile (
            "addl %1, %0\n\t"
            "addl %2, %0\n\t"
            "addl %3, %0\n\t"
            "addl %4, %0\n\t"
            "addl %5, %0\n\t"
            "addl %6, %0\n\t"
            "addl %7, %0\n\t"
            "addl %8, %0\n\t"
            : "+r" (result)
            : "r" (r1), "r" (r2), "r" (r3), "r" (r4),
              "r" (r5), "r" (r6), "r" (r7), "r" (r8)
            : "cc"
        );
        
        global_counter += 8;
    }
    
    /* ============================================
       Final computation to prevent dead code elimination
       ============================================ */
    {
        /* Mix all results together */
        int final_result = result;
        final_result += (int)double_var1;
        final_result += (int)float_var1;
        final_result += global_counter;
        
        /* Complex final asm with memory operand */
        asm volatile (
            "addl %%ecx, %0\n\t"
            : "+r" (final_result)
            : "c" (volatile_int)
            : "cc"
        );
        
        /* Print to prevent optimization */
        printf("Result: %d (counter: %d)\n", final_result, global_counter);
        
        return final_result & 0xFF;  /* Return non-zero to indicate execution */
    }
}
