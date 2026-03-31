/* reload_test.c - Comprehensive test to trigger multiple reload types in GCC */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>  /* For vector types */

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
int global_int_matrix[16][16];

NOOPT int main(void) {
    /* Declare diverse variables of different types and sizes */
    int int_var = 1;
    long long_var = 2;
    float float_var = 3.0f;
    double double_var = 4.0;
    long long int64_var = 5;
    __m128i vec_var = _mm_set_epi32(6, 7, 8, 9);
    __m128 vec_float = _mm_set_ps(10.0f, 11.0f, 12.0f, 13.0f);
    
    /* Arrays for complex addressing */
    int array_1d[256];
    double array_2d[32][32];
    struct nested nested_array[8];
    struct nested *nested_ptr = &nested_array[0];
    
    /* Initialize arrays to prevent optimization */
    for (int i = 0; i < 256; i++) array_1d[i] = i;
    for (int i = 0; i < 32; i++)
        for (int j = 0; j < 32; j++)
            array_2d[i][j] = i * 32.0 + j;
    
    /* Initialize nested structure */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) nested_array[i].a[j] = i * 8 + j;
        for (int j = 0; j < 4; j++) nested_array[i].b[j] = i * 4.0 + j;
        nested_array[i].next = &nested_array[(i + 1) % 8];
    }
    
    /* Intermediate variables to force spills and reloads */
    volatile int vol_int = 100;
    volatile double vol_double = 200.0;
    
    /* ===== BLOCK A: Register Class Conflict ===== */
    /* Force integer to float register reload */
    {
        int input = int_var + 10;
        double output;
        
        /* Request floating-point register for integer computation */
        asm volatile (
            "/* Block A: Integer to FP register reload */\n\t"
            "mov %1, %%eax\n\t"
            "cvtsi2sd %%eax, %0\n\t"
            : "=f" (output)      /* Output in FP register */
            : "r" (input)        /* Input in general register */
            : "%eax", "memory"
        );
        
        double_var += output;
        printf("Block A output: %f\n", output);
    }
    
    /* ===== BLOCK B: Complex Address Reload ===== */
    /* Force address computation reload with complex indexing */
    {
        int i = vol_int % 16;
        int j = (vol_int * 3) % 16;
        int k = (vol_int * 5) % 16;
        long long result;
        
        /* Complex multi-dimensional array access */
        asm volatile (
            "/* Block B: Complex address reload */\n\t"
            "movq %1, %%rax\n\t"
            "addq %2, %%rax\n\t"
            "movq (%%rax), %0\n\t"
            : "=r" (result)
            : "r" (&global_int_matrix[i][j]), 
              "r" ((long)(k * sizeof(int)))
            : "%rax", "memory"
        );
        
        int64_var += result;
        printf("Block B result: %lld\n", result);
    }
    
    /* ===== BLOCK C: Early-Clobber Multiple Outputs ===== */
    /* Force reloads due to early clobber */
    {
        int in1 = int_var + 1;
        int in2 = int_var + 2;
        int in3 = int_var + 3;
        int out1, out2, out3;
        
        /* Multiple outputs with early clobber */
        asm volatile (
            "/* Block C: Early-clobber multiple outputs */\n\t"
            "movl %3, %0\n\t"    /* out1 = in1 */
            "imull %4, %0\n\t"   /* out1 *= in2 - EARLY CLOBBER of out1 */
            "movl %5, %1\n\t"    /* out2 = in3 */
            "addl %0, %1\n\t"    /* out2 += out1 */
            "movl %0, %2\n\t"    /* out3 = out1 */
            "subl %1, %2\n\t"    /* out3 -= out2 */
            : "=&r" (out1),      /* Early clobber - written before all inputs read */
              "=&r" (out2),      /* Early clobber */
              "=r" (out3)
            : "r" (in1), "r" (in2), "r" (in3)
            : "cc"
        );
        
        int_var = out1 + out2 + out3;
        printf("Block C outputs: %d, %d, %d\n", out1, out2, out3);
    }
    
    /* ===== BLOCK D: Secondary Reload Pattern ===== */
    /* Force secondary reload through immediate value */
    {
        __m128i vec_input = vec_var;
        __m128i vec_output;
        
        /* Pattern that often requires secondary reloads:
           Moving data between vector and general registers */
        asm volatile (
            "/* Block D: Secondary reload pattern */\n\t"
            "movd %1, %%eax\n\t"      /* Extract low 32 bits to GP register */
            "addl $0x1000, %%eax\n\t" /* Modify in GP register */
            "movd %%eax, %0\n\t"      /* Move back to vector register */
            "pslldq $4, %0\n\t"       /* Shift left in vector reg */
            : "=x" (vec_output)       /* Output in vector register */
            : "x" (vec_input)         /* Input in vector register */
            : "%eax", "memory"
        );
        
        vec_var = vec_output;
        
        /* Another secondary reload pattern: memory to vector with offset */
        double *ptr = &array_2d[8][8];
        __m128d vec_double;
        
        asm volatile (
            "movapd %1, %0\n\t"
            : "=x" (vec_double)
            : "m" (*ptr)
            : "memory"
        );
        
        vec_float = _mm_cvtpd_ps(vec_double);
        printf("Block D executed\n");
    }
    
    /* ===== BLOCK E: Mixed Mode Reloads ===== */
    /* Force reloads with different machine modes */
    {
        char char_var = 127;
        short short_var = 32767;
        int int_result;
        float float_result;
        
        /* Mixed mode operations forcing mode conversions */
        asm volatile (
            "/* Block E: Mixed mode reloads */\n\t"
            "movsbl %1, %%eax\n\t"    /* Sign extend char to int */
            "movswl %2, %%ebx\n\t"    /* Sign extend short to int */
            "addl %%ebx, %%eax\n\t"   /* Add them */
            "movl %%eax, %0\n\t"      /* Store int result */
            : "=r" (int_result)
            : "r" (char_var), "r" (short_var)
            : "%eax", "%ebx", "cc"
        );
        
        /* Float with different precision */
        asm volatile (
            "cvtsi2ssl %1, %0\n\t"    /* Convert int to float */
            : "=x" (float_result)
            : "r" (int_result)
        );
        
        float_var += float_result;
        printf("Block E: int=%d, float=%f\n", int_result, float_result);
    }
    
    /* ===== BLOCK F: High Register Pressure ===== */
    /* Force many simultaneous live values */
    {
        int a = int_var;
        int b = long_var;
        int c = vol_int;
        int d = global_int;
        int e = array_1d[128];
        int f = array_1d[129];
        int g = array_1d[130];
        int h = array_1d[131];
        
        /* Use all variables in one asm to maximize register pressure */
        asm volatile (
            "/* Block F: High register pressure */\n\t"
            "addl %1, %0\n\t"
            "addl %2, %0\n\t"
            "addl %3, %0\n\t"
            "addl %4, %0\n\t"
            "addl %5, %0\n\t"
            "addl %6, %0\n\t"
            "addl %7, %0\n\t"
            : "+r" (a)
            : "r" (b), "r" (c), "r" (d), "r" (e), "r" (f), "r" (g), "r" (h)
            : "cc"
        );
        
        int_var = a;
        printf("Block F result: %d\n", a);
    }
    
    /* ===== BLOCK G: Memory Operand with Complex Addressing ===== */
    /* Force address reload with structure pointer chain */
    {
        double result;
        struct nested *ptr = nested_ptr;
        
        /* Follow pointer chain with offset */
        asm volatile (
            "/* Block G: Complex structure addressing */\n\t"
            "movq %1, %%rax\n\t"           /* Load pointer */
            "movq 64(%%rax), %%rbx\n\t"    /* Load next pointer (offset 64) */
            "movsd 32(%%rbx), %0\n\t"      /* Load double from next->b[0] */
            : "=x" (result)
            : "r" (ptr)
            : "%rax", "%rbx", "memory"
        );
        
        double_var += result;
        printf("Block G: loaded %f\n", result);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = int_var + long_var + (int)float_var + (int)double_var + 
                   (int)int64_var + global_int + vol_int;
    
    /* Use vector results */
    int vec_checksum[4];
    _mm_storeu_si128((__m128i*)vec_checksum, vec_var);
    checksum += vec_checksum[0] + vec_checksum[1] + 
                vec_checksum[2] + vec_checksum[3];
    
    printf("Final checksum: %d\n", checksum);
    
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
