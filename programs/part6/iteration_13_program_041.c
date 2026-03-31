/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling dependency analysis
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fdump-rtl-sms -fno-unroll-loops -fno-tree-vectorize modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 1024

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline)) 
int complex_loop(int *data, int size) {
    volatile int N = 500;  /* Prevent constant propagation */
    volatile int seed = 42;
    
    /* Multiple accumulators to create register pressure */
    int state = seed;
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    int acc1 = 1, acc2 = 2, acc3 = 3, acc4 = 4;
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
    int var1 = 7, var2 = 11, var3 = 13, var4 = 17;
    
    /* Loop with true loop-carried dependency */
    for (int i = 0; i < N; ++i) {
        /* Loop-carried dependency: state from iteration i used in i+1 */
        state = (state * 1103515245 + 12345) ^ data[i % ARRAY_SIZE];
        
        /* Multiple independent arithmetic operations */
        tmp1 = var1 * i;           /* Multiplication */
        tmp2 = var2 + i;           /* Addition */
        tmp3 = tmp1 ^ tmp2;        /* XOR */
        tmp4 = var3 & i;           /* AND */
        tmp5 = var4 | i;           /* OR */
        tmp6 = tmp1 - tmp2;        /* Subtraction */
        tmp7 = tmp3 * tmp4;        /* Another multiplication */
        tmp8 = tmp5 + tmp6;        /* Another addition */
        
        /* Memory access with variable indexing */
        int idx = (i + state) % ARRAY_SIZE;
        sum1 += data[idx] * acc1;
        
        /* Conditional execution based on loop-variant condition */
        if (state & 1) {           /* Branch inside loop */
            sum2 += data[(idx * 3) % ARRAY_SIZE] ^ acc2;
        } else {
            sum3 += data[(idx * 5) % ARRAY_SIZE] | acc3;
        }
        
        /* More arithmetic to increase instruction count */
        acc1 = (acc1 * 3) % 1000;
        acc2 = (acc2 + 7) % 1000;
        acc3 = (acc3 ^ state) % 1000;
        acc4 = (acc4 * 5 + i) % 1000;
        
        /* Additional temporary computations */
        var1 = (var1 + tmp7) & 0xFF;
        var2 = (var2 ^ tmp8) & 0xFF;
        var3 = (var3 * 11 + tmp6) % 256;
        var4 = (var4 | tmp5) & 0xFF;
        
        /* Another loop-carried dependency chain */
        sum4 = sum4 + (state % 100);
    }
    
    /* Combine all results to prevent optimization */
    return state + sum1 + sum2 + sum3 + sum4 + acc1 + acc2 + acc3 + acc4;
}

/* Another complex loop with different pattern */
__attribute__((noinline))
int nested_loop(int *data, int size) {
    volatile int outer_limit = 100;
    volatile int inner_limit = 10;
    int result = 0;
    
    for (int i = 0; i < outer_limit; ++i) {
        int inner_state = i;
        int tmp_a = 1, tmp_b = 2, tmp_c = 3, tmp_d = 4;
        int tmp_e = 5, tmp_f = 6, tmp_g = 7, tmp_h = 8;
        
        for (int j = 0; j < inner_limit; ++j) {
            /* Loop-carried dependency in inner loop */
            inner_state = (inner_state * 1664525 + 1013904223) ^ data[(i + j) % size];
            
            /* Multiple parallel operations */
            tmp_a = tmp_a * inner_state + j;
            tmp_b = tmp_b ^ data[(inner_state + j) % size];
            tmp_c = tmp_c & (inner_state | j);
            tmp_d = tmp_d + (tmp_a * tmp_b);
            tmp_e = tmp_e - (tmp_c ^ tmp_d);
            tmp_f = tmp_f | (tmp_e & 0xFF);
            tmp_g = tmp_g * 3 + tmp_f;
            tmp_h = tmp_h ^ tmp_g;
            
            /* Conditional with complex expression */
            if ((inner_state + i + j) & 3) {
                result += tmp_h % 100;
            } else {
                result -= tmp_g % 50;
            }
        }
        
        result += inner_state;
    }
    
    return result;
}

int main() {
    /* Initialize data array with pseudo-random values */
    int data[ARRAY_SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Call complex loops multiple times */
    int total = 0;
    volatile int iterations = 3;
    
    for (int k = 0; k < iterations; ++k) {
        total += complex_loop(data, ARRAY_SIZE);
        total += nested_loop(data, ARRAY_SIZE);
        
        /* Modify data slightly each iteration */
        for (int i = 0; i < ARRAY_SIZE; i += 37) {
            data[i] = (data[i] * 13 + 7) % 1000;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
