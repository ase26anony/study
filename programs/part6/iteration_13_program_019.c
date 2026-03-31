/* modulo-sched-test.c
 * Test program to trigger GCC's modulo scheduling dependency analysis
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fno-unroll-loops -fno-tree-vectorize -fdump-rtl-sms modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define LOOP_ITERATIONS 500

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline)) 
int complex_loop_operation(int *data, int size, int iterations) {
    volatile int N = iterations;  /* Prevent constant propagation */
    int i, result = 0;
    
    /* Loop-carried state variables */
    int state = 123456789;
    int prev_state = 0;
    
    /* Multiple accumulators to create register pressure */
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int acc5 = 0, acc6 = 0, acc7 = 0, acc8 = 0;
    
    /* Temporary variables for independent operations */
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
    int tmp11, tmp12, tmp13, tmp14, tmp15;
    
    /* Volatile modifiers to prevent optimization */
    volatile int mod1 = 1103515245;
    volatile int mod2 = 12345;
    volatile int mod3 = 67890;
    
    for (i = 0; i < N; ++i) {
        /* ========== LOOP-CARRIED DEPENDENCY (distance=1) ========== */
        /* True loop-carried dependency: state depends on previous iteration */
        prev_state = state;  /* Use previous state before updating */
        state = (state * mod1 + mod2) ^ data[i % size];
        
        /* ========== INDEPENDENT ARITHMETIC OPERATIONS ========== */
        /* Multiple independent operations to create parallel scheduling opportunities */
        tmp1 = i * mod1;          /* Multiplication */
        tmp2 = i + mod2;          /* Addition */
        tmp3 = tmp1 ^ tmp2;       /* XOR */
        tmp4 = tmp1 & tmp2;       /* AND */
        tmp5 = tmp1 | tmp2;       /* OR */
        tmp6 = tmp1 - tmp2;       /* Subtraction */
        tmp7 = tmp3 * tmp4;       /* Chained operation */
        tmp8 = tmp5 ^ tmp6;       /* Another XOR */
        tmp9 = tmp7 & tmp8;       /* AND of results */
        tmp10 = tmp9 << 2;        /* Shift operation */
        
        /* More operations using the loop counter */
        tmp11 = (i * 3) / 2;
        tmp12 = tmp11 % 17;
        tmp13 = tmp12 * tmp12;
        tmp14 = tmp13 + i;
        tmp15 = tmp14 ^ tmp10;
        
        /* ========== MEMORY ACCESS WITH VARIABLE INDEXING ========== */
        /* Complex array access with loop-variant indexing */
        int idx1 = (i + state) % size;
        int idx2 = (i * 2 + prev_state) % size;
        int idx3 = (i * 3 + tmp15) % size;
        
        /* Conditional memory access based on loop-variant condition */
        if (state & 1) {  /* Branch inside loop */
            /* Memory dependency with address calculation */
            acc1 += data[idx1] * 3;
            acc2 += data[idx2] ^ tmp1;
        } else {
            acc3 += data[idx3] | tmp2;
            acc4 += data[(idx1 + idx2) % size] & tmp3;
        }
        
        /* ========== UPDATE MULTIPLE ACCUMULATORS ========== */
        /* Keep many variables live to increase register pressure */
        acc5 += tmp4 * tmp5;
        acc6 += tmp6 ^ tmp7;
        acc7 += tmp8 & tmp9;
        acc8 += tmp10 | tmp11;
        
        /* Additional operations to create more dependencies */
        if (i & 3) {  /* Another conditional */
            acc1 = (acc1 * 7) + tmp12;
            acc2 = (acc2 ^ tmp13) - 1;
        }
        
        /* Cross-iteration dependency through accumulators */
        acc3 = acc3 + acc4;  /* Creates dependency between acc3 and acc4 */
        acc4 = acc4 ^ acc5;  /* Creates dependency between acc4 and acc5 */
        
        /* Use previous state in calculation */
        acc6 = acc6 + prev_state;
        acc7 = acc7 ^ (prev_state & 0xFF);
    }
    
    /* Combine all results to prevent dead code elimination */
    result = state + acc1 + acc2 + acc3 + acc4 + acc5 + acc6 + acc7 + acc8;
    result = result ^ prev_state;  /* Use the final prev_state */
    
    return result;
}

/* Another complex loop with different pattern */
__attribute__((noinline))
int second_complex_loop(int *data, int size, int iterations) {
    volatile int limit = iterations;
    int sum = 0;
    int carry = 1;  /* Loop-carried dependency */
    
    for (int j = 0; j < limit; ++j) {
        /* Multiple independent calculations */
        int a = data[j % size] * 3;
        int b = data[(j + 1) % size] + 5;
        int c = data[(j + 2) % size] & 0xFF;
        int d = data[(j + 3) % size] | 0x80;
        
        /* Chain of operations */
        int e = a * b;
        int f = c + d;
        int g = e ^ f;
        int h = g << (j % 4);
        
        /* Loop-carried dependency */
        carry = (carry * 13 + h) % 1000;
        
        /* Conditional update */
        if (carry > 500) {
            sum += a + b;
        } else {
            sum += c - d;
        }
        
        /* More temporaries for register pressure */
        int t1 = sum * 2;
        int t2 = t1 ^ carry;
        int t3 = t2 & 0x7F;
        int t4 = t3 | 0x40;
        int t5 = t4 << 1;
        int t6 = t5 >> 2;
        int t7 = t6 * 3;
        int t8 = t7 + j;
        int t9 = t8 % 256;
        int t10 = t9 ^ 0x55;
        
        sum = sum + t10;
    }
    
    return sum + carry;
}

int main() {
    int i;
    int data[ARRAY_SIZE];
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Volatile to prevent constant folding of iterations */
    volatile int iterations = LOOP_ITERATIONS;
    
    printf("Starting complex loop operations...\n");
    
    /* Call first complex loop */
    int result1 = complex_loop_operation(data, ARRAY_SIZE, iterations);
    printf("Result 1: %d\n", result1);
    
    /* Call second complex loop */
    int result2 = second_complex_loop(data, ARRAY_SIZE, iterations / 2);
    printf("Result 2: %d\n", result2);
    
    /* Final combination to ensure all computations are used */
    int final_result = result1 ^ result2;
    printf("Final result: %d\n", final_result);
    
    return 0;
}
