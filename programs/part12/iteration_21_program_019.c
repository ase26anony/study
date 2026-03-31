/* test_sel_sched.c - Test program to trigger selective scheduler debugging output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight data-dependent loop that benefits from pipelining */
int compute_sum(int *a, int *b, int n) {
    int sum = 0;
    
    /* Main loop with data dependency and conditional control flow */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int val = a[i] * b[i];
        
        /* Conditional control flow to create multiple basic blocks */
        if (val > THRESHOLD) {
            /* Complex computation with inline assembly */
            int temp = val;
            asm volatile (
                "imul %1, %0\n\t"
                "add $1, %0"
                : "+r" (temp)
                : "r" (i)
                : "cc"
            );
            sum += temp;
        } else {
            /* Alternative path with different computation */
            sum += val / 2;
        }
        
        /* Additional inline assembly with multiple clobbers */
        asm volatile (
            "test %0, %0\n\t"
            "setg %%al"
            : 
            : "r" (val)
            : "rax", "cc"
        );
    }
    
    return sum;
}

/* Second function with nested loops for more scheduling complexity */
int matrix_multiply(int *mat1, int *mat2, int *result, int n) {
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int sum = 0;
            for (int k = 0; k < n; k++) {
                /* Triple nested loop with data dependencies */
                sum += mat1[i * n + k] * mat2[k * n + j];
                
                /* Inline assembly with memory constraint */
                asm volatile (
                    "mov %1, %%eax\n\t"
                    "add %%eax, %0"
                    : "+r" (sum)
                    : "m" (mat1[i * n + k])
                    : "eax", "cc"
                );
            }
            result[i * n + j] = sum;
            total += sum;
            
            /* Conditional with __builtin_expect for branch prediction */
            if (__builtin_expect(sum > 1000, 0)) {
                asm volatile (
                    "shr $2, %0"
                    : "+r" (result[i * n + j])
                    :
                    : "cc"
                );
            }
        }
    }
    
    return total;
}

/* Function with switch statement for control flow variety */
int process_array(int *arr, int n, int mode) {
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        switch (mode) {
            case 0:
                result += arr[i];
                break;
            case 1:
                result -= arr[i];
                /* Inline assembly with output constraint */
                asm volatile (
                    "neg %0"
                    : "+r" (result)
                    :
                    : "cc"
                );
                break;
            case 2:
                result *= arr[i];
                break;
            default:
                result ^= arr[i];
        }
        
        /* Prevent loop unrolling entirely to keep scheduling interesting */
        __asm__ __volatile__("" : "+r" (i));
    }
    
    return result;
}

int main() {
    /* Initialize arrays with predictable but non-constant values */
    int a[SIZE], b[SIZE];
    int mat1[16], mat2[16], result[16];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
    }
    
    for (int i = 0; i < 16; i++) {
        mat1[i] = i;
        mat2[i] = 16 - i;
    }
    
    /* Call functions to create scheduling regions */
    int sum1 = compute_sum(a, b, SIZE);
    int sum2 = matrix_multiply(mat1, mat2, result, 4);
    int sum3 = process_array(a, SIZE, 1);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d %d %d\n", sum1, sum2, sum3);
    
    /* Verify computation is correct */
    int expected_sum1 = 0;
    for (int i = 0; i < SIZE; i++) {
        int val = a[i] * b[i];
        if (val > THRESHOLD) {
            expected_sum1 += val * i + 1;
        } else {
            expected_sum1 += val / 2;
        }
    }
    
    if (sum1 != expected_sum1) {
        fprintf(stderr, "Error: sum1 mismatch\n");
        return 1;
    }
    
    return 0;
}
