/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling dependency analysis
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fno-unroll-loops -fno-tree-vectorize -fdump-rtl-sms -o modulo_test modulo-sched-test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 1024

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline)) 
int complex_loop(int *data, int size) {
    volatile int N = 500;  /* Prevent constant propagation */
    volatile int seed = 42; /* Volatile to prevent optimization */
    
    /* Multiple accumulator variables to create register pressure */
    int state = seed;
    int sum = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0, acc5 = 0;
    int acc6 = 0, acc7 = 0, acc8 = 0, acc9 = 0, acc10 = 0;
    
    /* Loop with true loop-carried dependency */
    for (int i = 0; i < N; ++i) {
        /* Loop-carried dependency: state from iteration i used in i+1 */
        int prev_state = state;
        
        /* Multiple independent arithmetic operations */
        int tmp1 = acc1 * i;           /* Multiplication */
        int tmp2 = acc2 + i;           /* Addition */
        int tmp3 = tmp1 ^ tmp2;        /* XOR */
        int tmp4 = acc3 & i;           /* AND */
        int tmp5 = acc4 | i;           /* OR */
        int tmp6 = acc5 - i;           /* Subtraction */
        int tmp7 = tmp3 << 2;          /* Shift */
        int tmp8 = tmp4 >> 1;          /* Shift */
        int tmp9 = tmp5 * 1103515245;  /* Large constant multiplication */
        int tmp10 = tmp6 + 12345;      /* Constant addition */
        
        /* Complex state update with loop-carried dependency */
        state = (prev_state * 1103515245 + 12345) ^ data[i % size];
        
        /* Memory access with variable indexing */
        int idx = (state + i) % size;
        int val = data[idx] * 3;
        
        /* Conditional execution based on loop-variant condition */
        if (state & 1) {  /* Branch inside loop */
            sum += data[(state + tmp1) % size];
            acc1 += val * 2;
        } else {
            acc2 += val / 2;
        }
        
        /* More arithmetic to increase instruction count */
        acc3 = acc3 * 3 + tmp2;
        acc4 = acc4 ^ tmp3;
        acc5 = acc5 | tmp4;
        acc6 = acc6 + tmp5 * 7;
        acc7 = acc7 - tmp6 / 3;
        acc8 = acc8 & tmp7;
        acc9 = acc9 ^ tmp8;
        acc10 = tmp9 + tmp10;
        
        /* Additional loop-carried dependency chain */
        if (i > 0) {
            int dep_chain = acc1 + acc2;
            acc1 = dep_chain * 2 - acc3;
        }
        
        /* Array access with complex addressing */
        int offset = (i * 7) % size;
        sum += data[offset] * data[(offset + 1) % size];
    }
    
    /* Combine all accumulators to prevent dead code elimination */
    int result = state + sum + acc1 + acc2 + acc3 + acc4 + acc5 +
                 acc6 + acc7 + acc8 + acc9 + acc10;
    
    return result;
}

int main() {
    /* Initialize array with pseudo-random values */
    int data[ARRAY_SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Volatile to prevent constant propagation of loop result */
    volatile int iterations = 3;
    volatile int final_result = 0;
    
    /* Execute multiple times to ensure loop is hot */
    for (int run = 0; run < iterations; ++run) {
        int result = complex_loop(data, ARRAY_SIZE);
        final_result += result;
        
        /* Modify data slightly between runs */
        for (int i = 0; i < ARRAY_SIZE; i += 17) {
            data[i] = (data[i] * 13 + 7) % 1000;
        }
    }
    
    printf("Final result: %d\n", final_result);
    return 0;
}
