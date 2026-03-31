/* modulo-sched-test.c
 * Designed to trigger GCC's modulo scheduling dependency analysis
 * with loop-carried dependencies and complex instruction scheduling
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inline function to ensure loop is processed independently */
__attribute__((noinline)) 
int complex_loop(int* data, int size, volatile int limit) {
    /* Multiple accumulators to create register pressure */
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    int state = 123456789;
    int sum = 0;
    
    /* Many temporary variables for high register pressure */
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10;
    int tmp11, tmp12, tmp13, tmp14, tmp15;
    
    /* Volatile variables to prevent optimization */
    volatile int v1 = 7, v2 = 13, v3 = 42;
    
    /* Loop with true loop-carried dependency */
    for (int i = 0; i < limit; ++i) {
        /* Loop-carried dependency: state depends on previous iteration */
        state = (state * 1103515245 + 12345) ^ data[i % size];
        
        /* Multiple independent arithmetic operations */
        tmp1 = acc1 * v1;
        tmp2 = acc2 + v2;
        tmp3 = tmp1 ^ tmp2;
        tmp4 = acc3 & v3;
        tmp5 = tmp3 | tmp4;
        tmp6 = i * 17;
        tmp7 = tmp5 - tmp6;
        tmp8 = state >> 3;
        tmp9 = tmp7 * tmp8;
        tmp10 = acc4 ^ tmp9;
        tmp11 = tmp10 + i;
        tmp12 = tmp11 * 3;
        tmp13 = tmp12 & 0xFF;
        tmp14 = tmp13 | state;
        tmp15 = tmp14 ^ (i << 2);
        
        /* Conditional execution based on loop-variant condition */
        if (state & 1) {
            /* Memory access with variable indexing */
            int idx = (state + i) % size;
            sum += data[idx] * v1;
            
            /* More arithmetic in conditional path */
            tmp15 = (tmp15 * 31) ^ data[(i + v2) % size];
        } else if (i & 3) {
            /* Another conditional path */
            sum -= data[(state >> 4) % size] & v3;
            tmp15 = tmp15 + (v2 * 7);
        }
        
        /* Update accumulators with loop-carried dependencies */
        acc1 = acc2 + tmp15;          /* acc1 depends on acc2 from previous iteration */
        acc2 = acc3 ^ state;          /* acc2 depends on acc3 from previous iteration */
        acc3 = acc4 * (i + 1);        /* acc3 depends on acc4 from previous iteration */
        acc4 = acc1 & tmp15;          /* acc4 depends on acc1 from current iteration */
        
        /* Additional arithmetic to increase instruction count */
        tmp1 = (tmp1 + tmp2) * (tmp3 - tmp4);
        tmp2 = (tmp5 | tmp6) & (tmp7 ^ tmp8);
        tmp3 = (tmp9 + tmp10) - (tmp11 * tmp12);
        tmp4 = (tmp13 & tmp14) | (tmp15 ^ state);
        
        /* Use temporaries to keep them live */
        acc1 ^= tmp1;
        acc2 += tmp2;
        acc3 |= tmp3;
        acc4 &= tmp4;
    }
    
    /* Combine all results to prevent dead code elimination */
    return (acc1 + acc2) ^ (acc3 - acc4) + sum;
}

/* Another complex loop with different pattern */
__attribute__((noinline))
int nested_loop(int* data, int size, volatile int outer_limit) {
    int total = 0;
    volatile int inner_limit = 50;
    
    for (int i = 0; i < outer_limit; ++i) {
        int local_acc = data[i % size];
        
        /* Inner loop with carried dependency */
        for (int j = 0; j < inner_limit; ++j) {
            /* Complex addressing with multiple dependencies */
            int idx1 = (i * j) % size;
            int idx2 = (i + j * 7) % size;
            int idx3 = (j * 11 + i) % size;
            
            /* Chain of dependent operations */
            int val1 = data[idx1] * 3;
            int val2 = data[idx2] + val1;      /* Depends on val1 */
            int val3 = data[idx3] ^ val2;      /* Depends on val2 */
            int val4 = val1 & val3;            /* Depends on val1 and val3 */
            int val5 = val2 | val4;            /* Depends on val2 and val4 */
            
            /* Loop-carried dependency through local_acc */
            local_acc = (local_acc * 6364136223846793005ULL + 1) ^ val5;
            
            /* Conditional with loop-variant condition */
            if ((i + j) & 1) {
                local_acc += data[(local_acc + j) % size];
            } else {
                local_acc -= data[(val5 + i) % size] & 0xFF;
            }
            
            /* More independent operations */
            int t1 = i * j;
            int t2 = t1 ^ j;
            int t3 = t2 + i;
            int t4 = t3 * 19;
            int t5 = t4 & 0xFFFF;
            
            local_acc ^= t5;
        }
        
        total += local_acc;
    }
    
    return total;
}

int main() {
    const int SIZE = 1024;
    int* data = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < SIZE; ++i) {
        data[i] = rand() % 1000;
    }
    
    /* Volatile loop limits to prevent constant propagation */
    volatile int limit1 = 500;
    volatile int limit2 = 100;
    
    /* Call both complex loops */
    int result1 = complex_loop(data, SIZE, limit1);
    int result2 = nested_loop(data, SIZE, limit2);
    
    /* Combine and print results */
    int final_result = result1 ^ result2;
    printf("Final result: %d\n", final_result);
    
    /* Additional volatile operations to prevent optimization */
    volatile int check = final_result;
    if (check & 1) {
        printf("Odd result\n");
    }
    
    free(data);
    return 0;
}
