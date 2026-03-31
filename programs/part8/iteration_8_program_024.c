/* reload_test.c - Comprehensive test to trigger multiple reload types in GCC */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>  /* For vector types */

/* Force no optimization on specific variables */
#define NO_OPT __attribute__((optimize("O0")))

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

NO_OPT int main(void) {
    /* Declare diverse variables of different types and sizes */
    int int_var = 1;
    long long_var = 2;
    float float_var = 3.0f;
    double double_var = 4.0;
    long long int64_var = 5;
    __m128i vec_var = _mm_set_epi32(6, 7, 8, 9);
    __m128 float_vec = _mm_set_ps(10.0f, 11.0f, 12.0f, 13.0f);
    
    /* Arrays and pointers for complex addressing */
    int array_1d[100];
    double array_2d[10][10];
    struct nested nested_array[5];
    struct nested *nested_ptr = &nested_array[0];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) array_1d[i] = i;
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 10; j++)
            array_2d[i][j] = i * 10.0 + j;
    
    /* Initialize nested structure */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 8; j++) nested_array[i].a[j] = i * 8 + j;
        for (int j = 0; j < 4; j++) nested_array[i].b[j] = i * 4.0 + j;
        nested_array[i].next = (i < 4) ? &nested_array[i+1] : NULL;
    }
    
    /* Volatile to prevent optimization */
    volatile int vol_int = 999;
    volatile double vol_double = 888.888;
    
    /* Intermediate variables for results */
    int out1 = 0, out2 = 0, out3 = 0;
    double out_double = 0.0;
    float out_float = 0.0f;
    long long out_int64 = 0;
    __m128i out_vec;
    
    /* ============================================
       BLOCK A: Register Class Conflict Reload
       Force integer to float register reload
       ============================================ */
    {
        int temp_int = int_var + 100;
        double temp_double;
        
        /* Request floating-point register for integer computation result */
        asm volatile (
            /* Convert integer to double through forced reload */
            "mov %1, %%eax\n\t"
            "cvtsi2sd %%eax, %0\n\t"
            : "=f" (temp_double)      /* Output in floating-point register */
            : "r" (temp_int)          /* Input in general-purpose register */
            : "%eax", "memory"
        );
        
        double_var += temp_double;
        printf("Block A result: %f\n", temp_double);
    }
    
    /* ============================================
       BLOCK B: Complex Address Reload
       Multi-dimensional array with complex index
       ============================================ */
    {
        int i = int_var % 10;
        int j = (int_var * 7) % 10;
        int k = (int_var * 13) % 10;
        double complex_result;
        
        /* Complex addressing: array_2d[i][j] + array_2d[j][k] * 3.0 */
        asm volatile (
            /* Compute first address: &array_2d[i][j] */
            "mov %2, %%rax\n\t"           /* i */
            "imul $80, %%rax, %%rax\n\t"  /* i * 10 * 8 */
            "mov %3, %%rbx\n\t"           /* j */
            "imul $8, %%rbx, %%rbx\n\t"   /* j * 8 */
            "add %%rbx, %%rax\n\t"        /* i*80 + j*8 */
            "lea %4, %%rcx\n\t"           /* base of array_2d */
            "add %%rcx, %%rax\n\t"        /* final address 1 */
            
            /* Compute second address: &array_2d[j][k] */
            "mov %3, %%rdx\n\t"           /* j */
            "imul $80, %%rdx, %%rdx\n\t"  /* j * 10 * 8 */
            "mov %5, %%rbx\n\t"           /* k */
            "imul $8, %%rbx, %%rbx\n\t"   /* k * 8 */
            "add %%rbx, %%rdx\n\t"        /* j*80 + k*8 */
            "add %%rcx, %%rdx\n\t"        /* final address 2 */
            
            /* Load and compute */
            "movsd (%%rax), %%xmm0\n\t"
            "movsd (%%rdx), %%xmm1\n\t"
            "mulsd %6, %%xmm1\n\t"        /* Multiply by 3.0 */
            "addsd %%xmm1, %%xmm0\n\t"
            "movsd %%xmm0, %0\n\t"
            : "=m" (complex_result)       /* Memory output constraint */
            : "0" (complex_result),       /* Same as output */
              "r" (i), "r" (j), "r" (array_2d), "r" (k),
              "X" (3.0)                   /* Constant multiplier */
            : "%rax", "%rbx", "%rcx", "%rdx", "%xmm0", "%xmm1", "memory"
        );
        
        printf("Block B result: %f\n", complex_result);
    }
    
    /* ============================================
       BLOCK C: Early-Clobber Multiple Outputs
       Two outputs with early clobber modifier
       ============================================ */
    {
        int in1 = int_var;
        int in2 = int_var * 2;
        int in3 = int_var * 3;
        int early_out1, early_out2;
        
        /* Early-clobber: out2 is written before all inputs are consumed */
        asm volatile (
            "mov %2, %%eax\n\t"       /* Use in1 */
            "add %3, %%eax\n\t"       /* Add in2 */
            "mov %%eax, %0\n\t"       /* Write to out1 (early) */
            "imul %4, %%eax\n\t"      /* Multiply by in3 */
            "mov %%eax, %1\n\t"       /* Write to out2 */
            : "=r" (early_out1), "=&r" (early_out2)  /* & = early clobber */
            : "r" (in1), "r" (in2), "r" (in3)
            : "%eax", "memory"
        );
        
        out1 = early_out1;
        out2 = early_out2;
        printf("Block C results: %d, %d\n", early_out1, early_out2);
    }
    
    /* ============================================
       BLOCK D: Secondary Reload Pattern
       Simulating instruction needing temporary register
       ============================================ */
    {
        long long large_constant = 0x123456789ABCDEF0LL;
        long long shifted_result;
        
        /* Pattern that might need secondary reload on some architectures */
        asm volatile (
            "mov %1, %%rax\n\t"           /* Load constant */
            "shl $4, %%rax\n\t"           /* Shift left 4 bits */
            "mov %%rax, %0\n\t"           /* Store result */
            : "=r" (shifted_result)
            : "i" (0x123456789ABCDEF0LL)  /* Large immediate */
            : "%rax", "memory"
        );
        
        out_int64 = shifted_result;
        printf("Block D result: %llx\n", shifted_result);
    }
    
    /* ============================================
       BLOCK E: Mixed Mode Reloads
       Different machine modes in same asm
       ============================================ */
    {
        int int_input = 255;
        float float_input = 127.5f;
        double double_output;
        
        /* Mixed integer and float operations forcing mode conversions */
        asm volatile (
            "mov %1, %%eax\n\t"
            "cvtsi2ss %2, %%xmm0\n\t"
            "cvtsi2ss %%eax, %%xmm1\n\t"
            "addss %%xmm1, %%xmm0\n\t"
            "cvtss2sd %%xmm0, %0\n\t"
            : "=f" (double_output)        /* Double output */
            : "r" (int_input),            /* Integer input */
              "f" (float_input)           /* Float input */
            : "%eax", "%xmm0", "%xmm1", "memory"
        );
        
        printf("Block E result: %f\n", double_output);
    }
    
    /* ============================================
       BLOCK F: High Register Pressure
       Many operands to force spill/reload
       ============================================ */
    {
        int a = int_var, b = int_var+1, c = int_var+2, d = int_var+3;
        int e = int_var+4, f = int_var+5, g = int_var+6, h = int_var+7;
        int r1, r2, r3, r4, r5, r6, r7, r8;
        
        /* Many operands to exceed available registers */
        asm volatile (
            "mov %8, %%eax\n\t"
            "add %9, %%eax\n\t"
            "mov %%eax, %0\n\t"
            "mov %10, %%ebx\n\t"
            "add %11, %%ebx\n\t"
            "mov %%ebx, %1\n\t"
            "mov %12, %%ecx\n\t"
            "add %13, %%ecx\n\t"
            "mov %%ecx, %2\n\t"
            "mov %14, %%edx\n\t"
            "add %15, %%edx\n\t"
            "mov %%edx, %3\n\t"
            "imul %0, %1\n\t"
            "imul %2, %3\n\t"
            "add %1, %3\n\t"
            "mov %3, %4\n\t"
            "mov %0, %5\n\t"
            "mov %1, %6\n\t"
            "mov %2, %7\n\t"
            : "=r" (r1), "=r" (r2), "=r" (r3), "=r" (r4),
              "=r" (r5), "=r" (r6), "=r" (r7), "=r" (r8)
            : "r" (a), "r" (b), "r" (c), "r" (d),
              "r" (e), "r" (f), "r" (g), "r" (h)
            : "%eax", "%ebx", "%ecx", "%edx", "memory"
        );
        
        printf("Block F results: %d %d %d %d\n", r1, r2, r3, r4);
    }
    
    /* ============================================
       BLOCK G: Memory-to-Memory with Displacement
       Complex addressing with displacement
       ============================================ */
    {
        int index = (int_var * 17) % 90;
        int offset = 10;
        int mem_result;
        
        /* array_1d[index + offset] + array_1d[index - offset] */
        asm volatile (
            "mov %2, %%eax\n\t"
            "add %3, %%eax\n\t"
            "mov %1, %%ebx\n\t"
            "mov (%4, %%rax, 4), %%ecx\n\t"  /* array_1d[index+offset] */
            "mov %2, %%eax\n\t"
            "sub %3, %%eax\n\t"
            "mov (%4, %%rax, 4), %%edx\n\t"  /* array_1d[index-offset] */
            "add %%edx, %%ecx\n\t"
            "mov %%ecx, %0\n\t"
            : "=m" (mem_result)
            : "m" (mem_result),
              "r" (index), "r" (offset), "r" (array_1d)
            : "%rax", "%rbx", "%rcx", "%rdx", "memory"
        );
        
        printf("Block G result: %d\n", mem_result);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = int_var + long_var + (int)float_var + (int)double_var;
    checksum += (int)(out_int64 & 0xFFFFFFFF);
    checksum += out1 + out2;
    
    /* Use all variables to ensure they're live */
    vol_int = checksum;
    vol_double = (double)checksum;
    
    /* Access all arrays to prevent optimization */
    for (int i = 0; i < 5; i++) {
        checksum += nested_array[i].a[0];
        checksum += (int)nested_array[i].b[0];
    }
    
    printf("Final checksum: %d\n", checksum);
    return checksum;
}
