/* reload_coverage.c - Complex program to trigger GCC reload pass switch cases */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining and inter-procedural analysis */
__attribute__((noinline, noipa, optimize("no-gcse", "no-tree-pre")))
static int trigger_reloads(
    volatile int idx1, volatile int idx2, volatile int idx3,
    volatile int stride1, volatile int stride2,
    volatile double scale1, volatile double scale2,
    volatile long offset1, volatile long offset2)
{
    /* High register pressure with mixed types */
    int local1 = idx1 * 3;
    int local2 = idx2 * 5;
    int local3 = idx3 * 7;
    double f1 = scale1 * 2.0;
    double f2 = scale2 * 3.0;
    int local4 = idx1 + idx2;
    double f3 = f1 + f2;
    int local5 = local1 ^ local2;
    
    /* Multi-dimensional array access with volatile indices */
    int arr3d[10][20][30];
    double dbl_arr[50][40];
    
    /* Complex pointer chains for address reloads */
    char *base_ptr = (char *)arr3d;
    int *int_ptr = (int *)arr3d;
    double *dbl_ptr = (double *)dbl_arr;
    
    /* Force RELOAD_FOR_INPUT_ADDRESS with multi-level addressing */
    int *addr1 = &arr3d[idx1 % 10][idx2 % 20][idx3 % 30];
    int *addr2 = addr1 + (stride1 % 100);
    
    /* Memory barrier to prevent optimization */
    __asm__ volatile("" : : : "memory");
    
    /* Complex address calculation requiring temporary registers */
    int *chain_ptr = (int *)((char *)addr2 + (offset1 % 1000) * sizeof(int));
    chain_ptr = chain_ptr + (stride2 % 50);
    
    /* Force RELOAD_FOR_OUTPUT_ADDRESS */
    int output_val;
    int *output_addr = &arr3d[(idx1 + idx2) % 10][(idx2 + idx3) % 20][0];
    
    /* Inline assembly to trigger RELOAD_FOR_OPERAND_ADDRESS */
    __asm__ volatile(
        "mov %[addr], %%rsi\n\t"
        "mov (%%rsi), %[out]\n\t"
        : [out] "=r" (output_val)
        : [addr] "m" (chain_ptr)
        : "rsi", "memory"
    );
    
    /* Mixed register class pressure */
    for (volatile int i = 0; i < 5; i++) {
        /* Integer computation */
        local1 = local1 * local2 + local3;
        
        /* Floating-point computation */
        f1 = f1 * f2 + scale1;
        
        /* Complex array access with volatile addressing */
        int idx = (idx1 + i) % 10;
        int jdx = (idx2 + i * 2) % 20;
        int kdx = (idx3 + i * 3) % 30;
        
        /* Multi-dimensional access requiring address computation */
        arr3d[idx][jdx][kdx] = (int)(f1 * 100.0) + local1;
        
        /* Pointer arithmetic with different types */
        char *tmp = (char *)&arr3d[idx][jdx][kdx];
        tmp += (offset2 % 100);
        int_ptr = (int *)tmp;
        
        /* Another inline assembly for RELOAD_FOR_OUTADDR_ADDRESS */
        int asm_out;
        __asm__ volatile(
            "lea (%[base], %[idx], 4), %%rax\n\t"
            "mov (%%rax), %[out]\n\t"
            : [out] "=r" (asm_out)
            : [base] "r" (int_ptr), [idx] "r" (stride1)
            : "rax", "memory"
        );
        
        local2 += asm_out;
    }
    
    /* More complex addressing for RELOAD_FOR_INPADDR_ADDRESS */
    double *dbl_addr = &dbl_arr[idx1 % 50][idx2 % 40];
    dbl_addr += stride1 % 20;
    
    /* Force floating-point spills */
    for (volatile int i = 0; i < 8; i++) {
        f3 = f3 * 1.1 + dbl_addr[i % 5];
        dbl_arr[(idx1 + i) % 50][(idx2 + i) % 40] = f3;
    }
    
    /* Final complex address calculation */
    int ***triple_ptr = (int ***)arr3d;
    int **double_ptr = triple_ptr[idx1 % 5];
    int *single_ptr = double_ptr[(idx2 % 3) * 2];
    
    /* Another inline assembly with memory operand */
    int final_result;
    __asm__ volatile(
        "imul %[a], %[b]\n\t"
        "add %[c], %[b]\n\t"
        : [b] "=r" (final_result)
        : [a] "r" (local1), [b] "0" (local2), [c] "m" (*single_ptr)
        : "cc"
    );
    
    /* Use all variables to prevent elimination */
    return final_result + (int)f1 + (int)f2 + (int)f3 + 
           local3 + local4 + local5 + output_val + *addr1;
}

int main(void) {
    /* Initialize with random values to prevent constant propagation */
    srand(42);
    
    volatile int idx1 = rand() % 100;
    volatile int idx2 = rand() % 100;
    volatile int idx3 = rand() % 100;
    volatile int stride1 = rand() % 100;
    volatile int stride2 = rand() % 100;
    volatile double scale1 = (double)(rand() % 100) / 10.0;
    volatile double scale2 = (double)(rand() % 100) / 10.0;
    volatile long offset1 = rand() % 1000;
    volatile long offset2 = rand() % 1000;
    
    /* Call the function with volatile arguments */
    int result = trigger_reloads(idx1, idx2, idx3, stride1, stride2,
                                scale1, scale2, offset1, offset2);
    
    printf("Result: %d\n", result);
    
    /* Prevent dead code elimination of the function */
    volatile int dummy = result;
    __asm__ volatile("" : : "r" (dummy));
    
    return 0;
}
