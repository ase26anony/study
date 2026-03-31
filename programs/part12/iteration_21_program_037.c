/* test_sel_sched.c - Program to trigger selective scheduler debugging output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight data-dependent loop that benefits from pipelining */
int compute_sum(int* restrict a, int* restrict b, int n) {
    int sum = 0;
    
    /* Main loop with data dependency and conditional control flow */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int temp = a[i] * b[i];
        
        /* Conditional control flow to create multiple basic blocks */
        if (temp > THRESHOLD) {
            /* Complex inline assembly to generate non-trivial RTL patterns */
            asm volatile (
                "addl %1, %0\n\t"
                "cmpl $1000, %0\n\t"
                "jle 1f\n\t"
                "subl $500, %0\n"
                "1:\n\t"
                : "+r"(sum)
                : "r"(temp)
                : "cc"
            );
        } else {
            /* Another inline assembly with different pattern */
            asm volatile (
                "imull $2, %1\n\t"
                "addl %1, %0"
                : "+r"(sum)
                : "r"(temp)
                : "cc"
            );
        }
        
        /* Additional computation to increase scheduling complexity */
        int extra = a[i] + b[i];
        if (extra & 1) {
            sum += extra;
        }
    }
    
    return sum;
}

/* Second function with nested loops for more scheduling opportunities */
int matrix_multiply(int size) {
    int result = 0;
    
    /* Nested loops create complex scheduling regions */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int val = i * j;
            
            /* Mixed operations with inline assembly */
            asm volatile (
                "movl %1, %%eax\n\t"
                "imull %%eax, %%eax\n\t"
                "addl %%eax, %0"
                : "+r"(result)
                : "r"(val)
                : "%eax", "cc"
            );
            
            /* Conditional with side effect */
            if (val % 3 == 0) {
                result -= val / 2;
            }
        }
    }
    
    return result;
}

/* Function with loop-carried dependency chain */
int dependency_chain(int n) {
    int x = 1;
    
    /* Long dependency chain that requires careful scheduling */
    for (int i = 0; i < n; i++) {
        /* Each iteration depends on previous result */
        x = x * 3 + i;
        
        /* Inline assembly with multiple clobbers */
        asm volatile (
            "testl %0, %0\n\t"
            "jns 1f\n\t"
            "negl %0\n"
            "1:\n\t"
            : "+r"(x)
            :
            : "cc"
        );
        
        /* Branch with computation */
        if (x > 1000000) {
            x = x / 2;
        }
    }
    
    return x;
}

int main() {
    /* Initialize arrays with predictable but non-constant values */
    int a[SIZE], b[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
    }
    
    /* Call functions to ensure they're not optimized away */
    int sum1 = compute_sum(a, b, SIZE);
    int sum2 = matrix_multiply(32);
    int sum3 = dependency_chain(1000);
    
    /* Use results to prevent dead code elimination */
    int total = sum1 + sum2 + sum3;
    
    /* Print result for verification */
    printf("Result: %d\n", total);
    
    /* Additional volatile asm to ensure scheduling happens */
    asm volatile ("" : : "r"(total));
    
    return 0;
}
