/* reload_coverage.c - Program to trigger multiple reload types in GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force complex address calculations that require reloads */
#define FORCE_RELOADS __attribute__((noinline, noipa, optimize("O2")))

/* Compiler barrier to prevent optimizations */
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Volatile variables to prevent constant propagation */
static volatile int v1, v2, v3, v4, v5, v6, v7, v8;
static volatile long vl1, vl2;
static volatile double vd1, vd2;

/* Function to trigger various reload types */
FORCE_RELOADS
static double trigger_reloads(
    volatile int idx1, volatile int idx2, 
    volatile int idx3, volatile int idx4,
    volatile long offset1, volatile long offset2,
    volatile double scale1, volatile double scale2)
{
    /* Create high register pressure with many live values */
    int local_ints[50];
    double local_doubles[40];
    int *ptr_chain1[10];
    double *ptr_chain2[10];
    long intermediate_results[20];
    double fp_results[30];
    
    /* Initialize arrays to create live ranges */
    for (int i = 0; i < 50; i++) {
        local_ints[i] = i * idx1;
        COMPILER_BARRIER();
    }
    
    for (int i = 0; i < 40; i++) {
        local_doubles[i] = i * scale1;
        COMPILER_BARRIER();
    }
    
    /* Complex multi-dimensional array access with volatile indices */
    /* This should trigger RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
    int multi_array[20][30];
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 30; j++) {
            /* Complex addressing: base + (idx1 * stride) + (idx2 + idx3) */
            int complex_idx = i * idx1 + (idx2 + idx3) * j;
            if (complex_idx < 600) {
                multi_array[i][j] = local_ints[complex_idx % 50] * idx4;
            }
            COMPILER_BARRIER();
        }
    }
    
    /* Pointer chain with multiple indirections */
    /* Should trigger RELOAD_FOR_OPERAND_ADDRESS */
    int **pptr1 = (int**)&ptr_chain1[0];
    int **pptr2 = (int**)&ptr_chain1[5];
    
    for (int i = 0; i < 10; i++) {
        ptr_chain1[i] = &local_ints[i * 3];
        COMPILER_BARRIER();
    }
    
    /* Complex pointer arithmetic with mixed types */
    /* Should trigger RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    char *char_ptr = (char*)local_ints;
    int *int_ptr = (int*)(char_ptr + offset1);
    long *long_ptr = (long*)((char*)int_ptr + offset2);
    
    /* Mixed register class pressure - integer and floating point */
    for (int i = 0; i < 30; i++) {
        /* Convert int to double, do FP math, convert back */
        double temp = (double)local_ints[i % 50];
        fp_results[i] = temp * scale2 + local_doubles[i % 40];
        COMPILER_BARRIER();
        
        /* Store back through complex address */
        int store_idx = (idx1 * i + idx2) % 50;
        local_ints[store_idx] = (int)fp_results[i];
        COMPILER_BARRIER();
    }
    
    /* Inline assembly to force specific register constraints */
    /* These should trigger RELOAD_FOR_OTHER and RELOAD_FOR_OPADDR_ADDR */
    int asm_result1, asm_result2;
    double asm_fp_result;
    
    /* Assembly with memory input, register output */
    __asm__ volatile (
        "movl (%1), %0\n\t"
        "addl %2, %0"
        : "=&r" (asm_result1)          /* Early clobber output */
        : "r" (&local_ints[idx1 % 50]), /* Address in register */
          "r" (idx2)                    /* Value in register */
        : "memory"
    );
    
    /* Assembly with complex addressing mode */
    __asm__ volatile (
        "movq (%[base], %[index], 4), %[result]\n\t"
        "addq %[offset], %[result]"
        : [result] "=r" (asm_result2)
        : [base] "r" (local_ints),
          [index] "r" ((long)idx3),
          [offset] "r" (offset1)
        : "memory"
    );
    
    /* Floating point assembly mixing registers */
    __asm__ volatile (
        "movsd (%1), %%xmm0\n\t"
        "mulsd %2, %%xmm0\n\t"
        "movsd %%xmm0, %0"
        : "=m" (asm_fp_result)        /* Memory output */
        : "r" (&local_doubles[idx4 % 40]), /* Address in register */
          "x" (scale1)                 /* XMM register input */
        : "xmm0", "memory"
    );
    
    /* More complex address calculations with pointer chains */
    /* Should trigger RELOAD_FOR_OTHER_ADDRESS */
    int ***ppptr = &pptr1;
    int value_from_chain = ***ppptr;
    
    /* Create a situation requiring address reload for output */
    int *output_ptr = &local_ints[0];
    for (int i = 0; i < 10; i++) {
        /* Complex store address calculation */
        int *store_addr = output_ptr + (idx1 * i + idx2) / sizeof(int);
        *store_addr = value_from_chain + i;
        COMPILER_BARRIER();
        
        /* Update pointer with complex calculation */
        output_ptr = (int*)((char*)output_ptr + (idx3 * i + idx4));
        COMPILER_BARRIER();
    }
    
    /* Final computation mixing all types */
    double final_result = 0.0;
    for (int i = 0; i < 20; i++) {
        /* Complex array access with multiple volatile indices */
        int array_idx = (i * idx1 + idx2 * idx3 - idx4) % 50;
        if (array_idx < 0) array_idx = -array_idx;
        
        /* Mixed-type computation */
        final_result += (double)local_ints[array_idx] * 
                       local_doubles[i % 40] * 
                       (1.0 + (i % 10) * 0.1);
        
        /* More complex addressing */
        long *long_addr = (long*)((char*)&intermediate_results[0] + 
                                 (offset1 * i + offset2) % sizeof(intermediate_results));
        *long_addr = (long)final_result;
        
        COMPILER_BARRIER();
    }
    
    /* Use inline assembly one more time with operand address */
    double final_asm_result;
    __asm__ volatile (
        "movsd (%1), %%xmm0\n\t"
        "addsd (%2), %%xmm0\n\t"
        "movsd %%xmm0, %0"
        : "=m" (final_asm_result)
        : "r" (&final_result),        /* First address */
          "r" (&local_doubles[idx1 % 40]) /* Second address */
        : "xmm0", "memory"
    );
    
    return final_result + final_asm_result + asm_fp_result;
}

int main(void) {
    srand(time(NULL));
    
    /* Initialize volatile variables with random values */
    v1 = rand() % 100;
    v2 = rand() % 100;
    v3 = rand() % 100;
    v4 = rand() % 100;
    v5 = rand() % 100;
    v6 = rand() % 100;
    v7 = rand() % 100;
    v8 = rand() % 100;
    vl1 = rand() % 1000;
    vl2 = rand() % 1000;
    vd1 = (double)(rand() % 100) / 10.0;
    vd2 = (double)(rand() % 100) / 10.0;
    
    /* Call the function with volatile arguments */
    double result = trigger_reloads(v1, v2, v3, v4, vl1, vl2, vd1, vd2);
    
    /* Use the result to prevent optimization */
    printf("Result: %f\n", result);
    
    /* Additional calls with different parameters to increase coverage */
    result += trigger_reloads(v2, v3, v4, v5, vl2, vl1, vd2, vd1);
    result += trigger_reloads(v3, v4, v5, v6, vl1 + vl2, vl1 - vl2, vd1 * 2, vd2 / 2);
    
    printf("Final result: %f\n", result);
    
    return 0;
}
