/* reload_coverage.c - Complex program to trigger GCC reload pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimizations that would simplify addressing */
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Function that will trigger multiple reload types */
__attribute__((noinline, noipa))
static int trigger_reloads(
    volatile int idx1, volatile int idx2, volatile int idx3,
    volatile int stride1, volatile int stride2,
    volatile double scale1, volatile double scale2)
{
    /* Create high register pressure with many live values */
    int arr1[128];      /* Integer array - pressure on integer regs */
    double arr2[64];    /* Double array - pressure on FP regs */
    long arr3[96];      /* Long array - mixed pressure */
    char arr4[256];     /* Char array - different addressing modes */
    
    int *ptr1, *ptr2, **ptr3;
    double *dptr1, *dptr2;
    long *lptr1;
    char *cptr1, **cptr2;
    
    volatile int local_idx;
    volatile double local_fp;
    int result = 0;
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 128; i++) arr1[i] = i * 3;
    for (int i = 0; i < 64; i++) arr2[i] = i * 1.5;
    for (int i = 0; i < 96; i++) arr3[i] = i * 7L;
    for (int i = 0; i < 256; i++) arr4[i] = (char)(i & 0xFF);
    
    COMPILER_BARRIER();
    
    /* ===== RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS ===== */
    /* Complex multi-level addressing with volatile indices */
    ptr1 = &arr1[idx1 * stride1 + idx2];
    ptr2 = &arr1[idx2 * stride2 + idx3];
    
    /* Chain of pointer dereferences requiring address computation */
    ptr3 = &ptr1;
    int temp1 = **ptr3 + idx1;
    
    /* Multi-dimensional access simulation */
    int offset1 = idx1 * stride1 + idx2;
    int offset2 = idx2 * stride2 + idx3;
    int offset3 = (idx1 + idx3) * (stride1 + stride2);
    
    /* Complex address calculation that needs temporary register */
    int val1 = arr1[offset1] + arr1[offset2] - arr1[offset3];
    
    COMPILER_BARRIER();
    
    /* ===== RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS ===== */
    /* Store results through complex address calculations */
    arr1[(idx1 * idx2 + idx3) % 128] = val1;
    
    /* Pointer arithmetic with different types */
    dptr1 = &arr2[idx1 % 64];
    dptr2 = &arr2[idx2 % 64];
    
    /* Mixed integer/float operations - pressure different register classes */
    local_fp = *dptr1 * scale1 + *dptr2 * scale2;
    arr2[idx3 % 64] = local_fp;
    
    COMPILER_BARRIER();
    
    /* ===== RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR ===== */
    /* Inline assembly with complex addressing constraints */
    int asm_result1, asm_result2;
    
    /* Assembly that takes memory address as input */
    __asm__ volatile (
        "movl (%1), %0\n\t"
        "addl %2, %0"
        : "=r" (asm_result1)
        : "r" (&arr1[idx1]), "r" (idx2)
        : "memory"
    );
    
    /* Assembly with output address constraint */
    __asm__ volatile (
        "leal (%1, %2, 4), %0\n\t"
        "movl (%0), %0"
        : "=r" (asm_result2)
        : "r" (arr1), "r" (idx3)
        : "memory"
    );
    
    COMPILER_BARRIER();
    
    /* ===== RELOAD_FOR_INPUT and RELOAD_OTHER ===== */
    /* Complex expression with many intermediate values */
    lptr1 = &arr3[idx1 % 96];
    cptr1 = &arr4[idx2 % 256];
    
    /* Chain of computations keeping many values live */
    long temp_long = *lptr1 * 3L;
    char temp_char = *cptr1 + (char)idx3;
    
    /* Multiple uses of volatile values in address calculations */
    cptr2 = (char**)&cptr1;
    char temp_char2 = **(cptr2 + (idx1 & 1));
    
    /* Mixed-type pointer arithmetic */
    int* int_from_char = (int*)(cptr1 + idx3 * sizeof(int));
    int val2 = *int_from_char;
    
    COMPILER_BARRIER();
    
    /* ===== RELOAD_FOR_OTHER_ADDRESS ===== */
    /* More complex addressing with multiple levels */
    int* nested_ptr = &arr1[arr1[idx1 % 128] % 128];
    int val3 = *nested_ptr + arr1[arr1[idx2 % 128] % 128];
    
    /* Pointer to pointer with offset */
    int** pptr = &nested_ptr;
    int val4 = **(pptr + (idx3 & 1));
    
    /* Final computation using all values */
    result = val1 + asm_result1 + asm_result2 + val2 + val3 + val4 
             + (int)temp_long + temp_char + temp_char2;
    
    /* Force spill/reload across call boundary */
    COMPILER_BARRIER();
    
    return result + (int)(local_fp * 100.0);
}

/* Helper to create more register pressure */
__attribute__((noinline))
static double fp_computation(volatile double a, volatile double b, 
                            volatile double c, volatile double d)
{
    /* Create FP register pressure */
    double arr[8];
    for (int i = 0; i < 8; i++) {
        arr[i] = a * i + b * (i+1) + c * (i+2) + d * (i+3);
    }
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += arr[i] * arr[7-i];
    }
    
    return sum;
}

int main(void)
{
    /* Initialize with random values to prevent constant propagation */
    volatile int idx1 = rand() % 100;
    volatile int idx2 = rand() % 100;
    volatile int idx3 = rand() % 100;
    volatile int stride1 = (rand() % 10) + 1;
    volatile int stride2 = (rand() % 10) + 1;
    volatile double scale1 = (rand() % 100) / 10.0;
    volatile double scale2 = (rand() % 100) / 10.0;
    
    /* Create additional register pressure before call */
    double fp_temp = fp_computation(scale1, scale2, scale1 * 2, scale2 * 2);
    
    /* Trigger the reload-intensive function */
    int result = trigger_reloads(idx1, idx2, idx3, stride1, stride2, 
                                scale1 + fp_temp * 0.001, scale2);
    
    printf("Result: %d (indices: %d, %d, %d)\n", 
           result, idx1, idx2, idx3);
    
    return 0;
}
