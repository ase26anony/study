/* Target: MIPS architecture with delay slots */
/* Compile with: -O2 -march=mips -fdump-rtl-reorg -fdump-rtl-all */
/* This program creates specific jump-to-label patterns that should trigger */
/* the uncovered delay slot filling logic in reorg.cc lines 2135-2149 */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if supported */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void compute_hash(int *input, int *output, int size) {
    /* Use distinct register sets to avoid resource conflicts */
    int reg_a = 0, reg_b = 0, reg_c = 0, reg_d = 0;
    int reg_x = 0, reg_y = 0, reg_z = 0;
    int temp1 = 0, temp2 = 0, temp3 = 0;
    
    /* Initialize with non-zero values to avoid constant propagation */
    reg_a = input[0] ^ 0x12345678;
    reg_b = input[1] ^ 0x9ABCDEF0;
    reg_c = input[2];
    reg_d = input[3];
    
    /* Create multiple basic blocks with label-oriented jumps */
    for (int i = 0; i < size; i++) {
        /* Vary the control flow to create scheduling pressure */
        if (__builtin_expect((input[i] & 1) == 0, 1)) {
            /* Pattern 1: Jump to label with simple arithmetic follower */
            /* This should create a simplejump to label */
            if (reg_a < reg_b) {
                /* Force a goto to create jump-to-label pattern */
                goto label_arithmetic_1;
            } else {
                /* Alternate path with memory barrier to constrain scheduling */
                __asm__ volatile("" ::: "memory");
                reg_x = reg_y + reg_z;
            }
            
            /* Continue with other operations */
            reg_c = reg_d ^ temp1;
            
            /* Another jump pattern */
            if (__builtin_expect(reg_c != 0, 0)) {
                goto label_arithmetic_2;
            }
            
            /* Mix integer operations */
            temp2 = temp3 * 3;
            continue;
            
        label_arithmetic_1:
            /* Simple, non-trapping arithmetic operation after label */
            /* This is the candidate for delay slot filling (next_trial) */
            /* Uses distinct registers from the jump's context */
            reg_x = reg_y + reg_z;  /* Simple add, no trap possible */
            
            /* Follow with more operations to create basic block */
            temp1 = reg_a & reg_b;
            reg_d = temp1 | 0xFF;
            continue;
            
        label_arithmetic_2:
            /* Another candidate instruction after label */
            /* Bitwise operation, non-trapping */
            temp3 = reg_c ^ reg_d;
            
            /* More operations to prevent sequence formation */
            reg_a = reg_b << 2;
            reg_b = reg_c >> 1;
            continue;
        }
        
        /* Second major path with different patterns */
        if (__builtin_expect((input[i] & 2) != 0, 0)) {
            int counter = 0;
            
            /* Nested loop to create complex control flow */
            while (counter < 3) {
                /* Create another jump-to-label pattern */
                if (temp2 > temp3) {
                    goto label_arithmetic_3;
                }
                
                /* Alternate operations */
                reg_y = reg_z - reg_x;
                counter++;
                
                /* Skip the label if not taken */
                if (counter == 2) continue;
                
            label_arithmetic_3:
                /* Another simple arithmetic candidate */
                reg_z = reg_x * 2;  /* Multiplication by 2 is safe */
                
                /* Ensure this doesn't become a SEQUENCE */
                temp2 = temp3 + 1;
                counter++;
            }
        }
        
        /* Mix with floating point in separate blocks to diversify resources */
        {
            float f1 = (float)reg_a;
            float f2 = (float)reg_b;
            /* Use floating point but not in the critical paths */
            if (f1 > f2) {
                reg_a++;
            }
        }
        
        /* Final accumulation */
        output[i] = reg_a + reg_b + reg_c + reg_d + temp1 + temp2 + temp3;
        
        /* Rotate registers to create data dependencies */
        int rotate = reg_a;
        reg_a = reg_b;
        reg_b = reg_c;
        reg_c = reg_d;
        reg_d = rotate;
    }
}

/* Secondary function with different patterns */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void process_matrix(int *matrix, int rows, int cols) {
    /* Create more jump-label patterns in different contexts */
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            int val = matrix[idx];
            
            /* Multiple conditional jumps to labels */
            if (__builtin_expect(val > 100, 1)) {
                if (val & 0x01) {
                    goto matrix_label_1;
                } else {
                    goto matrix_label_2;
                }
            }
            
            /* Default path */
            matrix[idx] = val * 2;
            continue;
            
        matrix_label_1:
            /* Simple arithmetic after label */
            matrix[idx] = val + 5;  /* Non-trapping addition */
            continue;
            
        matrix_label_2:
            /* Another simple operation */
            matrix[idx] = val - 3;  /* Non-trapping subtraction */
            continue;
        }
    }
}

int main() {
    const int SIZE = 256;
    int *input = (int*)malloc(SIZE * sizeof(int));
    int *output = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        input[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Call functions with jump-label patterns */
    compute_hash(input, output, SIZE);
    
    /* Create matrix for second function */
    int rows = 16, cols = 16;
    int *matrix = (int*)malloc(rows * cols * sizeof(int));
    for (int i = 0; i < rows * cols; i++) {
        matrix[i] = (i * 1664525 + 1013904223) & 0xFF;
    }
    
    process_matrix(matrix, rows, cols);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += output[i];
    }
    for (int i = 0; i < rows * cols; i++) {
        checksum += matrix[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    free(input);
    free(output);
    free(matrix);
    
    return 0;
}
