/* reload_test.c - Comprehensive test to trigger multiple reload scenarios */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Force noinline to prevent optimization */
#define NOINLINE __attribute__((noinline))

/* Global variables to increase register pressure */
volatile int global_int = 42;
volatile double global_double = 3.14159;
volatile float global_float = 2.71828f;

/* Complex structure for address calculations */
struct nested {
    int data[8][8];
    double matrix[4][4];
    struct nested *next;
};

NOINLINE int test_reloads(void) {
    /* Declare diverse variables to trigger different reload types */
    int int_var1 = 1, int_var2 = 2, int_var3 = 3;
    long long_var = 0x123456789ABCDEFLL;
    float float_var1 = 1.0f, float_var2 = 2.0f;
    double double_var1 = 1.0, double_var2 = 2.0;
    __m128i vec_var1, vec_var2;
    int *ptr1 = &int_var1;
    double *dptr = &double_var1;
    
    /* Multi-dimensional array for complex addressing */
    int multi_array[16][16];
    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 16; j++)
            multi_array[i][j] = i * 16 + j;
    
    /* Nested structure with pointer chain */
    struct nested nested1, nested2;
    struct nested *nptr = &nested1;
    nested1.next = &nested2;
    nested2.next = &nested1;
    
    /* Initialize vector variables */
    vec_var1 = _mm_set_epi32(1, 2, 3, 4);
    vec_var2 = _mm_set_epi32(5, 6, 7, 8);
    
    int result = 0;
    
    /* ============================================
       BLOCK A: Register Class Conflict Reload
       Force integer to float register reload
       ============================================ */
    {
        int temp_int = global_int + 100;
        double temp_double;
        
        /* This asm requires integer in float register - will trigger reload */
        asm volatile (
            "mov %[in], %%eax\n\t"           /* Move integer to eax */
            "cvtsi2sd %%eax, %[out]\n\t"     /* Convert to double in float reg */
            : [out] "=f" (temp_double)       /* Output in float register */
            : [in] "r" (temp_int)            /* Input in general register */
            : "%eax", "memory"
        );
        
        result += (int)temp_double;
    }
    
    /* ============================================
       BLOCK B: Complex Address Reload
       Multi-dimensional array with complex index
       ============================================ */
    {
        int idx1 = int_var1 * 3 + int_var2;
        int idx2 = int_var2 * 5 - int_var3;
        int temp_val;
        
        /* Complex addressing mode that may need reloading */
        asm volatile (
            "movl %[addr], %[out]\n\t"
            : [out] "=r" (temp_val)
            : [addr] "m" (multi_array[idx1][idx2])  /* Complex address */
            : "memory"
        );
        
        result += temp_val;
    }
    
    /* ============================================
       BLOCK C: Early-Clobber Multiple Outputs
       Two outputs with early clobber modifier
       ============================================ */
    {
        int out1, out2;
        int in1 = int_var1 + 10;
        int in2 = int_var2 + 20;
        int in3 = int_var3 + 30;
        
        /* Early clobber forces separate registers for outputs */
        asm volatile (
            "addl %[a], %[x]\n\t"    /* out1 modified early */
            "subl %[b], %[x]\n\t"
            "imull %[c], %[y]\n\t"   /* out2 also being modified */
            "addl %[x], %[y]\n\t"
            : [x] "=&r" (out1),      /* Early clobber - can't share with inputs */
              [y] "=&r" (out2)       /* Another early clobber */
            : [a] "r" (in1),
              [b] "r" (in2),
              [c] "r" (in3)
            : "cc"
        );
        
        result += out1 + out2;
    }
    
    /* ============================================
       BLOCK D: Secondary Reload Pattern
       Vector operation with immediate constant
       ============================================ */
    {
        __m128i vec_result;
        int imm_constant = 0x7F;
        
        /* Vector operation that might need secondary reload for constant */
        asm volatile (
            "movd %[imm], %%xmm0\n\t"        /* Load immediate to xmm0 */
            "paddb %[vec], %%xmm0\n\t"       /* Add vector */
            "movdqa %%xmm0, %[out]\n\t"      /* Store result */
            : [out] "=x" (vec_result)
            : [vec] "x" (vec_var1),
              [imm] "r" (imm_constant)       /* Constant in general reg */
            : "%xmm0"
        );
        
        /* Extract result to integer */
        int temp_arr[4];
        _mm_storeu_si128((__m128i*)temp_arr, vec_result);
        result += temp_arr[0];
    }
    
    /* ============================================
       BLOCK E: Memory-to-Memory Reload
       Force memory operand reload
       ============================================ */
    {
        double complex_array[8] = {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8};
        double temp_sum = 0.0;
        
        /* Multiple memory operands that might need reloading */
        asm volatile (
            "movsd (%[ptr1]), %%xmm0\n\t"
            "addsd 16(%[ptr1]), %%xmm0\n\t"
            "addsd 32(%[ptr1]), %%xmm0\n\t"
            "addsd 48(%[ptr1]), %%xmm0\n\t"
            "movsd %%xmm0, %[sum]\n\t"
            : [sum] "=m" (temp_sum)
            : [ptr1] "r" (complex_array)
            : "%xmm0", "memory"
        );
        
        result += (int)temp_sum;
    }
    
    /* ============================================
       BLOCK F: Mixed Mode Reloads
       Different machine modes in same asm
       ============================================ */
    {
        int int_result;
        float float_result;
        
        /* Mixed integer and float operations */
        asm volatile (
            "cvtsi2ss %[int_in], %%xmm0\n\t"     /* int to float */
            "mulss %[float_in], %%xmm0\n\t"      /* float multiply */
            "cvtss2si %%xmm0, %[int_out]\n\t"    /* float to int */
            "movss %%xmm0, %[float_out]\n\t"     /* keep float too */
            : [int_out] "=r" (int_result),
              [float_out] "=m" (float_result)
            : [int_in] "r" (int_var1),
              [float_in] "x" (float_var1)
            : "%xmm0", "memory"
        );
        
        result += int_result + (int)float_result;
    }
    
    /* ============================================
       BLOCK G: Pointer Chain with Offset
       Complex structure addressing
       ============================================ */
    {
        int struct_val;
        int offset = int_var1 * 8 + int_var2 * 4;
        
        /* Complex pointer arithmetic */
        asm volatile (
            "movl (%[base], %[idx], 4), %[out]\n\t"
            : [out] "=r" (struct_val)
            : [base] "r" (&nested1.data[0][0]),
              [idx] "r" (offset)
            : "memory"
        );
        
        result += struct_val;
    }
    
    /* ============================================
       BLOCK H: Large Immediate Reload
       64-bit constant that might need reload
       ============================================ */
    {
        long long big_constant = 0x123456789ABCDEF0LL;
        long long temp_ll;
        
        /* 64-bit operation that might need reloading */
        asm volatile (
            "mov %[in], %%rax\n\t"
            "add $0x1111111111111111, %%rax\n\t"
            "mov %%rax, %[out]\n\t"
            : [out] "=r" (temp_ll)
            : [in] "r" (big_constant)
            : "%rax"
        );
        
        result += (int)temp_ll;
    }
    
    /* ============================================
       BLOCK I: Multiple Clobber Reload
       Many clobbered registers
       ============================================ */
    {
        int a = int_var1, b = int_var2, c = int_var3;
        int r1, r2, r3;
        
        /* Clobber many registers to force spills/reloads */
        asm volatile (
            "movl %[a], %%eax\n\t"
            "movl %[b], %%ebx\n\t"
            "movl %[c], %%ecx\n\t"
            "addl %%ebx, %%eax\n\t"
            "imull %%ecx, %%eax\n\t"
            "movl %%eax, %[x]\n\t"
            "movl %%ebx, %[y]\n\t"
            "movl %%ecx, %[z]\n\t"
            : [x] "=r" (r1),
              [y] "=r" (r2),
              [z] "=r" (r3)
            : [a] "r" (a),
              [b] "r" (b),
              [c] "r" (c)
            : "%eax", "%ebx", "%ecx", "memory", "cc"
        );
        
        result += r1 + r2 + r3;
    }
    
    /* ============================================
       BLOCK J: Volatile Memory Access Pattern
       Force memory barrier and reloads
       ============================================ */
    {
        volatile int* volatile_ptr = &int_var1;
        int final_val;
        
        /* Multiple volatile accesses */
        asm volatile (
            "movl (%[ptr]), %%eax\n\t"
            "addl $100, %%eax\n\t"
            "movl %%eax, (%[ptr])\n\t"
            "movl (%[ptr]), %[out]\n\t"
            : [out] "=r" (final_val)
            : [ptr] "r" (volatile_ptr)
            : "%eax", "memory"
        );
        
        result += final_val;
    }
    
    return result;
}

int main(void) {
    int checksum = test_reloads();
    
    /* Use checksum to prevent dead code elimination */
    printf("Reload test checksum: %d\n", checksum);
    
    /* Return non-zero to indicate execution */
    return (checksum != 0) ? 0 : 1;
}
