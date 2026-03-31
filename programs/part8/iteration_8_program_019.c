/* reload_test.c - Comprehensive test to trigger various reload scenarios */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>

/* Force no optimization on specific variables */
#define VOL(var) asm volatile("" : "+r"(var))

/* Complex structure to force address computations */
struct nested {
    int a[8];
    double b[4];
    struct nested *next;
};

/* Global variables to force memory reloads */
int global_array[256];
double global_doubles[128];
__m128i global_vecs[64];

int main(void) {
    /* Diverse variable declarations with different types and modes */
    int int_var = 12345;
    long long_var = 6789012345LL;
    float float_var = 3.14159f;
    double double_var = 2.718281828459045;
    __m128i vec_var = _mm_setzero_si128();
    
    /* Arrays for complex addressing */
    int multi_array[16][32];
    struct nested nested_array[8];
    struct nested *nested_ptr = &nested_array[0];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 32; j++) {
            multi_array[i][j] = i * 100 + j;
        }
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            nested_array[i].a[j] = i * 10 + j;
        }
        for (int j = 0; j < 4; j++) {
            nested_array[i].b[j] = i * 1.5 + j * 0.25;
        }
        nested_array[i].next = &nested_array[(i + 1) % 8];
    }
    
    /* Intermediate variables to force register pressure */
    int temp1, temp2, temp3, temp4;
    double dtemp1, dtemp2;
    long ltemp1, ltemp2;
    
    /* BLOCK A: Register Class Conflict Reload */
    /* Force integer to float register reload */
    asm volatile (
        /* Input in integer register, output in floating-point register */
        "mov %1, %%eax\n\t"
        "cvtsi2ssl %%eax, %0\n\t"
        : "=f" (float_var)      /* Output in FP register */
        : "r" (int_var)         /* Input in general register */
        : "%eax", "memory"
    );
    VOL(float_var);
    
    /* BLOCK B: Complex Address Reload with Multiple Indexing */
    /* Complex array addressing that may not fit in one addressing mode */
    int index1 = 5, index2 = 17;
    asm volatile (
        /* Complex memory operand - compiler may need to reload address */
        "addl $1, %0\n\t"
        : "+m" (multi_array[index1 * 2 + 3][index2 * 3 - 7])
        : 
        : "cc"
    );
    
    /* Even more complex nested structure addressing */
    int struct_index = 3;
    int array_index = 5;
    asm volatile (
        "addl $2, %0\n\t"
        : "+m" (nested_array[struct_index].next->a[array_index * 2])
        :
        : "cc"
    );
    
    /* BLOCK C: Early-Clobber Multiple Output Reloads */
    /* Force early-clobber conflicts */
    int in1 = 100, in2 = 200, in3 = 300;
    int out1, out2, out3;
    
    asm volatile (
        /* out2 is early-clobber, written before all inputs are read */
        "mov %2, %0\n\t"        /* out1 = in1 */
        "imul %3, %0\n\t"       /* out1 *= in2 */
        "mov %0, %1\n\t"        /* out2 = out1 (early clobber!) */
        "add %4, %1\n\t"        /* out2 += in3 */
        "lea (%1,%2,2), %0\n\t" /* out1 = out2 + in1*2 */
        : "=&r" (out1), "=&r" (out2), "=r" (out3)  /* Two early-clobber outputs */
        : "r" (in1), "r" (in2), "r" (in3)
        : "cc"
    );
    VOL(out1); VOL(out2); VOL(out3);
    
    /* BLOCK D: Secondary Reload Patterns */
    /* Force secondary reloads through complex constraints */
    long long large_const = 0x123456789ABCDEF0LL;
    long long result;
    
    /* Pattern that might require secondary reload on some architectures */
    asm volatile (
        /* Complex operation requiring multiple steps */
        "mov %1, %%rax\n\t"
        "shr $32, %%rax\n\t"
        "imul %1, %%rax\n\t"
        "mov %%rax, %0\n\t"
        : "=r" (result)
        : "ri" (large_const)    /* May force reload if constant doesn't fit */
        : "%rax", "cc", "memory"
    );
    VOL(result);
    
    /* Mixed mode reloads */
    double double_result;
    asm volatile (
        /* Convert through integer intermediate (potential secondary reload) */
        "mov %1, %%eax\n\t"
        "cvtsi2sdq %%rax, %0\n\t"
        : "=x" (double_result)  /* SSE register */
        : "rm" (int_var)        /* Register or memory */
        : "%rax", "memory"
    );
    VOL(double_result);
    
    /* BLOCK E: Vector/SIMD Reloads with Complex Addressing */
    /* Vector load with complex address computation */
    __m128i vec_result;
    int vec_index = 12;
    
    asm volatile (
        "movdqu %1, %0\n\t"
        : "=x" (vec_result)
        : "m" (global_vecs[vec_index * 3 + 1])
        : "memory"
    );
    
    /* Vector operation with multiple constraints */
    asm volatile (
        "paddq %1, %0\n\t"
        : "+x" (vec_result)
        : "xm" (vec_var)        /* SSE register or memory */
        : "cc"
    );
    VOL(vec_result);
    
    /* BLOCK F: High Register Pressure to Force Spills and Reloads */
    /* Use many variables in one asm to increase register pressure */
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5, r6 = 6, r7 = 7, r8 = 8;
    int r9 = 9, r10 = 10, r11 = 11, r12 = 12, r13 = 13, r14 = 14, r15 = 15;
    
    asm volatile (
        /* Use all variables in complex computation */
        "add %1, %0\n\t"
        "imul %2, %0\n\t"
        "sub %3, %0\n\t"
        "add %4, %0\n\t"
        "imul %5, %0\n\t"
        "sub %6, %0\n\t"
        "add %7, %0\n\t"
        "imul %8, %0\n\t"
        "sub %9, %0\n\t"
        "add %10, %0\n\t"
        "imul %11, %0\n\t"
        "sub %12, %0\n\t"
        "add %13, %0\n\t"
        "imul %14, %0\n\t"
        : "+r" (r1)
        : "r" (r2), "r" (r3), "r" (r4), "r" (r5), 
          "r" (r6), "r" (r7), "r" (r8), "r" (r9),
          "r" (r10), "r" (r11), "r" (r12), "r" (r13),
          "r" (r14), "r" (r15)
        : "cc", "memory"
    );
    VOL(r1);
    
    /* BLOCK G: Memory-to-Memory Reload Pattern */
    /* Force memory operand reload with displacement */
    asm volatile (
        "movl $999, %0\n\t"
        : "=m" (global_array[128 + int_var % 64])  /* Complex displacement */
        :
        : "memory"
    );
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    checksum += int_var;
    checksum += long_var;
    checksum += *(unsigned int*)&float_var;
    checksum += *(unsigned long long*)&double_var;
    checksum += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
    checksum += out1 + out2 + out3;
    checksum += result;
    checksum += *(unsigned long long*)&double_result;
    
    /* Add array checksums */
    for (int i = 0; i < 8; i++) {
        checksum += nested_array[i].a[0];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return (int)(checksum % 1000);
}
