/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling dependency analysis
 * with loop-carried dependencies and complex instruction scheduling
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 1024

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline)) 
int complex_loop(int* data, int size, volatile int limit) {
    /* Multiple accumulators to create register pressure */
    int state = 123456789;
    int sum = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
    int a = 1103515245, b = 12345, c = 67890, d = 24680;
    
    /* Loop with true loop-carried dependency on 'state' */
    for (int i = 0; i < limit; ++i) {
        /* Loop-carried dependency: state from iteration i used in i+1 */
        state = (state * a + b) ^ data[i % size];
        
        /* Multiple independent arithmetic operations */
        tmp1 = state * i;          /* Multiplication */
        tmp2 = state + i;          /* Addition */
        tmp3 = tmp1 ^ tmp2;        /* XOR */
        tmp4 = tmp1 & tmp2;        /* AND */
        tmp5 = tmp1 | tmp2;        /* OR */
        tmp6 = tmp3 * c;
        tmp7 = tmp4 + d;
        tmp8 = tmp5 ^ c;
        tmp9 = tmp6 & tmp7;
        tmp10 = tmp8 | tmp9;
        
        /* Conditional execution based on loop-variant condition */
        if (state & 1) {
            /* Memory access with variable indexing */
            sum += data[(state ^ i) % size] * (i & 0xFF);
        } else if (i & 2) {
            /* Alternative path with different operations */
            sum -= data[(state + i) % size] / ((i & 0x7F) + 1);
        }
        
        /* Update multiple accumulators to keep them live */
        acc1 += tmp1;
        acc2 += tmp2;
        acc3 += tmp3;
        acc4 += tmp4;
        
        /* More operations to increase instruction count */
        acc1 = (acc1 * 3) / 2;
        acc2 = acc2 ^ (acc1 << 3);
        acc3 = acc3 | (acc2 >> 2);
        acc4 = acc4 & (acc3 + 1);
        
        /* Additional memory access with complex addressing */
        if ((i % 8) == 0) {
            int idx = (state + acc1) % size;
            sum += data[idx] * 2;
        }
    }
    
    /* Combine all results to prevent optimization */
    return sum + state + acc1 + acc2 + acc3 + acc4;
}

int main() {
    /* Initialize with random data */
    int data[ARRAY_SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Volatile loop limit to prevent constant propagation */
    volatile int N = 500;
    
    /* Call the complex loop function */
    int result = complex_loop(data, ARRAY_SIZE, N);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional test with different parameters */
    volatile int N2 = 300;
    int result2 = complex_loop(data, ARRAY_SIZE, N2);
    printf("Result2: %d\n", result2);
    
    return 0;
}
