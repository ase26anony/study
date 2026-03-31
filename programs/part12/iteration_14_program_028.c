/* reload_coverage.c - Program to trigger multiple reload types in GCC's reload pass */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining and inter-procedural analysis */
__attribute__((noinline, noipa, optimize("no-gcse", "no-tree-pre")))
static int trigger_reloads(
    volatile int v1, volatile int v2, volatile int v3,
    volatile int v4, volatile int v5, volatile int v6,
    volatile long v7, volatile long v8, volatile double v9)
{
    /* Create high register pressure with many live values */
    int local1 = v1 * 2;
    int local2 = v2 + v3;
    int local3 = v4 ^ v5;
    int local4 = v6 << 2;
    long local5 = v7 + v8;
    double local6 = (double)v9 * 3.14159;
    
    /* Multi-dimensional arrays with volatile indexing */
    int arr3d[10][10][10];
    double dbl_arr[20][20];
    char char_arr[100][50];
    
    /* Complex pointer chains */
    int *ptr1 = &arr3d[0][0][0];
    int **ptr2 = (int **)&ptr1;
    int ***ptr3 = (int ***)&ptr2;
    
    /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPUT */
    /* Multi-level array access with volatile indices */
    for (volatile int i = 0; i < 5; i++) {
        for (volatile int j = 0; j < 5; j++) {
            /* Complex address calculation requiring temporary register */
            arr3d[v1 + i][v2 + j][v3] = 
                arr3d[v4 - i][v5 - j][v6] * local1;
            
            /* Different addressing mode: base + index * scale */
            dbl_arr[i * v1][j * v2] = 
                dbl_arr[j * v3][i * v4] + local6;
        }
    }
    
    /* Memory barrier to prevent optimization */
    __asm__ volatile("" : : : "memory");
    
    /* Force RELOAD_FOR_OUTPUT_ADDRESS */
    /* Complex pointer arithmetic with different types */
    char *char_ptr = (char *)&char_arr[v1][v2];
    int *int_ptr = (int *)(char_ptr + v3 * sizeof(int));
    long *long_ptr = (long *)((char *)int_ptr + v4 * sizeof(long));
    
    /* Store through complex computed address */
    *long_ptr = local5;
    
    /* Force RELOAD_FOR_OPERAND_ADDRESS with inline assembly */
    int asm_result1, asm_result2;
    
    /* Assembly with memory operand constraints */
    __asm__ volatile(
        "movl (%[addr]), %[out1]\n\t"
        "addl $1, %[out1]\n\t"
        : [out1] "=&r" (asm_result1)
        : [addr] "m" (*int_ptr)
        : "memory"
    );
    
    /* Force RELOAD_FOR_OUTADDR_ADDRESS */
    /* Assembly that outputs to a memory address */
    int output_var = 0;
    int *output_addr = &output_var + v5;
    
    __asm__ volatile(
        "movl %[val], (%[addr])\n\t"
        : 
        : [val] "r" (asm_result1), [addr] "r" (output_addr)
        : "memory"
    );
    
    /* More complex addressing with mixed types */
    double *dbl_ptrs[10];
    for (int i = 0; i < 10; i++) {
        dbl_ptrs[i] = &dbl_arr[i][v6 % 20];
    }
    
    /* Chain of pointer dereferences */
    double chain_result = ***((double ***)&dbl_ptrs[v1 % 10]);
    chain_result += *(*(dbl_ptrs[v2 % 10] + v3) + v4);
    
    /* Force RELOAD_FOR_INPADDR_ADDRESS */
    /* Address of address computation */
    int addr_temp;
    int *addr_ptr = &addr_temp;
    int **addr_ptr_ptr = &addr_ptr;
    
    __asm__ volatile(
        "leal (%[base], %[index], 4), %[out]\n\t"
        : [out] "=r" (addr_temp)
        : [base] "r" (ptr1), [index] "r" (v7)
        : "cc"
    );
    
    /* Use computed address */
    int indirect_load = **addr_ptr_ptr;
    
    /* Force RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
    /* Complex expression mixing everything */
    int final_result = 
        arr3d[v1][v2][v3] +
        (int)dbl_arr[v4][v5] +
        char_arr[v6][local1 % 50] +
        asm_result1 +
        indirect_load +
        (int)chain_result +
        output_var;
    
    /* More register pressure */
    double fp1 = (double)local1 * 1.1;
    double fp2 = (double)local2 * 2.2;
    double fp3 = (double)local3 * 3.3;
    double fp4 = (double)local4 * 4.4;
    
    /* Mix integer and floating point to pressure different reg classes */
    __asm__ volatile("" : : "r" (fp1), "r" (fp2), "r" (fp3), "r" (fp4));
    
    /* Final computation using all values */
    final_result += (int)(fp1 + fp2 + fp3 + fp4);
    final_result += *((int *)long_ptr + v8 % 4);
    
    return final_result;
}

int main(void) {
    /* Initialize with random values to prevent constant propagation */
    srand(42);
    
    volatile int v1 = rand() % 10;
    volatile int v2 = rand() % 10;
    volatile int v3 = rand() % 10;
    volatile int v4 = rand() % 10;
    volatile int v5 = rand() % 10;
    volatile int v6 = rand() % 10;
    volatile long v7 = rand() % 100;
    volatile long v8 = rand() % 100;
    volatile double v9 = (double)rand() / RAND_MAX * 100.0;
    
    /* Call function multiple times with different volatile values */
    int result = 0;
    for (int i = 0; i < 10; i++) {
        result += trigger_reloads(
            v1 + i, v2 - i, v3 * i, 
            v4 ^ i, v5 | i, v6 & i,
            v7 + i, v8 - i, v9 * i
        );
        
        /* Modify volatiles to force recomputation */
        v1 += 1;
        v2 -= 1;
        v3 *= 2;
        v9 += 0.5;
    }
    
    printf("Result: %d\n", result);
    return 0;
}
