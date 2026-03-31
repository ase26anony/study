/* test_sel_sched.c - Test program to trigger sel_print_insn coverage */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight data-dependent loop that benefits from pipelining */
int compute_sum_with_conditions(int* a, int* b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies and conditional control flow */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int temp = a[i] * b[i];
        
        /* Conditional control flow creating multiple basic blocks */
        if (a[i] > THRESHOLD) {
            /* Complex computation path */
            temp += a[i] - b[i];
            
            /* Inline assembly with multiple operands to create complex RTL */
            asm volatile (
                "addl %1, %0\n\t"
                "cmpl %2, %0\n\t"
                "setg %%al\n\t"
                "movzbl %%al, %0"
                : "+r"(temp)
                : "r"(b[i]), "r"(THRESHOLD)
                : "cc", "al"
            );
        } else {
            /* Alternative path with different operations */
            temp = temp >> 1;
            
            /* Another inline assembly with clobbers */
            asm volatile (
                "imull %1, %0\n\t"
                "addl $1, %0"
                : "+r"(temp)
                : "r"(i)
                : "cc"
            );
        }
        
        /* Final accumulation with dependency chain */
        sum += temp;
        
        /* Additional inline assembly to create more scheduling complexity */
        asm volatile (
            "testl %0, %0\n\t"
            "jns 1f\n\t"
            "negl %0\n\t"
            "1:\n\t"
            "addl %%ecx, %0"
            : "+r"(sum)
            : "c"(i)
            : "cc"
        );
    }
    
    return sum;
}

/* Second function with nested loops for larger scheduling regions */
void process_matrix(int mat[SIZE][SIZE], int factor) {
    int acc = 0;
    
    /* Nested loops create complex control flow */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            /* Data-dependent computation */
            int val = mat[i][j];
            
            /* Multiple conditionals creating basic blocks */
            if (val > 0) {
                val *= factor;
                
                /* Inline assembly with memory operand */
                asm volatile (
                    "movl %1, %%eax\n\t"
                    "cltd\n\t"
                    "idivl %2\n\t"
                    "movl %%eax, %0"
                    : "=r"(val)
                    : "r"(val), "r"(factor + 1)
                    : "eax", "edx", "cc"
                );
            } else if (val < 0) {
                val = -val;
                
                /* Assembly with multiple outputs */
                int quotient, remainder;
                asm volatile (
                    "movl %2, %%eax\n\t"
                    "cltd\n\t"
                    "idivl %3\n\t"
                    "movl %%eax, %0\n\t"
                    "movl %%edx, %1"
                    : "=r"(quotient), "=r"(remainder)
                    : "r"(val), "r"(factor)
                    : "eax", "edx", "cc"
                );
                val = quotient + remainder;
            }
            
            /* Conditional store */
            if (val != 0) {
                mat[i][j] = val;
                acc += val;
            }
        }
        
        /* Loop-carried dependency */
        asm volatile (
            "addl %1, %0\n\t"
            "rorl $3, %0"
            : "+r"(acc)
            : "r"(i)
            : "cc"
        );
    }
}

/* Third function with switch statement for varied control flow */
int switch_based_computation(int x, int mode) {
    int result = 0;
    
    switch (mode % 4) {
        case 0:
            result = x * 2;
            asm volatile ("shll $1, %0" : "+r"(result) :: "cc");
            break;
        case 1:
            result = x + x;
            asm volatile ("leal (%0,%0), %0" : "+r"(result) ::);
            break;
        case 2:
            result = x >> 1;
            asm volatile ("sarl $1, %0" : "+r"(result) :: "cc");
            break;
        case 3:
            result = x ^ 0xAAAA;
            asm volatile ("xorl $0xAAAA, %0" : "+r"(result) ::);
            break;
    }
    
    return result;
}

int main() {
    /* Initialize arrays with predictable but non-constant values */
    int a[SIZE], b[SIZE];
    int matrix[SIZE][SIZE];
    
    /* Initialize with pattern to avoid dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3) % 1000;
        b[i] = (i * 7) % 1000;
        
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = (i * j) % 2000 - 1000;
        }
    }
    
    /* Force compiler to assume loops execute */
    __builtin_assume(SIZE > 0);
    
    /* Call functions to create scheduling regions */
    int sum1 = compute_sum_with_conditions(a, b, SIZE);
    process_matrix(matrix, 3);
    int sum2 = switch_based_computation(sum1, 2);
    
    /* Use results to prevent optimization */
    printf("Result 1: %d\n", sum1);
    printf("Result 2: %d\n", sum2);
    
    /* Compute checksum to verify correctness */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= a[i] ^ b[i];
    }
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
