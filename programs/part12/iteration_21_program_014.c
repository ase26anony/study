/* test_sel_sched.c - Program to trigger selective scheduler debugging output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight data-dependent loop for selective scheduling */
int compute_sum(int *a, int *b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies - encourages pipelining */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int prod = a[i] * b[i];
        
        /* Conditional control flow to create multiple basic blocks */
        if (prod > THRESHOLD) {
            /* Complex inline assembly to generate non-trivial RTL */
            asm volatile (
                "addl %1, %0\n\t"
                "cmpl $1000, %0\n\t"
                "jle 1f\n\t"
                "subl $500, %0\n"
                "1:"
                : "+r"(sum)
                : "r"(prod)
                : "cc"
            );
        } else {
            /* Another inline assembly with different pattern */
            asm volatile (
                "imull $2, %1\n\t"
                "addl %1, %0"
                : "+r"(sum)
                : "r"(prod)
                : "cc"
            );
        }
        
        /* Additional computation to increase scheduling complexity */
        int temp = a[i] + b[i];
        __builtin_assume(temp < 10000); /* Help compiler with assumptions */
        
        /* Nested loop with small iteration count */
        for (int j = 0; j < 4; j++) {
            temp = temp * 3 + j;
        }
        
        /* Use temp to avoid dead code elimination */
        if (temp < 0) {
            sum += temp; /* This path should never be taken */
        }
    }
    
    return sum;
}

/* Second function with different loop pattern */
int compute_weighted_sum(int *arr, int n) {
    int weighted_sum = 0;
    int weight = 7;
    
    /* Loop with pointer arithmetic and multiple operations */
    for (int i = 0; i < n; i += 2) {
        int val1 = arr[i];
        int val2 = (i + 1 < n) ? arr[i + 1] : 0;
        
        /* Complex expression with multiple dependencies */
        int combined = (val1 * weight) + (val2 * (weight - 1));
        
        /* Conditional with side effect */
        if (combined & 1) {
            /* Inline assembly with multiple clobbers */
            asm volatile (
                "rorl $3, %0\n\t"
                "andl $0xFFF, %0"
                : "+r"(combined)
                :
                : "cc"
            );
            weighted_sum += combined;
        } else {
            weighted_sum -= combined / 2;
        }
        
        /* Modify weight to create loop-carried dependency */
        weight = (weight * 13 + 1) & 0xFF;
    }
    
    return weighted_sum;
}

/* Function with switch statement for additional control flow */
int process_with_switch(int *data, int n) {
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        int val = data[i];
        
        /* Switch creates multiple basic blocks */
        switch (val % 5) {
            case 0:
                result += val;
                asm volatile ("nop" ::: "memory");
                break;
            case 1:
                result -= val * 2;
                break;
            case 2:
                result ^= val;
                asm volatile ("nop" ::: "memory");
                break;
            case 3:
                result = (result << 3) | (val & 7);
                break;
            default:
                result = (result + val) * 3;
                /* Complex assembly with input/output constraints */
                asm volatile (
                    "movl %1, %%eax\n\t"
                    "shrl $2, %%eax\n\t"
                    "addl %%eax, %0"
                    : "+r"(result)
                    : "r"(val)
                    : "%eax", "cc"
                );
        }
    }
    
    return result;
}

int main() {
    /* Initialize arrays with predictable but non-constant values */
    int a[SIZE], b[SIZE], c[SIZE * 2];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3 + 1) % 100;
        b[i] = (i * 7 + 3) % 100;
    }
    
    for (int i = 0; i < SIZE * 2; i++) {
        c[i] = (i * 11 + 5) % 200;
    }
    
    /* Call functions to create scheduling regions */
    int sum1 = compute_sum(a, b, SIZE);
    int sum2 = compute_weighted_sum(c, SIZE * 2);
    int sum3 = process_with_switch(a, SIZE);
    
    /* Use results to prevent dead code elimination */
    int total = sum1 + sum2 + sum3;
    
    /* Print result for verification */
    printf("Result: %d\n", total);
    
    /* Additional loop with volatile to force scheduling */
    volatile int counter = 0;
    for (int i = 0; i < 100; i++) {
        counter += i * i;
        asm volatile ("" ::: "memory");
    }
    
    return (total > 0) ? 0 : 1;
}
