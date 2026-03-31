/* test_sel_sched.c - Test program to trigger selective scheduler debug output */
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
        
        /* Conditional control flow to create multiple basic blocks */
        if (temp > THRESHOLD) {
            sum += temp;
        } else {
            /* Use inline assembly to create complex RTL patterns */
            int x = temp;
            int y = i;
            asm volatile (
                "addl %1, %0\n\t"
                "cmpl $0, %0\n\t"
                "setg %%al\n\t"
                "movzbl %%al, %0"
                : "+r"(x)
                : "r"(y)
                : "%al", "cc"
            );
            sum += x;
        }
        
        /* Additional arithmetic to increase scheduling complexity */
        sum = (sum * 13 + 7) & 0xFFF;
    }
    
    return sum;
}

/* Another function with nested loops for more scheduling regions */
void process_matrix(int mat[SIZE][SIZE], int factor) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            /* Complex computation with multiple dependencies */
            int val = mat[i][j];
            
            /* Conditional with inline assembly */
            if (val > 0) {
                asm volatile (
                    "imull %1, %0\n\t"
                    "addl $1, %0"
                    : "+r"(val)
                    : "r"(factor)
                    : "cc"
                );
            } else {
                asm volatile (
                    "negl %0\n\t"
                    "subl $1, %0"
                    : "+r"(val)
                    :
                    : "cc"
                );
            }
            
            mat[i][j] = val;
            
            /* Prevent dead code elimination */
            asm volatile ("" : : "r"(val) : "memory");
        }
    }
}

/* Vector dot product with software pipelining opportunities */
int dot_product(int *x, int *y, int n) {
    int result = 0;
    
    /* Unrolled loop for better scheduling */
    for (int i = 0; i < n; i += 4) {
        int sum0 = x[i] * y[i];
        int sum1 = x[i+1] * y[i+1];
        int sum2 = x[i+2] * y[i+2];
        int sum3 = x[i+3] * y[i+3];
        
        /* Complex accumulation with inline assembly */
        asm volatile (
            "addl %1, %0\n\t"
            "addl %2, %0\n\t"
            "addl %3, %0\n\t"
            "addl %4, %0"
            : "+r"(result)
            : "r"(sum0), "r"(sum1), "r"(sum2), "r"(sum3)
            : "cc"
        );
    }
    
    return result;
}

int main() {
    /* Initialize arrays with predictable but non-constant values */
    int a[SIZE], b[SIZE];
    int matrix[SIZE][SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3 + 7) % 256;
        b[i] = (i * 5 + 11) % 256;
        
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = (i * j + i + j) % 512 - 256;
        }
    }
    
    /* Force compiler to assume loop bounds for scheduling */
    int n = SIZE;
    __builtin_assume(n > 0 && n <= SIZE);
    
    /* Call functions that should trigger selective scheduling */
    int sum1 = compute_sum(a, b, n);
    process_matrix(matrix, 3);
    int sum2 = dot_product(a, b, n);
    
    /* Compute final result to prevent optimization */
    int final_result = sum1 + sum2 + matrix[0][0];
    
    printf("Result: %d\n", final_result);
    
    /* Verify computation is correct */
    int expected = 0;
    for (int i = 0; i < SIZE; i++) {
        int temp = a[i] * b[i];
        if (temp > THRESHOLD) expected += temp;
        else expected += temp + i;
        expected = (expected * 13 + 7) & 0xFFF;
    }
    
    printf("Expected base: %d\n", expected);
    
    return 0;
}
