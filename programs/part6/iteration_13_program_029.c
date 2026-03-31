/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling dependency analysis
 * with loop-carried dependencies and complex scheduling requirements
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline)) 
int process_loop(int* data, int size, volatile int limit) {
    /* Many local variables to create register pressure */
    int state = 0x12345678;  /* Loop-carried dependency variable */
    int sum = 0;
    int acc1 = 1, acc2 = 2, acc3 = 3, acc4 = 4, acc5 = 5;
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
    int a = 7, b = 11, c = 13, d = 17, e = 19;
    volatile int modifier = 3;  /* Prevent constant propagation */
    
    /* Main loop with true loop-carried dependency */
    for (int i = 0; i < limit; ++i) {
        /* Loop-carried dependency: state from iteration i used in i+1 */
        state = (state * 1103515245 + 12345) ^ data[i % size];
        
        /* Multiple independent arithmetic operations */
        tmp1 = a * i + modifier;
        tmp2 = b + i * c;
        tmp3 = tmp1 ^ tmp2;
        tmp4 = d * state + e;
        tmp5 = tmp3 & tmp4;
        tmp6 = tmp1 | tmp2;
        tmp7 = tmp4 - tmp3;
        tmp8 = tmp5 * tmp6;
        tmp9 = tmp7 / (modifier + 1);
        tmp10 = tmp8 % (modifier + 2);
        
        /* Conditional execution based on loop-variant condition */
        if (state & 1) {
            /* Memory access with variable indexing */
            sum += data[(state + i) % size] * modifier;
            acc1 += tmp1;
        } else {
            acc2 += tmp2;
        }
        
        /* More arithmetic to increase instruction count */
        if (i & 3) {  /* Another conditional */
            acc3 = acc3 * tmp3 + 1;
            acc4 = acc4 ^ tmp4;
        }
        
        /* Update multiple accumulators to keep them live */
        acc5 = acc5 + tmp5 - tmp6;
        acc1 = acc1 * 3 + tmp7;
        acc2 = acc2 ^ tmp8;
        acc3 = acc3 + tmp9 * 2;
        acc4 = acc4 | tmp10;
        
        /* Array access with complex indexing */
        int idx = (i * 7 + state) % size;
        sum += data[idx] >> modifier;
        
        /* More operations to create scheduling complexity */
        tmp1 = tmp1 + (tmp2 << 2);
        tmp3 = tmp3 * (tmp4 >> 1);
        tmp5 = (tmp5 & 0xFF) + (tmp6 & 0xFF00);
    }
    
    /* Combine all accumulators to prevent dead code elimination */
    return sum + state + acc1 + acc2 + acc3 + acc4 + acc5;
}

/* Another complex loop with different characteristics */
__attribute__((noinline))
int nested_loop(int* data, int size, volatile int outer_limit) {
    int total = 0;
    volatile int inner_mod = 2;
    
    for (int j = 0; j < outer_limit; ++j) {
        int local_state = j;
        int tmp[8];  /* Local array for additional complexity */
        
        for (int k = 0; k < 8; ++k) {
            /* Loop-carried within inner loop */
            local_state = local_state * 1664525 + 1013904223;
            tmp[k] = data[(local_state + k) % size] + inner_mod;
            
            /* Complex expression with multiple dependencies */
            if (k > 0) {
                tmp[k] = tmp[k] * tmp[k-1] - local_state;
            }
            
            /* Multiple arithmetic operations */
            int x = tmp[k] & 0xFF;
            int y = (tmp[k] >> 8) & 0xFF;
            int z = x * y + k;
            tmp[k] = tmp[k] ^ (z << 16);
        }
        
        /* Reduce tmp array */
        for (int k = 0; k < 8; ++k) {
            total += tmp[k] * (j + 1);
        }
    }
    
    return total;
}

int main() {
    const int SIZE = 1024;
    int* data = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Volatile loop limits to prevent optimization */
    volatile int limit1 = 500;
    volatile int limit2 = 100;
    
    /* Process loops */
    int result1 = process_loop(data, SIZE, limit1);
    int result2 = nested_loop(data, SIZE, limit2);
    
    /* Combine and print results */
    int final_result = result1 + result2;
    printf("Result: %d\n", final_result);
    
    free(data);
    return 0;
}
