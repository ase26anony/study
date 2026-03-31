/* test_sel_sched.c - Test program to trigger selective scheduler debug output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight, data-dependent loop that benefits from software pipelining */
int compute_sum(int *a, int *b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies and conditional control flow */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int temp = a[i] * b[i];
        
        /* Conditional control flow to create multiple basic blocks */
        if (temp > THRESHOLD) {
            /* Complex computation with inline assembly to generate non-trivial RTL */
            int adjusted;
            asm volatile (
                "imull %1, %0\n\t"
                "addl %2, %0"
                : "=r"(adjusted)
                : "r"(temp), "r"(i)
                : "cc"
            );
            sum += adjusted;
        } else {
            /* Different path with another inline assembly */
            int reduced;
            asm volatile (
                "subl %1, %0\n\t"
                "andl $0xFF, %0"
                : "=r"(reduced)
                : "r"(temp)
                : "cc"
            );
            sum += reduced;
        }
        
        /* Additional computation to increase scheduling complexity */
        if (i % 2 == 0) {
            /* Use builtin to help optimization */
            __builtin_assume(a[i] >= 0);
            asm volatile (
                "rorl $4, %0"
                : "+r"(sum)
                :
                : "cc"
            );
        }
    }
    
    return sum;
}

/* Second function with nested loops for more scheduling opportunities */
int matrix_multiply(int *mat1, int *mat2, int *result, int n) {
    int total = 0;
    
    /* Nested loops create complex scheduling regions */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            result[i * n + j] = 0;
            for (int k = 0; k < n; k++) {
                /* Data-dependent computation with multiple operations */
                int prod = mat1[i * n + k] * mat2[k * n + j];
                
                /* Conditional with inline assembly */
                if (prod > 0) {
                    asm volatile (
                        "addl %1, %0\n\t"
                        "cmpl $1000, %0\n\t"
                        "cmovgl %2, %0"
                        : "+r"(result[i * n + j])
                        : "r"(prod), "r"(i)
                        : "cc"
                    );
                } else {
                    asm volatile (
                        "subl %1, %0"
                        : "+r"(result[i * n + j])
                        : "r"(-prod)
                        : "cc"
                    );
                }
                
                total += result[i * n + j];
            }
        }
    }
    
    return total;
}

/* Helper function to initialize arrays */
void init_arrays(int *a, int *b, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
    }
}

int main() {
    int a[SIZE];
    int b[SIZE];
    int result[SIZE];
    
    /* Initialize arrays with predictable values */
    init_arrays(a, b, SIZE);
    
    /* Call functions that should trigger selective scheduling */
    int sum1 = compute_sum(a, b, SIZE);
    
    /* Create matrix for second test */
    int mat1[16];
    int mat2[16];
    int mat_result[16];
    
    for (int i = 0; i < 16; i++) {
        mat1[i] = i;
        mat2[i] = 16 - i;
    }
    
    int sum2 = matrix_multiply(mat1, mat2, mat_result, 4);
    
    /* Use results to prevent dead code elimination */
    printf("Result 1: %d\n", sum1);
    printf("Result 2: %d\n", sum2);
    
    /* Verify results match expectations */
    int expected1 = 0;
    for (int i = 0; i < SIZE; i++) {
        int temp = a[i] * b[i];
        if (temp > THRESHOLD) {
            expected1 += temp * i;
        } else {
            expected1 += temp & 0xFF;
        }
    }
    
    if (sum1 != expected1) {
        printf("Mismatch! Expected %d, got %d\n", expected1, sum1);
        return 1;
    }
    
    printf("Test passed!\n");
    return 0;
}
