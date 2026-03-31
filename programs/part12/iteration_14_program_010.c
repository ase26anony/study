/* reload_coverage.c - Complex program to trigger GCC reload pass edge cases */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and IPA optimizations */
__attribute__((noinline, noipa, optimize("O0")))
static int trigger_reloads(
    volatile int v1, volatile int v2, volatile int v3,
    volatile int v4, volatile int v5, volatile int v6,
    volatile long v7, volatile long v8, volatile double v9)
{
    /* High register pressure with mixed types */
    int local1 = v1 * 2;
    int local2 = v2 + v3;
    double local3 = v9 * 3.14159;
    long local4 = v7 >> 2;
    int local5 = v4 ^ v5;
    double local6 = local3 / 2.0;
    int local7 = v6 * 3;
    long local8 = v8 & 0xFFFF;
    
    /* Multi-dimensional arrays with volatile indexing */
    int arr3d[4][8][16];
    double dbl_arr[32][8];
    char *ptr_arr[12];
    
    /* Complex pointer chains */
    int ***triple_ptr;
    int **double_ptr;
    int *single_ptr;
    
    /* Force spills with many live values */
    __asm__ volatile("" : : : "memory");
    
    /* Initialize arrays with volatile-dependent patterns */
    for (volatile int i = 0; i < 4; i++) {
        for (volatile int j = 0; j < 8; j++) {
            for (volatile int k = 0; k < 16; k++) {
                /* Complex address calculation with volatile components */
                arr3d[i + v1 % 2][j + v2 % 3][k + v3 % 4] = 
                    (i * v1) + (j * v2) + (k * v3);
            }
        }
    }
    
    __asm__ volatile("" : : : "memory");
    
    /* Mixed floating-point and integer computations */
    for (volatile int i = 0; i < 32; i++) {
        for (volatile int j = 0; j < 8; j++) {
            /* Force address reloads with complex indexing */
            dbl_arr[(i * v4) % 32][(j + v5) % 8] = 
                (double)(arr3d[i % 4][j % 8][(i + j) % 16]) * v9;
        }
    }
    
    /* Pointer arithmetic with multiple indirections */
    single_ptr = &arr3d[0][0][0];
    double_ptr = (int **)&single_ptr;
    triple_ptr = (int ***)&double_ptr;
    
    /* Inline assembly to force specific reload types */
    int asm_result1, asm_result2;
    long asm_result3;
    double asm_result4;
    
    /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_INPUT_ADDRESS */
    __asm__ volatile(
        "mov %[addr], %%rsi\n\t"
        "mov (%%rsi), %[out1]\n\t"
        : [out1] "=r" (asm_result1)
        : [addr] "m" (single_ptr)
        : "rsi", "memory"
    );
    
    /* RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    __asm__ volatile(
        "lea (%[idx1], %[idx2], 4), %%rax\n\t"
        "mov %%rax, %[out2]\n\t"
        : [out2] "=r" (asm_result2)
        : [idx1] "r" (v1), [idx2] "r" (v2)
        : "rax"
    );
    
    /* Complex address calculation that needs temporary registers */
    volatile int *volatile_ptr = (volatile int *)single_ptr;
    for (volatile int i = 0; i < 8; i++) {
        /* Multiple volatile components in address */
        int idx = (v1 + i * v2) % 128;
        volatile_ptr[idx + v3] = local1 + i;
    }
    
    /* Mixed register class pressure */
    __asm__ volatile("" : : : "memory");
    
    /* Floating-point computations interspersed with integer */
    for (volatile int i = 0; i < 16; i++) {
        local3 = dbl_arr[i % 32][(i + v4) % 8] * 2.0;
        local6 = local3 + (double)local1;
        asm_result4 = local6;
        
        /* Integer computation that uses same registers */
        local2 = arr3d[(i + v5) % 4][(i * 2) % 8][i % 16] + v6;
        local5 = local2 * local1;
    }
    
    /* More inline assembly with memory constraints */
    __asm__ volatile(
        "movq %[dbl], %%xmm0\n\t"
        "cvttsd2si %%xmm0, %[out3]\n\t"
        : [out3] "=r" (asm_result3)
        : [dbl] "m" (local3)
        : "xmm0"
    );
    
    /* Final complex address computation */
    int final_result = 0;
    for (volatile int i = 0; i < 4; i++) {
        for (volatile int j = 0; j < 8; j++) {
            /* Nested array access with volatile components */
            final_result += arr3d[(i + v1) % 4][(j + v2) % 8]
                [(i * j + v3) % 16];
            
            /* Simultaneous floating-point operation */
            local3 += dbl_arr[(i * 2) % 32][j % 8];
        }
    }
    
    /* Force all values to be live at return */
    return final_result + local1 + local2 + local5 + local7 + 
           (int)local3 + (int)local6 + (int)asm_result4 + 
           asm_result1 + asm_result2 + (int)asm_result3;
}

int main(void) {
    srand(time(NULL));
    
    /* Volatile variables to prevent constant propagation */
    volatile int v1 = rand() % 100;
    volatile int v2 = rand() % 100;
    volatile int v3 = rand() % 100;
    volatile int v4 = rand() % 100;
    volatile int v5 = rand() % 100;
    volatile int v6 = rand() % 100;
    volatile long v7 = rand() % 1000;
    volatile long v8 = rand() % 1000;
    volatile double v9 = (double)(rand() % 100) / 10.0;
    
    /* Call the reload-intensive function multiple times */
    int total = 0;
    for (int i = 0; i < 10; i++) {
        total += trigger_reloads(v1 + i, v2 + i, v3 + i,
                                v4 + i, v5 + i, v6 + i,
                                v7 + i, v8 + i, v9 + i);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
