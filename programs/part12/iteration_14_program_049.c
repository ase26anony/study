/* reload_coverage.c - Program to trigger GCC reload pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimizations that would simplify addressing */
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Force noinline and no interprocedural analysis */
__attribute__((noinline, noipa))
static int trigger_reloads(
    volatile int idx1, volatile int idx2, volatile int idx3,
    volatile int stride1, volatile int stride2,
    volatile double scale1, volatile double scale2)
{
    /* Create high register pressure with mixed types */
    int local1 = idx1 * 3;
    int local2 = idx2 + 7;
    int local3 = idx3 - 5;
    double f1 = scale1 * 2.0;
    double f2 = scale2 / 3.0;
    int local4 = local1 ^ local2;
    double f3 = f1 + f2;
    
    /* Multi-dimensional arrays forcing complex addressing */
    int arr3d[10][20][30];
    double dbl_arr[50][40];
    char byte_arr[100][80];
    
    /* Various pointer types for different addressing modes */
    int *ptr1 = &arr3d[0][0][0];
    double *ptr2 = &dbl_arr[0][0];
    char *ptr3 = &byte_arr[0][0];
    long *ptr4 = (long *)ptr1;
    
    COMPILER_BARRIER();
    
    /* Complex pointer arithmetic with volatile indices */
    /* This should trigger RELOAD_FOR_INPUT_ADDRESS */
    int *addr1 = &arr3d[idx1 % 10][idx2 % 20][idx3 % 30];
    int val1 = *addr1;
    
    /* Multi-level pointer dereferencing */
    /* Should trigger RELOAD_FOR_INPADDR_ADDRESS */
    int **ptr_to_ptr = (int **)&addr1;
    int val2 = **ptr_to_ptr;
    
    COMPILER_BARRIER();
    
    /* Mixed-type addressing with scaling */
    /* Different scales: char=1, int=4, double=8 */
    double *dbl_addr = &dbl_arr[idx1 % 50][idx2 % 40];
    char *char_addr = &byte_arr[idx2 % 100][idx3 % 80];
    
    /* Complex address computation with multiple volatile components */
    /* Should trigger various address reloads */
    long complex_offset = (long)(idx1 * stride1 + idx2 * stride2);
    int *complex_addr = (int *)((char *)ptr1 + complex_offset);
    int val3 = *complex_addr;
    
    COMPILER_BARRIER();
    
    /* Inline assembly forcing specific register constraints */
    /* Should trigger RELOAD_FOR_OPERAND_ADDRESS */
    int asm_result1, asm_result2;
    __asm__ volatile(
        "movl %[input], %[output1]\n\t"
        "leal (%[input], %[idx], 4), %[output2]"
        : [output1] "=&r" (asm_result1),
          [output2] "=&r" (asm_result2)
        : [input] "r" (val1),
          [idx] "r" (idx1)
        : "cc"
    );
    
    /* More inline assembly with memory address input */
    /* Should trigger RELOAD_FOR_OUTADDR_ADDRESS */
    int asm_result3;
    __asm__ volatile(
        "movl (%[addr]), %[out]\n\t"
        "addl $1, %[out]"
        : [out] "=r" (asm_result3)
        : [addr] "m" (*complex_addr)
        : "memory"
    );
    
    COMPILER_BARRIER();
    
    /* Floating point operations to pressure FP registers */
    double fp_sum = 0.0;
    for (volatile int i = 0; i < 10; i++) {
        for (volatile int j = 0; j < 10; j++) {
            /* Complex addressing in FP array */
            fp_sum += dbl_arr[(i + idx1) % 50][(j + idx2) % 40];
            
            /* Integer array access in same loop */
            arr3d[i % 10][j % 20][(i + j) % 30] = 
                (int)(fp_sum * scale1) + local1 + local2;
        }
    }
    
    COMPILER_BARRIER();
    
    /* More pointer chains for address reloads */
    int *ptr_arr[5];
    ptr_arr[0] = &arr3d[0][0][0];
    ptr_arr[1] = &arr3d[1][0][0];
    ptr_arr[2] = &arr3d[2][0][0];
    ptr_arr[3] = &arr3d[3][0][0];
    ptr_arr[4] = &arr3d[4][0][0];
    
    /* Chain dereferencing with volatile index */
    int chain_val = *(*(ptr_arr + (idx1 % 5)) + (idx2 % 100));
    
    COMPILER_BARRIER();
    
    /* Output address computation */
    /* Should trigger RELOAD_FOR_OUTPUT_ADDRESS */
    int *out_addr = &arr3d[idx3 % 10][idx1 % 20][idx2 % 30];
    *out_addr = asm_result1 + asm_result2 + asm_result3 + chain_val;
    
    /* Another output with complex address */
    double *dbl_out = &dbl_arr[idx2 % 50][idx3 % 40];
    *dbl_out = fp_sum * scale2;
    
    COMPILER_BARRIER();
    
    /* Final computation mixing all types */
    int final_result = 
        val1 + val2 + val3 + 
        asm_result1 + asm_result2 + asm_result3 +
        chain_val + *out_addr +
        (int)(*dbl_out) + local3 + local4;
    
    return final_result + (int)(f1 + f2 + f3);
}

int main(void)
{
    /* Initialize with random values to prevent constant propagation */
    srand(42);
    
    volatile int idx1 = rand() % 100;
    volatile int idx2 = rand() % 100;
    volatile int idx3 = rand() % 100;
    volatile int stride1 = rand() % 10 + 1;
    volatile int stride2 = rand() % 10 + 1;
    volatile double scale1 = (rand() % 100) / 10.0;
    volatile double scale2 = (rand() % 100) / 10.0;
    
    printf("Initial values: idx1=%d, idx2=%d, idx3=%d\n", idx1, idx2, idx3);
    
    int result = trigger_reloads(idx1, idx2, idx3, stride1, stride2, scale1, scale2);
    
    printf("Result: %d\n", result);
    
    return 0;
}
