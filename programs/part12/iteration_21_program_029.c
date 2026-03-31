/* test_sel_sched.c - Program to trigger selective scheduler debugging output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight data-dependent loop for selective scheduling */
int compute_sum(int *a, int *b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies - encourages software pipelining */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int product = a[i] * b[i];
        
        /* Conditional control flow creates multiple basic blocks */
        if (product > THRESHOLD) {
            sum += product;
        } else {
            /* Alternative path with inline assembly */
            int temp = product;
            /* Complex inline asm with multiple constraints */
            asm volatile (
                "addl %1, %0\n\t"
                "cmpl $50, %0\n\t"
                "jle 1f\n\t"
                "subl $10, %0\n\t"
                "1:"
                : "+r"(temp)
                : "r"(i)
                : "cc", "memory"
            );
            sum += temp / 2;
        }
        
        /* Additional inline asm with clobbers */
        asm volatile (
            "testl %0, %0\n\t"
            "setg %%al\n\t"
            "addb %%al, %%bl\n\t"
            : 
            : "r"(sum)
            : "al", "bl", "cc"
        );
    }
    
    return sum;
}

/* Second function with nested loops for larger scheduling regions */
void process_matrix(int mat[SIZE][SIZE], int factor) {
    for (int i = 0; i < SIZE; i++) {
        int row_sum = 0;
        
        /* Inner loop with pointer arithmetic */
        int *row = mat[i];
        for (int j = 0; j < SIZE; j++) {
            /* Complex addressing mode */
            int val = row[j] * factor;
            
            /* Conditional with multiple operations */
            if (val & 1) {
                /* Inline asm with memory input */
                asm volatile (
                    "imull %1, %0\n\t"
                    "addl %%eax, %0\n\t"
                    : "+r"(val)
                    : "r"(j), "a"(i)
                    : "cc"
                );
                row_sum += val;
            } else {
                row_sum -= val >> 1;
            }
            
            /* Prevent dead code elimination */
            __asm__ __volatile__("" : : "r"(row_sum) : "memory");
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

/* Helper with loop unrolling hint */
int unrolled_computation(int *arr, int n) {
    int total = 0;
    
    /* Loop with known bounds for unrolling */
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        /* Use builtin to provide optimization hints */
        if (__builtin_expect(arr[i] > 0, 1)) {
            total += arr[i] * 3;
        } else {
            total -= arr[i];
        }
        
        /* Complex asm with multiple outputs */
        int low, high;
        asm volatile (
            "movl %2, %%eax\n\t"
            "mull %3\n\t"
            "movl %%eax, %0\n\t"
            "movl %%edx, %1\n\t"
            : "=r"(low), "=r"(high)
            : "r"(arr[i]), "r"(i)
            : "eax", "edx", "cc"
        );
        total += low;
    }
    
    return total;
}

int main() {
    /* Initialize data arrays */
    int a[SIZE], b[SIZE];
    int matrix[SIZE][SIZE];
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = (i * j) % 255;
        }
    }
    
    /* Force compiler to assume loop bounds */
    int n = SIZE;
    __builtin_assume(n > 0);
    __builtin_assume(n <= SIZE);
    
    /* Call functions to create scheduling regions */
    int sum1 = compute_sum(a, b, n);
    process_matrix(matrix, 3);
    int sum2 = unrolled_computation(a, n);
    
    /* Use results to prevent optimization */
    printf("Results: %d %d\n", sum1, sum2);
    printf("Matrix[0][0] = %d\n", matrix[0][0]);
    
    /* Verify computation */
    int verify = 0;
    for (int i = 0; i < n; i++) {
        int prod = a[i] * b[i];
        if (prod > THRESHOLD) verify += prod;
        else verify += (prod + i) / 2;
    }
    
    if (sum1 != verify) {
        fprintf(stderr, "Verification failed: %d != %d\n", sum1, verify);
        return 1;
    }
    
    return 0;
}
