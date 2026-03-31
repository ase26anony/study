/* test_sel_sched.c - Test program to trigger sel_print_insn coverage */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight, data-dependent loop that benefits from software pipelining */
int compute_sum(int *a, int *b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies - creates scheduling opportunities */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int temp = a[i] * b[i];
        
        /* Conditional control flow to create multiple basic blocks */
        if (temp > THRESHOLD) {
            /* Complex RTL pattern with inline assembly */
            asm volatile (
                "addl %1, %0\n\t"
                "adcl $0, %0"
                : "+r"(sum)
                : "r"(temp)
                : "cc"
            );
        } else {
            /* Another inline assembly with different pattern */
            asm volatile (
                "subl %1, %0\n\t"
                "cmpl $0, %0"
                : "+r"(sum)
                : "r"(temp)
                : "cc", "eax"
            );
        }
        
        /* Additional computation to increase instruction count in loop */
        int mod = i % 8;
        switch (mod) {
            case 0: sum += 1; break;
            case 1: sum -= 2; break;
            case 2: sum *= 3; break;
            case 3: sum /= 4; break;
            case 4: sum |= 0xFF; break;
            case 5: sum &= 0x0F; break;
            case 6: sum ^= 0xAA; break;
            case 7: sum <<= 1; break;
        }
    }
    
    return sum;
}

/* Another function with nested loops for more complex scheduling regions */
int matrix_multiply(int *mat1, int *mat2, int *result, int n) {
    int total = 0;
    
    /* Nested loops create larger scheduling regions */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int dot = 0;
            for (int k = 0; k < n; k++) {
                /* Data-dependent computation with multiple operations */
                dot += mat1[i * n + k] * mat2[k * n + j];
                
                /* Inline assembly with memory operand */
                asm volatile (
                    "movl %1, %%eax\n\t"
                    "imull %2, %%eax\n\t"
                    "addl %%eax, %0"
                    : "+r"(dot)
                    : "m"(mat1[i * n + k]), "m"(mat2[k * n + j])
                    : "%eax", "cc"
                );
            }
            
            /* Conditional store with side effects */
            if (dot != 0) {
                result[i * n + j] = dot;
                total += dot;
                
                /* Complex asm with multiple clobbers */
                asm volatile (
                    "movl %1, %%ecx\n\t"
                    "roll $3, %%ecx\n\t"
                    "addl %%ecx, %0"
                    : "+r"(total)
                    : "r"(dot)
                    : "%ecx", "cc"
                );
            }
        }
    }
    
    return total;
}

/* Function with pointer chasing to create unpredictable control flow */
int linked_list_sum(int *data, int *next, int start, int steps) {
    int sum = 0;
    int current = start;
    int count = 0;
    
    /* Loop with pointer chasing - hard to schedule */
    while (current != -1 && count < steps) {
        sum += data[current];
        
        /* Inline asm with unpredictable dependency */
        asm volatile (
            "movl %1, %%edx\n\t"
            "leal (%%edx,%%edx,2), %%edx\n\t"
            "addl %%edx, %0"
            : "+r"(sum)
            : "r"(data[current])
            : "%edx", "cc"
        );
        
        current = next[current];
        count++;
        
        /* Conditional with side effect */
        if (count % 16 == 0) {
            asm volatile ("mfence" ::: "memory");
        }
    }
    
    return sum;
}

int main() {
    /* Initialize arrays with predictable but non-trivial patterns */
    int a[SIZE], b[SIZE];
    int mat1[16 * 16], mat2[16 * 16], result[16 * 16];
    int next[SIZE], data[SIZE];
    
    /* Fill arrays with deterministic values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3) % 256;
        b[i] = (i * 5) % 256;
        data[i] = (i * 7) % 512;
        next[i] = (i + 1) % SIZE;
    }
    next[SIZE - 1] = -1;
    
    for (int i = 0; i < 16 * 16; i++) {
        mat1[i] = (i * 11) % 128;
        mat2[i] = (i * 13) % 128;
    }
    
    /* Call functions to ensure they're not optimized away */
    int sum1 = compute_sum(a, b, SIZE);
    int sum2 = matrix_multiply(mat1, mat2, result, 16);
    int sum3 = linked_list_sum(data, next, 0, SIZE / 2);
    
    /* Use results to prevent dead code elimination */
    int total = sum1 + sum2 + sum3;
    
    /* Print result to ensure program executes correctly */
    printf("Result: %d\n", total);
    
    /* Additional volatile operations to create scheduling pressure */
    volatile int check = 0;
    for (int i = 0; i < 1000; i++) {
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, %0"
            : "=r"(check)
            : "r"(check)
            : "%eax", "cc"
        );
    }
    
    return total == 0 ? 1 : 0;
}
