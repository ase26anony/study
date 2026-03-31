/* reload_test.c - Comprehensive test to trigger multiple reload scenarios */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Force no optimization on specific variables */
#define VOLATILE_VAR(var) volatile var

/* Complex structure to force address computations */
struct nested {
    int a[8][8];
    double b[4][4];
    struct nested *next;
};

/* Global variables to increase register pressure */
volatile int global_counter = 0;
volatile double global_double = 3.14159;
volatile __m128i global_vec;

int main(void) {
    /* 1. Diverse variable declarations with different types and sizes */
    VOLATILE_VAR(int) int_var1 = 12345;
    VOLATILE_VAR(int) int_var2 = 67890;
    VOLATILE_VAR(long long) ll_var1 = 0x123456789ABCDEF0LL;
    VOLATILE_VAR(long long) ll_var2 = 0xFEDCBA9876543210LL;
    VOLATILE_VAR(float) float_var1 = 1.2345f;
    VOLATILE_VAR(float) float_var2 = 6.7890f;
    VOLATILE_VAR(double) double_var1 = 2.718281828459045;
    VOLATILE_VAR(double) double_var2 = 1.414213562373095;
    VOLATILE_VAR(__m128i) vec_var1, vec_var2;
    VOLATILE_VAR(void*) ptr_var;
    
    /* Array with complex indexing */
    int multi_array[16][16];
    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 16; j++)
            multi_array[i][j] = i * 100 + j;
    
    /* Nested structure with pointer chain */
    struct nested nested1, nested2;
    struct nested *nested_ptr = &nested1;
    nested1.next = &nested2;
    nested2.next = &nested1;
    
    /* Initialize vector variables */
    vec_var1 = _mm_set_epi32(1, 2, 3, 4);
    vec_var2 = _mm_set_epi32(5, 6, 7, 8);
    
    /* 2. Series of inline assembly blocks to trigger different reload types */
    
    /* Block A: Register Class Conflict - Force integer to FP register reload */
    printf("Block A: Register class conflict\n");
    {
        int input = int_var1;
        double output;
        
        /* Request floating-point register for integer variable */
        asm volatile (
            "/* FP register constraint for integer */\n\t"
            "mov %1, %%eax\n\t"           /* Load integer into GPR */
            "cvtsi2sd %%eax, %0\n\t"      /* Convert to double in FP register */
            : "=f" (output)               /* Output in FP register */
            : "r" (input)                 /* Input in general register */
            : "%eax", "memory"
        );
        double_var1 = output;
        global_counter++;
    }
    
    /* Block B: Complex Address Reload - Multi-index array access */
    printf("Block B: Complex address reload\n");
    {
        int i = int_var1 & 0xF;
        int j = int_var2 & 0xF;
        int k = (i + j) & 0xF;
        int result;
        
        /* Complex addressing mode that may need reloading */
        asm volatile (
            "/* Complex array addressing */\n\t"
            "movl %1, %%eax\n\t"
            "addl %2, %%eax\n\t"
            "addl %3, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (result)
            : "m" (multi_array[i][j]),    /* Complex address computation */
              "m" (multi_array[j][k]),    /* Another complex address */
              "m" (multi_array[k][i])     /* Third complex address */
            : "%eax", "memory"
        );
        int_var1 = result;
        global_counter++;
    }
    
    /* Block C: Early-Clobber Multiple Outputs */
    printf("Block C: Early-clobber multiple outputs\n");
    {
        int in1 = int_var1;
        int in2 = int_var2;
        int out1, out2;
        long long out3;
        
        /* Multiple outputs with early clobber */
        asm volatile (
            "/* Early-clobber with multiple outputs */\n\t"
            "mov %2, %%eax\n\t"           /* Use input 1 */
            "imul %3, %%eax\n\t"          /* Multiply with input 2 */
            "mov %%eax, %0\n\t"           /* Output 1 (clobbers early) */
            "add $100, %%eax\n\t"
            "mov %%eax, %1\n\t"           /* Output 2 */
            "mov %2, %%rdx\n\t"           /* Reuse input 1 */
            "mov %3, %%rcx\n\t"           /* Reuse input 2 */
            "imul %%rcx, %%rdx\n\t"       /* 64-bit multiply */
            "mov %%rdx, %4\n\t"           /* Output 3 */
            : "=&r" (out1), "=&r" (out2), "=r" (out3)  /* Two early-clobber outputs */
            : "r" (in1), "r" (in2)
            : "%rax", "%rdx", "%rcx", "memory"
        );
        int_var1 = out1;
        int_var2 = out2;
        ll_var1 = out3;
        global_counter++;
    }
    
    /* Block D: Secondary Reload Pattern - Vector to integer transfer */
    printf("Block D: Secondary reload pattern\n");
    {
        __m128i vec_in = vec_var1;
        int64_t int_out1, int_out2;
        
        /* Pattern that often requires secondary reloads on some architectures */
        asm volatile (
            "/* Vector to scalar transfer (may need secondary reload) */\n\t"
            "movq %1, %0\n\t"             /* Extract low 64 bits */
            "pextrq $1, %1, %2\n\t"       /* Extract high 64 bits */
            : "=r" (int_out1), "=r" (int_out2)
            : "x" (vec_in)                /* Vector in XMM register */
            : "memory"
        );
        ll_var1 = int_out1;
        ll_var2 = int_out2;
        global_counter++;
    }
    
    /* Block E: Mixed Mode Reloads - Different data sizes */
    printf("Block E: Mixed mode reloads\n");
    {
        char char_var = 65;
        short short_var = 32000;
        int int_var = int_var1;
        float float_var = float_var1;
        double double_var = double_var1;
        long double ld_var = 3.14159265358979323846L;
        
        /* Mixed constraints forcing different reload modes */
        asm volatile (
            "/* Mixed mode operations */\n\t"
            "movsx %1, %%eax\n\t"         /* Sign extend char */
            "addw %2, %%ax\n\t"           /* Add short */
            "addl %3, %%eax\n\t"          /* Add int */
            "cvtsi2ss %%eax, %%xmm0\n\t"  /* Convert to float */
            "addss %4, %%xmm0\n\t"        /* Add float */
            "cvtss2sd %%xmm0, %%xmm1\n\t" /* Convert to double */
            "addsd %5, %%xmm1\n\t"        /* Add double */
            "movsd %%xmm1, %0\n\t"        /* Store result */
            : "=m" (double_var)           /* Memory output */
            : "r" (char_var), "r" (short_var), "r" (int_var),
              "x" (float_var), "x" (double_var)
            : "%eax", "%xmm0", "%xmm1", "memory"
        );
        double_var2 = double_var;
        global_counter++;
    }
    
    /* Block F: High Register Pressure - Many live variables */
    printf("Block F: High register pressure\n");
    {
        /* Use many variables to force spills and reloads */
        int r1 = int_var1 + 1;
        int r2 = int_var2 + 2;
        long long r3 = ll_var1 + 3;
        long long r4 = ll_var2 + 4;
        float r5 = float_var1 + 5.0f;
        float r6 = float_var2 + 6.0f;
        double r7 = double_var1 + 7.0;
        double r8 = double_var2 + 8.0;
        
        /* Complex computation with many operands */
        asm volatile (
            "/* High register pressure computation */\n\t"
            "mov %1, %%eax\n\t"
            "add %2, %%eax\n\t"
            "mov %3, %%rbx\n\t"
            "add %4, %%rbx\n\t"
            "cvtsi2ss %%eax, %%xmm0\n\t"
            "cvtsi2sd %%rbx, %%xmm1\n\t"
            "addss %5, %%xmm0\n\t"
            "addss %6, %%xmm0\n\t"
            "addsd %7, %%xmm1\n\t"
            "addsd %8, %%xmm1\n\t"
            "cvtss2sd %%xmm0, %%xmm2\n\t"
            "addsd %%xmm1, %%xmm2\n\t"
            "movsd %%xmm2, %0\n\t"
            : "=m" (r7)
            : "r" (r1), "r" (r2), "r" (r3), "r" (r4),
              "x" (r5), "x" (r6), "x" (r7), "x" (r8)
            : "%rax", "%rbx", "%xmm0", "%xmm1", "%xmm2", "memory"
        );
        double_var1 = r7;
        global_counter++;
    }
    
    /* Block G: Structure Pointer Chain - Complex memory addressing */
    printf("Block G: Structure pointer chain\n");
    {
        int idx1 = (int_var1 >> 2) & 0x7;
        int idx2 = (int_var2 >> 1) & 0x7;
        int idx3 = (int_var1 + int_var2) & 0x3;
        int result;
        
        /* Complex structure access with pointer chasing */
        asm volatile (
            "/* Structure pointer chain access */\n\t"
            "mov %1, %%rax\n\t"           /* Load nested pointer */
            "mov %2, %%rcx\n\t"           /* Load index 1 */
            "mov %3, %%rdx\n\t"           /* Load index 2 */
            "mov %4, %%rsi\n\t"           /* Load index 3 */
            "mov (%%rax), %%rax\n\t"      /* Follow next pointer */
            "imul $512, %%rcx, %%rcx\n\t" /* Calculate offset */
            "imul $8, %%rdx, %%rdx\n\t"
            "add %%rdx, %%rcx\n\t"
            "add %%rsi, %%rcx\n\t"
            "mov (%%rax, %%rcx, 4), %%eax\n\t" /* Array access */
            "mov %%eax, %0\n\t"
            : "=r" (result)
            : "r" (nested_ptr), "r" (idx1), "r" (idx2), "r" (idx3)
            : "%rax", "%rcx", "%rdx", "%rsi", "memory"
        );
        int_var2 = result;
        global_counter++;
    }
    
    /* 3. Compute checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    checksum += int_var1;
    checksum += int_var2;
    checksum += ll_var1;
    checksum += ll_var2;
    checksum += *(uint32_t*)&float_var1;
    checksum += *(uint32_t*)&float_var2;
    checksum += *(uint64_t*)&double_var1;
    checksum += *(uint64_t*)&double_var2;
    checksum += global_counter;
    
    /* Use checksum in a way that can't be optimized away */
    asm volatile (
        "/* Use checksum */\n\t"
        "add %0, %0\n\t"
        : "+r" (checksum)
        :
        : "cc"
    );
    
    printf("Final checksum: %llu\n", (unsigned long long)checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
