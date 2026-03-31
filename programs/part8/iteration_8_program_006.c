/* reload_test.c - Comprehensive test to trigger various reload scenarios */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>  /* For SSE intrinsics */

/* Force no optimization on specific variables */
#define VOLATILE_VAR volatile

/* Complex structure to force address computations */
struct nested {
    int a[8][8];
    double b[4][4];
    struct nested *next;
};

/* Global variables to increase register pressure */
int global_array[256];
double global_doubles[128];
__m128i global_vec[64];

/* Function to prevent optimization */
static int use_result(int x) {
    VOLATILE_VAR int result = x;
    return result;
}

int main(void) {
    /* Diverse variable declarations with different types and modes */
    int int_var1 = 12345;
    int int_var2 = 67890;
    long long_var = 0x123456789ABCDEFLL;
    float float_var = 3.14159f;
    double double_var = 2.718281828459045;
    __m128i vec_var = _mm_setzero_si128();
    int *int_ptr = &int_var1;
    double *double_ptr = &double_var;
    
    /* Array declarations for complex addressing */
    int multi_array[16][16];
    double dbl_multi[8][8][8];
    struct nested nested_struct;
    struct nested *nested_ptr = &nested_struct;
    
    /* Initialize arrays to prevent constant propagation */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            multi_array[i][j] = i * 100 + j;
        }
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                dbl_multi[i][j][k] = i * 100.0 + j * 10.0 + k;
            }
        }
    }
    
    /* Initialize nested structure */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            nested_struct.a[i][j] = i * 10 + j;
        }
    }
    nested_struct.next = NULL;
    
    int result = 0;
    
    /* ============================================
       BLOCK A: Register Class Conflict Reload
       Force integer to float register reload
       ============================================ */
    {
        double temp_double;
        /* Request floating-point register for integer-derived value */
        asm volatile (
            "/* Block A: Integer to FP register reload */\n\t"
            "mov %[intval], %%eax\n\t"
            "cvtsi2sd %%eax, %[out]\n\t"
            : [out] "=f" (temp_double)      /* Output in FP register */
            : [intval] "rm" (int_var1)      /* Input in reg/memory */
            : "%eax", "memory"
        );
        double_var += temp_double;
    }
    
    /* ============================================
       BLOCK B: Complex Address Reload
       Multi-dimensional array with complex addressing
       ============================================ */
    {
        int temp_result;
        /* Complex addressing mode that may need reloading */
        int idx1 = int_var1 & 0xF;
        int idx2 = int_var2 & 0xF;
        
        asm volatile (
            "/* Block B: Complex address reload */\n\t"
            "movl %[addr], %%eax\n\t"
            "movl (%%eax), %[out]\n\t"
            : [out] "=r" (temp_result)
            : [addr] "X" (&multi_array[idx1][idx2])  /* Complex address */
            : "%eax", "memory"
        );
        result += temp_result;
    }
    
    /* ============================================
       BLOCK C: Early-Clobber Multiple Outputs
       Two outputs with early clobber
       ============================================ */
    {
        int out1, out2;
        int in1 = int_var1;
        int in2 = int_var2;
        int in3 = int_var1 * 2;
        
        asm volatile (
            "/* Block C: Early-clobber multiple outputs */\n\t"
            "movl %[in1], %[out1]\n\t"      /* out1 gets in1 */
            "addl %[in2], %[out1]\n\t"      /* modify out1 */
            "movl %[out1], %[out2]\n\t"     /* out2 gets out1 */
            "imull %[in3], %[out2]\n\t"     /* modify out2 (clobbers early) */
            : [out1] "=&r" (out1), [out2] "=&r" (out2)  /* Early clobber! */
            : [in1] "r" (in1), [in2] "r" (in2), [in3] "r" (in3)
            : "cc"
        );
        result += out1 + out2;
    }
    
    /* ============================================
       BLOCK D: Secondary Reload Pattern
       Vector operation requiring intermediate
       ============================================ */
    {
        __m128i vec_temp;
        long long large_constant = 0x1234567890ABCDEFLL;
        
        /* Pattern that may require secondary reload on some architectures */
        asm volatile (
            "/* Block D: Secondary reload pattern */\n\t"
            "movq %[const], %%rax\n\t"      /* Load constant to GPR */
            "movq %%rax, %[out]\n\t"        /* Move to vector reg (may need reload) */
            : [out] "=x" (vec_temp)         /* Output in SSE register */
            : [const] "rmi" (large_constant) /* Input: reg/mem/immediate */
            : "%rax", "memory"
        );
        vec_var = _mm_add_epi64(vec_var, vec_temp);
    }
    
    /* ============================================
       BLOCK E: Memory-to-Memory with Register Pressure
       Force spill/reload due to register pressure
       ============================================ */
    {
        /* Create register pressure */
        int r1 = int_var1;
        int r2 = int_var2;
        int r3 = r1 + r2;
        int r4 = r1 * r2;
        int r5 = r2 - r1;
        int r6 = r3 + r4;
        int r7 = r4 - r5;
        int r8 = r5 * r6;
        int r9 = r6 + r7;
        int r10 = r7 * r8;
        
        /* Force many variables to be live */
        asm volatile (
            "/* Block E: Register pressure reloads */\n\t"
            "addl %%ebx, %%eax\n\t"
            "addl %%ecx, %%edx\n\t"
            "addl %%esi, %%edi\n\t"
            : 
            : "a" (r1), "b" (r2), "c" (r3), "d" (r4),
              "S" (r5), "D" (r6), "r" (r7), "r" (r8)
            : "cc", "memory"
        );
        
        /* Use results to prevent elimination */
        result += r9 + r10;
    }
    
    /* ============================================
       BLOCK F: Mixed Mode Reloads
       Different machine modes in same asm
       ============================================ */
    {
        float f_result;
        double d_result;
        int i_result;
        
        asm volatile (
            "/* Block F: Mixed mode reloads */\n\t"
            "cvtsi2ssl %[int_in], %[f_out]\n\t"     /* int to float */
            "cvtss2sd %[f_out], %[d_out]\n\t"       /* float to double */
            "cvttsd2si %[d_out], %[i_out]\n\t"      /* double to int */
            : [f_out] "=x" (f_result),
              [d_out] "=x" (d_result),
              [i_out] "=r" (i_result)
            : [int_in] "r" (int_var1)
            : "memory"
        );
        
        result += i_result + (int)f_result;
        double_var += d_result;
    }
    
    /* ============================================
       BLOCK G: Complex Structure Addressing
       Chain of pointer dereferences
       ============================================ */
    {
        int struct_result;
        /* Create complex address computation */
        int idx_i = (int_var1 >> 2) & 7;
        int idx_j = (int_var2 >> 1) & 7;
        
        asm volatile (
            "/* Block G: Complex structure addressing */\n\t"
            "movl %[ptr], %%rax\n\t"
            "movl %[i], %%ebx\n\t"
            "movl %[j], %%ecx\n\t"
            "leaq (%%rax, %%rbx, 8), %%rdx\n\t"     /* base + i*8 */
            "leaq (%%rdx, %%rcx, 4), %%rsi\n\t"     /* + j*4 */
            "movl (%%rsi), %[out]\n\t"              /* load from computed address */
            : [out] "=r" (struct_result)
            : [ptr] "r" (nested_struct.a),
              [i] "r" (idx_i),
              [j] "r" (idx_j)
            : "%rax", "%rbx", "%rcx", "%rdx", "%rsi", "memory"
        );
        
        result += struct_result;
    }
    
    /* ============================================
       BLOCK H: Volatile Memory Access Pattern
       Force memory reloads
       ============================================ */
    {
        VOLATILE_VAR int vol_array[100];
        for (int i = 0; i < 100; i++) {
            vol_array[i] = i * 3;
        }
        
        int sum = 0;
        /* Multiple volatile accesses force reloads */
        for (int i = 0; i < 10; i++) {
            int idx = (i * int_var1) % 100;
            asm volatile (
                "/* Block H: Volatile memory reload */\n\t"
                "movl %[mem], %%eax\n\t"
                "addl %%eax, %[sum]\n\t"
                : [sum] "+r" (sum)
                : [mem] "m" (vol_array[idx])
                : "%eax", "memory"
            );
        }
        result += sum;
    }
    
    /* Final computation to use all variables and prevent dead code elimination */
    result += int_var1 + int_var2 + (int)long_var + (int)float_var + (int)double_var;
    
    /* Extract scalar from vector */
    int vec_elem;
    asm volatile (
        "pextrd $0, %[vec], %[out]\n\t"
        : [out] "=r" (vec_elem)
        : [vec] "x" (vec_var)
    );
    result += vec_elem;
    
    /* Use nested pointer */
    if (nested_ptr->next == NULL) {
        result += nested_ptr->a[0][0];
    }
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", use_result(result));
    
    return use_result(result) & 0xFF;
}
