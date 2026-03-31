/* test_sel_sched.c - Program to trigger selective scheduler debug output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 500

/* Function with tight data-dependent loop that benefits from pipelining */
int compute_sum(int *a, int *b, int n) {
    int sum = 0;
    
    /* Main loop with data dependency - forces scheduler to work hard */
    for (int i = 0; i < n; i++) {
        int temp = a[i] * b[i];
        
        /* Conditional control flow creates multiple basic blocks */
        if (temp > THRESHOLD) {
            /* Complex operation with inline assembly */
            int adjusted;
            asm volatile (
                "imull %2, %1\n\t"           /* Multiply */
                "addl %1, %0\n\t"           /* Add to sum */
                : "+r"(sum), "=&r"(adjusted)
                : "r"(temp)
                : "cc"
            );
        } else {
            /* Different path with another asm statement */
            asm volatile (
                "addl %1, %0"
                : "+r"(sum)
                : "r"(temp)
                : "cc"
            );
        }
        
        /* Additional computation to create more scheduling opportunities */
        int extra = a[i] + b[i];
        if (extra % 2 == 0) {
            asm volatile (
                "xorl %1, %0"
                : "+r"(sum)
                : "r"(extra)
                : "cc"
            );
        }
    }
    
    return sum;
}

/* Second function with nested loops for more complex scheduling */
int matrix_multiply(int size) {
    int result = 0;
    
    /* Create small matrices */
    int A[4][4] = {{1,2,3,4},{5,6,7,8},{9,10,11,12},{13,14,15,16}};
    int B[4][4] = {{16,15,14,13},{12,11,10,9},{8,7,6,5},{4,3,2,1}};
    int C[4][4] = {{0}};
    
    /* Nested loops with carried dependencies */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int sum = 0;
            for (int k = 0; k < 4; k++) {
                /* Data-dependent computation */
                sum += A[i][k] * B[k][j];
                
                /* Inline asm with multiple clobbers */
                asm volatile (
                    "movl %1, %%eax\n\t"
                    "imull %2, %%eax\n\t"
                    "addl %%eax, %0"
                    : "+r"(sum)
                    : "r"(A[i][k]), "r"(B[k][j])
                    : "%eax", "cc"
                );
            }
            C[i][j] = sum;
            result += sum;
        }
    }
    
    return result;
}

/* Function with pointer chasing to create unpredictable but schedulable patterns */
int pointer_chase(int *data, int n) {
    int sum = 0;
    int index = 0;
    
    for (int i = 0; i < n; i++) {
        /* Data-dependent array access */
        index = data[index] % n;
        
        /* Complex asm with memory operand */
        int val = data[index];
        asm volatile (
            "movl (%1), %%eax\n\t"
            "addl %%eax, %0\n\t"
            "rorl $3, %0"
            : "+r"(sum)
            : "r"(&data[index])
            : "%eax", "cc", "memory"
        );
    }
    
    return sum;
}

int main() {
    /* Initialize arrays with predictable but non-trivial patterns */
    int a[SIZE], b[SIZE];
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3) % 1000;
        b[i] = (i * 7) % 1000;
    }
    
    /* Call functions to create scheduling regions */
    int sum1 = compute_sum(a, b, SIZE);
    int sum2 = matrix_multiply(SIZE);
    
    /* Create data for pointer chasing */
    int chase_data[100];
    for (int i = 0; i < 100; i++) {
        chase_data[i] = (i * 13) % 100;
    }
    int sum3 = pointer_chase(chase_data, 100);
    
    /* Use results to prevent dead code elimination */
    int total = sum1 + sum2 + sum3;
    
    /* Print result to ensure computation happens */
    printf("Result: %d\n", total);
    
    /* Verify correctness with a simple check */
    if (total != 0) {  /* Non-zero is expected */
        printf("Computation completed successfully.\n");
    }
    
    return 0;
}
