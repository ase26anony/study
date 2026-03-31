/* reload_test.c - Comprehensive test to trigger various reload scenarios */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Force variables to be in memory to increase reload opportunities */
#define NO_REGISTER __attribute__((noinline))

/* Complex struct to force address computations */
struct nested {
    int a[8];
    double b[4];
    struct nested *next;
};

/* Global variables to force memory reloads */
int global_array[256];
double global_doubles[128];
struct nested global_struct;

/* Function to prevent optimization */
NO_REGISTER int use_value(int x) {
    volatile int dummy = x;
    return dummy;
}

NO_REGISTER double use_double(double x) {
    volatile double dummy = x;
    return dummy;
}

int main(void) {
    /* Diverse variable declarations with different types and modes */
    int int_var1 = 12345;
    int int_var2 = 67890;
    long long ll_var1 = 0x123456789ABCDEF0LL;
    long long ll_var2 = 0xFEDCBA9876543210LL;
    float float_var1 = 3.14159f;
    float float_var2 = 2.71828f;
    double double_var1 = 3.141592653589793;
    double double_var2 = 2.718281828459045;
    
    /* Vector types for SIMD reloads */
    __m128i vec_var1 = _mm_set_epi32(1, 2, 3, 4);
    __m128i vec_var2 = _mm_set_epi32(5, 6, 7, 8);
    __m128 vec_float = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    
    /* Pointers and arrays for complex addressing */
    int array_2d[16][16];
    double *ptr_array[64];
    struct nested local_struct;
    struct nested *struct_ptr = &local_struct;
    
    /* Initialize arrays to prevent constant propagation */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            array_2d[i][j] = i * 16 + j;
        }
    }
    
    for (int i = 0; i < 64; i++) {
        ptr_array[i] = &global_doubles[i % 128];
    }
    
    /* BLOCK A: Register Class Conflict Reloads */
    /* Force integer to float register reload */
    double double_result;
    asm volatile (
        /* Request float register for integer-derived value */
        "mov %1, %%eax\n\t"
        "cvtsi2sd %%eax, %0"
        : "=f" (double_result)      /* Output in floating-point register */
        : "r" (int_var1)            /* Input in general-purpose register */
        : "%eax", "memory"
    );
    
    /* Force float to integer register reload */
    int int_result;
    asm volatile (
        /* Request integer register for float value */
        "cvttsd2si %1, %0"
        : "=r" (int_result)         /* Output in general-purpose register */
        : "f" (double_var1)         /* Input in floating-point register */
        : "memory"
    );
    
    /* BLOCK B: Complex Address Reloads with Different Addressing Modes */
    /* Multi-dimensional array access with complex index computation */
    int idx1 = use_value(5);
    int idx2 = use_value(7);
    int array_value;
    
    /* Complex addressing: base + index1*stride + index2 */
    asm volatile (
        "movl %c[array](,%1,4), %0\n\t"  /* array[idx1*4] */
        "addl %c[offset](%2), %0"        /* + array[idx2] */
        : "=r" (array_value)
        : "r" (idx1), 
          "r" (&array_2d[0][0] + idx2),  /* Force address computation */
          [array] "i" (sizeof(array_2d[0])),
          [offset] "i" (idx2 * sizeof(int))
        : "memory"
    );
    
    /* Pointer chain with displacement */
    double chain_result;
    struct_ptr->next = &global_struct;
    asm volatile (
        "movsd (%[ptr], %[idx], 8), %0"
        : "=x" (chain_result)       /* XMM register */
        : [ptr] "r" (global_doubles),
          [idx] "r" (use_value(16)) /* Force index computation */
        : "memory"
    );
    
    /* BLOCK C: Early-Clobber Multiple Output Reloads */
    int out1, out2, out3;
    int in1 = use_value(100);
    int in2 = use_value(200);
    int in3 = use_value(300);
    
    /* Multiple outputs with early clobber */
    asm volatile (
        "movl %2, %0\n\t"           /* out1 = in1 */
        "imull %3, %0\n\t"          /* out1 *= in2 */
        "movl %0, %1\n\t"           /* out2 = out1 (early clobber!) */
        "addl %4, %1\n\t"           /* out2 += in3 */
        "leal (%0, %1, 2), %3"      /* Reuse in2 as out3, complex addressing */
        : "=&r" (out1),             /* Early clobber output */
          "=&r" (out2),             /* Early clobber output */
          "=r" (out3)               /* Regular output */
        : "r" (in1),                /* Input operand used multiple times */
          "r" (in2),                /* Input operand reused and modified */
          "2" (in3)                 /* Input tied to output out3 */
        : "memory"
    );
    
    /* BLOCK D: Secondary Reload Patterns and Vector Operations */
    /* Vector reload with potential secondary reload */
    __m128i vec_result;
    asm volatile (
        /* Complex vector operation requiring multiple reloads */
        "movdqa %1, %0\n\t"
        "paddd %2, %0\n\t"
        "pslld $3, %0"
        : "=x" (vec_result)         /* XMM register constraint */
        : "xm" (vec_var1),          /* Memory or register, may need reload */
          "xm" (vec_var2)           /* Another vector operand */
        : "memory"
    );
    
    /* 64-bit immediate-like pattern that may need secondary reload */
    long long large_const = 0x123456789ABCDEF0LL;
    long long ll_result;
    asm volatile (
        /* Pattern that might require temporary register for 64-bit constant */
        "mov %1, %0\n\t"
        "xor %2, %0"
        : "=r" (ll_result)
        : "r" (ll_var1),
          "n" (0xFFFFFFFF00000000LL) /* Large constant may need temp register */
        : "memory"
    );
    
    /* Mixed-mode reload: float/double conversion with register pressure */
    double mixed_result;
    asm volatile (
        "cvtps2pd %1, %0\n\t"       /* Convert 4 floats to 2 doubles */
        "haddpd %0, %0"
        : "=x" (mixed_result)
        : "x" (vec_float)           /* XMM register input */
        : "memory"
    );
    
    /* BLOCK E: Memory-to-Memory Reload with Indexed Addressing */
    /* Force reload of complex address into register */
    int complex_idx = use_value(10) * use_value(3) + use_value(2);
    int mem_result;
    asm volatile (
        "movl (%[base], %[idx], 4), %0"
        : "=r" (mem_result)
        : [base] "r" (global_array),
          [idx] "r" (complex_idx)   /* Complex index computation */
        : "memory"
    );
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    checksum += use_value(int_result);
    checksum += use_value(array_value);
    checksum += (int)use_double(double_result);
    checksum += (int)use_double(chain_result);
    checksum += use_value(out1);
    checksum += use_value(out2);
    checksum += use_value(out3);
    checksum += ((int*)&vec_result)[0];
    checksum += (int)(ll_result & 0xFFFFFFFF);
    checksum += (int)use_double(mixed_result);
    checksum += use_value(mem_result);
    
    printf("Reload test checksum: %d\n", checksum);
    return checksum & 0xFF;
}
