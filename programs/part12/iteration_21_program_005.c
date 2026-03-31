/* test_sel_sched.c - Test program to trigger selective scheduler debug output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100

/* Function with tight, data-dependent loop that benefits from pipelining */
int compute_sum(int *a, int *b, int n) {
    int sum = 0;
    
    /* Main loop with data dependency - creates scheduling region */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int prod = a[i] * b[i];
        
        /* Conditional control flow to create multiple basic blocks */
        if (prod > THRESHOLD) {
            /* Complex computation path */
            sum += prod;
            
            /* Inline assembly to generate non-trivial RTL patterns */
            asm volatile (
                "addl %1, %0\n\t"
                "cmpl $1000, %0\n\t"
                "jle 1f\n\t"
                "subl $500, %0\n"
                "1:\n\t"
                : "+r"(sum)
                : "r"(prod)
                : "cc", "memory"
            );
        } else {
            /* Alternative path with different computation */
            sum += prod / 2;
            
            /* Another inline assembly with clobbers */
            asm volatile (
                "movl %1, %%eax\n\t"
                "shrl $1, %%eax\n\t"
                "addl %%eax, %0\n\t"
                : "+r"(sum)
                : "r"(prod)
                : "%eax", "cc"
            );
        }
        
        /* Additional computation to increase instruction count */
        if (i % 16 == 0) {
            /* Nested conditional for more complex control flow */
            asm volatile (
                "testl $1, %0\n\t"
                "jz 1f\n\t"
                "incl %0\n"
                "1:\n\t"
                : "+r"(sum)
                :
                : "cc"
            );
        }
    }
    
    return sum;
}

/* Second function with different loop pattern */
int compute_weighted_sum(int *a, int *b, int *weights, int n) {
    int total = 0;
    
    /* Unrolled loop pattern */
    for (int i = 0; i < n; i += 4) {
        int sum1 = a[i] * weights[i];
        int sum2 = b[i] * weights[i];
        
        /* Complex conditional with multiple branches */
        if (sum1 > sum2) {
            total += sum1 - sum2;
            
            /* Assembly with multiple outputs */
            int diff;
            asm volatile (
                "movl %2, %%eax\n\t"
                "subl %3, %%eax\n\t"
                "addl %%eax, %0\n\t"
                "movl %%eax, %1\n\t"
                : "+r"(total), "=r"(diff)
                : "r"(sum1), "r"(sum2)
                : "%eax", "cc"
            );
        } else {
            total += sum2 - sum1;
            
            /* Different assembly pattern */
            asm volatile (
                "xorl %%eax, %%eax\n\t"
                "movl %2, %%ebx\n\t"
                "movl %3, %%ecx\n\t"
                "cmpl %%ebx, %%ecx\n\t"
                "cmovgl %%ecx, %%eax\n\t"
                "addl %%eax, %0\n\t"
                : "+r"(total)
                : "r"(sum1), "r"(sum2)
                : "%eax", "%ebx", "%ecx", "cc"
            );
        }
        
        /* Prevent dead code elimination */
        asm volatile ("" : : "r"(total) : "memory");
    }
    
    return total;
}

/* Function with pointer chasing loop */
int linked_list_sum(int *data, int *next, int start, int steps) {
    int sum = 0;
    int idx = start;
    int count = 0;
    
    /* Loop with unpredictable control flow */
    while (idx != -1 && count < steps) {
        sum += data[idx];
        
        /* Conditional with side effect */
        if (sum < 0) {
            sum = 0;
            asm volatile ("nop\n\t" : : : "memory");
        }
        
        idx = next[idx];
        count++;
        
        /* Memory barrier to prevent reordering */
        asm volatile ("" : : : "memory");
    }
    
    return sum;
}

int main() {
    /* Initialize arrays with predictable but non-trivial patterns */
    int a[SIZE], b[SIZE], weights[SIZE], next[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
        weights[i] = (i % 10) + 1;
        next[i] = (i + 1) % SIZE;
    }
    next[SIZE - 1] = -1;
    
    /* Call functions to create multiple scheduling regions */
    int sum1 = compute_sum(a, b, SIZE);
    int sum2 = compute_weighted_sum(a, b, weights, SIZE);
    int sum3 = linked_list_sum(a, next, 0, SIZE / 2);
    
    /* Use results to prevent dead code elimination */
    int total = sum1 + sum2 + sum3;
    
    /* Print result for verification */
    printf("Result: %d\n", total);
    
    /* Additional computation to increase optimization opportunities */
    volatile int check = 0;
    for (int i = 0; i < 100; i++) {
        check += a[i % SIZE];
        
        /* Force compiler to keep this loop */
        asm volatile ("" : "+r"(check) : : "memory");
    }
    
    return (total > 0) ? 0 : 1;
}
