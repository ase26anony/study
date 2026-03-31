/* reload_test.c - Test program to trigger multiple reload types in GCC */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>

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
    /* Diverse variable declarations with different types/sizes */
    int int_var1 = 1, int_var2 = 2, int_var3 = 3;
    long long_var1 = 1000LL, long_var2 = 2000LL;
    float float_var1 = 1.0f, float_var2 = 2.0f;
    double double_var1 = 1.0, double_var2 = 2.0;
    __m128i vector_var1, vector_var2;
    
    /* Arrays for complex addressing */
    int multi_array[16][32];
    double dbl_array[64];
    struct nested nested_array[8];
    struct nested *ptr_chain = &nested_array[0];
    
    /* Initialize arrays to prevent constant propagation */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 32; j++) {
            multi_array[i][j] = i * 32 + j;
        }
    }
    
    for (int i = 0; i < 64; i++) {
        dbl_array[i] = i * 0.1;
    }
    
    /* BLOCK A: Register class conflict reload */
    /* Force integer to float register reload */
    {
        int temp_int = int_var1 + 5;
        double temp_double;
        
        /* Request floating-point register for integer computation result */
        asm volatile (
            "mov %1, %%eax\n\t"           /* Integer in general-purpose reg */
            "cvtsi2sd %%eax, %%xmm0\n\t"  /* Convert to double in xmm reg */
            "movsd %%xmm0, %0\n\t"        /* Store result */
            : "=m" (temp_double)          /* Memory output */
            : "r" (temp_int)              /* Integer in general-purpose reg */
            : "%eax", "%xmm0", "memory"
        );
        
        double_var1 += temp_double;
    }
    
    /* BLOCK B: Complex address reload with multi-dimensional array */
    {
        int i = int_var1 & 0xF;  /* 0-15 */
        int j = int_var2 & 0x1F; /* 0-31 */
        int result;
        
        /* Complex addressing: base + i*128 + j*4 (not directly addressable) */
        asm volatile (
            "movl %1, %%eax\n\t"
            : "=r" (result)
            : "m" (multi_array[i][j])  /* Complex address computation */
            : "%eax"
        );
        
        int_var3 += result;
    }
    
    /* BLOCK C: Early-clobber multiple outputs */
    {
        int out1, out2;
        int in1 = int_var1, in2 = int_var2, in3 = int_var3;
        
        /* Early-clobber on out2 forces reloads */
        asm volatile (
            "movl %3, %0\n\t"    /* out1 = in1 */
            "addl %4, %0\n\t"    /* out1 += in2 */
            "movl %0, %1\n\t"    /* out2 = out1 (clobbers early) */
            "imull %5, %1\n\t"   /* out2 *= in3 */
            : "=&r" (out1), "=&r" (out2)  /* Both early-clobber */
            : "0" (in1), "r" (in2), "r" (in3)
            : "cc"
        );
        
        int_var1 = out1;
        int_var2 = out2;
    }
    
    /* BLOCK D: Secondary reload pattern - vector to integer */
    {
        __m128i vec = _mm_set_epi32(1, 2, 3, 4);
        int extracted[4];
        
        /* Extract elements - may require secondary reloads on some arches */
        asm volatile (
            "movd %1, %0\n\t"
            "pextrd $1, %1, %2\n\t"
            "pextrd $2, %1, %3\n\t"
            "pextrd $3, %1, %4\n\t"
            : "=r" (extracted[0]), "=r" (extracted[1]), 
              "=r" (extracted[2]), "=r" (extracted[3])
            : "x" (vec)
            : "memory"
        );
        
        for (int k = 0; k < 4; k++) {
            int_var3 += extracted[k];
        }
    }
    
    /* BLOCK E: Memory address reload with structure pointer chain */
    {
        double result;
        
        /* Complex address: nested_array[2].b[1] */
        asm volatile (
            "movsd %1, %%xmm0\n\t"
            "addsd %2, %%xmm0\n\t"
            "movsd %%xmm0, %0\n\t"
            : "=m" (result)
            : "m" (nested_array[2].b[1]), 
              "m" (dbl_array[int_var1 & 0x3F])
            : "%xmm0", "memory"
        );
        
        double_var2 = result;
    }
    
    /* BLOCK F: Mixed mode reloads (different data sizes) */
    {
        char byte_result;
        short short_result;
        int int_result;
        long long ll_result;
        
        /* Different sized operations force different machine modes */
        asm volatile (
            "movb %4, %0\n\t"
            "movw %5, %1\n\t"
            "movl %6, %2\n\t"
            "movq %7, %3\n\t"
            : "=r" (byte_result), "=r" (short_result),
              "=r" (int_result), "=r" (ll_result)
            : "r" ((char)int_var1), "r" ((short)int_var2),
              "r" (int_var3), "r" (long_var1)
            : "memory"
        );
        
        /* Use results to prevent elimination */
        int_var1 += byte_result;
        int_var2 += short_result;
        int_var3 += int_result;
        long_var2 += ll_result;
    }
    
    /* BLOCK G: High register pressure with many clobbers */
    {
        int a = int_var1, b = int_var2, c = int_var3;
        int d, e, f, g, h;
        
        /* Many register operands and clobbers force spill/reload */
        asm volatile (
            "movl %5, %0\n\t"
            "addl %6, %0\n\t"
            "movl %0, %1\n\t"
            "subl %7, %1\n\t"
            "movl %1, %2\n\t"
            "imull %5, %2\n\t"
            "movl %2, %3\n\t"
            "shrl $2, %3\n\t"
            "movl %3, %4\n\t"
            "andl $0xFF, %4\n\t"
            : "=&r" (d), "=&r" (e), "=&r" (f), "=&r" (g), "=&r" (h)
            : "r" (a), "r" (b), "r" (c)
            : "cc"
        );
        
        int_var1 = d + e + f + g + h;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = int_var1 + int_var2 + int_var3;
    checksum += (int)long_var1 + (int)long_var2;
    checksum += (int)double_var1 + (int)double_var2;
    checksum += (int)float_var1 + (int)float_var2;
    
    /* Use global variables to prevent optimization */
    global_int += checksum;
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;
}
