/* reload_coverage.c - Program to trigger GCC reload pass uncovered lines */
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
    /* Create high register pressure with many live values */
    int local1 = v1 * 2;
    int local2 = v2 + v3;
    int local3 = v4 ^ v5;
    int local4 = v6 << 2;
    long local5 = v7 + v8;
    double local6 = (double)v9 * 3.14;
    
    /* Multi-dimensional array access with volatile indices */
    int arr3d[10][10][10];
    double darr[20][20];
    
    /* Complex pointer chains for address reloads */
    int *ptr1 = &arr3d[0][0][0];
    int **ptr2 = (int **)&ptr1;
    int ***ptr3 = (int ***)&ptr2;
    
    /* Force RELOAD_FOR_INPUT_ADDRESS with complex addressing */
    for (int i = 0; i < v1 % 5; i++) {
        for (int j = 0; j < v2 % 5; j++) {
            /* Multi-level array access with volatile offset */
            arr3d[i][j][v3 % 10] = 
                *(*(*(ptr3) + i) + j * 10 + (v4 % 10));
        }
    }
    
    /* Memory barrier to prevent optimization */
    __asm__ volatile("" : : : "memory");
    
    /* Mixed register class pressure */
    double fp_sum = 0.0;
    for (int i = 0; i < v5 % 8; i++) {
        for (int j = 0; j < v6 % 8; j++) {
            /* Floating point computation */
            darr[i][j] = (double)arr3d[i][j][0] * local6;
            fp_sum += darr[i][j];
            
            /* Integer computation interleaved */
            local1 += arr3d[j][i][v7 % 5];
        }
    }
    
    /* Inline assembly to force specific reload types */
    int asm_result1, asm_result2;
    long asm_result3;
    
    /* RELOAD_FOR_OPERAND_ADDRESS - memory operand */
    __asm__ volatile(
        "movl (%1), %0\n\t"
        : "=r" (asm_result1)
        : "r" (&local1)
        : "memory"
    );
    
    /* RELOAD_FOR_INPUT_ADDRESS with complex constraint */
    __asm__ volatile(
        "leaq (%1, %2, 4), %0\n\t"
        : "=r" (asm_result3)
        : "r" (&arr3d[0][0][0]), "r" (v8)
        : "cc"
    );
    
    /* RELOAD_FOR_OUTPUT_ADDRESS - output in memory */
    __asm__ volatile(
        "movl %1, (%0)\n\t"
        : 
        : "r" (&local2), "r" (asm_result1)
        : "memory"
    );
    
    /* More complex addressing with multiple volatile indices */
    int * volatile_ptr = (int *)((char *)ptr1 + v1 * sizeof(int));
    int ** double_ptr = &volatile_ptr;
    
    /* Chain dereferencing for address computation reloads */
    int chain_val = ***ptr3 + **(double_ptr) + 
                    arr3d[v2 % 10][v3 % 10][v4 % 10] +
                    *(int *)((char *)&darr[0][0] + v5 * sizeof(double));
    
    /* Force RELOAD_FOR_OTHER_ADDRESS with unusual addressing */
    struct {
        int a[5];
        double b[3];
        char c[7];
    } mixed;
    
    /* Cross-type pointer arithmetic */
    char *byte_ptr = (char *)&mixed;
    int *int_ptr = (int *)(byte_ptr + v6);
    double *dbl_ptr = (double *)(byte_ptr + v7);
    
    /* Access through computed pointers */
    *int_ptr = chain_val;
    *dbl_ptr = fp_sum;
    
    /* Final computation using all values to keep them live */
    int final_result = local1 + local2 + local3 + local4 + 
                      (int)local5 + (int)local6 + 
                      asm_result1 + (int)asm_result3 + 
                      chain_val + (int)fp_sum;
    
    /* Another memory barrier */
    __asm__ volatile("" : : : "memory");
    
    return final_result + v1 + v2 + v3 + v4 + v5 + v6;
}

/* Helper to create more register pressure */
__attribute__((noinline))
static double compute_fp(volatile double a, volatile double b, 
                         volatile double c, volatile double d)
{
    /* Force floating point register pressure */
    double t1 = a * b;
    double t2 = c / d;
    double t3 = t1 + t2;
    double t4 = t3 - a;
    double t5 = t4 * b;
    double t6 = t5 / c;
    double t7 = t6 + d;
    double t8 = t7 - t1;
    double t9 = t8 * t2;
    double t10 = t9 / t3;
    
    /* Mix with integer computation */
    int i1 = (int)t1;
    int i2 = (int)t2;
    int i3 = i1 * i2;
    
    return t10 + (double)i3;
}

int main(void)
{
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
    
    /* Create additional floating point pressure */
    volatile double f1 = (double)(rand() % 100) / 5.0;
    volatile double f2 = (double)(rand() % 100) / 7.0;
    volatile double f3 = (double)(rand() % 100) / 3.0;
    volatile double f4 = (double)(rand() % 100) / 9.0;
    
    /* Call functions to create register pressure */
    double fp_result = compute_fp(f1, f2, f3, f4);
    
    /* Main function with complex addressing */
    int result = trigger_reloads(v1, v2, v3, v4, v5, v6, v7, v8, v9);
    
    /* Use results to prevent optimization */
    printf("Result: %d (FP: %f)\n", result + (int)fp_result, fp_result);
    
    return 0;
}
