/* reload_coverage.c - Program to trigger multiple reload types in GCC's reload pass */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force function to not be inlined or optimized away */
__attribute__((noinline, noipa))
static int trigger_reloads(
    volatile int idx1, volatile int idx2, volatile int idx3,
    volatile int stride1, volatile int stride2,
    volatile double scale1, volatile double scale2,
    volatile long offset1, volatile long offset2)
{
    /* Create high register pressure with many live values */
    int local1 = idx1 * 2;
    int local2 = idx2 + 7;
    int local3 = idx3 - 3;
    double local4 = scale1 * 2.5;
    double local5 = scale2 / 1.7;
    int local6 = stride1 ^ 0xFF;
    int local7 = stride2 << 2;
    long local8 = offset1 + 1000;
    long local9 = offset2 - 500;
    
    /* Multi-dimensional arrays to force memory addressing reloads */
    int arr3d[10][10][10];
    double dbl_arr[20][15];
    char byte_grid[50][40];
    
    /* Complex pointer chains for address reloads */
    int *ptr1 = &arr3d[0][0][0];
    int **ptr2 = (int **)&ptr1;
    int ***ptr3 = (int ***)&ptr2;
    
    /* Compiler barrier to prevent optimization */
    __asm__ volatile("" : : : "memory");
    
    /* RELOAD_FOR_INPUT_ADDRESS: Complex array addressing with volatile indices */
    for (int i = 0; i < 5; i++) {
        /* Multi-level array access with volatile components */
        arr3d[idx1 % 10][(idx2 + i) % 10][(idx3 * stride1) % 10] = 
            local1 + local2 * (i + stride2);
        
        /* Mixed-type addressing */
        dbl_arr[(idx1 + stride1) % 20][(idx2 * 2) % 15] = 
            local4 * (i + 1) + local5;
    }
    
    /* RELOAD_FOR_OUTPUT_ADDRESS: Complex address computation for stores */
    int * volatile out_ptr = &arr3d[idx1 % 10][idx2 % 10][0];
    for (int i = 0; i < 8; i++) {
        /* Address calculation with multiple volatile components */
        *(out_ptr + (idx3 + stride2 * i) % 10) = 
            local3 + local6 * (i ^ local7);
    }
    
    /* RELOAD_FOR_OPERAND_ADDRESS: Inline assembly with memory operand */
    int asm_result1, asm_result2;
    __asm__ volatile (
        "movl (%1), %0\n\t"
        "addl $42, %0"
        : "=r" (asm_result1)
        : "r" (&local1)
        : "cc"
    );
    
    /* RELOAD_FOR_OUTADDR_ADDRESS: Assembly with output address */
    int output_target;
    __asm__ volatile (
        "leal (%1, %2, 4), %0\n\t"
        "movl $0xDEADBEEF, (%0)"
        : "=&r" (output_target)
        : "r" (&arr3d[0][0][0]), "r" (idx1)
        : "memory"
    );
    
    /* RELOAD_FOR_INPADDR_ADDRESS: Assembly with input address computation */
    int *addr_temp;
    __asm__ volatile (
        "leal (%1, %2, 2), %0"
        : "=r" (addr_temp)
        : "r" (&byte_grid[0][0]), "r" (stride1 * 40 + stride2)
    );
    
    /* Use the computed address */
    *addr_temp = local1 & 0xFF;
    
    /* Mixed register class pressure - integer and floating point */
    double fp_sum = 0.0;
    for (int i = 0; i < 12; i++) {
        /* Integer to float conversion forces register class moves */
        fp_sum += (double)arr3d[i % 10][0][0] * scale1;
        fp_sum -= (double)local2 * scale2;
        
        /* Float to integer conversion */
        local1 += (int)(fp_sum * 10.0);
    }
    
    /* RELOAD_FOR_OTHER_ADDRESS: Pointer chasing with volatile offsets */
    char *char_ptr = (char *)&byte_grid[0][0];
    for (int i = 0; i < 25; i++) {
        /* Complex address calculation */
        char_ptr += (offset1 % 40) + (offset2 % 3);
        *char_ptr = (char)(local3 + i);
        
        /* Another level of indirection */
        int *int_ptr = (int *)(char_ptr + (stride1 % 4));
        *int_ptr = local6 ^ local7;
    }
    
    /* RELOAD_OTHER: Mixed operations causing various reloads */
    {
        /* Force spills with many simultaneous live values */
        int temp1 = local1 * local2;
        int temp2 = local3 / (local6 + 1);
        double temp3 = local4 * local5;
        long temp4 = local8 ^ local9;
        
        /* Use all temporaries in complex expression */
        int final_result = temp1 + temp2 + (int)temp3 + (int)temp4;
        
        /* Compiler barrier to extend live ranges */
        __asm__ volatile("" : : "r"(temp1), "r"(temp2), "r"(temp3), "r"(temp4) : "memory");
        
        return final_result + asm_result1 + output_target;
    }
}

int main(void) {
    srand(time(NULL));
    
    /* Initialize volatile variables with random values */
    volatile int idx1 = rand() % 100;
    volatile int idx2 = rand() % 100;
    volatile int idx3 = rand() % 100;
    volatile int stride1 = rand() % 10 + 1;
    volatile int stride2 = rand() % 10 + 1;
    volatile double scale1 = (rand() % 100) / 10.0;
    volatile double scale2 = (rand() % 100) / 10.0;
    volatile long offset1 = rand() % 1000;
    volatile long offset2 = rand() % 1000;
    
    printf("Testing reload coverage...\n");
    
    int result = trigger_reloads(idx1, idx2, idx3, stride1, stride2,
                                scale1, scale2, offset1, offset2);
    
    printf("Result: %d\n", result);
    printf("Reload test completed.\n");
    
    return 0;
}
