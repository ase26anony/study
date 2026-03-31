/* reload_coverage.c - Complex program to trigger GCC reload pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
__attribute__((noinline, noipa))
static int trigger_reloads(
    volatile int v1, volatile int v2, volatile int v3,
    volatile int v4, volatile int v5, volatile int v6,
    volatile long v7, volatile long v8, volatile double v9)
{
    /* Create high register pressure with mixed types */
    int local1 = v1 * 2;
    int local2 = v2 + v3;
    double local3 = (double)v4 * v9;
    long local4 = v7 ^ v8;
    int local5 = v5 - v6;
    double local6 = local3 * 2.5;
    
    /* Multi-dimensional arrays with volatile indexing */
    int arr3d[10][10][10];
    double dbl_arr[20][15];
    char char_arr[100][50];
    
    /* Complex pointer chains */
    int *ptr1 = &arr3d[0][0][0];
    int **ptr2 = (int **)&ptr1;
    int ***ptr3 = (int ***)&ptr2;
    
    /* Force memory barriers to extend live ranges */
    __asm__ volatile("" : : : "memory");
    
    /* RELOAD_FOR_INPUT_ADDRESS: Complex address calculation */
    for (volatile int i = 0; i < v1 % 5; i++) {
        for (volatile int j = 0; j < v2 % 5; j++) {
            for (volatile int k = 0; k < v3 % 5; k++) {
                /* Multi-level array access with volatile indices */
                arr3d[i + v1 % 3][j * v2 % 10][k + v3 % 7] = 
                    local1 + local2 + (int)local3;
            }
        }
    }
    
    __asm__ volatile("" : : : "memory");
    
    /* RELOAD_FOR_OUTPUT_ADDRESS: Complex store address */
    volatile int store_idx = v4 % 20;
    volatile int store_jdx = v5 % 15;
    
    /* Mixed type pointer arithmetic */
    char *char_ptr = (char *)&dbl_arr[store_idx][store_jdx];
    int *int_ptr = (int *)(char_ptr + v6 * sizeof(double));
    
    /* Inline assembly forcing address reloads */
    int asm_result1, asm_result2;
    
    /* RELOAD_FOR_OPERAND_ADDRESS: Address as assembly operand */
    __asm__ volatile(
        "movl (%1), %0\n\t"
        "addl $1, %0"
        : "=r" (asm_result1)
        : "r" (int_ptr)
        : "memory"
    );
    
    /* RELOAD_FOR_INPADDR_ADDRESS: Input address computation */
    long *long_ptr = (long *)((char *)arr3d + v7 * 100 + v8 * 10);
    
    __asm__ volatile(
        "movq (%1), %0\n\t"
        "imulq $2, %0"
        : "=r" (local4)
        : "r" (long_ptr)
        : "memory"
    );
    
    /* More complex addressing with different scales */
    double *dbl_ptr1 = &dbl_arr[v1 % 20][v2 % 15];
    double *dbl_ptr2 = dbl_ptr1 + v3 * 2;
    
    /* Mixed integer/float operations */
    for (volatile int i = 0; i < 5; i++) {
        dbl_ptr2[i] = (double)arr3d[i][i % 10][i % 5] * 3.14159;
    }
    
    __asm__ volatile("" : : : "memory");
    
    /* RELOAD_FOR_OUTADDR_ADDRESS: Output address in assembly */
    int output_buffer[10];
    volatile int out_idx = v4 % 10;
    
    __asm__ volatile(
        "leal (%1, %2, 4), %0\n\t"
        "movl %%eax, (%0)"
        : "=r" (asm_result2)
        : "r" (output_buffer), "r" (out_idx)
        : "eax", "memory"
    );
    
    /* Complex pointer chain dereferencing */
    int chain_value = 0;
    if (v5 > 0) {
        int *temp = *ptr2;
        int **temp2 = *ptr3;
        chain_value = **temp2 + *temp;
    }
    
    /* RELOAD_FOR_OTHER_ADDRESS: Other address computations */
    struct {
        int a[5];
        double b[3];
        char c[10];
    } mixed_struct;
    
    /* Access with complex offset */
    volatile int struct_idx = v6 % 5;
    mixed_struct.a[struct_idx] = chain_value;
    mixed_struct.b[struct_idx % 3] = local6;
    
    /* Pointer arithmetic with different types */
    char *base = (char *)&mixed_struct;
    int *int_at_offset = (int *)(base + v7 * 8 + v8 * 4);
    
    /* Final computation using all values */
    int result = local1 + local2 + local5 + 
                 (int)local3 + (int)local6 + 
                 chain_value + asm_result1 + asm_result2 +
                 *int_at_offset + arr3d[0][0][0];
    
    /* Force one more memory barrier */
    __asm__ volatile("" : : : "memory");
    
    return result;
}

int main(void) {
    srand(time(NULL));
    
    /* Initialize volatile variables with random values */
    volatile int v1 = rand() % 100;
    volatile int v2 = rand() % 100;
    volatile int v3 = rand() % 100;
    volatile int v4 = rand() % 100;
    volatile int v5 = rand() % 100;
    volatile int v6 = rand() % 100;
    volatile long v7 = rand() % 100;
    volatile long v8 = rand() % 100;
    volatile double v9 = (double)(rand() % 100) / 10.0;
    
    printf("Initial values: v1=%d, v2=%d, v3=%d, v4=%d, v5=%d, v6=%d, "
           "v7=%ld, v8=%ld, v9=%.2f\n",
           v1, v2, v3, v4, v5, v6, v7, v8, v9);
    
    /* Call the function multiple times with different values */
    int total = 0;
    for (int i = 0; i < 3; i++) {
        int result = trigger_reloads(v1 + i, v2 + i, v3 + i,
                                    v4 + i, v5 + i, v6 + i,
                                    v7 + i, v8 + i, v9 + i);
        total += result;
        printf("Iteration %d: result = %d\n", i, result);
    }
    
    printf("Total: %d\n", total);
    
    return 0;
}
