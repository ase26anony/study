/* sel-sched-trigger.c
 * Program designed to trigger GCC selective scheduler debug dumping
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 sel-sched-trigger.c -o sel-sched-trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int volatile_bound = 1000;
volatile int volatile_seed = 42;

/* Function with complex control flow */
static inline int complex_conditional(int a, int b, int c) {
    /* Multiple conditional paths */
    if (a > b) {
        return (a * b) / (c + 1);
    } else if (a < b) {
        return (b - a) * c;
    } else {
        return (a + b) ^ c;
    }
}

/* Pointer chasing pattern */
static int chase_pointer(int *data, int start, int steps) {
    int idx = start;
    int sum = 0;
    for (int i = 0; i < steps; i++) {
        sum += data[idx];
        idx = data[idx] % steps;  /* Data-dependent next index */
        asm volatile("" : : : "memory");  /* Inline assembly barrier */
    }
    return sum;
}

int main(void) {
    const int N = 1024;
    const int M = 512;
    
    /* Initialize arrays with pseudo-random data */
    int *data_a = (int*)malloc(N * sizeof(int));
    int *data_b = (int*)malloc(N * sizeof(int));
    float *data_f = (float*)malloc(N * sizeof(float));
    
    /* Simple PRNG for initialization */
    unsigned int seed = time(NULL) ^ volatile_seed;
    for (int i = 0; i < N; i++) {
        seed = seed * 1103515245 + 12345;
        data_a[i] = (seed >> 16) & 0x7FFF;
        data_b[i] = (seed >> 8) & 0xFF;
        data_f[i] = (float)(seed & 0xFF) / 256.0f;
    }
    
    /* Volatile loop bound to prevent optimization */
    int bound = volatile_bound;
    if (bound > N) bound = N;
    
    /* Complex nested loops with data dependencies */
    long long total_sum = 0;
    float fp_acc = 0.0f;
    
    /* Outer loop with volatile counter */
    for (volatile int outer = 0; outer < 10; outer++) {
        int inner_bound = bound - (outer * 10);
        if (inner_bound < 100) inner_bound = 100;
        
        /* First computation kernel: data-dependent arithmetic */
        #pragma GCC unroll 4
        for (int i = 1; i < inner_bound; i++) {
            /* Cross-iteration dependency */
            int temp = data_a[i] * data_a[i-1];
            
            /* Mixed-width operations */
            long long wide_op = (long long)temp * data_b[i];
            
            /* Conditional move via ternary */
            int cond_val = (data_a[i] > data_b[i]) ? 
                          (temp / (data_b[i] + 1)) : 
                          (temp % (data_b[i] + 1));
            
            /* Floating-point operation */
            fp_acc += data_f[i] * 1.2345f;
            
            /* Complex addressing mode */
            total_sum += wide_op + cond_val + 
                        data_a[(i * 17) % N] - 
                        data_b[(i * 13) % N];
            
            /* Periodic inline assembly */
            if (i % 32 == 0) {
                asm volatile("nop" : : : "memory");
            }
        }
        
        /* Second computation kernel: matrix-vector like */
        for (int j = 0; j < M; j++) {
            float row_sum = 0.0f;
            for (int k = 0; k < 8; k++) {
                int idx = (j * 8 + k) % N;
                row_sum += data_f[idx] * (k + 1);
                
                /* Switch statement for control flow complexity */
                switch (data_a[idx] % 4) {
                    case 0:
                        row_sum *= 0.9f;
                        break;
                    case 1:
                        row_sum += 1.1f;
                        break;
                    case 2:
                        row_sum -= 0.1f;
                        break;
                    case 3:
                        row_sum /= 1.01f;
                        break;
                }
            }
            fp_acc += row_sum;
            
            /* Division with non-constant divisor */
            if (row_sum != 0.0f) {
                total_sum += (long long)(1000.0f / row_sum);
            }
        }
        
        /* Pointer chasing pattern */
        int chase_result = chase_pointer(data_a, outer % N, 50);
        total_sum += chase_result;
        
        /* External function call to prevent optimization */
        int rand_val = rand() % 100;
        if (rand_val < 10) {
            bound = (bound + 10) % N;
        }
    }
    
    /* Final reduction with XOR */
    uint64_t final_hash = 0;
    for (int i = 0; i < N; i += 8) {
        uint64_t chunk = 0;
        for (int j = 0; j < 8 && (i + j) < N; j++) {
            chunk ^= ((uint64_t)data_a[i + j] << (j * 8));
        }
        final_hash ^= chunk;
    }
    
    /* Ensure side effects are observable */
    printf("Result: total_sum = %lld, fp_acc = %f, hash = 0x%016llx\n",
           total_sum, fp_acc, (unsigned long long)final_hash);
    
    /* Cleanup */
    free(data_a);
    free(data_b);
    free(data_f);
    
    return 0;
}
