#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force no optimization across this function */
__attribute__((noinline, noipa))
static int trigger_reloads(
    volatile int v1, volatile int v2, volatile int v3,
    volatile int v4, volatile int v5, volatile int v6,
    volatile int v7, volatile int v8, volatile int v9,
    volatile int v10, volatile int v11, volatile int v12)
{
    /* Create high register pressure with many live values */
    int local1 = v1 * 2;
    int local2 = v2 + v3;
    double local3 = (double)v4 * 1.5;
    int local4 = v5 ^ v6;
    double local5 = (double)v7 / 3.0;
    int local6 = v8 << 2;
    int local7 = v9 >> 1;
    double local8 = (double)v10 * 2.71828;
    int local9 = v11 % 13;
    int local10 = v12 * 3;
    
    /* Multi-dimensional array access with volatile indices */
    int arr3d[10][10][10];
    double darr2d[20][20];
    
    /* Complex pointer chains for address reloads */
    int *ptr1 = &arr3d[v1][v2][0];
    int *ptr2 = ptr1 + v3;
    int **ptr3 = &ptr2;
    int ***ptr4 = &ptr3;
    
    /* Force memory barrier to extend live ranges */
    __asm__ volatile("" : : : "memory");
    
    /* Complex addressing mode: base + index*scale + displacement */
    for (volatile int i = 0; i < 5; i++) {
        /* RELOAD_FOR_INPUT_ADDRESS: complex array addressing */
        arr3d[v1 + i][v2 * 2][v3 % 10] = 
            arr3d[v4 % 10][v5 + i * 3][v6] + 
            arr3d[v7][v8][v9 + i];
        
        /* Mixed register class pressure */
        darr2d[v10 + i][v11] = 
            (double)arr3d[v1][v2 + i][v3] * 
            local3 + 
            local8;
    }
    
    /* Inline assembly to force specific reload types */
    int asm_result1, asm_result2;
    double asm_result3;
    
    /* RELOAD_FOR_OPERAND_ADDRESS: address as input to asm */
    __asm__ volatile(
        "movl (%1), %0\n\t"
        : "=r" (asm_result1)
        : "r" (&arr3d[v1][v2][v3])
        : "memory"
    );
    
    /* RELOAD_FOR_OUTPUT_ADDRESS: address computation for output */
    __asm__ volatile(
        "leal (%1, %2, 4), %0\n\t"
        : "=r" (asm_result2)
        : "r" (ptr1), "r" (v4)
        : "cc"
    );
    
    /* RELOAD_FOR_INPADDR_ADDRESS: complex address input */
    __asm__ volatile(
        "movsd (%1), %%xmm0\n\t"
        "cvtsd2si %%xmm0, %0\n\t"
        : "=r" (asm_result2)
        : "r" (&darr2d[v5][v6] + v7 * sizeof(double))
        : "xmm0"
    );
    
    /* More complex pointer arithmetic */
    long *lptr = (long*)((char*)ptr4 + v8 * 8);
    int *final_ptr = (int*)(*(*(ptr4)) + v9 * 4 + v10);
    
    /* RELOAD_FOR_OUTADDR_ADDRESS: output address reload */
    int *out_addr;
    __asm__ volatile(
        "leal (%1, %2, 2), %0\n\t"
        : "=r" (out_addr)
        : "r" (final_ptr), "r" (v11)
        : "cc"
    );
    
    /* Use the computed address */
    *out_addr = v12;
    
    /* Force floating-point operations to pressure FP registers */
    for (volatile int i = 0; i < 8; i++) {
        local3 = local3 * 1.1 + local5;
        local5 = local5 / 1.05 - local8;
        local8 = local8 * 0.99 + local3;
        
        /* Integer operations to pressure integer registers */
        local1 = local1 + local2 * i;
        local2 = local2 ^ local4;
        local4 = local4 | local6;
        local6 = local6 & local7;
        local7 = local7 + local9;
        local9 = local9 - local10;
        local10 = local10 * (i + 1);
    }
    
    /* Another memory barrier */
    __asm__ volatile("" : : : "memory");
    
    /* Complex final computation using all values */
    int result = 
        local1 + local2 + local4 + local6 + 
        local7 + local9 + local10 + 
        (int)local3 + (int)local5 + (int)local8 +
        asm_result1 + asm_result2 + *out_addr +
        arr3d[v1][v2][v3] + 
        (int)darr2d[v4][v5];
    
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
    volatile int v7 = rand() % 100;
    volatile int v8 = rand() % 100;
    volatile int v9 = rand() % 100;
    volatile int v10 = rand() % 100;
    volatile int v11 = rand() % 100;
    volatile int v12 = rand() % 100;
    
    /* Call the function that should trigger reloads */
    int result = trigger_reloads(v1, v2, v3, v4, v5, v6,
                                v7, v8, v9, v10, v11, v12);
    
    printf("Result: %d\n", result);
    
    return 0;
}
