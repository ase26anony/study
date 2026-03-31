/* reload_test.c - Comprehensive test to trigger various reload scenarios */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>  /* For vector types */

/* Force variables to be in memory to increase reload opportunities */
#define NO_INLINE __attribute__((noinline))

/* Complex structure to force address computations */
struct nested {
    int a[8];
    double b[4];
    struct nested *next;
};

/* Global variables to force memory operations */
int global_array[256];
double global_doubles[128];
struct nested global_structs[16];

NO_INLINE int trigger_reloads(void) {
    /* Declare diverse variables with different types and sizes */
    int int_var = 12345;
    long long_var = 6789012345LL;
    float float_var = 3.14159f;
    double double_var = 2.718281828459045;
    int *int_ptr = &int_var;
    double *double_ptr = &double_var;
    __m128i vec_var = _mm_set_epi32(1, 2, 3, 4);
    int index1 = 7, index2 = 13;
    volatile int result = 0;  /* volatile to prevent optimization */
    
    /* Array with complex indexing */
    int multi_array[32][16];
    for (int i = 0; i < 32; i++)
        for (int j = 0; j < 16; j++)
            multi_array[i][j] = i * 100 + j;
    
    /* Pointer chain */
    struct nested local_struct;
    struct nested *struct_ptr = &local_struct;
    local_struct.next = &global_structs[0];
    
    /* ===== BLOCK A: Register Class Conflict ===== */
    /* Force integer to float register reload */
    {
        int int_input = int_var;
        double float_output;
        
        /* Request floating-point register for integer-derived value */
        asm volatile (
            "/* Register class conflict reload */\n\t"
            "mov %1, %%eax\n\t"
            "cvtsi2sd %%eax, %0\n\t"
            : "=f" (float_output)    /* Output in floating-point register */
            : "r" (int_input)        /* Input in general-purpose register */
            : "%eax", "memory"
        );
        
        double_var += float_output;
    }
    
    /* ===== BLOCK B: Complex Address Reload ===== */
    /* Force address computation reload with complex addressing mode */
    {
        int array_value;
        /* Complex address computation that may need reloading */
        int complex_index = (index1 * 17 + index2 * 3) % 32;
        
        asm volatile (
            "/* Complex address reload */\n\t"
            "movl %1, %0\n\t"
            : "=r" (array_value)
            : "m" (multi_array[complex_index][index2 * 2 % 16])  /* Complex address */
            : "memory"
        );
        
        result += array_value;
    }
    
    /* ===== BLOCK C: Early-Clobber Multiple Outputs ===== */
    /* Force reloads due to early-clobber outputs */
    {
        int out1, out2;
        int in1 = int_var;
        int in2 = long_var;
        int in3 = index1;
        
        asm volatile (
            "/* Early-clobber multiple outputs */\n\t"
            "movl %2, %0\n\t"      /* out1 gets in1 */
            "imull %3, %0\n\t"     /* out1 *= in2 (uses in2) */
            "movl %4, %1\n\t"      /* out2 gets in3 - EARLY CLOBBER! */
            "addl %1, %0\n\t"      /* out1 += out2 */
            : "=&r" (out1), "=&r" (out2)  /* Both early-clobber */
            : "r" (in1), "r" (in2), "r" (in3)
            : "cc"
        );
        
        result += out1 + out2;
    }
    
    /* ===== BLOCK D: Secondary Reload Pattern ===== */
    /* Force secondary reloads through complex constraints */
    {
        __m128i vec_result;
        long long large_constant = 0x123456789ABCDEF0LL;
        
        /* Pattern that may require secondary reload on some architectures */
        asm volatile (
            "/* Secondary reload pattern */\n\t"
            "movq %1, %%rax\n\t"          /* May need GPR reload first */
            "movq %%rax, %0\n\t"          /* Then to vector register */
            : "=v" (vec_result)           /* Vector register constraint */
            : "ri" (large_constant)       /* Register or immediate */
            : "%rax", "memory"
        );
        
        /* Use the result to prevent elimination */
        int_vec[0] = _mm_extract_epi32(vec_result, 0);
    }
    
    /* ===== BLOCK E: Memory-to-Memory with Intermediate Register ===== */
    /* Force reload for memory-to-memory operations */
    {
        double temp1, temp2;
        
        asm volatile (
            "/* Memory-to-memory via register reload */\n\t"
            "movsd %1, %%xmm0\n\t"
            "addsd %2, %%xmm0\n\t"
            "movsd %%xmm0, %0\n\t"
            : "=m" (temp1)                /* Memory output */
            : "m" (double_var), "m" (global_doubles[8])  /* Two memory inputs */
            : "%xmm0", "memory"
        );
        
        double_var = temp1;
    }
    
    /* ===== BLOCK F: Multiple Constraint Alternatives ===== */
    /* Force reload by using constraint that may not be satisfiable directly */
    {
        int alt_result;
        int complex_expr = (int_var * 3 + 7) / 2;
        
        /* "g" constraint allows register or memory, but complex expression
           may force register reload */
        asm volatile (
            "/* Multiple constraint alternatives */\n\t"
            "movl %1, %0\n\t"
            "notl %0\n\t"
            : "=r" (alt_result)
            : "g" (complex_expr * 2 + 1)  /* Complex expression */
            : "cc"
        );
        
        result ^= alt_result;
    }
    
    /* ===== BLOCK G: Volatile asm with Many Clobbers ===== */
    /* Force many register spills/reloads */
    {
        asm volatile (
            "/* Many clobbers forcing reloads */\n\t"
            "movl %0, %%eax\n\t"
            "movl %%eax, %%ebx\n\t"
            "movl %%ebx, %%ecx\n\t"
            "movl %%ecx, %%edx\n\t"
            : 
            : "m" (result)
            : "%eax", "%ebx", "%ecx", "%edx", 
              "%esi", "%edi", "%r8", "%r9", "%r10",
              "%xmm0", "%xmm1", "%xmm2", "%xmm3",
              "memory", "cc"
        );
    }
    
    /* Compute final checksum to prevent dead code elimination */
    result += int_var + (int)long_var + (int)float_var + (int)double_var;
    result += int_ptr != NULL;
    result += (uintptr_t)double_ptr & 0xFF;
    
    /* Use all modified variables */
    for (int i = 0; i < 4; i++) {
        result += ((int*)&vec_var)[i];
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
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            global_structs[i].a[j] = i * 100 + j;
        }
        for (int j = 0; j < 4; j++) {
            global_structs[i].b[j] = i * 10.0 + j;
        }
        global_structs[i].next = (i < 15) ? &global_structs[i + 1] : NULL;
    }
    
    int checksum = trigger_reloads();
    
    printf("Reload test checksum: %d\n", checksum);
    return checksum & 0xFF;  /* Return non-zero to indicate execution */
}
