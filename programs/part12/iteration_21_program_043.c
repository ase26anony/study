/* test_sel_sched.c - Test program to trigger selective scheduler debug output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight data-dependent loop that benefits from software pipelining */
int compute_sum(int* restrict a, int* restrict b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies - forces scheduler to work hard */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int temp = a[i] * b[i];
        
        /* Conditional control flow - creates multiple basic blocks */
        if (temp > THRESHOLD) {
            sum += temp;
            
            /* Inline assembly with multiple operands - creates complex RTL */
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
            sum += temp / 2;
            
            /* Another inline assembly with clobbers */
            asm volatile (
                "movl %1, %%eax\n\t"
                "shrl $1, %%eax\n\t"
                "addl %%eax, %0\n\t"
                : "+r"(sum)
                : "r"(temp)
                : "%eax", "cc"
            );
        }
        
        /* Additional computation to increase scheduling pressure */
        a[i] = (a[i] + b[i]) & 0xFF;
        b[i] = (b[i] - a[i]) & 0xFF;
    }
    
    return sum;
}

/* Second function with nested loops for more scheduling regions */
int matrix_multiply(int n) {
    int sum = 0;
    
    /* Nested loops create complex control flow */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int val = i * j;
            
            /* Conditional with multiple branches */
            if (val & 1) {
                sum += val * 3;
                
                /* Complex inline assembly */
                asm volatile (
                    "imull $3, %1\n\t"
                    "addl %1, %0\n\t"
                    "testl $0xF, %0\n\t"
                    "jz 1f\n\t"
                    "andl $0xFFF, %0\n"
                    "1:\n\t"
                    : "+r"(sum)
                    : "r"(val)
                    : "cc"
                );
            } else {
                sum += val / 2;
            }
            
            /* Prevent dead code elimination */
            __asm__ __volatile__("" : "+r"(sum));
        }
    }
    
    return sum;
}

/* Function with loop unrolling hint */
void process_array(int* arr, int n) {
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        /* Pattern that encourages software pipelining */
        arr[i] = arr[i] * 7 + 3;
        
        /* Use builtin to provide optimization hints */
        if (__builtin_expect(arr[i] > 1000, 0)) {
            arr[i] = arr[i] % 256;
        }
    }
}

int main() {
    /* Allocate and initialize arrays */
    int* a = (int*)malloc(SIZE * sizeof(int));
    int* b = (int*)malloc(SIZE * sizeof(int));
    
    if (!a || !b) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3) % 256;
        b[i] = (i * 7) % 256;
    }
    
    /* Call computation functions - these should trigger selective scheduling */
    int sum1 = compute_sum(a, b, SIZE);
    
    /* Process array to create more scheduling opportunities */
    process_array(a, SIZE);
    
    int sum2 = matrix_multiply(32);
    
    /* Use results to prevent dead code elimination */
    printf("Result 1: %d\n", sum1);
    printf("Result 2: %d\n", sum2);
    printf("Array element 0: %d\n", a[0]);
    
    /* Verify computation (simple checksum) */
    int verify = (sum1 > 0 && sum2 > 0) ? 0 : 1;
    
    free(a);
    free(b);
    
    return verify;
}
