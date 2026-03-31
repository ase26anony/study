/* Test program to trigger sel-sched debugging output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight data-dependent loop that benefits from pipelining */
int compute_sum(int *a, int *b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies and conditional control flow */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int temp = a[i] * b[i];
        
        /* Conditional control flow to create multi-block scheduling region */
        if (temp > THRESHOLD) {
            sum += temp;
            
            /* Inline assembly to create complex RTL patterns */
            asm volatile (
                "addl %1, %0\n\t"
                "cmpl $1000, %0\n\t"
                "jle 1f\n\t"
                "subl $500, %0\n"
                "1:"
                : "+r"(sum)
                : "r"(temp)
                : "cc"
            );
        } else {
            /* Another inline assembly with different pattern */
            asm volatile (
                "imull %1, %0\n\t"
                "addl $1, %0"
                : "+r"(sum)
                : "r"(i)
                : "cc"
            );
        }
        
        /* Additional computation to increase scheduling complexity */
        int idx = (i * 3) % n;
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %%eax, %0"
            : "+r"(sum)
            : "r"(a[idx])
            : "%eax", "cc"
        );
    }
    
    return sum;
}

/* Second function with nested loops for more scheduling opportunities */
void process_matrix(int mat[SIZE][SIZE], int factor) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            /* Complex conditional with multiple basic blocks */
            if (mat[i][j] > 0) {
                mat[i][j] *= factor;
                
                /* Inline assembly with memory operand */
                asm volatile (
                    "movl (%1), %%eax\n\t"
                    "addl %%eax, (%0)\n\t"
                    "incl (%0)"
                    : 
                    : "r"(&mat[i][j]), "r"(&factor)
                    : "%eax", "memory", "cc"
                );
            } else if (mat[i][j] < 0) {
                mat[i][j] /= factor;
                
                /* Another inline assembly variant */
                asm volatile (
                    "cltd\n\t"
                    "idivl %1\n\t"
                    "movl %%eax, (%0)"
                    : 
                    : "r"(&mat[i][j]), "r"(&factor)
                    : "%eax", "%edx", "memory", "cc"
                );
            }
            
            /* Use __builtin_assume to provide optimization hints */
            if (j > 0) {
                int diff = mat[i][j] - mat[i][j-1];
                __builtin_assume(diff < 1000);
            }
        }
    }
}

/* Third function with switch statement for control flow variety */
int switch_computation(int x, int *arr, int n) {
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        switch (x % 4) {
            case 0:
                result += arr[i] * 2;
                asm volatile ("shll $1, %0" : "+r"(result) :: "cc");
                break;
            case 1:
                result -= arr[i];
                asm volatile ("negl %0" : "+r"(arr[i]) :: "cc");
                break;
            case 2:
                result ^= arr[i];
                asm volatile ("xorl %1, %0" : "+r"(result) : "r"(arr[i]) : "cc");
                break;
            case 3:
                result |= arr[i];
                asm volatile ("orl %1, %0" : "+r"(result) : "r"(arr[i]) : "cc");
                break;
        }
        
        x = (x * 1103515245 + 12345) & 0x7fffffff;
    }
    
    return result;
}

int main() {
    /* Initialize data arrays */
    int a[SIZE], b[SIZE];
    int matrix[SIZE][SIZE];
    
    /* Initialize with predictable but non-constant values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
    }
    
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = (i * j) % 200 - 100;
        }
    }
    
    /* Call functions to create scheduling regions */
    int sum1 = compute_sum(a, b, SIZE);
    process_matrix(matrix, 3);
    int sum2 = switch_computation(42, a, SIZE);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d %d\n", sum1, sum2);
    printf("Matrix[0][0] = %d\n", matrix[0][0]);
    
    /* Verify computation is correct */
    int verify_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        int temp = a[i] * b[i];
        if (temp > THRESHOLD) {
            verify_sum += temp;
        }
    }
    
    if (sum1 != verify_sum) {
        printf("Verification failed: %d != %d\n", sum1, verify_sum);
        return 1;
    }
    
    return 0;
}
