/* reload_test.c - Comprehensive test to trigger multiple reload types in GCC */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>  /* For SSE intrinsics */

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
float global_float_array[32];
__m128i global_vector;

NOOPT int main(void) {
    /* Diverse variable declarations with different types and sizes */
    int int_var1 = 1, int_var2 = 2, int_var3 = 3;
    long long_var1 = 100, long_var2 = 200;
    float float_var1 = 1.0f, float_var2 = 2.0f;
    double double_var1 = 1.0, double_var2 = 2.0;
    __m128i vector_var1, vector_var2;
    int *ptr_int = &int_var1;
    double *ptr_double = &double_var1;
    
    /* Multi-dimensional array for complex addressing */
    int multi_array[16][8][4];
    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 8; j++)
            for (int k = 0; k < 4; k++)
                multi_array[i][j][k] = i * 100 + j * 10 + k;
    
    /* Complex structure with pointer chain */
    struct nested struct1, struct2;
    struct1.next = &struct2;
    struct2.next = &struct1;
    for (int i = 0; i < 8; i++) struct1.a[i] = i * 10;
    for (int i = 0; i < 4; i++) struct1.b[i] = i * 1.5;
    
    /* Volatile to prevent optimization */
    volatile int vol_int = 999;
    volatile double vol_double = 2.71828;
    
    int result = 0;
    
    /* ============================================
       BLOCK A: Register Class Conflict Reload
       Force integer to float register reload
       ============================================ */
    {
        int temp_int = int_var1 + int_var2;
        double temp_double;
        
        /* Request floating-point register for integer computation result */
        asm volatile (
            /* Integer computation in general-purpose register */
            "addl %[in1], %[in2]\n\t"
            /* Force move to floating-point register (will likely need reload) */
            "movq %[in2], %[out]\n\t"
            : [out] "=f" (temp_double)      /* Output in FP register */
            : [in1] "r" (temp_int),         /* Input in general-purpose register */
              [in2] "r" (int_var3)          /* Another input */
            : "cc"
        );
        
        /* Use the result to prevent dead code elimination */
        result += (int)temp_double;
        printf("Block A result: %d\n", (int)temp_double);
    }
    
    /* ============================================
       BLOCK B: Complex Address Reload
       Multi-dimensional array with complex addressing
       ============================================ */
    {
        int index1 = int_var1 * 2;
        int index2 = int_var2 * 3;
        int index3 = int_var3;
        int array_value;
        
        /* Complex addressing mode that may need reloading */
        asm volatile (
            "movl %[addr], %[out]\n\t"
            : [out] "=r" (array_value)
            : [addr] "m" (multi_array[index1][index2][index3])
            : "memory"
        );
        
        result += array_value;
        printf("Block B array value: %d\n", array_value);
        
        /* Another complex address with structure pointer chain */
        double struct_value;
        asm volatile (
            "movsd %[addr], %[out]\n\t"
            : [out] "=x" (struct_value)     /* SSE register */
            : [addr] "m" (struct1.next->next->b[2])  /* Complex address */
            : "memory"
        );
        
        result += (int)struct_value;
    }
    
    /* ============================================
       BLOCK C: Early-Clobber Multiple Outputs
       Force reloads due to register conflicts
       ============================================ */
    {
        int out1, out2, out3;
        int in1 = int_var1 * 5;
        int in2 = int_var2 * 7;
        int in3 = int_var3 * 11;
        
        /* Early-clobber on out2 means it's written before all inputs are read */
        asm volatile (
            "movl %[in1], %[out1]\n\t"      /* out1 gets in1 */
            "addl %[in2], %[out1]\n\t"      /* modify out1 (uses in2) */
            "movl %[out1], %[out2]\n\t"     /* out2 gets out1 (early clobber!) */
            "imull %[in3], %[out2]\n\t"     /* out2 *= in3 */
            "leal (%[out2], %[in1]), %[out3]\n\t"  /* out3 = out2 + in1 */
            : [out1] "=&r" (out1),          /* Early-clobber not needed here but shows variety */
              [out2] "=&r" (out2),          /* Early-clobber - written before in3 is consumed? */
              [out3] "=r" (out3)
            : [in1] "r" (in1),
              [in2] "r" (in2),
              [in3] "r" (in3)
            : "cc"
        );
        
        result += out1 + out2 + out3;
        printf("Block C outputs: %d, %d, %d\n", out1, out2, out3);
    }
    
    /* ============================================
       BLOCK D: Secondary Reload Patterns
       Multiple constraints and register classes
       ============================================ */
    {
        /* Pattern 1: Immediate to vector register (often needs GP register intermediate) */
        __m128i vec_result;
        long long large_imm = 0x123456789ABCDEF0LL;
        
        asm volatile (
            "movq %[imm], %%rax\n\t"        /* GP register intermediate */
            "movq %%rax, %[out]\n\t"        /* Then to output */
            : [out] "=x" (vec_result)       /* SSE register constraint */
            : [imm] "ri" (large_imm)        /* Register or immediate */
            : "rax", "memory"
        );
        
        /* Pattern 2: Memory to vector with complex address */
        double complex_load;
        asm volatile (
            "movsd (%[base], %[index], 8), %[out]\n\t"  /* base + index*8 */
            : [out] "=x" (complex_load)
            : [base] "r" (global_float_array),
              [index] "r" (int_var1 * 2)
            : "memory"
        );
        
        /* Pattern 3: Multiple register classes in one asm */
        int gp_val;
        float fp_val;
        asm volatile (
            "movl %[in_int], %%eax\n\t"
            "cvtsi2ssl %%eax, %[out_fp]\n\t"  /* Convert int to float */
            "movl %%eax, %[out_gp]\n\t"
            : [out_gp] "=r" (gp_val),
              [out_fp] "=x" (fp_val)
            : [in_int] "rm" (int_var2)        /* Register or memory */
            : "eax", "cc"
        );
        
        result += gp_val + (int)fp_val + (int)complex_load;
    }
    
    /* ============================================
       BLOCK E: High Register Pressure
       Many live variables force spill/reload
       ============================================ */
    {
        /* Use many variables to increase register pressure */
        int r1 = int_var1 + 1;
        int r2 = int_var2 + 2;
        int r3 = int_var3 + 3;
        int r4 = long_var1;
        int r5 = long_var2;
        float f1 = float_var1 + 1.0f;
        float f2 = float_var2 + 2.0f;
        double d1 = double_var1 + 1.0;
        double d2 = double_var2 + 2.0;
        
        /* Complex computation using all variables */
        asm volatile (
            "addl %[v1], %[v2]\n\t"
            "addl %[v3], %[v2]\n\t"
            "cvtsi2ssl %[v2], %[vf1]\n\t"
            "addss %[vf2], %[vf1]\n\t"
            "cvtss2sd %[vf1], %[vd1]\n\t"
            "addsd %[vd2], %[vd1]\n\t"
            "cvtsd2si %[vd1], %[out]\n\t"
            : [out] "=r" (r1)
            : [v1] "r" (r1), [v2] "r" (r2), [v3] "r" (r3),
              [vf1] "x" (f1), [vf2] "x" (f2),
              [vd1] "x" (d1), [vd2] "x" (d2)
            : "cc"
        );
        
        result += r1;
    }
    
    /* ============================================
       BLOCK F: Volatile Memory Operations
       Force memory reloads
       ============================================ */
    {
        /* Sequence of volatile operations */
        asm volatile ("" : : : "memory");  /* Memory barrier */
        
        int mem1, mem2;
        asm volatile (
            "movl %[in1], %[out1]\n\t"
            "movl %[in2], %[out2]\n\t"
            : [out1] "=r" (mem1),
              [out2] "=r" (mem2)
            : [in1] "m" (vol_int),         /* Memory constraint */
              [in2] "m" (global_int)       /* Global variable */
            : "memory"
        );
        
        /* Modify and write back */
        vol_int = mem1 + mem2;
        asm volatile ("" : : : "memory");  /* Another barrier */
        
        result += mem1 * mem2;
    }
    
    printf("Final result: %d\n", result);
    return result;
}
