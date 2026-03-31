/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling dependency analysis
 * with loop-carried dependencies and complex scheduling patterns
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 1024

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline)) 
int compute_loop(int *data, int size, volatile int limit) {
    /* Multiple accumulators to create register pressure */
    int state = 123456789;
    int sum = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int acc5 = 0, acc6 = 0, acc7 = 0, acc8 = 0;
    
    /* Volatile modifiers to prevent constant propagation */
    volatile int mod1 = 1103515245;
    volatile int mod2 = 12345;
    volatile int mask = 0x7FFFFFFF;
    
    /* Main loop with complex dependencies */
    for (int i = 0; i < limit; ++i) {
        /* 1. Loop-carried dependency: state depends on previous iteration */
        state = (state * mod1 + mod2) ^ data[i % size];
        
        /* 2. Multiple independent arithmetic operations */
        int tmp1 = acc1 * i;           /* Multiplication */
        int tmp2 = acc2 + (i & 0xFF);  /* Addition with mask */
        int tmp3 = tmp1 ^ tmp2;        /* XOR operation */
        int tmp4 = acc3 | tmp3;        /* OR operation */
        int tmp5 = acc4 & tmp4;        /* AND operation */
        int tmp6 = tmp5 - acc5;        /* Subtraction */
        int tmp7 = tmp6 << 2;          /* Shift left */
        int tmp8 = tmp7 >> 1;          /* Shift right */
        
        /* 3. Memory access with variable indexing */
        int idx = (state + i) % size;
        int val = data[idx] * 3;
        
        /* 4. Conditional execution based on loop-variant condition */
        if (state & 1) {
            sum += data[(val + i) % size];
            acc1 = (acc1 + val) & mask;
        } else {
            sum -= data[(val - i + size) % size];
            acc2 = (acc2 ^ val) | 1;
        }
        
        /* 5. More arithmetic to increase instruction count */
        acc3 = acc3 * 17 + i;
        acc4 = acc4 ^ (tmp8 * 13);
        acc5 = (acc5 + tmp1) % 10007;
        acc6 = acc6 - tmp2 + tmp3;
        acc7 = (acc7 << 3) | (i & 7);
        acc8 = (acc8 * 3) ^ (state & 0xFF);
        
        /* 6. Additional loop-carried dependency chain */
        acc1 = acc1 + acc2;
        acc2 = acc2 ^ acc3;
        acc3 = acc3 * acc4;
        acc4 = acc4 - acc5;
    }
    
    /* Combine all accumulators to prevent dead code elimination */
    return sum + state + acc1 + acc2 + acc3 + acc4 + 
           acc5 + acc6 + acc7 + acc8;
}

/* Another complex loop with different pattern */
__attribute__((noinline))
int compute_loop2(int *data, int size, volatile int iterations) {
    int x = 1, y = 2, z = 3;
    int total = 0;
    
    volatile int a = 5, b = 7, c = 11;
    
    for (int i = 0; i < iterations; ++i) {
        /* Multiple interdependent calculations */
        int t1 = x * a + i;
        int t2 = y * b - i;
        int t3 = z * c ^ i;
        
        /* Loop-carried dependencies */
        x = (t1 + y) & 0xFFF;
        y = (t2 ^ z) | 0x3FF;
        z = (t3 * x) % 1009;
        
        /* Conditional memory access */
        if ((i & 3) == 0) {
            total += data[(x + i) % size];
        } else if ((i & 3) == 1) {
            total -= data[(y + i) % size];
        } else {
            total ^= data[(z + i) % size];
        }
        
        /* More operations for scheduling complexity */
        int u1 = (x << 2) + (y >> 1);
        int u2 = (y * 3) ^ (z & 0xF);
        int u3 = (z + 7) | (x % 17);
        
        x = x ^ u1;
        y = y + u2;
        z = z * u3;
    }
    
    return total + x + y + z;
}

int main() {
    /* Initialize with pseudo-random data */
    int data[ARRAY_SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Volatile loop bounds to prevent optimization */
    volatile int limit1 = 500;
    volatile int limit2 = 300;
    
    /* Call both complex loops */
    int result1 = compute_loop(data, ARRAY_SIZE, limit1);
    int result2 = compute_loop2(data, ARRAY_SIZE, limit2);
    
    /* Combine and print results */
    int final_result = result1 + result2;
    printf("Result: %d\n", final_result);
    
    return 0;
}
