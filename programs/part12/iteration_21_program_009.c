/* test_sel_sched.c - Test program to trigger selective scheduler debugging */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight data-dependent loop that benefits from pipelining */
int compute_sum(int* restrict a, int* restrict b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies and conditional control flow */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int temp = a[i] * b[i];
        
        /* Conditional control flow to create multiple basic blocks */
        if (temp > THRESHOLD) {
            /* Complex RTL pattern with inline assembly */
            int adjusted;
            asm volatile (
                "imull %1, %0\n\t"
                "addl %2, %0"
                : "+r"(temp)
                : "r"(a[i]), "r"(b[i])
                : "cc"
            );
            
            /* Another inline assembly with multiple outputs */
            int hi, lo;
            asm volatile (
                "movl %2, %%eax\n\t"
                "mull %3\n\t"
                "movl %%eax, %0\n\t"
                "movl %%edx, %1"
                : "=r"(lo), "=r"(hi)
                : "r"(a[i]), "r"(b[i])
                : "eax", "edx", "cc"
            );
            
            adjusted = hi + lo;
            sum += adjusted;
        } else {
            /* Simple path with different operations */
            sum += temp;
        }
        
        /* Additional computation to increase scheduling complexity */
        if (i % 8 == 0) {
            /* Vector-like operation simulated with inline assembly */
            int vec_op;
            asm volatile (
                "paddd %1, %0"
                : "+x"(vec_op)
                : "x"(temp)
                : "cc"
            );
            sum += vec_op;
        }
    }
    
    return sum;
}

/* Second function with nested loops for more scheduling opportunities */
int matrix_multiply(int n) {
    int A[SIZE], B[SIZE], C[SIZE];
    int result = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        A[i] = i;
        B[i] = SIZE - i;
    }
    
    /* Nested loop with complex addressing */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int idx = (i * n + j) % SIZE;
            
            /* Data-dependent computation with inline assembly */
            int prod;
            asm volatile (
                "movl %1, %%eax\n\t"
                "imull %2, %%eax\n\t"
                "movl %%eax, %0"
                : "=r"(prod)
                : "r"(A[idx]), "r"(B[idx])
                : "eax", "cc"
            );
            
            C[idx] = prod;
            
            /* Conditional update */
            if (prod > (i * j)) {
                result += prod;
                
                /* More inline assembly for complex patterns */
                asm volatile (
                    "addl %1, %0\n\t"
                    "adcl $0, %0"
                    : "+r"(result)
                    : "r"(idx)
                    : "cc"
                );
            }
        }
    }
    
    return result;
}

/* Third function with switch statement for control flow variety */
int control_flow_test(int x) {
    int result = 0;
    
    switch (x % 4) {
        case 0:
            asm volatile ("movl $1, %0" : "=r"(result));
            break;
        case 1:
            asm volatile ("movl $2, %0\n\taddl $3, %0" : "=r"(result));
            break;
        case 2:
            asm volatile ("movl $4, %0\n\tsubl $1, %0" : "=r"(result));
            break;
        case 3:
            asm volatile ("movl $8, %0\n\timull $2, %0" : "=r"(result));
            break;
    }
    
    return result;
}

int main() {
    int a[SIZE], b[SIZE];
    int total_sum = 0;
    
    /* Initialize arrays with predictable values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = i * 2;
        b[i] = i * 3;
    }
    
    /* Call functions to create scheduling regions */
    total_sum += compute_sum(a, b, SIZE);
    total_sum += matrix_multiply(32);
    total_sum += control_flow_test(SIZE);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total_sum);
    
    /* Additional loop with __builtin_assume for optimization hints */
    int optimized_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        int val = a[i] + b[i];
        __builtin_assume(val >= 0);  /* Hint to optimizer */
        
        if (val > 1000) {
            /* Complex inline assembly with memory operand */
            asm volatile (
                "lock addl %1, %0"
                : "+m"(optimized_sum)
                : "r"(val)
                : "cc"
            );
        } else {
            optimized_sum += val;
        }
    }
    
    printf("Optimized sum: %d\n", optimized_sum);
    
    return 0;
}
