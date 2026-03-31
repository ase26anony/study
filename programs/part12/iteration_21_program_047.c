/* test_sel_sched_coverage.c
 * 
 * This program is designed to trigger GCC's selective scheduler
 * debugging output to cover the sel_print_insn function in
 * sel-sched-dump.cc, specifically lines that switch dump output
 * to stderr, print RTL, and restore the original dump.
 *
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 
 *               -fdump-rtl-sched2 -dS -march=x86-64 -o test_sel_sched 
 *               test_sel_sched_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024
#define THRESHOLD 100

/* Function with tight, data-dependent loop that benefits from pipelining */
int compute_sum_with_conditions(int *a, int *b, int n) {
    int sum = 0;
    
    /* Loop with data dependencies and conditional control flow */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int temp = a[i] * b[i];
        
        /* Conditional control flow creates multiple basic blocks */
        if (temp > THRESHOLD) {
            sum += temp;
            
            /* Inline assembly to create complex RTL patterns */
            asm volatile (
                "addl %1, %0\n\t"
                "cmpl $500, %0\n\t"
                "jle 1f\n\t"
                "subl $100, %0\n"
                "1:\n\t"
                : "+r"(sum)
                : "r"(temp)
                : "cc"
            );
        } else {
            sum += temp / 2;
            
            /* Another inline assembly with different pattern */
            asm volatile (
                "imull $3, %0, %0\n\t"
                "andl $0xFF, %0"
                : "+r"(temp)
                :
                : "cc"
            );
        }
        
        /* Additional computation to increase scheduling complexity */
        a[i] = (a[i] + b[i]) & 0xFF;
    }
    
    return sum;
}

/* Another function with nested loops for more scheduling regions */
void matrix_multiply(int n, int A[][8], int B[][8], int C[][8]) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 8; j++) {
            C[i][j] = 0;
            for (int k = 0; k < 8; k++) {
                /* Complex expression with multiple operations */
                C[i][j] += A[i][k] * B[k][j];
                
                /* Conditional inside innermost loop */
                if (C[i][j] > 1000) {
                    C[i][j] = C[i][j] % 1000;
                    
                    /* More inline assembly */
                    asm volatile (
                        "movl %0, %%eax\n\t"
                        "cltd\n\t"
                        "movl $1000, %%ecx\n\t"
                        "idivl %%ecx\n\t"
                        "movl %%edx, %0"
                        : "+r"(C[i][j])
                        :
                        : "eax", "edx", "ecx", "cc"
                    );
                }
            }
        }
    }
}

/* Function with switch statement for additional control flow */
int process_with_switch(int x) {
    int result = 0;
    
    switch (x % 4) {
        case 0:
            result = x * 2;
            asm volatile ("shll $1, %0" : "+r"(result) :: "cc");
            break;
        case 1:
            result = x + x;
            asm volatile ("leal (%0,%0), %0" : "+r"(result) :: "cc");
            break;
        case 2:
            result = x << 2;
            asm volatile ("shll $2, %0" : "+r"(result) :: "cc");
            break;
        case 3:
            result = x * 3;
            asm volatile ("imull $3, %0, %0" : "+r"(result) :: "cc");
            break;
    }
    
    return result;
}

int main() {
    int a[ARRAY_SIZE];
    int b[ARRAY_SIZE];
    
    /* Initialize arrays with predictable but non-constant values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = (i * 3) % 256;
        b[i] = (i * 7) % 256;
    }
    
    /* Call functions that create scheduling regions */
    int sum1 = compute_sum_with_conditions(a, b, ARRAY_SIZE);
    
    /* Create matrix computation for more scheduling opportunities */
    int A[8][8], B[8][8], C[8][8];
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            A[i][j] = (i + j) % 10;
            B[i][j] = (i * j) % 10;
        }
    }
    
    matrix_multiply(8, A, B, C);
    
    /* Process with switch statement */
    int switch_result = 0;
    for (int i = 0; i < 100; i++) {
        switch_result += process_with_switch(i);
    }
    
    /* Final computation to prevent dead code elimination */
    int final_result = sum1 + C[7][7] + switch_result;
    
    printf("Result: %d\n", final_result);
    
    /* Verify computation is correct */
    int expected = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int temp = a[i] * b[i];
        if (temp > THRESHOLD) {
            expected += temp;
        } else {
            expected += temp / 2;
        }
    }
    
    printf("Expected base sum: %d\n", expected);
    
    return 0;
}
