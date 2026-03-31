/* reload_coverage.c - Complex program to trigger GCC reload pass switch cases */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force no optimization/inlining to preserve complex addressing */
__attribute__((noinline, noipa, optimize("O0")))
static int trigger_reloads(
    volatile int v1, volatile int v2, volatile int v3,
    volatile int v4, volatile int v5, volatile int v6,
    volatile long v7, volatile long v8, volatile double v9)
{
    /* High register pressure with mixed types */
    int local1 = v1 * 2;
    int local2 = v2 + v3;
    double local3 = (double)v4 * v9;
    long local4 = v7 ^ v8;
    int local5 = v5 - v6;
    double local6 = local3 * 2.5;
    
    /* Multi-dimensional arrays with volatile indexing */
    int arr3d[4][8][16];
    double dbl_arr[32][8];
    char* ptr_arr[12];
    
    /* Complex pointer chains */
    int*** triple_ptr = (int***)malloc(sizeof(int**) * 4);
    for (int i = 0; i < 4; i++) {
        triple_ptr[i] = (int**)malloc(sizeof(int*) * 8);
        for (int j = 0; j < 8; j++) {
            triple_ptr[i][j] = (int*)malloc(sizeof(int) * 16);
        }
    }
    
    /* Volatile indices prevent constant propagation */
    volatile int idx1 = v1 % 4;
    volatile int idx2 = v2 % 8;
    volatile int idx3 = v3 % 16;
    volatile int idx4 = v4 % 32;
    volatile int idx5 = v5 % 8;
    
    /* Complex addressing mode 1: base + index*scale + displacement */
    /* Should trigger RELOAD_FOR_INPUT_ADDRESS */
    int val1 = arr3d[idx1][idx2][idx3 + 2];
    
    /* Complex addressing mode 2: pointer chain dereference */
    /* Should trigger RELOAD_FOR_INPADDR_ADDRESS */
    int val2 = *(*(triple_ptr[idx1] + idx2) + idx3);
    
    /* Mixed register class pressure */
    /* Should trigger RELOAD_FOR_OUTPUT_ADDRESS */
    dbl_arr[idx4][idx5] = (double)val1 * local3 + (double)val2 / local6;
    
    /* Inline assembly with complex constraints */
    /* Should trigger RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    int asm_result1, asm_result2;
    void* addr1 = &arr3d[idx1][idx2][0];
    void* addr2 = &dbl_arr[idx4][0];
    
    __asm__ volatile (
        /* Input: memory address, Output: register */
        "movq (%[in1]), %[out1]\n\t"
        /* Complex address computation in constraints */
        : [out1] "=&r" (asm_result1), [out2] "=r" (asm_result2)
        : [in1] "m" (*(volatile int*)addr1),
          [in2] "r" (addr2),
          "m" (*(volatile double*)addr2)
        : "memory"
    );
    
    /* More complex addressing with different scales */
    char* byte_ptr = (char*)triple_ptr;
    volatile long offset1 = v7 * 8;
    volatile long offset2 = v8 * 4;
    
    /* Should trigger RELOAD_FOR_OTHER_ADDRESS */
    int val3 = *(int*)(byte_ptr + offset1 + offset2 + 16);
    
    /* Nested addressing computation */
    /* Should trigger RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_OUTPUT_ADDRESS */
    int* nested_ptr = *(triple_ptr[idx1 % 3] + idx2 % 7);
    for (volatile int i = 0; i < 4; i++) {
        nested_ptr[i * 2 + v6 % 4] = 
            arr3d[(idx1 + i) % 4][(idx2 + v5) % 8][(idx3 + v4) % 16];
    }
    
    /* Floating-point to integer conversion pressure */
    /* Mixes register classes */
    int int_sum = 0;
    double dbl_sum = 0.0;
    for (volatile int i = 0; i < 8; i++) {
        for (volatile int j = 0; j < 8; j++) {
            /* Complex array indexing */
            int array_idx = (i * v1 + j * v2 + v3) % 32;
            dbl_sum += dbl_arr[array_idx][j % 8];
            int_sum += arr3d[i % 4][j % 8][array_idx % 16];
        }
    }
    
    /* Another inline asm with address output */
    /* Should trigger RELOAD_FOR_OUTADDR_ADDRESS */
    void* computed_addr;
    __asm__ volatile (
        "leaq (%[base], %[index], 4), %[out]\n\t"
        : [out] "=r" (computed_addr)
        : [base] "r" (byte_ptr),
          [index] "r" (offset1)
        : "cc"
    );
    
    /* Use computed address */
    int final_val = *(int*)computed_addr;
    
    /* Cleanup */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            free(triple_ptr[i][j]);
        }
        free(triple_ptr[i]);
    }
    free(triple_ptr);
    
    /* Return volatile sum to prevent optimization */
    return (int)(val1 + val2 + val3 + asm_result1 + asm_result2 + 
                 int_sum + final_val + (int)dbl_sum + local1 + local2 + local5);
}

/* Helper to create more register pressure */
__attribute__((noinline))
static double complex_calc(volatile double* a, volatile double* b, 
                          volatile int* idx, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        /* Complex addressing with different scales */
        sum += a[i * 2] * b[idx[i % 8] * 3];
    }
    return sum;
}

int main(void) {
    /* Initialize volatile values with randomness */
    volatile int v1 = rand() % 100;
    volatile int v2 = rand() % 100;
    volatile int v3 = rand() % 100;
    volatile int v4 = rand() % 100;
    volatile int v5 = rand() % 100;
    volatile int v6 = rand() % 100;
    volatile long v7 = rand() % 100;
    volatile long v8 = rand() % 100;
    volatile double v9 = (double)(rand() % 100) / 10.0;
    
    /* Arrays for additional pressure */
    volatile double dbl_array[32];
    volatile int idx_array[32];
    
    for (int i = 0; i < 32; i++) {
        dbl_array[i] = (double)(rand() % 1000) / 100.0;
        idx_array[i] = rand() % 16;
    }
    
    /* Call complex calculation */
    double intermediate = complex_calc((double*)dbl_array, 
                                      (double*)dbl_array,
                                      (int*)idx_array, 16);
    
    /* Main reload-triggering call */
    int result = trigger_reloads(v1, v2, v3, v4, v5, v6, v7, v8, v9 + intermediate);
    
    printf("Result: %d\n", result);
    return 0;
}
