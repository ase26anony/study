/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling dependency analysis
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fno-unroll-loops -fno-tree-vectorize -fdump-rtl-sms modulo-sched-test.c -o modulo-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 1024

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline))
int process_loop(int *data, int size) {
    volatile int N = 500;  /* Prevent constant propagation */
    int i;
    
    /* Multiple accumulator variables to create register pressure */
    int state = 123456789;
    int sum = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0, acc5 = 0;
    int acc6 = 0, acc7 = 0, acc8 = 0, acc9 = 0, acc10 = 0;
    
    /* Many temporary variables for high register pressure */
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
    int tmp11, tmp12, tmp13, tmp14, tmp15;
    
    /* Modifiers to prevent constant folding */
    volatile int mod1 = 1103515245;
    volatile int mod2 = 12345;
    volatile int mod3 = 7;
    volatile int mod4 = 13;
    
    for (i = 0; i < N; i++) {
        /* 1. Loop-carried dependency: state depends on previous iteration */
        state = (state * mod1 + mod2) ^ data[i % size];
        
        /* 2. Multiple independent arithmetic operations */
        tmp1 = acc1 * i;           /* Multiplication */
        tmp2 = acc2 + i;           /* Addition */
        tmp3 = tmp1 ^ tmp2;        /* XOR */
        tmp4 = acc3 & i;           /* AND */
        tmp5 = acc4 | i;           /* OR */
        tmp6 = acc5 - i;           /* Subtraction */
        tmp7 = tmp4 * mod3;        /* Another multiplication */
        tmp8 = tmp5 + mod4;        /* Another addition */
        tmp9 = tmp6 ^ tmp7;        /* More XOR */
        tmp10 = tmp8 & tmp9;       /* AND of two temporaries */
        
        /* 3. Memory access with variable indexing */
        if (state & 1) {           /* Conditional execution */
            sum += data[(state + i) % size];
        }
        
        /* 4. More arithmetic with loop-carried dependencies */
        tmp11 = acc6 * state;      /* Depends on state from this iteration */
        tmp12 = acc7 + tmp11;      /* Chain of dependencies */
        tmp13 = acc8 ^ tmp12;
        tmp14 = acc9 & tmp13;
        tmp15 = acc10 | tmp14;
        
        /* 5. Update accumulators with loop-carried dependencies */
        acc1 = tmp3 + 1;           /* Depends on tmp3 from this iteration */
        acc2 = tmp10 * 2;          /* Will be used in next iteration */
        acc3 = acc1 ^ acc2;        /* Cross-accumulator dependency */
        acc4 = acc3 + i;           /* Mixed with loop counter */
        acc5 = acc4 & 0xFF;        /* Mask operation */
        acc6 = tmp15;              /* Carry forward */
        acc7 = acc6 + acc1;        /* Combine two accumulators */
        acc8 = acc7 * mod3;
        acc9 = acc8 ^ state;       /* Mix with state */
        acc10 = acc9 % 17;         /* Modulo operation */
        
        /* 6. Additional conditional operation */
        if ((i & 3) == 0) {        /* Another branch for scheduling complexity */
            tmp1 = data[(i * 2) % size];  /* Different indexing pattern */
            acc2 = acc2 ^ tmp1;
        }
        
        /* 7. More parallel operations to fill VLIW slots */
        tmp4 = (acc3 << 2) | (acc4 >> 1);  /* Shift operations */
        tmp5 = (acc5 * 3) + (acc6 / 2);    /* Mixed mul/div */
        tmp6 = tmp4 ^ tmp5;
        
        /* 8. Final accumulation with true loop-carried dependency */
        sum = sum + tmp6;          /* sum depends on sum from previous iteration */
    }
    
    /* Combine all accumulators to prevent dead code elimination */
    int result = sum + acc1 + acc2 + acc3 + acc4 + acc5 + 
                 acc6 + acc7 + acc8 + acc9 + acc10 + state;
    
    return result;
}

int main() {
    int data[ARRAY_SIZE];
    int i, result;
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = rand() % 1000;
    }
    
    /* Process the loop */
    result = process_loop(data, ARRAY_SIZE);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    return 0;
}
