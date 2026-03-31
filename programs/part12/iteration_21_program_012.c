/* test_sel_sched_coverage.c
 * Designed to trigger selective scheduler debugging output
 * Specifically targets sel_print_insn() in sel-sched-dump.cc
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 100
#define ITERATIONS 1000000

/* Force loop to be unrolled and scheduled */
__attribute__((noinline))
static int process_array(int *a, int *b, int size) {
    int sum = 0;
    int i;
    
    /* Tight loop with data dependencies - good for software pipelining */
    for (i = 0; i < size; i++) {
        /* Data-dependent computation with carried dependency */
        int temp = a[i] * b[i];
        
        /* Conditional control flow to create multiple basic blocks */
        if (temp > THRESHOLD) {
            sum += temp;
            
            /* Inline assembly with multiple operands to create complex RTL */
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
                "imull $3, %0\n\t"
                "andl $0xFF, %0"
                : "+r"(temp)
                :: "cc"
            );
        }
        
        /* Additional computation to increase instruction count */
        a[i] = (a[i] + b[i]) & 0xFF;
        b[i] = (b[i] - a[i]) & 0xFF;
    }
    
    return sum;
}

/* Another function with different pattern to increase scheduling complexity */
__attribute__((noinline))
static int nested_loops(int *arr, int n) {
    int total = 0;
    int i, j;
    
    /* Nested loops create interesting scheduling regions */
    for (i = 0; i < n; i++) {
        int acc = arr[i];
        
        /* Inner loop with small trip count */
        for (j = 0; j < 4; j++) {
            acc = (acc * 1103515245 + 12345) & 0x7FFFFFFF;
            
            /* Conditional with inline assembly */
            if (acc & 1) {
                asm volatile (
                    "rorl $13, %0\n\t"
                    "xorl $0xDEADBEEF, %0"
                    : "+r"(acc)
                    :: "cc"
                );
            }
        }
        
        total ^= acc;
    }
    
    return total;
}

/* Main function with multiple hot loops */
int main(void) {
    int *array1, *array2;
    int i, result1, result2;
    
    /* Allocate and initialize arrays */
    array1 = (int*)malloc(SIZE * sizeof(int));
    array2 = (int*)malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (i = 0; i < SIZE; i++) {
        array1[i] = (i * 1103515245 + 12345) & 0xFF;
        array2[i] = (i * 1664525 + 1013904223) & 0xFF;
    }
    
    /* Process multiple times to ensure loops are hot */
    result1 = 0;
    for (i = 0; i < ITERATIONS / SIZE; i++) {
        result1 += process_array(array1, array2, SIZE);
    }
    
    /* Call another scheduling-intensive function */
    result2 = nested_loops(array1, SIZE);
    
    /* Use results to prevent dead code elimination */
    printf("Result1: %d\n", result1);
    printf("Result2: %d\n", result2);
    printf("Final: %d\n", result1 ^ result2);
    
    free(array1);
    free(array2);
    
    return 0;
}
