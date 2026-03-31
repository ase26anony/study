/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling dependency analysis
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fno-unroll-loops -fno-tree-vectorize -fdump-rtl-sms modulo-sched-test.c -o modulo-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 1024

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline)) 
int complex_loop(int *data, int size, volatile int limit) {
    /* Loop-carried state variables */
    int state = 123456789;
    int prev_state = 0;
    
    /* Multiple accumulators to create register pressure */
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int acc5 = 0, acc6 = 0, acc7 = 0, acc8 = 0;
    int acc9 = 0, acc10 = 0, acc11 = 0, acc12 = 0;
    
    /* Volatile variables to prevent optimization */
    volatile int v_mod = 7;
    volatile int v_mask = 0xFF;
    volatile int v_scale = 3;
    
    /* True loop-carried dependency: state from iteration i used in i+1 */
    for (int i = 0; i < limit; ++i) {
        /* Loop-carried dependency chain */
        prev_state = state;
        state = (prev_state * 1103515245 + 12345) ^ data[i % size];
        
        /* Multiple independent arithmetic operations */
        int tmp1 = acc1 * i;
        int tmp2 = acc2 + i;
        int tmp3 = tmp1 ^ tmp2;
        int tmp4 = acc3 & v_mask;
        int tmp5 = acc4 | (i << 2);
        int tmp6 = tmp3 * tmp4;
        int tmp7 = tmp5 + tmp6;
        int tmp8 = acc5 - tmp7;
        int tmp9 = acc6 * v_scale;
        int tmp10 = tmp8 ^ tmp9;
        
        /* Memory access with variable indexing (creates address calculation) */
        int idx = (i + state) % size;
        int val = data[idx] * v_scale;
        
        /* Conditional execution based on loop-variant condition */
        if (state & 1) {  /* Creates control flow in scheduling graph */
            acc1 += val;
            acc2 ^= data[(idx + 1) % size];
        } else {
            acc3 |= val;
            acc4 &= data[(idx + v_mod) % size];
        }
        
        /* More arithmetic to increase instruction count */
        acc5 = acc5 * 3 + tmp10;
        acc6 = (acc6 << 1) | (state & 1);
        acc7 = acc7 + (tmp1 >> 2);
        acc8 = acc8 ^ (tmp2 * 2);
        acc9 = acc9 - (tmp3 & 0xFFFF);
        acc10 = acc10 + (tmp4 | 0xAA);
        acc11 = acc11 * (tmp5 % 16);
        acc12 = (acc12 + i) * (v_mod + 1);
        
        /* Another loop-carried dependency */
        int temp = acc1;
        acc1 = acc2;
        acc2 = temp + acc1;  /* acc2 depends on previous iteration's acc1 */
    }
    
    /* Combine all accumulators to prevent dead code elimination */
    int result = state + prev_state + acc1 + acc2 + acc3 + acc4 + acc5 + acc6 +
                 acc7 + acc8 + acc9 + acc10 + acc11 + acc12;
    
    return result;
}

/* Another complex loop with different pattern */
__attribute__((noinline))
int nested_loop(int *data, int size, volatile int outer_limit) {
    int sum = 0;
    volatile int inner_limit = 5;
    
    for (int i = 0; i < outer_limit; ++i) {
        int local_acc = 0;
        int prev = data[i % size];
        
        /* Inner loop with carried dependency */
        for (int j = 0; j < inner_limit; ++j) {
            /* Loop-carried: prev from iteration j used in j+1 */
            int curr = (prev * 1664525 + 1013904223) ^ data[(i + j) % size];
            local_acc += curr;
            prev = curr;
            
            /* Multiple operations to create scheduling nodes */
            int t1 = local_acc * j;
            int t2 = t1 ^ (j << 3);
            int t3 = t2 & 0x7F;
            int t4 = t3 | (j * 2);
            local_acc = (local_acc + t4) % 1000;
        }
        
        sum += local_acc;
        
        /* Conditional with loop-variant condition */
        if (i & 1) {
            sum ^= data[(i * 3) % size];
        } else {
            sum += data[(i * 7) % size];
        }
    }
    
    return sum;
}

int main() {
    /* Initialize with random data */
    int data[ARRAY_SIZE];
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Volatile limits to prevent constant propagation */
    volatile int limit1 = 500;
    volatile int limit2 = 200;
    
    /* Execute both complex loops */
    int result1 = complex_loop(data, ARRAY_SIZE, limit1);
    int result2 = nested_loop(data, ARRAY_SIZE, limit2);
    
    /* Combine and print results to prevent optimization */
    int final_result = result1 + result2;
    printf("Final result: %d\n", final_result);
    
    return 0;
}
