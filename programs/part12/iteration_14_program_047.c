#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define FORCE_SPILL __asm__ volatile("" : : : "memory")

/* Force no optimization/inlining */
__attribute__((noinline, noipa))
static int trigger_reloads(
    volatile int v1, volatile int v2, volatile int v3,
    volatile int v4, volatile int v5, volatile int v6,
    volatile int v7, volatile int v8, volatile int v9,
    volatile int v10, volatile int v11, volatile int v12)
{
    /* Create high register pressure with mixed types */
    int local1 = v1 * 2;
    int local2 = v2 + v3;
    double f1 = (double)v4 * 1.5;
    double f2 = (double)v5 * 2.5;
    long long ll1 = (long long)v6 * v7;
    long long ll2 = (long long)v8 * v9;
    
    /* Multi-dimensional arrays with volatile indices */
    int arr3d[4][8][16];
    double darr[32][8];
    char carr[128][64];
    
    /* Complex pointer chains */
    int *ptr1 = &arr3d[0][0][0];
    int **ptr2 = (int**)&ptr1;
    int ***ptr3 = (int***)&ptr2;
    
    /* Force spills with many live values */
    FORCE_SPILL;
    
    /* RELOAD_FOR_INPUT_ADDRESS: Complex addressing mode 1 */
    int idx1 = v1 % 4;
    int idx2 = v2 % 8;
    int idx3 = v3 % 16;
    int offset = v4 % 128;
    
    /* Multi-level array access with volatile components */
    int val1 = arr3d[idx1][idx2 + v5 % 4][idx3 * 2 + v6 % 8];
    FORCE_SPILL;
    
    /* RELOAD_FOR_OUTPUT_ADDRESS: Store with complex address */
    arr3d[v7 % 3][v8 % 7][v9 % 15] = local1 + local2 + v10;
    
    /* Mixed type operations to pressure different register classes */
    f1 = f1 * f2 + (double)val1;
    int int_from_float = (int)(f1 * 100.0);
    
    /* RELOAD_FOR_OPERAND_ADDRESS: Inline assembly with memory operand */
    int asm_result1, asm_result2;
    __asm__ volatile (
        "movl (%1), %0\n\t"
        "addl $42, %0"
        : "=r" (asm_result1)
        : "r" (&arr3d[idx1][idx2][idx3])
        : "memory"
    );
    
    FORCE_SPILL;
    
    /* RELOAD_FOR_INPADDR_ADDRESS: Assembly with input address computation */
    int * volatile volatile_ptr = &arr3d[0][0][0];
    __asm__ volatile (
        "leal (%1, %2, 4), %0\n\t"
        "movl (%0), %0"
        : "=r" (asm_result2)
        : "r" (volatile_ptr), "r" (v11)
        : "memory"
    );
    
    /* RELOAD_FOR_OUTADDR_ADDRESS: Assembly with output address */
    int out_addr;
    __asm__ volatile (
        "movl %%eax, %0\n\t"
        : "=m" (out_addr)
        : 
        : "%eax", "memory"
    );
    
    /* Complex pointer arithmetic with different types */
    char *cptr = (char*)carr[v1 % 128];
    int *iptr = (int*)(cptr + v2 * sizeof(int) + v3);
    double *dptr = &darr[v4 % 32][v5 % 8];
    
    /* Chain dereferences */
    int chain_val = *(*(*(ptr3)));
    FORCE_SPILL;
    
    /* Multi-dimensional access with runtime computation */
    for (int i = 0; i < v6 % 4; i++) {
        for (int j = 0; j < v7 % 8; j++) {
            /* Complex address calculation */
            darr[i][j] = (double)arr3d[i][j][v8 % 16] * f1;
            
            /* More pointer arithmetic */
            char *tmp = carr[i * 32 + j] + v9 % 64;
            *tmp = (char)(darr[i][j] * 10.0);
        }
    }
    
    /* RELOAD_FOR_OTHER_ADDRESS: Multiple address computations in one expr */
    int complex_addr_result = 
        arr3d[v1 % 4][v2 % 8][v3 % 16] +
        *(int*)((char*)&arr3d[v4 % 4][v5 % 8][0] + v6 * sizeof(int)) +
        **(int**)(&ptr2 + v7 % 2);
    
    /* RELOAD_OTHER: Force other reloads with register constraints */
    int constrained1, constrained2;
    __asm__ volatile (
        "movl %2, %0\n\t"
        "addl %3, %0\n\t"
        "movl %0, %1"
        : "=&r" (constrained1), "=m" (constrained2)
        : "r" (v10), "r" (v11)
        : "memory"
    );
    
    /* Final computation using all values */
    int result = 
        local1 + local2 + 
        int_from_float + 
        asm_result1 + asm_result2 +
        chain_val + complex_addr_result +
        constrained1 + (int)f1 + (int)f2 +
        (int)ll1 % 1000 + (int)ll2 % 1000;
    
    FORCE_SPILL;
    
    return result + v12;
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
    volatile int v7 = rand() % 100;
    volatile int v8 = rand() % 100;
    volatile int v9 = rand() % 100;
    volatile int v10 = rand() % 100;
    volatile int v11 = rand() % 100;
    volatile int v12 = rand() % 100;
    
    /* Call function to trigger reloads */
    int result = trigger_reloads(v1, v2, v3, v4, v5, v6, 
                                 v7, v8, v9, v10, v11, v12);
    
    printf("Result: %d\n", result);
    
    return 0;
}
