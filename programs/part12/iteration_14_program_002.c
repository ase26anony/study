/* reload_coverage.c - Complex program to trigger GCC reload pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimizations that would simplify addressing */
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Function that will trigger various reload types */
__attribute__((noinline, noipa))
static int trigger_reloads(
    volatile int idx1, volatile int idx2, volatile int idx3,
    volatile int stride1, volatile int stride2,
    volatile double scale1, volatile double scale2)
{
    /* Create high register pressure with many live values */
    int arr1[100];      /* Integer array - pressure on integer regs */
    double arr2[50];    /* Double array - pressure on FP regs */
    long arr3[75];      /* Long array - mixed pressure */
    char arr4[200];     /* Char array - different addressing */
    
    /* Many scalar variables to increase register pressure */
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    double ftemp1, ftemp2, ftemp3, ftemp4;
    long ltemp1, ltemp2;
    void *ptr1, *ptr2, *ptr3;
    
    /* Initialize arrays with some values */
    for (int i = 0; i < 50; i++) {
        arr2[i] = i * 1.5;
        if (i < 75) arr3[i] = i * 3;
        if (i < 100) arr1[i] = i * 2;
        if (i < 200) arr4[i] = i % 256;
    }
    
    COMPILER_BARRIER();
    
    /* ===== RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS ===== */
    /* Complex multi-level addressing with volatile indices */
    temp1 = arr1[idx1 * stride1 + idx2];
    COMPILER_BARRIER();
    
    /* Multi-dimensional access simulation with pointer chains */
    int *ptr_arr[10];
    for (int i = 0; i < 10; i++) {
        ptr_arr[i] = &arr1[i * 10];
    }
    
    /* Complex address calculation requiring temporary register */
    temp2 = *(*(ptr_arr + idx1) + idx2);
    COMPILER_BARRIER();
    
    /* ===== RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS ===== */
    /* Store with complex address calculation */
    arr3[idx1 * stride2 + idx3] = temp1 * temp2;
    COMPILER_BARRIER();
    
    /* Pointer arithmetic with different types */
    char *cptr = arr4;
    cptr += idx1 * sizeof(int) + idx2;
    *((int*)cptr) = temp1;
    COMPILER_BARRIER();
    
    /* ===== RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR ===== */
    /* Inline assembly forcing address reloads */
    int asm_result1, asm_result2;
    
    /* Assembly with memory input and register output */
    __asm__ volatile (
        "movl (%1), %0\n\t"
        : "=r" (asm_result1)
        : "r" (&arr1[idx1 + idx2])
        : "memory"
    );
    
    COMPILER_BARRIER();
    
    /* Assembly with complex addressing mode */
    __asm__ volatile (
        "leaq (%1, %2, 4), %0\n\t"
        : "=r" (ptr1)
        : "r" (arr1), "r" ((long)idx1)
        : "cc"
    );
    
    /* ===== RELOAD_FOR_INPUT (general) and RELOAD_OTHER ===== */
    /* Mixed integer/float operations to pressure different register classes */
    ftemp1 = arr2[idx1] * scale1;
    ftemp2 = arr2[idx2] * scale2;
    
    /* Convert float to int and back - forces moves between register files */
    temp3 = (int)(ftemp1 + ftemp2);
    ftemp3 = (double)temp3 * 1.5;
    
    COMPILER_BARRIER();
    
    /* More complex addressing with type mixing */
    ltemp1 = arr3[idx1] + (long)arr1[idx2] * (long)idx3;
    
    /* Pointer chain dereferencing */
    int **pptr = &ptr_arr[idx1 % 10];
    temp4 = **pptr;
    
    /* ===== Additional complex scenarios ===== */
    /* Nested addressing with multiple volatile indices */
    temp5 = arr1[(idx1 * stride1 + idx2) % 100];
    temp6 = arr1[(idx2 * stride2 + idx3) % 100];
    
    /* Floating point array with complex indexing */
    ftemp4 = arr2[(idx1 + idx2) % 50] * arr2[(idx2 + idx3) % 50];
    
    /* Mixed type pointer arithmetic */
    void *base_ptr = arr1;
    int offset = idx1 * sizeof(int) + idx2 * 2;
    temp7 = *((int*)((char*)base_ptr + offset));
    
    COMPILER_BARRIER();
    
    /* Another inline assembly with output address reload */
    __asm__ volatile (
        "movq %1, %0\n\t"
        "addq $16, %0\n\t"
        : "=&r" (ptr2)
        : "r" (&arr3[0])
        : "cc"
    );
    
    /* Store through computed pointer */
    *((int*)ptr2) = temp5 + temp6;
    
    /* Final complex computation mixing all types */
    double final_result = ftemp1 + ftemp2 + ftemp3 + ftemp4;
    final_result += (double)(temp1 + temp2 + temp3 + temp4 + temp5 + temp6 + temp7);
    final_result += (double)ltemp1;
    
    /* Use all variables to prevent optimization */
    volatile int dummy = asm_result1 + (int)ptr1 + (int)ptr2 + (int)ptr3;
    (void)dummy;
    
    return (int)final_result + idx1 + idx2 + idx3;
}

int main(void) {
    /* Initialize volatile variables with random values */
    volatile int idx1 = rand() % 50;
    volatile int idx2 = rand() % 50;
    volatile int idx3 = rand() % 50;
    volatile int stride1 = (rand() % 10) + 1;
    volatile int stride2 = (rand() % 10) + 1;
    volatile double scale1 = (rand() % 100) / 10.0;
    volatile double scale2 = (rand() % 100) / 10.0;
    
    /* Call the function multiple times with different values */
    int result = 0;
    for (int i = 0; i < 10; i++) {
        result += trigger_reloads(
            idx1 + i, 
            idx2 + i * 2, 
            idx3 + i * 3,
            stride1 + i,
            stride2 + i * 2,
            scale1 + i * 0.5,
            scale2 + i * 0.3
        );
        
        /* Modify volatiles to prevent CSE */
        idx1 += 7;
        idx2 += 11;
        idx3 += 13;
    }
    
    printf("Result: %d\n", result);
    return 0;
}
