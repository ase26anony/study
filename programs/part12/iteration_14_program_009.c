/* reload_coverage.c - Complex program to trigger GCC reload pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent optimizations that would simplify addressing */
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Force register pressure and complex addressing */
__attribute__((noinline, noipa))
static int trigger_reloads(
    volatile int idx1, volatile int idx2, volatile int idx3,
    volatile int stride1, volatile int stride2,
    volatile double scale1, volatile double scale2,
    volatile long offset1, volatile long offset2)
{
    /* Create high register pressure with mixed types */
    int local1 = idx1 * 2;
    int local2 = idx2 + idx3;
    double local3 = scale1 * 2.0;
    double local4 = scale2 / 3.0;
    
    /* Multi-dimensional arrays forcing spill decisions */
    int arr3d[10][10][10];
    double dbl_arr[20][20];
    char char_arr[100][50];
    
    /* Complex pointer chains */
    int *ptr1 = &arr3d[0][0][0];
    int **ptr2 = (int **)&ptr1;
    double *dptr1 = &dbl_arr[0][0];
    double **dptr2 = &dptr1;
    
    /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
    /* Multi-level addressing with volatile indices */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            /* Complex 3D array access - forces address computation reloads */
            arr3d[idx1 + i][idx2 + j][idx3] = 
                (int)(dbl_arr[idx2 + i][idx1 + j] * scale1);
            
            /* Pointer arithmetic with mixed scales */
            char_arr[(i * stride1) + (j * stride2)][idx3] = 
                (char)(arr3d[i][j][0] & 0xFF);
        }
    }
    
    COMPILER_BARRIER();
    
    /* Inline assembly to force specific reload types */
    int asm_result1, asm_result2;
    double asm_dbl_result;
    
    /* RELOAD_FOR_OPERAND_ADDRESS - address as input to asm */
    __asm__ volatile(
        "movl (%[addr]), %[out]\n\t"
        : [out] "=r" (asm_result1)
        : [addr] "r" (&arr3d[idx1][idx2][idx3])
        : "memory"
    );
    
    /* RELOAD_FOR_OUTADDR_ADDRESS - address computation for output */
    __asm__ volatile(
        "movl %[in], (%[addr])\n\t"
        : 
        : [in] "r" (asm_result1), [addr] "r" (&arr3d[idx2][idx1][idx3] + offset1)
        : "memory"
    );
    
    /* Mixed register class pressure - integer and FP */
    for (int i = 0; i < 10; i++) {
        /* Convert int to double and back - forces moves between reg classes */
        double temp = (double)arr3d[i][0][0];
        temp = temp * scale1 + scale2;
        dbl_arr[i][0] = temp;
        
        /* More complex addressing */
        int * volatile ptr = &arr3d[i][idx1 % 10][idx2 % 10];
        *ptr = (int)(temp * 100.0);
    }
    
    COMPILER_BARRIER();
    
    /* Additional complex addressing modes */
    /* Base + index * scale + displacement */
    long *lptr = (long *)&arr3d[0][0][0];
    for (long i = 0; i < 50; i++) {
        /* Force address reload with different scales */
        lptr[i * 2 + offset1] = (long)(dbl_arr[i % 20][0] * 1000.0);
    }
    
    /* RELOAD_FOR_INPADDR_ADDRESS - address of input address */
    int *addr_of_addr = &arr3d[idx1][idx2][0];
    __asm__ volatile(
        "movl (%[addr_ptr]), %[out]\n\t"
        "addl $1, %[out]\n\t"
        : [out] "=&r" (asm_result2)
        : [addr_ptr] "r" (&addr_of_addr)
        : "memory"
    );
    
    /* RELOAD_FOR_OTHER_ADDRESS */
    /* Complex pointer chain dereference */
    int ***ptr3 = &ptr2;
    int result = ***ptr3;
    
    /* Mix computations to keep values live */
    local1 += asm_result1;
    local2 += asm_result2;
    local3 += (double)result;
    local4 += dbl_arr[idx1 % 20][idx2 % 20];
    
    /* Final complex expression with multiple address computations */
    return (int)(local1 + local2 + local3 + local4 + 
                 arr3d[idx1 % 10][idx2 % 10][idx3 % 10] +
                 char_arr[idx1 % 100][idx2 % 50] +
                 (int)(dbl_arr[idx3 % 20][0] * 10.0));
}

int main(void) {
    /* Initialize with random values to prevent constant propagation */
    volatile int idx1 = rand() % 5;
    volatile int idx2 = rand() % 5;
    volatile int idx3 = rand() % 5;
    volatile int stride1 = rand() % 10 + 1;
    volatile int stride2 = rand() % 10 + 1;
    volatile double scale1 = (double)(rand() % 100) / 10.0;
    volatile double scale2 = (double)(rand() % 100) / 10.0;
    volatile long offset1 = rand() % 100;
    volatile long offset2 = rand() % 100;
    
    /* Call the function multiple times with different values */
    int total = 0;
    for (int i = 0; i < 10; i++) {
        total += trigger_reloads(
            idx1 + i, idx2 + i, idx3 + i,
            stride1 + i, stride2 + i,
            scale1 + i, scale2 + i,
            offset1 + i, offset2 + i
        );
        
        /* Modify volatiles to prevent optimization */
        idx1 += rand() % 3;
        idx2 += rand() % 3;
        scale1 += 0.5;
    }
    
    printf("Result: %d\n", total);
    return 0;
}
