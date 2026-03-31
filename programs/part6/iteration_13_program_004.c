/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling dependency analysis
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fno-unroll-loops -fno-tree-vectorize -fdump-rtl-sms modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 1024

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline)) 
int process_loop(int *data, int size) {
    volatile int N = 500;  /* Prevent constant propagation */
    volatile int seed = 123;
    
    /* Multiple accumulators to create register pressure */
    int state = seed;
    int sum = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int acc5 = 0, acc6 = 0, acc7 = 0, acc8 = 0;
    
    /* Many temporary variables for high register pressure */
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
    int tmp11, tmp12, tmp13, tmp14, tmp15;
    
    /* Loop with true loop-carried dependency: state depends on previous iteration */
    for (int i = 0; i < N; ++i) {
        /* Loop-carried dependency: state from iteration i used in i+1 */
        state = (state * 1103515245 + 12345) ^ data[i % size];
        
        /* Multiple independent arithmetic operations */
        tmp1 = acc1 * i;          /* Multiplication */
        tmp2 = acc2 + i;          /* Addition */
        tmp3 = tmp1 ^ tmp2;       /* XOR */
        tmp4 = acc3 & i;          /* AND */
        tmp5 = acc4 | i;          /* OR */
        tmp6 = tmp3 << 2;         /* Shift */
        tmp7 = tmp4 >> 1;         /* Shift */
        tmp8 = tmp5 * 3;          /* Multiplication */
        tmp9 = tmp6 + tmp7;       /* Addition */
        tmp10 = tmp8 - tmp9;      /* Subtraction */
        tmp11 = tmp10 * 7;        /* Multiplication */
        tmp12 = state % 17;       /* Modulo */
        tmp13 = tmp11 & 0xFF;     /* AND with constant */
        tmp14 = tmp12 | 0x80;     /* OR with constant */
        tmp15 = tmp13 ^ tmp14;    /* XOR */
        
        /* Memory access with variable indexing (creates address calculation) */
        int idx = (i + state) % size;
        int val = data[idx] * 2;
        
        /* Conditional execution based on loop-variant condition */
        if (state & 1) {  /* Branch inside loop */
            sum += data[state % size];
            acc1 += val * 3;
        } else {
            acc2 += val / 2;
        }
        
        /* More arithmetic to keep variables live */
        acc3 = acc3 + tmp15;
        acc4 = acc4 ^ tmp10;
        acc5 = acc5 * 2 + tmp1;
        acc6 = acc6 | tmp2;
        acc7 = acc7 & tmp3;
        acc8 = acc8 + tmp4;
        
        /* Additional loop-carried dependency chain */
        acc1 = acc1 + acc2;  /* acc1 depends on previous acc1 and acc2 */
        acc2 = acc2 ^ acc3;  /* acc2 depends on previous acc2 and acc3 */
        
        /* Complex expression with multiple operations */
        sum = sum + ((acc1 * acc2) >> 1) + ((acc3 & acc4) << 2);
    }
    
    /* Combine all accumulators to prevent dead code elimination */
    int result = state + sum + acc1 + acc2 + acc3 + acc4 + 
                 acc5 + acc6 + acc7 + acc8;
    
    return result;
}

int main() {
    /* Initialize array with pseudo-random values */
    int data[ARRAY_SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Process the loop */
    int result = process_loop(data, ARRAY_SIZE);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    /* Additional volatile operations to prevent optimization */
    volatile int check = result;
    if (check > 1000000) {
        printf("Large result detected\n");
    }
    
    return 0;
}
