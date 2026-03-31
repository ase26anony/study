/* reload_test.c - Comprehensive test to trigger multiple reload types */
#include <stdio.h>
#include <stdint.h>
#include <xmmintrin.h>  /* For SSE intrinsics */

/* Force no optimization on specific variables */
#define VOL(var) (*(volatile __typeof__(var)*)&(var))

/* Complex structure to force address computations */
struct nested {
    int a[8];
    double b[4];
    struct nested *next;
};

/* Global variables to increase register pressure */
int global_int = 42;
double global_double = 3.14159;
__m128i global_vec = {0};

int main(void) {
    /* Diverse variable declarations with different types and modes */
    int int_var = 1;
    long long_var = 2;
    float float_var = 3.0f;
    double double_var = 4.0;
    __m128i vec_var = _mm_set_epi32(1, 2, 3, 4);
    int *int_ptr = &int_var;
    double *double_ptr = &double_var;
    
    /* Arrays for complex addressing */
    int multi_array[16][8];
    struct nested nested_array[4];
    struct nested *nested_ptr = &nested_array[0];
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            multi_array[i][j] = i * 8 + j;
        }
    }
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            nested_array[i].a[j] = i * 100 + j;
        }
        for (int j = 0; j < 4; j++) {
            nested_array[i].b[j] = i * 10.0 + j;
        }
        nested_array[i].next = (i < 3) ? &nested_array[i + 1] : NULL;
    }
    
    /* Intermediate variables to force spills */
    int temp1, temp2, temp3;
    double dtemp1, dtemp2;
    __m128i vtemp;
    
    /* ===== BLOCK A: Register Class Conflict ===== */
    /* Force integer to float register reload */
    asm volatile (
        /* Input in integer register, output in floating register */
        "mov %1, %%eax\n\t"
        "cvtsi2ss %%eax, %0\n\t"
        : "=f" (float_var)        /* Output in floating-point register */
        : "r" (int_var)           /* Input in general-purpose register */
        : "%eax", "memory"
    );
    
    /* Use the result to prevent optimization */
    VOL(float_var) = float_var;
    
    /* ===== BLOCK B: Complex Address Reload ===== */
    /* Complex addressing mode that may need reloading */
    int index1 = 5, index2 = 3;
    int complex_addr_result;
    
    asm volatile (
        /* Complex addressing: base + index1*32 + index2*4 */
        "movl %c[array](,%1,32), %%eax\n\t"
        "addl %c[offset](%%eax,%2,4), %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (complex_addr_result)
        : "r" (index1), "r" (index2),
          [array] "i" ((long)multi_array),
          [offset] "i" ((long)&multi_array[0][0])
        : "%eax", "memory"
    );
    
    /* ===== BLOCK C: Early-Clobber Multiple Outputs ===== */
    /* Force early-clobber reloads */
    int out1, out2;
    int in1 = 100, in2 = 200, in3 = 300;
    
    asm volatile (
        /* Early-clobber output overwritten before all inputs consumed */
        "movl %3, %0\n\t"      /* out1 = in1 */
        "imull %4, %0\n\t"     /* out1 *= in2 - CLOBBERS out1 early */
        "movl %0, %1\n\t"      /* out2 = out1 */
        "addl %5, %1\n\t"      /* out2 += in3 */
        : "=&r" (out1), "=&r" (out2)  /* Both early-clobber */
        : "0" (0), "r" (in1), "r" (in2), "r" (in3)
        : "cc"
    );
    
    /* ===== BLOCK D: Secondary Reload Pattern ===== */
    /* Pattern that may require secondary reload on some architectures */
    long long large_const = 0x123456789ABCDEF0LL;
    long long result;
    
    asm volatile (
        /* Moving large constant may need temporary register */
        "movabsq %1, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=r" (result)
        : "i" (large_const)
        : "%rax"
    );
    
    /* ===== BLOCK E: Memory to Memory with Intermediate Register ===== */
    /* Force memory-to-memory move through register */
    double mem_to_mem_result;
    
    asm volatile (
        "movsd %1, %%xmm0\n\t"
        "movsd %%xmm0, %0\n\t"
        : "=m" (mem_to_mem_result)
        : "m" (global_double)
        : "%xmm0", "memory"
    );
    
    /* ===== BLOCK F: Multiple Constraint Alternatives ===== */
    /* Use constraint alternatives to force reload decisions */
    int alt_result;
    int alt_input = 999;
    
    asm volatile (
        "movl %1, %0\n\t"
        "addl $111, %0\n\t"
        : "=r,r,m" (alt_result)  /* Multiple output constraints */
        : "r,m,r" (alt_input)    /* Multiple input constraints */
        : "cc"
    );
    
    /* ===== BLOCK G: Vector Register Pressure ===== */
    /* Force vector register spills and reloads */
    __m128i vec1 = _mm_set_epi32(1, 2, 3, 4);
    __m128i vec2 = _mm_set_epi32(5, 6, 7, 8);
    __m128i vec3, vec4, vec5;
    
    /* Multiple vector operations to increase register pressure */
    asm volatile (
        "movdqa %1, %0\n\t"
        "paddd %2, %0\n\t"
        : "=x" (vec3)
        : "x" (vec1), "x" (vec2)
        : "memory"
    );
    
    asm volatile (
        "movdqa %1, %0\n\t"
        "pslld $2, %0\n\t"
        : "=x" (vec4)
        : "x" (vec3)
        : "memory"
    );
    
    /* ===== BLOCK H: Pointer Chain Addressing ===== */
    /* Complex pointer chain that needs address reload */
    int chain_result;
    struct nested *current = nested_ptr;
    
    for (int i = 0; i < 3; i++) {
        if (current && current->next) {
            asm volatile (
                "movl (%1), %%eax\n\t"
                "addl 32(%1), %%eax\n\t"
                "movl %%eax, %0\n\t"
                : "=r" (chain_result)
                : "r" (&current->a[index1])  /* Complex address */
                : "%eax", "memory"
            );
            current = current->next;
        }
    }
    
    /* ===== BLOCK I: Mixed Mode Operations ===== */
    /* Operations mixing different machine modes */
    int mixed_int;
    double mixed_double;
    
    /* Integer to double conversion requiring mode change */
    asm volatile (
        "cvtsi2sd %1, %0\n\t"
        : "=x" (mixed_double)
        : "r" (int_var)
        : "memory"
    );
    
    /* Double to integer conversion */
    asm volatile (
        "cvttsd2si %1, %0\n\t"
        : "=r" (mixed_int)
        : "x" (mixed_double)
        : "memory"
    );
    
    /* ===== BLOCK J: Volatile Memory Access Pattern ===== */
    /* Force memory reloads with volatile accesses */
    volatile int *vol_ptr = &int_var;
    int volatile_result;
    
    for (int i = 0; i < 10; i++) {
        asm volatile (
            "movl (%1), %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "+r" (volatile_result)
            : "r" (vol_ptr)
            : "%eax", "memory"
        );
    }
    
    /* ===== Compute checksum to prevent optimization ===== */
    unsigned long long checksum = 0;
    
    checksum += int_var;
    checksum += long_var;
    checksum += *(unsigned int*)&float_var;
    checksum += *(unsigned long long*)&double_var;
    checksum += complex_addr_result;
    checksum += out1 + out2;
    checksum += result;
    checksum += *(unsigned long long*)&mem_to_mem_result;
    checksum += alt_result;
    checksum += ((unsigned long long*)&vec3)[0] + ((unsigned long long*)&vec3)[1];
    checksum += chain_result;
    checksum += mixed_int;
    checksum += *(unsigned long long*)&mixed_double;
    checksum += volatile_result;
    
    /* Use all variables to prevent dead code elimination */
    printf("Checksum: %llu\n", checksum);
    
    /* Return checksum as program result */
    return (int)(checksum & 0xFFFFFFFF);
}
