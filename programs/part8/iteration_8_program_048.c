/* reload_test.c - Test program to trigger multiple reload scenarios in GCC */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Force variables to be in memory to increase reload opportunities */
#define NO_INLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Complex struct to force address computations */
struct nested {
    int a[8];
    double b[4];
    struct nested *next;
};

/* Global variables to force memory operations */
int global_int USED = 42;
double global_double USED = 3.14159;
int global_array[256] USED;
struct nested global_struct USED;

NO_INLINE int test_reloads(void) {
    /* Diverse variable types to trigger different machine modes */
    int int_var = 123;
    long long ll_var = 9876543210LL;
    float float_var = 2.71828f;
    double double_var = 1.41421356;
    int *ptr_var = &global_int;
    __m128i vec_var;
    __m128 vec_float;
    
    /* Arrays for complex addressing */
    int array_2d[16][16];
    double array_3d[4][4][4];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            array_2d[i][j] = i * 100 + j;
        }
    }
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                array_3d[i][j][k] = i * 100.0 + j * 10.0 + k;
            }
        }
    }
    
    /* BLOCK A: Register Class Conflict */
    /* Force integer to float register reload */
    {
        int input = int_var;
        double output;
        
        /* Request float register for integer input - will require reload */
        asm volatile (
            "movq %1, %%xmm0\n\t"      /* Move integer to XMM register */
            "movq %%xmm0, %0\n\t"      /* Move back to output */
            : "=f" (output)            /* Float register constraint for integer */
            : "r" (input)              /* Integer in general register */
            : "xmm0"
        );
        
        double_var = output;  /* Use result */
    }
    
    /* BLOCK B: Complex Address Reload */
    /* Multi-dimensional array access with complex addressing */
    {
        int i = int_var & 0xF;
        int j = (int_var >> 4) & 0xF;
        int k = (int_var >> 8) & 0x3;
        int result1, result2;
        double result3;
        
        /* Complex addressing mode that may need reloading */
        asm volatile (
            "movl (%1), %0\n\t"        /* Load from complex address */
            : "=r" (result1)
            : "r" (&array_2d[i][j])    /* Complex address computation */
            : "memory"
        );
        
        /* Even more complex 3D array addressing */
        asm volatile (
            "movsd (%1), %%xmm0\n\t"
            "movsd %%xmm0, %0\n\t"
            : "=f" (result3)
            : "r" (&array_3d[i][j][k])  /* Very complex address */
            : "xmm0", "memory"
        );
        
        int_var = result1 + (int)result3;
    }
    
    /* BLOCK C: Early-Clobber Multiple Outputs */
    /* Force reloads due to early clobber */
    {
        int in1 = int_var;
        int in2 = ll_var;
        int out1, out2, out3;
        
        /* Multiple outputs with early clobber */
        asm volatile (
            "movl %2, %0\n\t"          /* out1 = in1 */
            "imull %3, %0\n\t"         /* out1 *= in2 - clobbers early */
            "movl %0, %1\n\t"          /* out2 = out1 */
            "addl %3, %1\n\t"          /* out2 += in2 */
            : "=&r" (out1), "=&r" (out2), "=r" (out3)  /* Two early-clobber outputs */
            : "r" (in1), "r" (in2)
            : "cc"
        );
        
        /* Use all outputs to prevent optimization */
        int_var = out1 + out2 + out3;
    }
    
    /* BLOCK D: Secondary Reload Patterns */
    /* Simulate patterns requiring secondary reloads */
    {
        __m128i vec_in = _mm_set_epi32(1, 2, 3, 4);
        __m128i vec_out;
        int temp;
        
        /* Pattern that might require secondary reload on some architectures */
        asm volatile (
            "movdqa %1, %0\n\t"        /* Vector move */
            "psrld $2, %0\n\t"         /* Vector shift */
            : "=x" (vec_out)
            : "x" (vec_in)
        );
        
        /* Mix vector and scalar operations - may require intermediate reloads */
        asm volatile (
            "movd %1, %%xmm0\n\t"      /* Move scalar to vector reg */
            "paddd %2, %%xmm0\n\t"     /* Add vectors */
            "movd %%xmm0, %0\n\t"      /* Move back to scalar */
            : "=r" (temp)
            : "r" (int_var), "x" (vec_out)
            : "xmm0"
        );
        
        int_var = temp;
    }
    
    /* Additional stress test: Multiple constraints in single asm */
    {
        long long in_ll = ll_var;
        double in_dbl = double_var;
        int out_int;
        double out_dbl;
        
        asm volatile (
            "cvtsi2sd %2, %%xmm0\n\t"  /* Convert int to double */
            "addsd %3, %%xmm0\n\t"     /* Add double */
            "cvttsd2si %%xmm0, %0\n\t" /* Convert back to int */
            "movsd %%xmm0, %1\n\t"     /* Also keep double result */
            : "=r" (out_int), "=f" (out_dbl)
            : "r" (in_ll), "f" (in_dbl)
            : "xmm0", "cc"
        );
        
        ll_var = out_int;
        double_var = out_dbl;
    }
    
    /* Complex pointer chain to force address reloads */
    {
        struct nested local_struct;
        struct nested *ptr1 = &local_struct;
        struct nested *ptr2 = &global_struct;
        int offset = int_var & 0x7;
        
        /* Chain of pointer accesses */
        asm volatile (
            "movl (%1, %2, 4), %0\n\t"  /* array[index * 4] */
            : "=r" (int_var)
            : "r" (ptr1->a), "r" (offset)
            : "memory"
        );
        
        /* More complex: base + index * scale + struct field offset */
        asm volatile (
            "movsd (%1, %2, 8), %%xmm0\n\t"
            "movsd %%xmm0, %0\n\t"
            : "=f" (double_var)
            : "r" (ptr2->b), "r" (offset)
            : "xmm0", "memory"
        );
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = int_var;
    checksum += (int)ll_var;
    checksum += (int)float_var;
    checksum += (int)double_var;
    checksum += (intptr_t)ptr_var;
    
    /* Use vector variable to prevent optimization */
    vec_var = _mm_set_epi32(checksum, checksum, checksum, checksum);
    checksum += _mm_extract_epi32(vec_var, 0);
    
    return checksum;
}

int main(void) {
    int result = test_reloads();
    printf("Result: %d\n", result);
    return result & 0xFF;  /* Return non-zero to indicate execution */
}
