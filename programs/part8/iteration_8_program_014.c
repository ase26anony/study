/* reload_test.c - Comprehensive test to trigger multiple reload types */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Force no optimization on specific variables */
#define VOLATILE_VAR(var) volatile var

/* Complex structure to force address computations */
struct nested {
    int a[8][8];
    double b[4][4];
    struct nested *next;
};

/* Global variables to increase register pressure */
int global_int = 42;
double global_double = 3.14159;
__m128i global_vec = {0};

int main(void) {
    /* 1. Diverse variable declarations with different types and modes */
    VOLATILE_VAR(int) int_var = 100;
    VOLATILE_VAR(long long) ll_var = 0x123456789ABCDEF0LL;
    VOLATILE_VAR(float) float_var = 2.71828f;
    VOLATILE_VAR(double) double_var = 1.41421356;
    VOLATILE_VAR(__m128i) vec_var = _mm_set_epi32(1, 2, 3, 4);
    
    /* Array with complex indexing */
    int multi_array[16][16];
    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 16; j++)
            multi_array[i][j] = i * 100 + j;
    
    /* Pointer chain for complex address computation */
    struct nested nested1, nested2;
    struct nested *nested_ptr = &nested1;
    nested1.next = &nested2;
    nested2.next = &nested1;
    
    /* Initialize structure arrays */
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            nested1.a[i][j] = i * 10 + j;
    
    /* 2. BLOCK A: Register class conflict reload */
    /* Force integer to float register reload */
    {
        int int_input = int_var;
        double float_output;
        
        /* Request float register for integer input - will require reload */
        asm volatile (
            "mov %1, %%eax\n\t"          /* Move integer to eax */
            "cvtsi2sd %%eax, %0\n\t"     /* Convert to double in float reg */
            : "=f" (float_output)        /* Output in floating-point register */
            : "r" (int_input)            /* Input in general-purpose register */
            : "%eax", "memory"
        );
        
        double_var += float_output;
    }
    
    /* 3. BLOCK B: Complex address reload with non-trivial addressing mode */
    {
        int i = int_var & 0xF;
        int j = (int_var >> 4) & 0xF;
        int result;
        
        /* Complex array addressing that may need reload */
        asm volatile (
            "movl %c[array](%[i],%[j],4), %0\n\t"  /* array[i][j] with scaling */
            : "=r" (result)
            : [array] "i" (multi_array), 
              [i] "r" (i), 
              [j] "r" (j),
              "m" (multi_array[i][j])    /* Memory constraint with complex address */
            : "memory"
        );
        
        int_var += result;
    }
    
    /* 4. BLOCK C: Early-clobber multiple outputs */
    {
        int in1 = int_var;
        int in2 = int_var * 2;
        int out1, out2;
        
        /* Early-clobber forces separate registers for outputs */
        asm volatile (
            "addl %2, %0\n\t"    /* out1 += in1 */
            "subl %3, %1\n\t"    /* out2 -= in2 */
            : "=&r" (out1), "=&r" (out2)  /* Early-clobber outputs */
            : "r" (in1), "r" (in2),
              "0" (in1), "1" (in2)        /* Input-output operands */
            : "cc"
        );
        
        int_var = out1 + out2;
    }
    
    /* 5. BLOCK D: Secondary reload pattern with vector types */
    {
        __m128i vec_in = vec_var;
        __m128i vec_out;
        long long scalar = ll_var;
        
        /* Pattern that may require secondary reload on some architectures */
        asm volatile (
            "movq %2, %0\n\t"            /* Move scalar to vector reg (may need temp) */
            "paddq %1, %0\n\t"           /* Add vectors */
            : "=x" (vec_out)             /* Output in SSE register */
            : "x" (vec_in),              /* Input in SSE register */
              "r" (scalar)               /* Scalar in general-purpose register */
            : "memory"
        );
        
        vec_var = vec_out;
    }
    
    /* 6. BLOCK E: Memory address reload with structure pointer chain */
    {
        int offset1 = int_var & 0x7;
        int offset2 = (int_var >> 3) & 0x7;
        int struct_result;
        
        /* Complex structure addressing with pointer chasing */
        asm volatile (
            "movl (%[ptr],%[off1],4), %0\n\t"  /* ptr->a[off1][0] */
            "addl 32(%[ptr],%[off2],4), %0\n\t" /* Add ptr->a[off2][4] */
            : "=r" (struct_result)
            : [ptr] "r" (&nested1.a[0][0]),
              [off1] "r" (offset1 * 8),   /* Complex offset calculation */
              [off2] "r" (offset2 * 8),
              "m" (nested1.a[offset1][0]), /* Memory constraints force address reload */
              "m" (nested1.a[offset2][4])
            : "memory"
        );
        
        int_var ^= struct_result;
    }
    
    /* 7. BLOCK F: Mixed-mode reloads (different data sizes) */
    {
        char char_var = int_var & 0xFF;
        short short_var = int_var & 0xFFFF;
        int int_result;
        
        /* Mixed size operations requiring different reload modes */
        asm volatile (
            "movsbl %1, %0\n\t"          /* Sign extend byte to int */
            "addw %2, %0\n\t"            /* Add short to int */
            : "=r" (int_result)
            : "r" (char_var),            /* Byte-sized but in general reg */
              "r" (short_var)            /* Short-sized but in general reg */
            : "cc"
        );
        
        int_var = int_result;
    }
    
    /* 8. BLOCK G: High register pressure to force spill/reload */
    {
        /* Declare many variables to increase register pressure */
        VOLATILE_VAR(int) r0 = int_var + 1;
        VOLATILE_VAR(int) r1 = int_var + 2;
        VOLATILE_VAR(int) r2 = int_var + 3;
        VOLATILE_VAR(int) r3 = int_var + 4;
        VOLATILE_VAR(int) r4 = int_var + 5;
        VOLATILE_VAR(int) r5 = int_var + 6;
        VOLATILE_VAR(int) r6 = int_var + 7;
        VOLATILE_VAR(int) r7 = int_var + 8;
        
        /* Operation using all variables - likely to cause spills */
        asm volatile (
            "addl %1, %0\n\t"
            "addl %2, %0\n\t"
            "addl %3, %0\n\t"
            "addl %4, %0\n\t"
            "addl %5, %0\n\t"
            "addl %6, %0\n\t"
            "addl %7, %0\n\t"
            "addl %8, %0\n\t"
            : "+r" (r0)
            : "r" (r1), "r" (r2), "r" (r3),
              "r" (r4), "r" (r5), "r" (r6),
              "r" (r7)
            : "cc"
        );
        
        int_var = r0;
    }
    
    /* 9. Compute checksum to prevent dead code elimination */
    int checksum = int_var;
    checksum ^= (int)(double_var * 1000);
    checksum ^= (int)(float_var * 1000);
    checksum ^= (int)(ll_var & 0xFFFFFFFF);
    checksum ^= (int)(ll_var >> 32);
    
    /* Use vector checksum if available */
    int vec_data[4];
    _mm_storeu_si128((__m128i*)vec_data, vec_var);
    for (int i = 0; i < 4; i++)
        checksum ^= vec_data[i];
    
    /* Print to prevent optimization */
    printf("Reload test checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}
