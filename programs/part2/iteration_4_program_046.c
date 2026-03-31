/* This program is designed to trigger the uncovered delay slot filling logic
 * in GCC's reorg.cc, specifically lines 2135-2149.
 * It creates patterns of unconditional jumps to labels followed by simple
 * arithmetic operations that are eligible for moving into delay slots.
 * Compile with: -O2 -march=mips -fdump-rtl-reorg
 */

#include <stdio.h>
#include <stdlib.h>

/* Force MIPS target if not already default */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void compute_hash(int *input, int *output, int size) {
    int i, j;
    int temp1, temp2, temp3, temp4;
    int acc = 0;
    
    /* Use distinct register sets to avoid resource conflicts */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0;
    int s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    
    /* Memory barrier to constrain scheduling */
    __sync_synchronize();
    
    for (i = 0; i < size; i++) {
        /* Create multiple jump contexts with different patterns */
        if (__builtin_expect((input[i] & 1) == 0, 0)) {
            /* Pattern 1: Jump to label with simple arithmetic follower */
            goto label1;
back_from_label1:
            continue;
        } else if (__builtin_expect((input[i] & 2) == 0, 1)) {
            /* Pattern 2: Another jump pattern */
            goto label2;
back_from_label2:
            /* Mix in some FP operations in different paths */
            float ftemp = (float)input[i];
            ftemp = ftemp * 0.5f;
            s1 += (int)ftemp;
            continue;
        } else {
            /* Pattern 3: Yet another jump pattern */
            goto label3;
back_from_label3:
            /* Simple arithmetic that uses different registers */
            r3 = r3 ^ input[i];
            continue;
        }
        
        /* These labels must be placed right before simple, non-trapping operations */
    label1:
        /* Simple arithmetic: eligible for delay slot */
        /* Uses registers not in conflict with jump context */
        temp1 = s2 + s3;  /* Non-trapping integer addition */
        s2 = temp1;
        acc += temp1;
        goto back_from_label1;
        
    label2:
        /* Another simple operation */
        temp2 = r4 - s4;  /* Non-trapping subtraction */
        r4 = temp2;
        acc ^= temp2;
        goto back_from_label2;
        
    label3:
        /* Bitwise operation - safe and non-trapping */
        temp3 = r1 & 0xFF;  /* Mask operation */
        r1 = temp3;
        acc |= temp3;
        goto back_from_label3;
    }
    
    /* Additional loop with nested jumps to increase scheduling pressure */
    for (j = 0; j < size; j++) {
        volatile int *ptr = &input[j];  /* volatile to prevent optimization */
        if (__builtin_expect(*ptr > 100, 0)) {
            goto label4;
back_from_label4:
            s3 = s3 + 1;
        } else {
            goto label5;
back_from_label5:
            s4 = s4 - 1;
        }
        
        /* More label patterns */
    label4:
        temp4 = s1 * 2;  /* Multiplication by constant - safe */
        s1 = temp4;
        acc += temp4;
        goto back_from_label4;
        
    label5:
        r2 = r2 >> 1;  /* Shift operation - non-trapping */
        acc ^= r2;
        goto back_from_label5;
    }
    
    /* Final computation to prevent dead code elimination */
    output[0] = acc + r1 + r2 + r3 + r4 + s1 + s2 + s3 + s4;
    
    /* Another memory barrier */
    asm volatile("" ::: "memory");
}

/* Secondary function with different patterns */
#ifdef __GNUC__
__attribute__((target("arch=mips")))
#endif
void process_matrix(int *matrix, int rows, int cols) {
    int i, j;
    int a = 0, b = 0, c = 0, d = 0;
    
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            int idx = i * cols + j;
            
            /* Create more jump-to-label patterns */
            if (__builtin_expect(matrix[idx] < 0, 0)) {
                goto neg_label;
back_from_neg:
                a = a + 1;
            } else if (__builtin_expect(matrix[idx] == 0, 1)) {
                goto zero_label;
back_from_zero:
                b = b + 1;
            } else {
                goto pos_label;
back_from_pos:
                c = c + 1;
            }
            
        neg_label:
            d = d - matrix[idx];  /* Safe subtraction */
            matrix[idx] = d;
            goto back_from_neg;
            
        zero_label:
            d = d | 0x1;  /* Bitwise OR with constant */
            matrix[idx] = d;
            goto back_from_zero;
            
        pos_label:
            d = d & ~0x1;  /* Bitwise AND with constant */
            matrix[idx] = d;
            goto back_from_pos;
        }
    }
    
    /* Use results */
    matrix[0] = a + b + c;
}

int main() {
    const int SIZE = 100;
    int *input = (int*)malloc(SIZE * sizeof(int));
    int *output = (int*)malloc(SIZE * sizeof(int));
    int *matrix = (int*)malloc(SIZE * SIZE * sizeof(int));
    
    /* Initialize with pattern to trigger various branches */
    for (int i = 0; i < SIZE; i++) {
        input[i] = (i * 17) % 100;  /* Pattern that creates mix of conditions */
    }
    
    for (int i = 0; i < SIZE * SIZE; i++) {
        matrix[i] = (i % 101) - 50;  /* Mix of negative, zero, positive */
    }
    
    /* Call functions with jump patterns */
    compute_hash(input, output, SIZE);
    process_matrix(matrix, SIZE, SIZE);
    
    /* Print results to prevent optimization */
    printf("Result: %d\n", output[0]);
    printf("Matrix[0]: %d\n", matrix[0]);
    
    free(input);
    free(output);
    free(matrix);
    
    return 0;
}
