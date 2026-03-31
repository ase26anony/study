/* reload_coverage.c - Complex program to trigger GCC reload pass switch cases */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimizations that would simplify addressing */
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Force noinline and no interprocedural analysis */
__attribute__((noinline, noipa, optimize("no-gcse", "no-tree-pre")))
static int trigger_reloads(
    volatile int v1, volatile int v2, volatile int v3,
    volatile int v4, volatile int v5, volatile int v6,
    volatile long v7, volatile long v8, volatile double v9)
{
    /* Create high register pressure with many live values */
    int local1 = v1 * 2;
    int local2 = v2 + v3;
    double local3 = (double)v4 * v9;
    long local4 = v7 ^ v8;
    int local5 = v5 - v6;
    double local6 = local3 / 2.0;
    
    /* Multi-dimensional arrays with volatile indexing */
    int arr3d[4][8][16];
    double dbl_arr[32][8];
    char *ptr_arr[12];
    
    /* Complex pointer chains */
    char *base_ptr = (char *)arr3d;
    int **int_ptr_ptr = (int **)malloc(sizeof(int *) * 20);
    long ***triple_ptr = (long ***)malloc(sizeof(long **) * 10);
    
    /* Force address reloads through complex addressing */
    for (volatile int i = 0; i < 4; i++) {
        for (volatile int j = 0; j < 8; j++) {
            /* RELOAD_FOR_INPUT_ADDRESS: complex array indexing */
            arr3d[i][j][v1 % 16] = 
                arr3d[(v2 + i) % 4][(v3 + j) % 8][(v4 + v1) % 16] * 
                local1 + local2;
            
            /* Mixed register class pressure */
            dbl_arr[i * 8 + j][v5 % 8] = 
                (double)arr3d[i][j][v6 % 16] * local3 + local6;
            
            COMPILER_BARRIER();
        }
    }
    
    /* Pointer arithmetic with multiple indirections */
    char *ptr1 = base_ptr + v1 * sizeof(int) * 8 * 16;
    char *ptr2 = ptr1 + v2 * sizeof(int) * 16;
    int *ptr3 = (int *)(ptr2 + v3 * sizeof(int));
    
    /* RELOAD_FOR_OPERAND_ADDRESS through inline assembly */
    int asm_result1, asm_result2;
    __asm__ volatile (
        /* Input address reload */
        "movl (%[addr1]), %[out1]\n\t"
        /* Output address reload */
        "movl %[in1], (%[addr2])\n\t"
        : [out1] "=&r" (asm_result1), "=m" (*ptr3)
        : [addr1] "r" (&local1), [in1] "r" (local2), [addr2] "r" (ptr3)
        : "memory"
    );
    
    /* More complex assembly with address computations */
    long asm_addr;
    __asm__ volatile (
        /* RELOAD_FOR_OUTADDR_ADDRESS */
        "lea (%[base], %[idx], 4), %[out]\n\t"
        : [out] "=r" (asm_addr)
        : [base] "r" (base_ptr), [idx] "r" (v7)
        : "cc"
    );
    
    /* Create RELOAD_FOR_INPADDR_ADDRESS scenario */
    int *indirect_arr[8];
    for (int i = 0; i < 8; i++) {
        indirect_arr[i] = &arr3d[0][i][0];
    }
    
    /* Complex addressing with multiple volatile components */
    int ***triple_indirect = (int ***)&indirect_arr;
    int volatile_idx = v4 % 8;
    
    /* This should trigger multiple address reload types */
    int final_result = 
        (*((**triple_indirect) + 
          (v1 * volatile_idx + v2) / (v3 + 1) * 16 + 
          (v5 % 4) * 4 + 
          (v6 % 4)));
    
    /* Force output address reloads */
    double *dbl_ptr = &dbl_arr[v1 % 32][v2 % 8];
    *dbl_ptr = local3 * 2.0 - local6;
    
    /* RELOAD_FOR_OTHER_ADDRESS through unusual pointer casting */
    uintptr_t intptr = (uintptr_t)base_ptr;
    intptr += (v7 * sizeof(int) * 8 * 16) + (v8 * sizeof(int) * 16);
    int *final_ptr = (int *)intptr;
    
    /* Mix integer and FP operations to pressure different reg classes */
    for (volatile int i = 0; i < 100; i++) {
        local3 = local3 * 1.01 + (double)local1;
        local1 = local1 * 3 - local2;
        local2 = local2 + (*final_ptr % 256);
        COMPILER_BARRIER();
    }
    
    /* Cleanup */
    free(int_ptr_ptr);
    free(triple_ptr);
    
    /* Return volatile sum to prevent elimination */
    return final_result + local1 + (int)local3 + asm_result1 + (int)asm_addr;
}

/* Main function with volatile initialization */
int main(void) {
    /* Initialize with random values to prevent constant propagation */
    volatile int v1 = rand() % 100;
    volatile int v2 = rand() % 100;
    volatile int v3 = rand() % 100;
    volatile int v4 = rand() % 100;
    volatile int v5 = rand() % 100;
    volatile int v6 = rand() % 100;
    volatile long v7 = rand() % 100;
    volatile long v8 = rand() % 100;
    volatile double v9 = (double)(rand() % 100) / 10.0;
    
    /* Call the complex function multiple times */
    int total = 0;
    for (int i = 0; i < 10; i++) {
        total += trigger_reloads(v1 + i, v2, v3, v4, v5, v6, v7, v8, v9);
        COMPILER_BARRIER();
    }
    
    printf("Result: %d\n", total);
    return 0;
}
