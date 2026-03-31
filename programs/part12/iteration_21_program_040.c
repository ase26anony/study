/* test_sel_sched.c - Program to trigger selective scheduler debug output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight data-dependent loop for selective scheduling */
int compute_sum(int *a, int *b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies - encourages pipelining */
    for (int i = 0; i < n; i++) {
        int temp = a[i] * b[i];
        
        /* Conditional control flow to create multi-block region */
        if (temp > THRESHOLD) {
            /* Complex inline assembly to generate non-trivial RTL */
            asm volatile (
                "addl %1, %0\n\t"
                "cmpl $1000, %0\n\t"
                "jle 1f\n\t"
                "subl $500, %0\n\t"
                "1:"
                : "+r"(sum)
                : "r"(temp)
                : "cc"
            );
        } else {
            /* Different arithmetic with inline assembly */
            asm volatile (
                "imull $2, %1\n\t"
                "addl %1, %0"
                : "+r"(sum)
                : "r"(temp)
                : "cc"
            );
        }
        
        /* Additional computation to increase scheduling complexity */
        int idx = i & 0xFF;
        asm volatile (
            "movl %1, %%eax\n\t"
            "andl $0x7F, %%eax\n\t"
            "addl %%eax, %0"
            : "+r"(sum)
            : "r"(idx)
            : "%eax", "cc"
        );
    }
    
    return sum;
}

/* Second function with nested loops for more scheduling opportunities */
void process_matrix(int mat[][16], int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        int row_sum = 0;
        
        /* Inner loop with stride access pattern */
        for (int j = 0; j < cols; j += 2) {
            int val = mat[i][j];
            
            /* Complex conditional with inline assembly */
            if (val & 1) {
                asm volatile (
                    "rorl $4, %0\n\t"
                    "xorl $0xAA, %0"
                    : "+r"(val)
                    :
                    : "cc"
                );
            }
            
            row_sum += val;
            
            /* Prevent dead code elimination */
            asm volatile ("" : : "r"(row_sum));
        }
        
        /* Store result with memory barrier */
        asm volatile (
            "movl %1, %0\n\t"
            : "=m"(mat[i][0])
            : "r"(row_sum)
            : "memory"
        );
    }
}

/* Helper to initialize arrays */
void init_arrays(int *a, int *b, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (i * 3) % 256;
        b[i] = (i * 7) % 256;
    }
}

int main() {
    int a[SIZE], b[SIZE];
    int matrix[8][16];
    
    /* Initialize data */
    init_arrays(a, b, SIZE);
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = (i * 16 + j) * 11;
        }
    }
    
    /* Compute results using both functions */
    int result1 = compute_sum(a, b, SIZE);
    process_matrix(matrix, 8, 16);
    
    /* Compute verification sum to prevent optimization */
    int verify = result1;
    for (int i = 0; i < 8; i++) {
        verify += matrix[i][0];
    }
    
    /* Print result to ensure execution */
    printf("Result: %d\n", verify);
    
    return 0;
}
