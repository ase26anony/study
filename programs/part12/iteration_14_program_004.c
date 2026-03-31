/* reload_coverage.c - Complex program to trigger GCC reload pass edge cases */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Prevent inlining and inter-procedural analysis */
__attribute__((noinline, noipa))
static int trigger_reloads(
    volatile int v1, volatile int v2, volatile int v3,
    volatile int v4, volatile int v5, volatile int v6,
    volatile long v7, volatile long v8)
{
    /* High register pressure with mixed types */
    int local1 = v1 * 2;
    int local2 = v2 + v3;
    double f1 = (double)v4 * 1.5;
    double f2 = (double)v5 * 2.5;
    int local3 = v6 & 0xFF;
    long local4 = v7 >> 3;
    long local5 = v8 << 2;
    
    /* Multi-dimensional arrays with volatile indexing */
    int arr3d[4][8][16];
    double dbl_arr[32][8];
    char* char_ptr_arr[64];
    
    /* Complex pointer chains */
    int*** ptr_chain1;
    long** ptr_chain2;
    
    /* Initialize arrays */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 16; k++) {
                arr3d[i][j][k] = i * 256 + j * 16 + k;
            }
        }
    }
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 8; j++) {
            dbl_arr[i][j] = (double)(i * 8 + j) * 0.1;
        }
    }
    
    /* Memory barrier to prevent optimization */
    __asm__ volatile("" : : : "memory");
    
    /* Complex addressing mode 1: Base + index * scale + displacement */
    /* This should trigger RELOAD_FOR_INPUT_ADDRESS */
    int idx1 = v1 % 4;
    int idx2 = v2 % 8;
    int idx3 = v3 % 16;
    int result1 = arr3d[idx1][idx2][idx3];
    
    /* More complex: arr3d[v1][v2 + v3][v4 * v5] */
    int idx4 = (v2 + v3) % 8;
    int idx5 = (v4 * v5) % 16;
    int result2 = arr3d[v1 % 4][idx4][idx5];
    
    /* Memory barrier */
    __asm__ volatile("" : : : "memory");
    
    /* Complex addressing with pointer arithmetic */
    /* Should trigger RELOAD_FOR_OPERAND_ADDRESS */
    int* base_ptr = &arr3d[0][0][0];
    int offset1 = v1 * 128 + v2 * 16 + v3;
    int* ptr1 = base_ptr + offset1;
    int result3 = *ptr1;
    
    /* Double indirection with different types */
    /* Should trigger RELOAD_FOR_OUTPUT_ADDRESS */
    double* dbl_ptr = &dbl_arr[0][0];
    int offset2 = v4 * 8 + v5;
    double* ptr2 = dbl_ptr + offset2;
    double result4 = *ptr2;
    
    /* Inline assembly with register constraints */
    /* Should trigger RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    int asm_input = v6;
    int asm_output1, asm_output2;
    
    __asm__ volatile(
        "movl %[input], %%eax\n\t"
        "leal (%%eax, %%eax, 2), %%ecx\n\t"
        "movl %%ecx, %[out1]\n\t"
        "leal 100(%%eax, %%eax, 4), %%edx\n\t"
        "movl %%edx, %[out2]"
        : [out1] "=&r" (asm_output1), [out2] "=&r" (asm_output2)
        : [input] "mr" (asm_input)
        : "eax", "ecx", "edx", "memory"
    );
    
    /* More complex: Inline assembly taking memory address */
    /* Should trigger RELOAD_FOR_OTHER_ADDRESS */
    int mem_var = v7;
    int asm_output3;
    
    __asm__ volatile(
        "movl (%[mem]), %%eax\n\t"
        "imull $3, %%eax\n\t"
        "movl %%eax, %[out3]"
        : [out3] "=r" (asm_output3)
        : [mem] "r" (&mem_var)
        : "eax", "memory"
    );
    
    /* Mixed register class pressure */
    /* Integer and floating-point computations interleaved */
    double f3 = f1 * 2.0 + (double)result1;
    int int1 = (int)f3 + result2;
    double f4 = f2 / 1.5 + (double)result3;
    int int2 = (int)f4 * asm_output1;
    
    /* Complex output addressing with store */
    /* Should trigger RELOAD_FOR_OUTPUT_ADDRESS */
    int store_idx1 = (v1 + v2) % 4;
    int store_idx2 = (v3 + v4) % 8;
    int store_idx3 = (v5 + v6) % 16;
    arr3d[store_idx1][store_idx2][store_idx3] = 
        result1 + result2 + asm_output2 + asm_output3;
    
    /* Pointer chain dereferencing */
    /* Create artificial pointer chain */
    int** temp_ptr1 = (int**)malloc(sizeof(int*) * 8);
    for (int i = 0; i < 8; i++) {
        temp_ptr1[i] = (int*)malloc(sizeof(int) * 4);
        for (int j = 0; j < 4; j++) {
            temp_ptr1[i][j] = i * 4 + j + v1;
        }
    }
    
    /* Complex: *(*(base + idx1) + idx2) */
    int chain_idx1 = v2 % 8;
    int chain_idx2 = v3 % 4;
    int result5 = temp_ptr1[chain_idx1][chain_idx2];
    
    /* Cleanup */
    for (int i = 0; i < 8; i++) {
        free(temp_ptr1[i]);
    }
    free(temp_ptr1);
    
    /* Memory barrier before return */
    __asm__ volatile("" : : : "memory");
    
    /* Return complex expression to prevent optimization */
    return result1 + result2 + result3 + (int)result4 + 
           asm_output1 + asm_output2 + asm_output3 + result5 +
           int1 + int2 + local1 + local2 + local3 + (int)local4 + (int)local5;
}

int main(void) {
    /* Initialize volatile variables with random values */
    volatile int v1 = rand() % 100;
    volatile int v2 = rand() % 100;
    volatile int v3 = rand() % 100;
    volatile int v4 = rand() % 100;
    volatile int v5 = rand() % 100;
    volatile int v6 = rand() % 100;
    volatile long v7 = rand() % 100;
    volatile long v8 = rand() % 100;
    
    /* Call function with many volatile arguments */
    int result = trigger_reloads(v1, v2, v3, v4, v5, v6, v7, v8);
    
    printf("Result: %d\n", result);
    
    /* Additional calls with different values to explore more paths */
    v1 = rand() % 50;
    v2 = rand() % 50;
    v3 = rand() % 50;
    result += trigger_reloads(v1, v2, v3, v4, v5, v6, v7, v8);
    
    printf("Final result: %d\n", result);
    
    return 0;
}
