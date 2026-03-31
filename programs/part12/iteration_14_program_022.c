/* reload_coverage.c - Complex program to trigger GCC reload pass coverage */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent constant propagation */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile long vl1 = 100, vl2 = 200, vl3 = 300;
volatile double vd1 = 1.5, vd2 = 2.5;

/* Force no optimization/inlining on the target function */
__attribute__((noinline, noipa, optimize("O0")))
int trigger_reloads(volatile int idx1, volatile int idx2, 
                    volatile long off1, volatile long off2,
                    volatile double scale1, volatile double scale2) {
    /* Create high register pressure with many live values */
    int local1 = idx1 * 2;
    int local2 = idx2 + 100;
    int local3 = local1 ^ local2;
    double dlocal1 = scale1 * 2.0;
    double dlocal2 = scale2 / 3.0;
    long llocal1 = off1 << 2;
    long llocal2 = off2 >> 1;
    
    /* Multi-dimensional arrays with volatile indexing */
    int arr3d[10][10][10];
    double darr2d[20][20];
    char carr4d[5][5][5][5];
    
    /* Complex pointer chains */
    int *ptr1 = &arr3d[0][0][0];
    int **ptr2 = (int **)&ptr1;
    int ***ptr3 = (int ***)&ptr2;
    
    /* Force memory barriers to extend live ranges */
    __asm__ volatile("" : : : "memory");
    
    /* RELOAD_FOR_INPUT_ADDRESS: Complex array addressing */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            /* Multi-level array access with volatile indices */
            carr4d[idx1 % 5][idx2 % 5][i][j] = 
                arr3d[(i + idx1) % 10][(j + idx2) % 10][(i * j) % 10] + 
                (int)(darr2d[i][j] * scale1);
        }
    }
    
    __asm__ volatile("" : : : "memory");
    
    /* RELOAD_FOR_OUTPUT_ADDRESS: Complex pointer arithmetic */
    long complex_offset = off1 * 8 + off2 * 4 + idx1 * 2 + idx2;
    int * volatile_ptr = (int *)((uintptr_t)ptr1 + complex_offset);
    
    /* Mixed register class pressure */
    for (int i = 0; i < 10; i++) {
        /* Integer and floating-point computations interleaved */
        darr2d[i][i] = dlocal1 * i + dlocal2;
        arr3d[i][i % 10][i % 10] = (int)darr2d[i][i] + local1 + local2;
        
        /* More complex addressing */
        for (int j = 0; j < 10; j++) {
            /* RELOAD_FOR_INPADDR_ADDRESS: Address of address computation */
            int *addr_of_elem = &arr3d[(i + idx1) % 10][(j + idx2) % 10][0];
            *addr_of_elem += *((int *)((uintptr_t)addr_of_elem + off1));
        }
    }
    
    __asm__ volatile("" : : : "memory");
    
    /* Inline assembly to force specific reload types */
    int asm_result1, asm_result2;
    long asm_addr;
    
    /* RELOAD_FOR_OPERAND_ADDRESS: Assembly with memory operand */
    __asm__ volatile(
        "mov %[base], %[out1]\n\t"
        "add %[offset], %[out1]\n\t"
        "mov (%[out1]), %[out2]"
        : [out1] "=&r" (asm_addr), [out2] "=r" (asm_result1)
        : [base] "r" (ptr1), [offset] "r" (off1)
        : "memory"
    );
    
    /* RELOAD_FOR_OUTADDR_ADDRESS: Assembly that computes output address */
    __asm__ volatile(
        "lea (%[base], %[idx], 4), %[out]\n\t"
        "movl $42, (%[out])"
        : [out] "=&r" (asm_result2)
        : [base] "r" (ptr1), [idx] "r" (idx1)
        : "memory"
    );
    
    /* RELOAD_FOR_OTHER_ADDRESS: More complex pointer indirection */
    int ****ptr4 = (int ****)malloc(sizeof(int ***) * 5);
    for (int i = 0; i < 5; i++) {
        ptr4[i] = (int ***)&ptr3;
    }
    
    /* Access through multiple pointer levels */
    int final_value = ****ptr4[idx1 % 5];
    
    /* RELOAD_OTHER: Mixed operations causing various reloads */
    double mixed_result = 0.0;
    for (int i = 0; i < 100; i++) {
        /* Force spills across register classes */
        mixed_result += (double)arr3d[i % 10][(i / 10) % 10][0] * dlocal1;
        mixed_result -= darr2d[i % 20][(i * 2) % 20] * dlocal2;
        
        /* Integer operations keeping pressure */
        local1 = (local1 * 1103515245 + 12345) & 0x7fffffff;
        local2 ^= local1;
        local3 += local2;
    }
    
    /* Complex return expression to keep everything alive */
    return (int)mixed_result + local1 + local2 + local3 + 
           asm_result1 + asm_result2 + final_value + 
           *volatile_ptr + carr4d[0][0][0][0];
}

int main(void) {
    /* Initialize with random values for unpredictable addressing */
    srand(42);
    
    volatile int idx1 = rand() % 100;
    volatile int idx2 = rand() % 100;
    volatile long off1 = rand() % 1000;
    volatile long off2 = rand() % 1000;
    volatile double scale1 = (double)(rand() % 100) / 10.0;
    volatile double scale2 = (double)(rand() % 100) / 10.0;
    
    /* Call the function multiple times with different values */
    int total = 0;
    for (int i = 0; i < 10; i++) {
        idx1 = (idx1 * 3 + 7) % 97;
        idx2 = (idx2 * 5 + 11) % 89;
        total += trigger_reloads(idx1, idx2, off1 + i, off2 - i, 
                                scale1 + i, scale2 - i);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
