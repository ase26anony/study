/* reload_coverage.c - Complex program to trigger GCC reload pass edge cases */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimizations that would simplify addressing */
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Force register pressure and complex addressing */
__attribute__((noinline, noipa))
static int trigger_reloads(
    volatile int idx1, volatile int idx2, volatile int idx3,
    volatile int stride1, volatile int stride2, volatile long offset,
    volatile double scale, volatile int *ext_ptr)
{
    /* Create high register pressure with mixed types */
    int local_arr[128];          /* Integer pressure */
    double fp_arr[64];           /* FP pressure */
    long long_arr[96];           /* Long pressure */
    char *ptr_chain[32];         /* Pointer pressure */
    
    int * volatile ptr1 = local_arr;
    double * volatile ptr2 = fp_arr;
    volatile int v_idx;
    
    /* Initialize arrays to create live ranges */
    for (int i = 0; i < 128; i++) local_arr[i] = i * 3;
    for (int i = 0; i < 64; i++) fp_arr[i] = i * 1.5;
    for (int i = 0; i < 96; i++) long_arr[i] = i * 5L;
    
    COMPILER_BARRIER();
    
    /* ===== RELOAD_FOR_INPUT_ADDRESS ===== */
    /* Complex addressing requiring temporary registers */
    int result1 = local_arr[idx1 * stride1 + idx2];
    COMPILER_BARRIER();
    
    /* Multi-level pointer arithmetic */
    int * volatile mid_ptr = &local_arr[idx2];
    int result2 = *(mid_ptr + idx3 * 2 - stride2);
    COMPILER_BARRIER();
    
    /* ===== RELOAD_FOR_INPADDR_ADDRESS ===== */
    /* Address of address computation */
    int ** volatile pptr = &mid_ptr;
    int result3 = **pptr;
    COMPILER_BARRIER();
    
    /* ===== RELOAD_FOR_OUTPUT_ADDRESS ===== */
    /* Complex store address */
    fp_arr[idx1 * 2 + idx3] = scale * idx2;
    COMPILER_BARRIER();
    
    /* ===== RELOAD_FOR_OUTADDR_ADDRESS ===== */
    /* Store through computed pointer address */
    double * volatile store_ptr = &fp_arr[stride1];
    *store_ptr = *store_ptr * 2.0 + 1.0;
    COMPILER_BARRIER();
    
    /* ===== RELOAD_FOR_OPERAND_ADDRESS ===== */
    /* Inline assembly with memory operand */
    int asm_result;
    __asm__ volatile (
        "movl (%1), %0\n\t"
        "addl $42, %0"
        : "=r" (asm_result)
        : "r" (&local_arr[idx1 + idx2])
        : "cc"
    );
    COMPILER_BARRIER();
    
    /* ===== RELOAD_FOR_OPADDR_ADDR ===== */
    /* More complex assembly with address computation */
    long asm_result2;
    __asm__ volatile (
        "movq (%[base], %[index], 4), %[out]\n\t"
        "imulq $3, %[out], %[out]"
        : [out] "=&r" (asm_result2)
        : [base] "r" (long_arr),
          [index] "r" ((long)idx3)
        : "cc"
    );
    COMPILER_BARRIER();
    
    /* ===== RELOAD_FOR_OTHER_ADDRESS ===== */
    /* Mixed type pointer chain */
    char * volatile char_ptr = (char *)long_arr;
    for (int i = 0; i < 8; i++) {
        ptr_chain[i] = char_ptr + i * offset;
    }
    COMPILER_BARRIER();
    
    /* Access through pointer chain */
    int chain_result = *(int *)(ptr_chain[3] + stride2 * 4);
    COMPILER_BARRIER();
    
    /* ===== RELOAD_OTHER and mixed operations ===== */
    /* Force spills between register classes */
    double fp_sum = 0.0;
    for (int i = 0; i < 16; i++) {
        /* Integer to FP conversion causing register moves */
        fp_sum += (double)local_arr[i * stride1] * scale;
        
        /* FP to integer conversion */
        int int_val = (int)(fp_arr[i] * 2.0);
        local_arr[i + 32] = int_val + idx1;
    }
    COMPILER_BARRIER();
    
    /* Complex 2D-like access pattern */
    int matrix_result = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            /* Non-linear addressing */
            int pos = (i * stride1 + j * stride2) & 127;
            matrix_result += local_arr[pos] * (i - j);
        }
    }
    COMPILER_BARRIER();
    
    /* Additional assembly with multiple constraints */
    int final_result;
    __asm__ volatile (
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "imull %%eax, %%eax\n\t"
        "movl %%eax, %[out]"
        : [out] "=rm" (final_result)
        : [in1] "rm" (result1),
          [in2] "rm" (result2)
        : "%eax", "cc"
    );
    
    /* Use all computed values to prevent elimination */
    v_idx = idx1 ^ idx2 ^ idx3;
    return (result1 + result2 + result3 + asm_result + 
            (int)asm_result2 + chain_result + matrix_result + 
            final_result + (int)fp_sum + v_idx);
}

int main(void) {
    /* Initialize volatile variables with random values */
    volatile int idx1 = rand() % 50;
    volatile int idx2 = rand() % 50;
    volatile int idx3 = rand() % 50;
    volatile int stride1 = (rand() % 10) + 1;
    volatile int stride2 = (rand() % 10) + 1;
    volatile long offset = (rand() % 100) + 8;
    volatile double scale = (rand() % 100) / 10.0 + 0.5;
    
    int ext_array[200];
    for (int i = 0; i < 200; i++) {
        ext_array[i] = rand();
    }
    
    /* Call the reload-intensive function multiple times */
    int total = 0;
    for (int iter = 0; iter < 100; iter++) {
        /* Modify parameters slightly each iteration */
        idx1 = (idx1 + 1) % 50;
        idx2 = (idx2 * 3) % 50;
        
        total += trigger_reloads(idx1, idx2, idx3, stride1, stride2,
                                offset, scale, ext_array);
        
        /* Prevent loop unrolling */
        COMPILER_BARRIER();
    }
    
    printf("Result: %d\n", total);
    return 0;
}
