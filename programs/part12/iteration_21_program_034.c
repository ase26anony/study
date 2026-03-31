/* test_sel_sched.c - Program to trigger selective scheduler debug output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight, data-dependent loop for selective scheduling */
int compute_sum(int *a, int *b, int n) {
    int sum = 0;
    
    /* Main loop with carried dependency - good for pipelining */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with conditional */
        int prod = a[i] * b[i];
        
        /* Conditional control flow to create multiple basic blocks */
        if (prod > THRESHOLD) {
            sum += prod;
        } else {
            sum += a[i] + b[i];
        }
        
        /* Inline assembly to create complex RTL patterns */
        asm volatile (
            "addl %1, %0\n\t"
            "cmpl $0, %0\n\t"
            "setg %%al\n\t"
            "movzbl %%al, %0"
            : "+r"(sum)
            : "r"(i)
            : "cc", "al"
        );
    }
    
    return sum;
}

/* Another function with nested loops for more scheduling opportunities */
void process_matrix(int mat[SIZE][SIZE], int factor) {
    int temp;
    
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            /* Complex computation with multiple operations */
            temp = mat[i][j] * factor;
            
            /* Conditional with side effect */
            if (temp & 1) {
                temp = (temp << 3) | (temp >> 5);
            } else {
                temp = (temp >> 2) + factor;
            }
            
            /* Another inline assembly with multiple clobbers */
            asm volatile (
                "imull %1, %0\n\t"
                "addl $1, %0"
                : "+r"(temp)
                : "r"(j)
                : "cc"
            );
            
            mat[i][j] = temp;
        }
    }
}

/* Function with pointer chasing loop */
int linked_sum(int *data, int *next, int start, int steps) {
    int sum = 0;
    int idx = start;
    
    for (int i = 0; i < steps; i++) {
        __builtin_assume(idx >= 0 && idx < SIZE);
        sum += data[idx];
        
        /* Conditional with unpredictable branch */
        if (sum > 1000000) {
            sum = sum >> 1;
        }
        
        idx = next[idx];
        
        /* Memory barrier asm to prevent reordering */
        asm volatile("" ::: "memory");
    }
    
    return sum;
}

int main() {
    int a[SIZE], b[SIZE];
    int next[SIZE];
    int matrix[SIZE][SIZE];
    
    /* Initialize arrays with predictable but non-constant values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = i * 3 + 1;
        b[i] = i * 2 - 5;
        next[i] = (i + 1) % SIZE;
        
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = i * j + 1;
        }
    }
    
    /* Use __builtin_assume to provide optimization hints */
    __builtin_assume(SIZE > 0);
    
    /* Call functions that should trigger selective scheduling */
    int sum1 = compute_sum(a, b, SIZE);
    process_matrix(matrix, 7);
    int sum2 = linked_sum(a, next, 0, SIZE / 2);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d %d\n", sum1, sum2);
    
    /* Verify computation is correct */
    int verify_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        verify_sum += a[i] + b[i];
    }
    printf("Verification sum: %d\n", verify_sum);
    
    return 0;
}
