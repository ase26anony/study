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
            /* Complex computation that creates scheduling opportunities */
            sum += temp;
            
            /* Inline assembly to generate non-trivial RTL patterns */
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
            /* Alternative path with different operations */
            sum -= temp / 2;
            
            /* Another inline assembly with multiple clobbers */
            asm volatile (
                "imull $3, %0\n\t"
                "addl $1, %0"
                : "+r"(sum)
                :
                : "cc"
            );
        }
        
        /* Additional computation to increase instruction count */
        a[i] = (a[i] + b[i]) & 0xFF;
    }
    
    return sum;
}

/* Second function with nested loops for more complex scheduling */
int matrix_multiply(int size) {
    int result = 0;
    
    /* Create arrays for matrix-like operations */
    int mat1[SIZE];
    int mat2[SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < size; i++) {
        mat1[i] = i * 2;
        mat2[i] = i * 3;
    }
    
    /* Nested loops create interesting scheduling regions */
    for (int i = 0; i < size; i += 8) {
        int block_sum = 0;
        
        /* Inner loop with unroll-friendly pattern */
        for (int j = 0; j < 8; j++) {
            int idx = i + j;
            if (idx < size) {
                int prod = mat1[idx] * mat2[idx];
                
                /* Conditional with multiple operations */
                if (prod & 1) {
                    block_sum += prod;
                    
                    /* Complex inline assembly */
                    asm volatile (
                        "movl %1, %%eax\n\t"
                        "shrl $2, %%eax\n\t"
                        "addl %%eax, %0\n\t"
                        "testl $0xF, %0\n\t"
                        "setnz %%al\n\t"
                        "movzbl %%al, %%eax\n\t"
                        "addl %%eax, %0"
                        : "+r"(block_sum)
                        : "r"(prod)
                        : "eax", "cc"
                    );
                } else {
                    block_sum -= prod >> 1;
                }
            }
        }
        
        result += block_sum;
    }
    
    return result;
}

/* Function with switch statement for varied control flow */
int process_with_switch(int *data, int n) {
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        int val = data[i];
        
        /* Switch creates multiple basic blocks */
        switch (val & 0x3) {
            case 0:
                total += val * 2;
                asm volatile ("nop" ::: "cc");
                break;
            case 1:
                total += val * 3;
                asm volatile ("nop" ::: "cc");
                break;
            case 2:
                total += val * 4;
                asm volatile ("nop\n\tnop" ::: "cc");
                break;
            case 3:
                total += val * 5;
                asm volatile ("nop\n\tnop\n\tnop" ::: "cc");
                break;
        }
    }
    
    return total;
}

int main() {
    int array_a[SIZE];
    int array_b[SIZE];
    
    /* Initialize arrays with predictable but non-trivial patterns */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (i * 17) & 0xFF;
        array_b[i] = (i * 23) & 0xFF;
    }
    
    /* Call functions to create scheduling regions */
    int sum1 = compute_sum(array_a, array_b, SIZE);
    int sum2 = matrix_multiply(SIZE / 4);
    int sum3 = process_with_switch(array_a, SIZE);
    
    /* Use results to prevent dead code elimination */
    int final_result = sum1 + sum2 + sum3;
    
    /* Print result to ensure computation happens */
    printf("Result: %d\n", final_result);
    
    /* Additional loop to ensure scheduler sees hot code */
    volatile int sink = 0;
    for (int i = 0; i < 1000; i++) {
        sink += array_a[i % SIZE] * array_b[i % SIZE];
    }
    
    return (final_result > 0) ? 0 : 1;
}
