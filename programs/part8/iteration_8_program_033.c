/* reload_test.c - Comprehensive test to trigger various reload scenarios */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>  /* For vector types */

/* Force noinline to prevent optimization */
#define NOINLINE __attribute__((noinline))

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

NOINLINE int test_reloads(void) {
    /* Diverse variable types for different machine modes */
    int int_var = 12345;
    long long ll_var = 0x123456789ABCDEF0LL;
    float float_var = 3.14159f;
    double double_var = 2.718281828459045;
    __m128i vec_var = _mm_set_epi32(1, 2, 3, 4);
    int *int_ptr = &int_var;
    double *double_ptr = &double_var;
    
    /* Arrays for complex addressing */
    int multi_array[16][32];
    double dbl_multi[8][16];
    
    /* Struct with pointer chain */
    struct nested local_struct;
    struct nested *struct_ptr = &local_struct;
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 32; j++) {
            multi_array[i][j] = i * 100 + j;
        }
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            dbl_multi[i][j] = i * 10.0 + j * 0.1;
        }
    }
    
    /* Initialize struct */
    memset(&local_struct, 0, sizeof(local_struct));
    for (int i = 0; i < 8; i++) {
        local_struct.a[i] = i * 1000;
    }
    local_struct.next = &global_struct;
    
    volatile int result = 0;  /* Prevent optimization */
    
    /* ===== BLOCK A: Register Class Conflict ===== */
    /* Force integer to float register reload */
    {
        int temp_int = int_var;
        double temp_dbl;
        
        /* Request float register for integer value */
        asm volatile (
            "mov %1, %%eax\n\t"           /* Load integer into eax */
            "cvtsi2sd %%eax, %0\n\t"      /* Convert to double in float reg */
            : "=f" (temp_dbl)             /* Output in floating-point reg */
            : "r" (temp_int)              /* Input in general-purpose reg */
            : "%eax", "memory"
        );
        
        double_var += temp_dbl;
        result += (int)temp_dbl;
    }
    
    /* ===== BLOCK B: Complex Address Reload ===== */
    /* Force address computation reload with complex indexing */
    {
        int idx1 = 5, idx2 = 10;
        int temp_val;
        
        /* Complex addressing mode that may need reloading */
        asm volatile (
            "movl (%[addr]), %0\n\t"
            : "=r" (temp_val)
            : [addr] "r" (&multi_array[idx1 * 2 + 1][idx2 * 3 - 2])
            : "memory"
        );
        
        /* Even more complex address with multiple computations */
        double complex_dbl;
        asm volatile (
            "movsd (%[addr]), %0\n\t"
            : "=x" (complex_dbl)
            : [addr] "r" (&dbl_multi[idx1][idx2] + global_array[idx2])
            : "memory"
        );
        
        result += temp_val + (int)complex_dbl;
    }
    
    /* ===== BLOCK C: Early-Clobber Multiple Outputs ===== */
    /* Force reloads due to early-clobber constraints */
    {
        int in1 = 100, in2 = 200, in3 = 300;
        int out1, out2, out3;
        
        /* Multiple outputs with early-clobber on one */
        asm volatile (
            "mov %2, %0\n\t"              /* out1 = in1 */
            "add %3, %0\n\t"              /* out1 += in2 */
            "mov %0, %1\n\t"              /* out2 = out1 (early clobber!) */
            "imul %4, %1\n\t"             /* out2 *= in3 */
            "lea (%0,%1,2), %0\n\t"       /* out1 = out1 + 2*out2 */
            : "=&r" (out1), "=&r" (out2), "=r" (out3)
            : "r" (in1), "r" (in2), "r" (in3)
            : "cc"
        );
        
        result += out1 + out2 + out3;
    }
    
    /* ===== BLOCK D: Secondary Reload Patterns ===== */
    /* Force secondary reloads through complex constraints */
    {
        long long large_const = 0x1234567890ABCDEFLL;
        __m128i vec_result;
        
        /* Pattern that often requires secondary reloads:
           Moving 64-bit constant to vector register */
        asm volatile (
            "movq %1, %0\n\t"
            "punpcklqdq %0, %0\n\t"
            : "=x" (vec_result)
            : "r" (large_const)  /* May need GPR intermediate */
            : "memory"
        );
        
        /* Another pattern: memory operand requiring index register */
        int index_reg_val;
        asm volatile (
            "movl (%[base],%[index],4), %0\n\t"
            : "=r" (index_reg_val)
            : [base] "r" (global_array),
              [index] "r" (int_var & 0xFF)
            : "memory"
        );
        
        result += index_reg_val;
    }
    
    /* ===== BLOCK E: Mixed Mode Reloads ===== */
    /* Force reloads with different machine modes */
    {
        /* SImode (32-bit) */
        int si_val;
        asm volatile ("movl %1, %0" : "=r" (si_val) : "r" (int_var));
        
        /* DImode (64-bit) */
        long long di_val;
        asm volatile ("movq %1, %0" : "=r" (di_val) : "r" (ll_var));
        
        /* SFmode (float) */
        float sf_val;
        asm volatile ("movss %1, %0" : "=x" (sf_val) : "x" (float_var));
        
        /* DFmode (double) */
        double df_val;
        asm volatile ("movsd %1, %0" : "=x" (df_val) : "x" (double_var));
        
        result += si_val + (int)di_val + (int)sf_val + (int)df_val;
    }
    
    /* ===== BLOCK F: Memory Spills ===== */
    /* Force register pressure to cause spills */
    {
        /* Use many variables to increase register pressure */
        int r1 = result, r2 = r1 * 2, r3 = r2 * 3, r4 = r3 * 4;
        int r5 = r4 * 5, r6 = r5 * 6, r7 = r6 * 7, r8 = r7 * 8;
        
        /* Complex computation forcing spills */
        asm volatile (
            "mov %1, %%eax\n\t"
            "add %2, %%eax\n\t"
            "imul %3, %%eax\n\t"
            "add %4, %%eax\n\t"
            "sub %5, %%eax\n\t"
            "add %6, %%eax\n\t"
            "imul %7, %%eax\n\t"
            "add %8, %%eax\n\t"
            "mov %%eax, %0\n\t"
            : "=r" (result)
            : "r" (r1), "r" (r2), "r" (r3), "r" (r4),
              "r" (r5), "r" (r6), "r" (r7), "r" (r8)
            : "%eax", "cc"
        );
    }
    
    /* ===== BLOCK G: Pointer Chain Reloads ===== */
    /* Force reloads through pointer chasing */
    {
        int chain_result = 0;
        struct nested *current = struct_ptr;
        
        /* Complex pointer chain access */
        asm volatile (
            "mov (%[ptr]), %%eax\n\t"          /* current->a[0] */
            "add 4(%[ptr]), %%eax\n\t"         /* current->a[1] */
            "mov 8(%[ptr]), %%ecx\n\t"         /* current->a[2] */
            "add %%ecx, %%eax\n\t"
            "mov 64(%[ptr]), %%ecx\n\t"        /* current->b[0] as int */
            "cvtsd2si %%xmm0, %%ecx\n\t"       /* Use xmm register */
            "add %%ecx, %%eax\n\t"
            "mov %[result], %%ecx\n\t"
            "add %%eax, %%ecx\n\t"
            "mov %%ecx, %[result]\n\t"
            : [result] "+r" (chain_result)
            : [ptr] "r" (current)
            : "%eax", "%ecx", "%xmm0", "memory"
        );
        
        result += chain_result;
    }
    
    return result;
}

int main(void) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    for (int i = 0; i < 128; i++) {
        global_doubles[i] = i * 1.5;
    }
    
    memset(&global_struct, 0, sizeof(global_struct));
    global_struct.next = NULL;
    
    /* Run the test multiple times to ensure execution */
    int total = 0;
    for (int i = 0; i < 3; i++) {
        total += test_reloads();
    }
    
    printf("Result: %d\n", total);
    return total & 0xFF;  /* Return non-zero to prevent optimization */
}
