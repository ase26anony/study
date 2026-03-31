/* reload_test.c - Test program to exercise GCC's reload pass */
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
    /* Diverse variable declarations with different types and sizes */
    int i = 10, j = 20, k = 30;
    long long ll1 = 100, ll2 = 200;
    float f1 = 1.5f, f2 = 2.5f;
    double d1 = 3.14, d2 = 6.28;
    __m128i v1 = _mm_set_epi32(1, 2, 3, 4);
    __m128i v2 = _mm_set_epi32(5, 6, 7, 8);
    
    /* Arrays for complex addressing */
    int array1[100] = {0};
    double array2[50][50];
    struct nested nested_array[10];
    struct nested *ptr = &nested_array[0];
    
    /* Initialize arrays */
    for (int idx = 0; idx < 100; idx++) {
        array1[idx] = idx * 2;
    }
    
    for (int x = 0; x < 50; x++) {
        for (int y = 0; y < 50; y++) {
            array2[x][y] = x * 100.0 + y;
        }
    }
    
    /* BLOCK A: Register Class Conflict */
    /* Force integer to float register reload */
    {
        int int_for_float = 12345;
        double float_result;
        
        /* Request float register for integer value */
        asm volatile (
            "mov %1, %%eax\n\t"          /* Load integer into GPR */
            "cvtsi2sd %%eax, %0\n\t"     /* Convert to double in FP register */
            : "=f" (float_result)        /* Output in FP register */
            : "r" (int_for_float)        /* Input in general register */
            : "%eax", "memory"
        );
        
        d1 += float_result;  /* Use result to prevent elimination */
    }
    
    /* BLOCK B: Complex Address Reload */
    /* Multi-dimensional array with complex index computation */
    {
        double complex_load;
        int idx1 = i * 3 + j;
        int idx2 = k * 2 - j;
        
        /* Complex addressing mode that may need reloading */
        asm volatile (
            "movsd (%1), %0\n\t"
            : "=x" (complex_load)        /* Output in XMM register */
            : "r" (&array2[idx1][idx2])  /* Complex address in register */
            : "memory"
        );
        
        d2 += complex_load;
    }
    
    /* BLOCK C: Early-Clobber Multiple Outputs */
    {
        int out1, out2;
        int in1 = i + j;
        int in2 = j + k;
        int in3 = k + i;
        
        /* Early clobber on second output forces separate register */
        asm volatile (
            "mov %2, %0\n\t"     /* out1 = in1 */
            "add %3, %0\n\t"     /* out1 += in2 */
            "mov %4, %1\n\t"     /* out2 = in3 (early clobber!) */
            "imul %0, %1\n\t"    /* out2 *= out1 */
            : "=&r" (out1), "=&r" (out2)  /* Both early-clobber */
            : "r" (in1), "r" (in2), "r" (in3)
            : "cc"
        );
        
        i = out1 + out2;  /* Use outputs */
    }
    
    /* BLOCK D: Secondary Reload Pattern */
    /* Simulate pattern requiring intermediate register */
    {
        __m128i vec_result;
        long long large_constant = 0x123456789ABCDEF0LL;
        
        /* Pattern that might require secondary reload on some archs */
        asm volatile (
            "movq %1, %%rax\n\t"         /* Load 64-bit constant to GPR */
            "movq %%rax, %0\n\t"         /* Move to low 64 bits of vector */
            "pslldq $8, %0\n\t"          /* Shift to high 64 bits */
            "movq %%rax, %%xmm1\n\t"     /* Another copy in vector reg */
            "por %%xmm1, %0\n\t"         /* Combine both halves */
            : "=x" (vec_result)
            : "r" (large_constant)       /* 64-bit immediate might need reload */
            : "%rax", "%xmm1", "cc"
        );
        
        v1 = _mm_add_epi32(v1, vec_result);
    }
    
    /* BLOCK E: Memory-to-Memory with Register Pressure */
    /* Create high register pressure to force spills */
    {
        int r1, r2, r3, r4, r5, r6, r7, r8;
        
        /* Many register operands to increase pressure */
        asm volatile (
            "mov %4, %0\n\t"
            "add %5, %0\n\t"
            "mov %6, %1\n\t"
            "sub %7, %1\n\t"
            "imul %0, %1\n\t"
            "mov %8, %2\n\t"
            "mov %9, %3\n\t"
            "lea (%2,%3,2), %2\n\t"
            : "=&r" (r1), "=&r" (r2), "=&r" (r3), "=r" (r4)
            : "r" (i), "r" (j), "r" (k), "r" (ll1), 
              "r" (array1[10]), "r" (array1[20])
            : "cc"
        );
        
        /* Use all results */
        int sum = r1 + r2 + r3 + r4;
        array1[0] = sum;
    }
    
    /* BLOCK F: Mixed Mode Reloads */
    {
        float float_from_double;
        int int_from_float;
        
        /* Convert double to float (mode change) */
        asm volatile (
            "cvtsd2ss %1, %0\n\t"
            : "=x" (float_from_double)
            : "x" (d1)
        );
        
        /* Convert float to int (another mode change) */
        asm volatile (
            "cvtss2si %1, %0\n\t"
            : "=r" (int_from_float)
            : "x" (float_from_double)
        );
        
        i += int_from_float;
    }
    
    /* BLOCK G: Pointer Chain with Offset */
    /* Complex pointer arithmetic that may need reload */
    {
        double chain_result;
        struct nested *current = ptr;
        int offset = i * sizeof(struct nested) / 2;
        
        /* Pointer chain with offset computation */
        asm volatile (
            "mov (%1), %%rax\n\t"        /* Load current->a[0] */
            "cvtsi2sd %%rax, %0\n\t"     /* Convert to double */
            "addsd 32(%1), %0\n\t"       /* Add current->b[0] */
            : "=x" (chain_result)
            : "r" (current), "r" (offset)
            : "%rax", "memory"
        );
        
        d2 += chain_result;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = i + j + k;
    checksum += (int)ll1 + (int)ll2;
    checksum += (int)f1 + (int)f2;
    checksum += (int)d1 + (int)d2;
    
    /* Access vector elements */
    int *vptr = (int*)&v1;
    for (int idx = 0; idx < 4; idx++) {
        checksum += vptr[idx];
    }
    
    /* Use array to prevent elimination */
    checksum += array1[0] + array1[99];
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
