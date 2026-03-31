/* 
 * Test program to trigger uncovered delay slot filling logic in GCC's reorg.cc
 * Specifically targets lines 2135-2149 in the reorg pass
 * Compile with: gcc -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-all -o reorg_test reorg_test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS architecture if supported */
#ifdef __GNUC__
#define MIPS_TARGET __attribute__((target("arch=mips")))
#else
#define MIPS_TARGET
#endif

/* Volatile to prevent optimization of critical sections */
static volatile int memory_barrier;

/* Resource separation: Use distinct variable sets to avoid resource conflicts */
MIPS_TARGET
void delay_slot_test(int *result, const int *data, int size) {
    /* Separate register sets for different operations */
    int set1_var1 = 0, set1_var2 = 0;  /* Used before jumps */
    int set2_var1 = 0, set2_var2 = 0;  /* Used after labels (delay slot candidates) */
    int set3_var1 = 0, set3_var2 = 0;  /* Used in other paths */
    
    int i;
    
    /* Memory barrier to constrain scheduling */
    asm volatile("" ::: "memory");
    
    for (i = 0; i < size; i++) {
        int val = data[i];
        
        /* Complex control flow to create scheduling pressure */
        if (__builtin_expect((val & 0x1) != 0, 1)) {
            /* Path 1: Jump to label with simple arithmetic after it */
            set1_var1 = val * 3;
            set1_var2 = set1_var1 + 7;
            
            /* Unconditional jump to label - will become simplejump_p */
            if (set1_var2 > 100) {
                /* 
                 * This goto creates a simple jump to label1
                 * The instruction immediately after label1 should be eligible
                 * for delay slot filling
                 */
                goto target_label1;
            }
            
            /* Alternate computation if jump not taken */
            set3_var1 = val ^ 0xFF;
            continue;
            
        target_label1:
            /* 
             * Simple, non-trapping integer operation
             * This is next_trial - must not reference/set resources in &set or &needed
             * Uses completely separate variable set from jump preparation
             */
            set2_var1 = set2_var2 + 5;  /* Simple add - no trap possible */
            
            /* Continue with more operations */
            result[i] = set2_var1 * 2;
            continue;
        }
        
        /* Second pattern with different label */
        if (__builtin_expect((val & 0x2) != 0, 0)) {
            set1_var1 = val << 2;
            set1_var2 = set1_var1 - 10;
            
            if (set1_var2 < 50) {
                /* Another simple jump to different label */
                goto target_label2;
            }
            
            set3_var2 = val | 0x0F;
            continue;
            
        target_label2:
            /* Another eligible delay slot candidate */
            set2_var2 = set2_var1 ^ 0xAA;  /* Bitwise op - no trapping */
            
            result[i] = set2_var2 + i;
            continue;
        }
        
        /* Third pattern - more complex to increase scheduling opportunities */
        if (__builtin_expect((val & 0x4) != 0, 0)) {
            int temp = val * val;
            
            if (temp % 3 == 0) {
                /* Jump with arithmetic after label */
                goto target_label3;
            }
            
            result[i] = temp;
            continue;
            
        target_label3:
            /* Safe operation: multiplication by constant (no overflow trap in C) */
            set2_var1 = set2_var1 * 3;
            
            result[i] = set2_var1;
            continue;
        }
        
        /* Default case - still include jumps to maintain density */
        if (__builtin_expect(val > 0, 1)) {
            set1_var1 = val + 100;
            
            if (set1_var1 != 0) {
                goto target_label4;
            }
            
            result[i] = 0;
            continue;
            
        target_label4:
            /* Another simple operation after label */
            set2_var2 = set2_var2 & 0x7F;  /* Masking operation - safe */
            
            result[i] = set2_var2;
            continue;
        }
        
        result[i] = val;
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
}

/* Additional test with nested loops for more complex CFG */
MIPS_TARGET
void nested_loop_test(int *matrix, int rows, int cols) {
    int i, j;
    int acc1 = 0, acc2 = 0;  /* Separate accumulators for resource separation */
    
    for (i = 0; i < rows; i++) {
        /* Outer loop with conditional jumps */
        if (__builtin_expect(i % 2 == 0, 1)) {
            acc1 = i * cols;
            
            if (acc1 > 0) {
                goto matrix_label1;
            }
            
            acc2 = 0;
            continue;
            
        matrix_label1:
            /* Eligible instruction: simple addition with constant */
            acc2 = acc2 + 2;
        }
        
        for (j = 0; j < cols; j++) {
            int idx = i * cols + j;
            int val = matrix[idx];
            
            /* Inner loop with frequent jumps */
            if (__builtin_expect(val > 127, 0)) {
                acc1 = val - 128;
                
                if (acc1 < 64) {
                    goto inner_label1;
                }
                
                matrix[idx] = val;
                continue;
                
            inner_label1:
                /* Another safe operation for delay slot */
                acc2 = acc2 ^ val;  /* Bitwise XOR - no traps */
                
                matrix[idx] = acc2;
                continue;
            }
            
            /* Another jump pattern */
            if (__builtin_expect(val < 0, 0)) {
                int temp = -val;
                
                if (temp > 10) {
                    goto inner_label2;
                }
                
                matrix[idx] = temp;
                continue;
                
            inner_label2:
                /* Simple shift operation - safe */
                acc1 = temp << 1;
                
                matrix[idx] = acc1;
                continue;
            }
            
            matrix[idx] = val * 2;
        }
    }
}

/* Mix integer and float operations to diversify resource usage */
MIPS_TARGET
void mixed_ops_test(float *farr, int *iarr, int n) {
    int i;
    int int_acc = 0;
    float float_acc = 0.0f;
    
    for (i = 0; i < n; i++) {
        /* Integer path with jumps */
        if (__builtin_expect(iarr[i] > 0, 1)) {
            int temp = iarr[i] * 2;
            
            if (temp % 4 == 0) {
                goto float_label;
            }
            
            int_acc += temp;
            continue;
            
        float_label:
            /* Switch to float operation - creates different resource pattern */
            float_acc += farr[i];
            
            /* Then back to integer with another jump */
            if (float_acc > 100.0f) {
                goto int_label;
            }
            
            continue;
            
        int_label:
            /* Simple integer operation after label */
            int_acc = int_acc & 0xFF;  /* Safe masking */
            
            continue;
        }
        
        /* Default float processing */
        float_acc -= farr[i];
    }
    
    /* Use results to prevent elimination */
    memory_barrier = int_acc + (int)float_acc;
}

MIPS_TARGET
int main() {
    const int SIZE = 1000;
    int *data = (int*)malloc(SIZE * sizeof(int));
    int *result = (int*)malloc(SIZE * sizeof(int));
    float *fdata = (float*)malloc(SIZE * sizeof(float));
    
    /* Initialize with pattern that creates varied branch behavior */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (i * 37) % 255;  /* Semi-random pattern */
        fdata[i] = (float)(i % 100) * 0.5f;
    }
    
    /* Execute tests to trigger delay slot filling logic */
    delay_slot_test(result, data, SIZE);
    
    int matrix[10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    nested_loop_test(&matrix[0][0], 10, 10);
    
    mixed_ops_test(fdata, data, SIZE);
    
    /* Print checksum to ensure computations aren't optimized away */
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += result[i];
    }
    printf("Result checksum: %d\n", sum);
    printf("Memory barrier value: %d\n", memory_barrier);
    
    free(data);
    free(result);
    free(fdata);
    
    return 0;
}
