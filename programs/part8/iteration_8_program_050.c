/* reload_test.c - Comprehensive test to trigger multiple reload types in GCC */
#include <stdio.h>
#include <stdint.h>
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
    /* Diverse variable declarations with different types and modes */
    int int_var = 1;
    long long_var = 2;
    long long longlong_var = 3;
    float float_var = 4.0f;
    double double_var = 5.0;
    __m128i vector_var;
    __m128 float_vector;
    
    /* Arrays for complex addressing */
    int array_1d[100];
    int array_2d[10][10];
    double double_array[50];
    struct nested nested_array[20];
    struct nested *nested_ptr = &nested_array[0];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) array_1d[i] = i;
    for (int i = 0; i < 10; i++) 
        for (int j = 0; j < 10; j++) 
            array_2d[i][j] = i * 10 + j;
    for (int i = 0; i < 50; i++) double_array[i] = i * 0.5;
    
    /* Initialize nested structure */
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 8; j++) nested_array[i].a[j] = i * 8 + j;
        for (int j = 0; j < 4; j++) nested_array[i].b[j] = i * 4 + j;
        nested_array[i].next = (i < 19) ? &nested_array[i + 1] : NULL;
    }
    
    volatile int prevent_opt = 0;
    int result = 0;
    
    /* ============================================================
       BLOCK A: Register Class Conflict Reload
       Force integer to float register reload and vice versa
       ============================================================ */
    {
        double temp_double;
        int temp_int;
        
        /* Integer in floating-point constraint - forces reload */
        asm volatile (
            "mov %1, %%eax\n\t"           /* Use integer in GP register */
            "cvtsi2sd %%eax, %%xmm0\n\t"  /* Convert to double in XMM */
            "movsd %%xmm0, %0\n\t"        /* Store result */
            : "=m" (temp_double)          /* Memory output */
            : "r" (int_var)               /* Integer in general register */
            : "%eax", "%xmm0", "memory"
        );
        
        /* Float in integer constraint - forces another reload */
        asm volatile (
            "movd %1, %%eax\n\t"          /* Move float to integer reg */
            "addl $1, %%eax\n\t"          /* Operate on it */
            "movd %%eax, %0\n\t"          /* Move back */
            : "=r" (temp_int)             /* Integer output */
            : "x" (float_var)             /* Float in XMM register */
            : "%eax"
        );
        
        result += temp_double + temp_int;
        prevent_opt = temp_int;
    }
    
    /* ============================================================
       BLOCK B: Complex Address Reload with Multiple Indexing
       Force address computation reloads
       ============================================================ */
    {
        int addr_result;
        int i = 5, j = 7, k = 3;
        
        /* Complex addressing mode that may need reloading */
        asm volatile (
            "movl %[base], %%eax\n\t"
            "addl %[index1], %%eax\n\t"
            "sall $2, %%eax\n\t"          /* scale by 4 */
            "addl %[index2], %%eax\n\t"
            "movl (%%eax), %[out]\n\t"
            : [out] "=r" (addr_result)
            : [base] "r" (&array_2d[0][0]),
              [index1] "r" (i * 10 + j),  /* Complex index computation */
              [index2] "r" (k * sizeof(int))
            : "%eax", "memory"
        );
        
        /* Even more complex nested structure access */
        double struct_result;
        int idx1 = 2, idx2 = 3;
        
        asm volatile (
            "mov %[ptr], %%rax\n\t"
            "mov %[idx1], %%rbx\n\t"
            "shl $5, %%rbx\n\t"           /* Multiply by structure size */
            "add %%rbx, %%rax\n\t"        /* ptr + idx1 * sizeof(struct) */
            "mov %[idx2], %%rbx\n\t"
            "shl $3, %%rbx\n\t"           /* Multiply by 8 for double */
            "add $16, %%rax\n\t"          /* Skip int array */
            "add %%rbx, %%rax\n\t"        /* Add double index offset */
            "movsd (%%rax), %[out]\n\t"
            : [out] "=x" (struct_result)
            : [ptr] "r" (nested_ptr),
              [idx1] "r" (idx1),
              [idx2] "r" (idx2)
            : "%rax", "%rbx", "memory"
        );
        
        result += addr_result + (int)struct_result;
        prevent_opt = addr_result;
    }
    
    /* ============================================================
       BLOCK C: Early-Clobber Multiple Output Reloads
       Force reloads due to register conflicts
       ============================================================ */
    {
        int out1, out2, out3;
        int in1 = 100, in2 = 200, in3 = 300;
        
        /* Multiple outputs with early clobber */
        asm volatile (
            "movl %[in1], %[out1]\n\t"    /* out1 gets in1 */
            "addl %[in2], %[out1]\n\t"    /* out1 += in2 */
            "movl %[out1], %[out2]\n\t"   /* out2 gets out1 (early!) */
            "imull %[in3], %[out2]\n\t"   /* out2 *= in3 */
            "movl %[out2], %[out3]\n\t"   /* out3 gets out2 */
            "subl %[in1], %[out3]\n\t"    /* out3 -= in1 */
            : [out1] "=&r" (out1),        /* Early clobber! */
              [out2] "=&r" (out2),        /* Early clobber! */
              [out3] "=r" (out3)
            : [in1] "r" (in1),
              [in2] "r" (in2),
              [in3] "r" (in3)
            : /* No explicit clobbers, but early-clobber causes reloads */
        );
        
        /* Another early-clobber with floating point */
        double fout1, fout2;
        double fin1 = 1.5, fin2 = 2.5;
        
        asm volatile (
            "movsd %[fin1], %[fout1]\n\t"
            "mulsd %[fin2], %[fout1]\n\t"
            "movsd %[fout1], %[fout2]\n\t"  /* Early move! */
            "addsd %[fin1], %[fout2]\n\t"
            : [fout1] "=&x" (fout1),        /* Early clobber XMM */
              [fout2] "=x" (fout2)
            : [fin1] "x" (fin1),
              [fin2] "x" (fin2)
        );
        
        result += out1 + out2 + out3 + (int)(fout1 + fout2);
        prevent_opt = out1;
    }
    
    /* ============================================================
       BLOCK D: Secondary Reload Patterns
       Force multi-step reloads
       ============================================================ */
    {
        /* Pattern that often requires secondary reloads:
           Moving between different register files with constraints */
        __m128i vec_result;
        long long large_constant = 0x123456789ABCDEF0LL;
        
        /* This may require loading the constant via GP register first */
        asm volatile (
            "movq %[const], %%rax\n\t"    /* Load 64-bit constant to GP */
            "movq %%rax, %[out]\n\t"      /* Move to vector reg (may need reload) */
            "punpcklqdq %[out], %[out]\n\t" /* Duplicate to full 128-bit */
            : [out] "=x" (vec_result)
            : [const] "r" (large_constant)
            : "%rax"
        );
        
        /* Memory operand with offset that may need reloading */
        int mem_result;
        int offset = 64;
        
        asm volatile (
            "movl %[offset], %%eax\n\t"
            "addq %[base], %%rax\n\t"
            "movl (%%rax), %[out]\n\t"
            : [out] "=r" (mem_result)
            : [base] "r" (array_1d),
              [offset] "r" (offset * sizeof(int))
            : "%rax", "memory"
        );
        
        /* Force spill/reload with many register constraints */
        int r1, r2, r3, r4, r5, r6;
        
        asm volatile (
            "movl $1, %0\n\t"
            "movl $2, %1\n\t"
            "movl $3, %2\n\t"
            "movl $4, %3\n\t"
            "movl $5, %4\n\t"
            "movl $6, %5\n\t"
            : "=r" (r1), "=r" (r2), "=r" (r3),
              "=r" (r4), "=r" (r5), "=r" (r6)
            : /* No inputs */
            : /* Many outputs force register pressure */
        );
        
        result += mem_result + r1 + r2 + r3 + r4 + r5 + r6;
        prevent_opt = mem_result;
    }
    
    /* ============================================================
       BLOCK E: Mixed Mode Reloads (Different Data Sizes)
       ============================================================ */
    {
        char char_var = 'A';
        short short_var = 1234;
        int int_var2 = 5678;
        long long ll_var = 987654321LL;
        
        /* Mixed size operations forcing mode conversions */
        asm volatile (
            "movsbl %[char], %%eax\n\t"   /* Sign extend char to int */
            "addw %[short], %%ax\n\t"     /* Add short to int */
            "addl %[int], %%eax\n\t"      /* Add int */
            "movq %[ll], %%rcx\n\t"       /* Load long long */
            "addq %%rcx, %%rax\n\t"       /* Add to accumulated */
            "movl %%eax, %[out]\n\t"      /* Store result */
            : [out] "=r" (result)
            : [char] "r" (char_var),
              [short] "r" (short_var),
              [int] "r" (int_var2),
              [ll] "r" (ll_var)
            : "%rax", "%rcx"
        );
        
        /* Floating point with different precisions */
        float f1 = 1.234f;
        double d1 = 5.678;
        long double ld1 = 9.012;
        
        asm volatile (
            "cvtss2sd %[f1], %%xmm0\n\t"  /* float to double */
            "addsd %[d1], %%xmm0\n\t"     /* add double */
            "movq %%xmm0, %%rax\n\t"      /* move to integer reg */
            "movq %%rax, %[out]\n\t"      /* store as long long */
            : [out] "=r" (ll_var)
            : [f1] "x" (f1),
              [d1] "x" (d1)
            : "%rax", "%xmm0"
        );
        
        result += (int)ll_var;
        prevent_opt = (int)ll_var;
    }
    
    /* Final result computation to prevent dead code elimination */
    result += prevent_opt;
    
    printf("Result: %d\n", result);
    
    /* Use computed result to affect return value */
    return result & 0xFF;
}
