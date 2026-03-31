/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling dependency analysis
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fno-unroll-loops -fno-tree-vectorize -fdump-rtl-sms modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline)) 
int compute_loop(int *data, int size) {
    volatile int N = 500;  /* Prevent constant propagation */
    volatile int seed = 42; /* Volatile to prevent optimization */
    
    /* Many local variables to create register pressure */
    int state = seed;
    int sum = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
    int a = 1103515245, b = 12345, c = 67890, d = 54321;
    int e = 255, f = 65535, g = 16777215;
    
    /* Loop with true loop-carried dependency */
    for (int i = 0; i < N; ++i) {
        /* Loop-carried dependency: state from previous iteration */
        state = (state * a + b) ^ data[i % size];
        
        /* Multiple independent arithmetic operations */
        tmp1 = state * i;          /* Multiplication */
        tmp2 = state + i;          /* Addition */
        tmp3 = tmp1 ^ tmp2;        /* XOR */
        tmp4 = tmp1 & e;           /* AND with mask */
        tmp5 = tmp2 | f;           /* OR with mask */
        tmp6 = tmp3 * c;           /* Another multiplication */
        tmp7 = tmp4 + d;           /* Another addition */
        tmp8 = tmp5 ^ g;           /* Another XOR */
        tmp9 = tmp6 & tmp7;        /* Combine operations */
        tmp10 = tmp8 | tmp9;       /* More combination */
        
        /* Memory access with variable indexing */
        int idx = (state + i) % size;
        if (idx < 0) idx = -idx;
        
        /* Conditional execution based on loop-variant condition */
        if (state & 1) {
            /* True loop-carried dependency for sum */
            sum += data[idx] * (i & 3);
        } else {
            /* Alternative path with different operations */
            sum -= data[(idx * 3) % size] >> 1;
        }
        
        /* Update multiple accumulators to keep them live */
        acc1 += tmp1;
        acc2 += tmp2;
        acc3 += tmp3;
        acc4 += tmp4;
        
        /* More operations to increase instruction count */
        tmp1 = tmp2 * tmp3;
        tmp2 = tmp4 + tmp5;
        tmp3 = tmp6 ^ tmp7;
        tmp4 = tmp8 & tmp9;
        
        /* Another conditional with different condition */
        if ((i & 7) == 0) {
            acc1 ^= tmp10;
            acc2 |= state;
        }
        
        /* Array access with complex indexing */
        int offset = (acc1 + i) % size;
        if (offset < 0) offset = -offset;
        tmp5 = data[offset] * data[(offset + 1) % size];
        
        /* More arithmetic chains */
        tmp6 = tmp5 * 314159;
        tmp7 = tmp6 / 271828;
        tmp8 = tmp7 << 3;
        tmp9 = tmp8 >> 2;
        tmp10 = tmp9 & 0xFF;
        
        /* Update accumulators again */
        acc3 += tmp6;
        acc4 += tmp7;
    }
    
    /* Combine all results to prevent dead code elimination */
    return sum + acc1 + acc2 + acc3 + acc4 + state;
}

int main() {
    const int SIZE = 1024;
    int *data = (int*)malloc(SIZE * sizeof(int));
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Call the computation loop */
    int result = compute_loop(data, SIZE);
    
    /* Use the result to prevent optimization */
    printf("Result: %d\n", result);
    
    free(data);
    return 0;
}
