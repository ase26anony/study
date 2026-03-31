/* reload_coverage.c - Program to trigger multiple reload types in GCC's reload pass */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force GCC to generate complex addressing modes */
#define FORCE_SPILL __asm__ volatile("" : : : "memory")

/* Function that will trigger various reload types */
__attribute__((noinline, noipa))
static int trigger_reloads(
    volatile int idx1, volatile int idx2, volatile int idx3,
    volatile int idx4, volatile int idx5, volatile int stride1,
    volatile int stride2, volatile double scale1, volatile double scale2)
{
    /* Create high register pressure with many live values */
    int local1 = idx1 * 3;
    int local2 = idx2 * 5;
    int local3 = idx3 * 7;
    int local4 = idx4 * 11;
    int local5 = idx5 * 13;
    double dlocal1 = scale1 * 2.0;
    double dlocal2 = scale2 * 3.0;
    double dlocal3 = scale1 * scale2;
    
    /* Multi-dimensional arrays to force memory addressing reloads */
    int arr3d[10][10][10];
    double darr2d[20][20];
    char carr[100][50];
    
    /* Complex pointer chains for address reloads */
    int *ptr1 = &arr3d[0][0][0];
    int **ptr2 = (int **)&ptr1;
    int ***ptr3 = (int ***)&ptr2;
    double *dptr1 = &darr2d[0][0];
    double **dptr2 = &dptr1;
    
    /* Force spills across register classes */
    FORCE_SPILL;
    
    /* RELOAD_FOR_INPUT_ADDRESS: Complex array addressing */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* Complex addressing with volatile indices */
            int *row = arr3d[i][j];
            for (int k = 0; k < 10; k++) {
                /* arr3d[i][j][k] with volatile offset calculation */
                row[k + (idx1 % 5)] = local1 + local2 * (i + j + k);
            }
        }
    }
    
    FORCE_SPILL;
    
    /* RELOAD_FOR_OUTPUT_ADDRESS: Store with complex address */
    double result = 0.0;
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++) {
            /* Complex output address calculation */
            darr2d[(i + idx2) % 20][(j + idx3) % 20] = 
                dlocal1 * i + dlocal2 * j + dlocal3;
            result += darr2d[i][j];
        }
    }
    
    FORCE_SPILL;
    
    /* RELOAD_FOR_INPADDR_ADDRESS: Address of address computation */
    {
        /* Complex pointer arithmetic */
        char *cptr = &carr[idx1 % 50][idx2 % 20];
        char **cptr2 = &cptr;
        char ***cptr3 = &cptr2;
        
        /* Inline assembly that needs address reloads */
        int asm_result;
        __asm__ volatile (
            "mov %[input], %%rax\n\t"
            "mov (%%rax), %%rbx\n\t"
            "add $1, %%rbx\n\t"
            "mov %%rbx, %[output]"
            : [output] "=r" (asm_result)
            : [input] "r" (&local1)
            : "rax", "rbx", "memory"
        );
        local1 = asm_result;
    }
    
    FORCE_SPILL;
    
    /* RELOAD_FOR_OUTADDR_ADDRESS: Output address reload */
    {
        int out_addr_val;
        int *out_addr_ptr = &out_addr_val;
        
        /* Assembly with output address constraint */
        __asm__ volatile (
            "lea (%[base], %[index], 4), %[out]"
            : [out] "=r" (out_addr_ptr)
            : [base] "r" (ptr1), [index] "r" (idx4)
            : "cc"
        );
        
        *out_addr_ptr = local2 + local3;
    }
    
    FORCE_SPILL;
    
    /* RELOAD_FOR_OPERAND_ADDRESS: Multiple address operands */
    {
        long complex_addr1 = (long)ptr1 + idx1 * sizeof(int);
        long complex_addr2 = (long)dptr1 + idx2 * sizeof(double);
        
        /* Mixed type pointer arithmetic */
        int *final_ptr = (int *)(complex_addr1 + complex_addr2);
        *final_ptr = local4;
        
        /* Another inline assembly with complex addressing */
        double asm_double;
        __asm__ volatile (
            "movsd (%[addr1], %[idx], 8), %[out]\n\t"
            "addsd (%[addr2], %[idx], 8), %[out]"
            : [out] "=x" (asm_double)
            : [addr1] "r" (dptr1), [addr2] "r" (&darr2d[10][0]), 
              [idx] "r" ((long)idx3)
            : "memory"
        );
        result += asm_double;
    }
    
    FORCE_SPILL;
    
    /* RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
    {
        /* Complex nested addressing */
        int ****quad_ptr = (int ****)malloc(sizeof(int ***));
        *quad_ptr = ptr3;
        
        /* Multiple levels of indirection */
        int value = ****quad_ptr + idx5;
        
        /* Force address computation across multiple statements */
        int *addr1 = *ptr2 + idx1 * stride1;
        int *addr2 = addr1 + idx2 * stride2;
        int *addr3 = addr2 + idx3;
        
        /* Use all computed addresses */
        *addr1 = value;
        *(addr2 + 1) = value * 2;
        *(addr3 + 2) = value * 3;
        
        free(quad_ptr);
    }
    
    FORCE_SPILL;
    
    /* Mix integer and FP to pressure different register files */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 50; j++) {
            /* Integer to float conversion and back */
            double temp = (double)carr[i][j];
            temp = temp * scale1 + scale2;
            carr[i][j] = (char)((int)temp % 256);
            
            /* More complex addressing */
            if ((i + j) % 2 == 0) {
                int *int_ptr = (int *)&carr[i][j];
                *int_ptr = (*int_ptr + local5) & 0xFF;
            }
        }
    }
    
    /* Return volatile sum to prevent optimization */
    return (int)result + local1 + local2 + local3 + local4 + local5;
}

int main(void)
{
    srand(time(NULL));
    
    /* Volatile variables to prevent constant propagation */
    volatile int v1 = rand() % 100;
    volatile int v2 = rand() % 100;
    volatile int v3 = rand() % 100;
    volatile int v4 = rand() % 100;
    volatile int v5 = rand() % 100;
    volatile int stride1 = rand() % 10 + 1;
    volatile int stride2 = rand() % 10 + 1;
    volatile double scale1 = (double)(rand() % 100) / 10.0;
    volatile double scale2 = (double)(rand() % 100) / 10.0;
    
    printf("Starting complex reload test...\n");
    
    int result = trigger_reloads(v1, v2, v3, v4, v5, 
                                 stride1, stride2, scale1, scale2);
    
    printf("Result: %d\n", result);
    printf("Test completed.\n");
    
    return 0;
}
