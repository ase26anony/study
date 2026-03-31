/* reload_test.c - Comprehensive test to trigger multiple reload types in GCC */

#include <stdint.h>
#include <stdio.h>
#include <xmmintrin.h>  /* For vector types */

/* Force noinline to prevent optimization */
#define NOINLINE __attribute__((noinline))

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
struct nested global_struct[4];

NOINLINE int trigger_reloads(void) {
    /* Diverse variable types to force different machine modes */
    int int_var = 123;
    long long_var = 456LL;
    float float_var = 1.5f;
    double double_var = 2.71828;
    int *int_ptr = &int_var;
    double *double_ptr = &double_var;
    
    /* Arrays for complex addressing */
    int multi_array[16][8];
    double dbl_array[32];
    float float_multi[4][8][4];
    
    /* Vector types */
    __m128i vec_var;
    __m128 float_vec;
    
    /* Volatile to prevent optimization */
    volatile int result = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            multi_array[i][j] = i * 8 + j;
        }
    }
    
    for (int i = 0; i < 32; i++) {
        dbl_array[i] = i * 0.1;
    }
    
    /* ============================================
       BLOCK A: Register Class Conflict Reload
       ============================================ */
    {
        int temp_int = 999;
        double temp_double = 1.234;
        
        /* Force integer into floating-point register */
        asm volatile (
            /* Output in FP register, input in general register */
            "movq %1, %%xmm0\n\t"      /* Move integer to FP reg */
            "movq %%xmm0, %0\n\t"      /* Move back */
            : "=f" (temp_double)       /* FP register output */
            : "r" (temp_int)           /* General register input */
            : "%xmm0"
        );
        
        /* Use both to keep them live */
        result += (int)temp_double;
    }
    
    /* ============================================
       BLOCK B: Complex Address Reload
       ============================================ */
    {
        int i = 3, j = 5, k = 2;
        int addr_result;
        
        /* Complex array addressing that may need reload */
        asm volatile (
            "movl %[complex_addr], %[out]\n\t"
            : [out] "=r" (addr_result)
            : [complex_addr] "m" (float_multi[i][j*2][k+1])
            : "memory"
        );
        
        /* More complex: pointer chain with computation */
        struct nested *ptr = &global_struct[0];
        ptr->next = &global_struct[1];
        ptr->next->next = &global_struct[2];
        
        int chain_result;
        asm volatile (
            "movl (%[ptr], %[idx], 8), %[out]\n\t"
            : [out] "=r" (chain_result)
            : [ptr] "r" (ptr->next->a), 
              [idx] "r" (i * 2 + j)
            : "memory"
        );
        
        result += addr_result + chain_result;
    }
    
    /* ============================================
       BLOCK C: Early-Clobber Multiple Outputs
       ============================================ */
    {
        int in1 = 100, in2 = 200, in3 = 300;
        int out1, out2, out3;
        
        /* Early clobber on second output */
        asm volatile (
            "movl %2, %0\n\t"          /* out1 = in1 */
            "addl %3, %0\n\t"          /* out1 += in2 */
            "movl %0, %1\n\t"          /* out2 = out1 (early clobber!) */
            "subl %4, %1\n\t"          /* out2 -= in3 */
            "movl %1, %0\n\t"          /* out1 = out2 */
            : "=&r" (out1), "=&r" (out2), "=r" (out3)
            : "r" (in1), "r" (in2), "r" (in3)
            : "cc"
        );
        
        /* Another with mixed types */
        double dbl_out1, dbl_out2;
        int int_in = 500;
        
        asm volatile (
            "cvtsi2sd %2, %%xmm0\n\t"
            "movsd %%xmm0, %0\n\t"
            "addsd %3, %0\n\t"
            "movsd %0, %1\n\t"
            : "=&f" (dbl_out1), "=f" (dbl_out2)
            : "r" (int_in), "f" (double_var)
            : "%xmm0"
        );
        
        result += out1 + out2 + (int)dbl_out1;
    }
    
    /* ============================================
       BLOCK D: Secondary Reload Patterns
       ============================================ */
    {
        /* Pattern 1: Large immediate that may need temp register */
        long long large_val = 0x123456789ABCDEF0LL;
        long long ll_result;
        
        asm volatile (
            "movq %1, %0\n\t"
            "addq $0x7FFFFFFFFFFFFFFF, %0\n\t"  /* Large immediate */
            : "=r" (ll_result)
            : "r" (large_val)
            : "cc"
        );
        
        /* Pattern 2: Vector load with complex address */
        __m128i vec_load;
        int *vec_src = &multi_array[0][0];
        
        asm volatile (
            "movdqu (%1, %2, 4), %0\n\t"
            : "=x" (vec_load)
            : "r" (vec_src), "r" (int_var)
            : "memory"
        );
        
        /* Pattern 3: Mixed-size operations requiring reload */
        char char_array[64];
        for (int i = 0; i < 64; i++) char_array[i] = i;
        
        int char_sum = 0;
        asm volatile (
            "movsbl (%1, %2, 1), %0\n\t"
            : "=r" (char_sum)
            : "r" (char_array), "r" (int_var & 63)
            : "memory"
        );
        
        result += (int)ll_result + char_sum;
    }
    
    /* ============================================
       BLOCK E: Memory-to-Memory with Register Pressure
       ============================================ */
    {
        /* Create register pressure with many live variables */
        int r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5, r6 = 6, r7 = 7, r8 = 8;
        int r9 = 9, r10 = 10, r11 = 11, r12 = 12, r13 = 13, r14 = 14, r15 = 15;
        
        /* Force spills and reloads */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %2, %%eax\n\t"
            "addl %3, %%eax\n\t"
            "addl %4, %%eax\n\t"
            "addl %5, %%eax\n\t"
            "addl %6, %%eax\n\t"
            "addl %7, %%eax\n\t"
            "addl %8, %%eax\n\t"
            "addl %9, %%eax\n\t"
            "addl %10, %%eax\n\t"
            "addl %11, %%eax\n\t"
            "addl %12, %%eax\n\t"
            "addl %13, %%eax\n\t"
            "addl %14, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=m" (multi_array[0][0])  /* Memory output */
            : "r" (r1), "r" (r2), "r" (r3), "r" (r4), "r" (r5),
              "r" (r6), "r" (r7), "r" (r8), "r" (r9), "r" (r10),
              "r" (r11), "r" (r12), "r" (r13), "r" (r14)
            : "%eax", "cc", "memory"
        );
        
        /* Use all variables to keep them live */
        result += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + 
                 r9 + r10 + r11 + r12 + r13 + r14 + r15;
    }
    
    /* ============================================
       BLOCK F: Computed Goto with Address Reload
       ============================================ */
    {
        static void *jump_table[] = { &&label1, &&label2, &&label3 };
        int idx = int_var % 3;
        int goto_result = 0;
        
        /* Indirect jump through memory */
        asm volatile (
            "jmp *%1\n\t"
            : "=r" (goto_result)
            : "r" (jump_table[idx])
            : "memory"
        );
        
    label1:
        goto_result = 100;
        goto done;
    label2:
        goto_result = 200;
        goto done;
    label3:
        goto_result = 300;
        goto done;
    done:
        result += goto_result;
    }
    
    return result;
}

int main(void) {
    /* Initialize global struct */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            global_struct[i].a[j] = i * 100 + j;
        }
        for (int j = 0; j < 4; j++) {
            global_struct[i].b[j] = i * 10.0 + j;
        }
        if (i < 3) {
            global_struct[i].next = &global_struct[i + 1];
        }
    }
    
    /* Call reload trigger multiple times with different params */
    int total = 0;
    for (int i = 0; i < 3; i++) {
        total += trigger_reloads();
    }
    
    printf("Result: %d\n", total);
    return total & 0xFF;  /* Return non-zero to indicate execution */
}
