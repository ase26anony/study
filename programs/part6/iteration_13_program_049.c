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
    
    /* Loop-carried state variables */
    int state = 0x12345678;
    int sum = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    
    /* Many temporary variables to create register pressure */
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
    int tmp11, tmp12, tmp13, tmp14, tmp15;
    
    /* Volatile modifiers to prevent optimization */
    volatile int mod1 = 3, mod2 = 7, mod3 = 11;
    
    for (i = 0; i < N; ++i) {
        /* 1. Loop-carried dependency: state depends on previous iteration */
        state = (state * 1103515245 + 12345) ^ data[i % size];
        
        /* 2. Multiple independent arithmetic operations */
        tmp1 = i * mod1;
        tmp2 = i + mod2;
        tmp3 = tmp1 ^ tmp2;
        tmp4 = tmp1 & tmp2;
        tmp5 = tmp1 | tmp2;
        tmp6 = tmp3 * tmp4;
        tmp7 = tmp5 + tmp6;
        tmp8 = tmp7 >> 2;
        tmp9 = tmp8 << 1;
        tmp10 = tmp9 % 17;
        
        /* More operations to increase instruction count */
        tmp11 = (tmp10 * 3) / 2;
        tmp12 = tmp11 + i;
        tmp13 = tmp12 ^ state;
        tmp14 = tmp13 & 0xFF;
        tmp15 = tmp14 * mod3;
        
        /* 3. Conditional memory access with loop-variant index */
        if (state & 1) {  /* Nested control flow */
            /* Complex array indexing */
            int idx = (state + i) % size;
            sum += data[idx] * tmp15;
        } else {
            int idx2 = (state ^ i) % size;
            sum -= data[idx2] & tmp15;
        }
        
        /* 4. Multiple accumulators with loop-carried dependencies */
        acc1 = acc1 + tmp1;      /* Simple accumulation */
        acc2 = acc2 ^ tmp3;      /* XOR accumulation */
        acc3 = (acc3 * 13) + tmp7;  /* Multiplicative accumulation */
        acc4 = (acc4 << 1) | (tmp15 & 1);  /* Shift accumulation */
        
        /* 5. Additional arithmetic to create more scheduling nodes */
        tmp1 = tmp1 + (acc1 & 0xF);
        tmp3 = tmp3 * (acc2 | 0x1);
        tmp7 = tmp7 - (acc3 % 19);
        tmp15 = tmp15 ^ (acc4 << 2);
        
        /* 6. Another conditional with different condition */
        if ((i & 3) == 0) {
            tmp1 = tmp1 * 2;
            tmp3 = tmp3 + data[(i * 7) % size];
        } else if ((i & 3) == 1) {
            tmp7 = tmp7 / 2;
            tmp15 = tmp15 | data[(i * 11) % size];
        }
        
        /* Update accumulators again to keep dependencies */
        acc1 += tmp1;
        acc2 ^= tmp3;
        acc3 += tmp7;
        acc4 ^= tmp15;
    }
    
    /* Combine all results to prevent dead code elimination */
    return sum + acc1 + acc2 + acc3 + acc4 + state;
}

int main() {
    int data[ARRAY_SIZE];
    int i, result;
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Process the loop */
    result = process_loop(data, ARRAY_SIZE);
    
    /* Print result to ensure computation isn't optimized away */
    printf("Result: %d\n", result);
    
    /* Additional loop with different characteristics */
    {
        volatile int limit = 300;
        int x = 0, y = 1, z = 2;
        int a = 0, b = 0, c = 0, d = 0;
        
        for (i = 0; i < limit; ++i) {
            /* Fibonacci-like recurrence with multiple dependencies */
            int next = x + y + z;
            x = y;
            y = z;
            z = next;
            
            /* Multiple independent chains */
            a = (a * 3 + i) % 100;
            b = (b ^ (i * 5)) & 0xFF;
            c = c + data[(a + b) % ARRAY_SIZE];
            d = d - data[(x * y) % ARRAY_SIZE];
            
            /* Complex condition */
            if ((a + b + c + d) & 1) {
                a = a ^ b;
                c = c + d;
            } else {
                b = b | c;
                d = d - a;
            }
        }
        
        printf("Secondary result: %d\n", x + y + z + a + b + c + d);
    }
    
    return 0;
}
