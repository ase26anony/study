/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling pass and exercise
 * the dependency edge calculation logic in modulo-sched.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline))
int process_loop(int* data, int size, int init) {
    volatile int N = 500;           /* Prevent constant propagation */
    volatile int mod1 = 7;          /* Volatile modifiers */
    volatile int mod2 = 13;
    
    /* Loop-carried state with true dependency distance=1 */
    int state = init;
    
    /* Multiple accumulators to create register pressure */
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0, acc5 = 0;
    int acc6 = 0, acc7 = 0, acc8 = 0, acc9 = 0, acc10 = 0;
    
    /* Many temporary variables for high register pressure */
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
    int tmp11, tmp12, tmp13, tmp14, tmp15;
    
    for (int i = 0; i < N; ++i) {
        /* 1. Loop-carried dependency: state from iteration i used in i+1 */
        state = (state * 1103515245 + 12345) ^ data[i % size];
        
        /* 2. Multiple independent arithmetic operations */
        tmp1 = acc1 * i;            /* Multiplication */
        tmp2 = acc2 + (i & 0xFF);   /* Addition with mask */
        tmp3 = tmp1 ^ tmp2;         /* XOR operation */
        tmp4 = acc3 | (i << 3);     /* OR with shift */
        tmp5 = acc4 & (i * mod1);   /* AND with multiplication */
        tmp6 = tmp3 - tmp4;         /* Subtraction */
        tmp7 = tmp5 * tmp6;         /* Another multiplication */
        tmp8 = acc5 ^ tmp7;         /* More XOR */
        tmp9 = acc6 + (tmp8 >> 2);  /* Addition with shift */
        tmp10 = acc7 | tmp9;        /* OR operation */
        tmp11 = acc8 & tmp10;       /* AND operation */
        tmp12 = tmp11 * mod2;       /* Multiplication with volatile */
        tmp13 = acc9 - tmp12;       /* Subtraction */
        tmp14 = acc10 ^ tmp13;      /* XOR */
        tmp15 = tmp14 + (i % 16);   /* Addition with modulo */
        
        /* 3. Memory access with variable indexing (loop-carried dependency) */
        int idx = (state + i) % size;
        int val = data[idx] * (i & 0x3);
        
        /* 4. Conditional execution based on loop-variant condition */
        if (state & 1) {            /* Branch inside loop */
            /* Complex address calculation */
            int addr = (idx * 3 + mod1) % size;
            acc1 += data[addr] * val;
        } else {
            acc2 ^= val + tmp15;
        }
        
        /* 5. Update multiple accumulators to keep them live */
        acc3 = tmp1 + acc3;
        acc4 = tmp2 - acc4;
        acc5 = tmp3 * acc5;
        acc6 = tmp4 ^ acc6;
        acc7 = tmp5 | acc7;
        acc8 = tmp6 + acc8;
        acc9 = tmp7 - acc9;
        acc10 = tmp8 * acc10;
        
        /* More arithmetic to increase instruction count */
        acc1 = (acc1 * 3) ^ tmp9;
        acc2 = (acc2 + 5) | tmp10;
        acc3 = (acc3 - 7) & tmp11;
        acc4 = (acc4 * 11) ^ tmp12;
        acc5 = (acc5 + 13) | tmp13;
        
        /* Additional loop-carried dependency chain */
        acc6 = acc6 * 17 + state;
        acc7 = acc7 ^ (state >> 4);
    }
    
    /* Combine all results to prevent dead code elimination */
    return state + acc1 + acc2 + acc3 + acc4 + acc5 + 
           acc6 + acc7 + acc8 + acc9 + acc10;
}

/* Another loop with different characteristics */
__attribute__((noinline))
int process_loop2(int* data, int size, int init) {
    volatile int M = 300;
    int sum = init;
    int prod = 1;
    int xor_acc = 0;
    
    for (int i = 0; i < M; ++i) {
        /* Different loop-carried dependency pattern */
        sum = sum + data[(sum + i) % size];
        
        /* Independent operations */
        int t1 = data[i % size] * 3;
        int t2 = data[(i + 1) % size] + 5;
        int t3 = t1 & t2;
        int t4 = t1 | t2;
        int t5 = t3 ^ t4;
        
        /* Conditional with both paths used */
        if (t5 > 0) {
            prod *= (t5 % 256) + 1;
        } else {
            xor_acc ^= t5;
        }
        
        /* More operations to increase complexity */
        sum = (sum ^ t5) + prod;
        prod = (prod * 2) ^ xor_acc;
        xor_acc = xor_acc + (i & 0xF);
    }
    
    return sum + prod + xor_acc;
}

int main() {
    const int SIZE = 1024;
    int* data = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Process multiple times to increase execution time */
    int result1 = 0;
    int result2 = 0;
    
    for (int iter = 0; iter < 10; ++iter) {
        result1 = process_loop(data, SIZE, result1 + iter);
        result2 = process_loop2(data, SIZE, result2 + iter);
    }
    
    /* Combine and print results to prevent optimization */
    printf("Final result: %d\n", result1 + result2);
    
    /* Use results to prevent dead code elimination */
    if (result1 > result2) {
        printf("Loop1 dominated\n");
    } else {
        printf("Loop2 dominated or equal\n");
    }
    
    free(data);
    return 0;
}
