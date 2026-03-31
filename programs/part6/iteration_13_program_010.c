/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling dependency analysis
 * with loop-carried dependencies and complex instruction scheduling
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline)) 
int process_loop(int *data, int size, volatile int limit) {
    /* Multiple accumulator variables to create register pressure */
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0, acc5 = 0;
    int acc6 = 0, acc7 = 0, acc8 = 0, acc9 = 0, acc10 = 0;
    
    /* Loop-carried state with true dependency (distance = 1) */
    int state = 0x12345678;
    
    /* Volatile modifiers to prevent constant propagation */
    volatile int mod1 = 7, mod2 = 13, mod3 = 19;
    
    /* Main loop with volatile limit to prevent unrolling */
    for (int i = 0; i < limit; ++i) {
        /* ========== LOOP-CARRIED DEPENDENCY ========== */
        /* True loop-carried dependency: state_i depends on state_{i-1} */
        state = (state * 1103515245 + 12345) ^ data[i % size];
        
        /* ========== INDEPENDENT ARITHMETIC OPERATIONS ========== */
        /* Multiple independent operations to create parallel scheduling opportunities */
        int tmp1 = acc1 * mod1 + i;      /* Multiplication */
        int tmp2 = acc2 & mod2 | i;      /* Bitwise operations */
        int tmp3 = acc3 ^ mod3 ^ state;  /* XOR chain */
        int tmp4 = (acc4 << 3) + (i >> 2); /* Shift operations */
        int tmp5 = acc5 - mod1 * i;      /* Mixed arithmetic */
        
        /* More operations to increase instruction count */
        int tmp6 = tmp1 * tmp2 + tmp3;
        int tmp7 = tmp4 | tmp5 & tmp6;
        int tmp8 = (tmp7 << 2) ^ (tmp6 >> 1);
        int tmp9 = tmp8 * 0x5A827999 + tmp5;
        int tmp10 = tmp9 - tmp4 * tmp3;
        
        /* ========== MEMORY ACCESS WITH VARIABLE INDEXING ========== */
        /* Complex array access with loop-variant indexing */
        int idx = (state + i * mod1) % size;
        int mem_val = data[idx] + data[(idx + mod2) % size];
        
        /* ========== CONDITIONAL CONTROL FLOW ========== */
        /* Branch inside loop to add scheduling complexity */
        if (state & 1) {
            /* Conditional memory access */
            acc1 += data[(state + acc2) % size] * mod1;
            acc2 ^= mem_val + tmp1;
        } else {
            acc1 -= data[(acc3 + i) % size] / (mod2 + 1);
            acc2 |= mem_val * tmp2;
        }
        
        /* Another conditional with different condition */
        if ((i & 3) == 0) {
            acc3 = acc3 * 3 + tmp3;
            acc4 = acc4 ^ (tmp4 << 1);
        } else if ((i & 3) == 1) {
            acc3 = acc3 / 2 + tmp5;
            acc4 = acc4 | (tmp6 >> 2);
        }
        
        /* ========== UPDATE MULTIPLE ACCUMULATORS ========== */
        /* Keep many variables live across iterations */
        acc5 = acc5 + tmp7 * i;
        acc6 = acc6 ^ tmp8 + state;
        acc7 = acc7 | tmp9 - i;
        acc8 = acc8 & tmp10 * mod3;
        acc9 = acc9 + (tmp1 ^ tmp2 ^ tmp3);
        acc10 = acc10 - (tmp4 | tmp5 | tmp6);
        
        /* Additional operations to create more dependencies */
        acc1 = acc1 + (acc2 >> 1);
        acc2 = acc2 ^ (acc3 << 2);
        acc3 = acc3 * 0x9E3779B9 + acc4;
        acc4 = acc4 | (acc5 & 0x7FFFFFFF);
        
        /* Prevent dead code elimination */
        if ((i % 128) == 0) {
            acc6 = acc6 + acc7 - acc8;
            acc7 = acc7 ^ acc9 | acc10;
        }
    }
    
    /* Combine all accumulators to ensure computations aren't optimized away */
    int result = acc1 + acc2 + acc3 + acc4 + acc5 + 
                 acc6 + acc7 + acc8 + acc9 + acc10 + state;
    
    return result;
}

/* Wrapper function to create additional compilation unit context */
__attribute__((noinline))
int complex_loop_wrapper(int seed) {
    /* Initialize data array with pseudo-random values */
    const int DATA_SIZE = 1024;
    int *data = (int*)malloc(DATA_SIZE * sizeof(int));
    
    srand(seed);
    for (int i = 0; i < DATA_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Volatile loop limit to prevent constant propagation */
    volatile int loop_limit = 500 + (seed % 100);
    
    /* Call the main processing loop */
    int result = process_loop(data, DATA_SIZE, loop_limit);
    
    free(data);
    return result;
}

int main() {
    /* Initialize random seed */
    srand(time(NULL));
    
    /* Run multiple iterations to increase chance of scheduling */
    int total_result = 0;
    volatile int outer_loop_limit = 10;
    
    for (int iter = 0; iter < outer_loop_limit; ++iter) {
        int seed = rand() % 10000;
        total_result ^= complex_loop_wrapper(seed);
        
        /* Additional computation to prevent optimization */
        volatile int dummy = iter * 137;
        total_result += dummy & 0xFF;
    }
    
    printf("Final result: %d\n", total_result);
    return 0;
}
