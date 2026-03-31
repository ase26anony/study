/* reload_coverage.c - Complex program to trigger GCC reload pass edge cases */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimizations that would simplify addressing */
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Force register pressure and complex addressing */
__attribute__((noinline, noipa))
static int trigger_reloads(
    volatile int v1, volatile int v2, volatile int v3,
    volatile int v4, volatile int v5, volatile int v6,
    volatile long v7, volatile long v8)
{
    /* Create high register pressure with mixed types */
    int local1 = v1 * 2;
    int local2 = v2 + v3;
    double f1 = (double)v4 * 1.5;
    double f2 = (double)v5 * 2.5;
    int local3, local4, local5, local6, local7, local8;
    double f3, f4, f5, f6;
    
    /* Multi-dimensional array access with volatile indices */
    int arr3d[10][10][10];
    double darr[20][20];
    
    /* Complex pointer chains */
    int *ptr1 = &arr3d[0][0][0];
    int **ptr2 = (int **)&ptr1;
    int ***ptr3 = (int ***)&ptr2;
    
    /* Force RELOAD_FOR_INPUT_ADDRESS with multi-level addressing */
    for (int i = 0; i < 5; i++) {
        /* arr3d[volatile][volatile][volatile] */
        int idx1 = v1 + i;
        int idx2 = v2 + i * 2;
        int idx3 = v3 + i * 3;
        
        /* Complex address calculation requiring temporary register */
        arr3d[idx1 % 10][idx2 % 10][idx3 % 10] = 
            *(*(*(ptr3) + (idx1 % 10)) + (idx2 % 10) * 10 + (idx3 % 10));
        
        /* Mixed floating-point operations */
        darr[idx1 % 20][idx2 % 20] = f1 * f2 + (double)idx3;
    }
    
    COMPILER_BARRIER();
    
    /* Inline assembly to force RELOAD_FOR_OPERAND_ADDRESS */
    int asm_result1, asm_result2;
    __asm__ volatile(
        /* Input: memory address, Output: register */
        "movl (%[addr]), %[out1]\n\t"
        "addl $1, %[out1]\n\t"
        : [out1] "=&r" (asm_result1)
        : [addr] "m" (arr3d[v1 % 10][v2 % 10][v3 % 10])
        : "memory"
    );
    
    /* Another assembly with different constraints */
    int *addr_ptr = &arr3d[v4 % 10][v5 % 10][v6 % 10];
    __asm__ volatile(
        /* Complex addressing in assembly operand */
        "leal (%[base], %[index], 4), %[out2]\n\t"
        : [out2] "=r" (asm_result2)
        : [base] "r" (addr_ptr), [index] "r" (v7)
        : "cc"
    );
    
    COMPILER_BARRIER();
    
    /* Force RELOAD_FOR_OUTPUT_ADDRESS with pointer arithmetic */
    int *output_ptr;
    __asm__ volatile(
        /* Compute address and store in register */
        "leal (%[a], %[b], 2), %[out]\n\t"
        : [out] "=r" (output_ptr)
        : [a] "r" (ptr1), [b] "r" (v8)
        : "cc"
    );
    
    /* Use the computed address */
    *output_ptr = asm_result1 + asm_result2;
    
    /* More complex addressing with type conversions */
    char *char_ptr = (char *)ptr1;
    int *int_ptr_from_char;
    
    /* Force RELOAD_FOR_INPADDR_ADDRESS */
    __asm__ volatile(
        "movl %%ecx, %[out]\n\t"
        : [out] "=r" (int_ptr_from_char)
        : "c" (char_ptr + v1 * sizeof(int) + v2)
        : 
    );
    
    /* Access through computed pointer */
    local3 = *int_ptr_from_char;
    
    /* Nested addressing with volatile components */
    long complex_offset = v7 * 16 + v8 * 8 + v1 * 4;
    int *complex_addr = (int *)((char *)ptr1 + complex_offset);
    
    /* Force multiple reload types in one expression */
    local4 = complex_addr[v2 % 5] + 
             arr3d[(v3 + v4) % 10][(v5 + v6) % 10][(v7 + v8) % 10] +
             *(int *)(char_ptr + v1 * 3 + v2 * 7);
    
    /* Floating-point array with complex indexing */
    for (int i = 0; i < 8; i++) {
        int idx_a = (v1 + i * v2) % 20;
        int idx_b = (v3 + i * v4) % 20;
        int idx_c = (v5 + i * v6) % 20;
        
        /* Mixed-type computation */
        darr[idx_a][idx_b] = darr[idx_c][idx_a] * 
                            (double)arr3d[idx_b % 10][idx_c % 10][idx_a % 10] +
                            f1 - f2;
    }
    
    COMPILER_BARRIER();
    
    /* Final computation using all variables */
    int result = local1 + local2 + local3 + local4 + asm_result1 + asm_result2;
    result += (int)(f1 + f2 + darr[0][0] + darr[10][10]);
    result += *output_ptr + *complex_addr;
    
    return result;
}

int main(void) {
    /* Initialize volatile variables with random values */
    volatile int v1 = rand() % 100;
    volatile int v2 = rand() % 100;
    volatile int v3 = rand() % 100;
    volatile int v4 = rand() % 100;
    volatile int v5 = rand() % 100;
    volatile int v6 = rand() % 100;
    volatile long v7 = rand() % 100;
    volatile long v8 = rand() % 100;
    
    /* Call function with many volatile arguments */
    int result = trigger_reloads(v1, v2, v3, v4, v5, v6, v7, v8);
    
    printf("Result: %d\n", result);
    
    /* Additional calls with different values to explore more paths */
    for (int i = 0; i < 3; i++) {
        v1 = rand() % 100;
        v2 = rand() % 100;
        result += trigger_reloads(v1, v2, v3, v4, v5, v6, v7, v8);
    }
    
    printf("Final result: %d\n", result);
    return 0;
}
