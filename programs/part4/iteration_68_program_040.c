/* sel-sched-trigger.c - Program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int volatile_bound = 1000;
volatile int volatile_seed = 42;

/* External function to create dependencies */
extern int rand(void);

/* Complex computation with data dependencies */
static inline int64_t complex_op(int64_t a, int64_t b, int64_t c) {
    /* Mixed-width operations create register pressure */
    int32_t a32 = (int32_t)a;
    int64_t b64 = b;
    int32_t c32 = (int32_t)c;
    
    /* Conditional move pattern */
    int64_t result = (a > b) ? (a32 * b64) : (b64 / (c32 | 1));
    
    /* Floating point to integer conversion */
    double d = (double)result;
    d = d * 1.234567 - 0.987654;
    
    /* Inline assembly to create fixed RTL */
    asm volatile ("" : "+r" (result) : : "memory");
    
    return (int64_t)d + result;
}

/* Matrix-vector like computation */
void compute_kernel(int64_t* restrict data, int64_t* restrict result, 
                    int size, int stride) {
    int i, j;
    volatile int vol_i = 0;
    
    /* Outer loop with volatile bound */
    for (i = 0; i < size; i += stride) {
        int64_t acc = 0;
        int64_t prev = data[i];
        
        /* Inner loop with carried dependency */
        #pragma GCC unroll 4
        for (j = i + 1; j < i + stride && j < size; j++) {
            /* Data-dependent computation */
            int64_t curr = data[j];
            
            /* Complex addressing mode */
            int64_t* ptr = &data[(j * 13) % size];
            
            /* Branch with substantial computation in both paths */
            if (curr > prev) {
                /* Branch 1: FP and integer mix */
                double fp_val = (double)curr * 0.12345;
                acc += (int64_t)(fp_val * prev) + complex_op(curr, prev, acc);
                
                /* Memory access with non-trivial addressing */
                *ptr = (*ptr + acc) ^ (prev << 3);
            } else {
                /* Branch 2: Different operation mix */
                int64_t diff = prev - curr;
                acc += (diff * diff) / ((curr | 1) + 1);
                acc ^= complex_op(prev, curr, diff);
                
                /* Another memory access */
                data[(j * 17) % size] = acc;
            }
            
            /* Update with dependency */
            prev = curr + (acc & 0xFF);
            
            /* Volatile update prevents loop unrolling */
            vol_i++;
        }
        
        /* Reduction with conditional */
        result[i / stride] = (acc > 0) ? acc : -acc;
        
        /* External call prevents optimization */
        if (rand() % 1000 == 0) {
            result[i / stride] ^= rand();
        }
    }
}

/* Second computation kernel with switch statement */
void compute_switch_kernel(int64_t* restrict data, int64_t* restrict out, 
                          int size) {
    int i;
    
    for (i = 0; i < size; i++) {
        int op = data[i] % 7;
        int64_t val = data[i];
        
        /* Switch creates multiple basic blocks */
        switch (op) {
            case 0:
                /* FP division with non-constant divisor */
                out[i] = (int64_t)((double)val / (double)((i % 5) + 1));
                break;
            case 1:
                /* Integer multiplication with overflow */
                out[i] = val * 0x123456789ABCDEFLL;
                break;
            case 2:
                /* Bit manipulation */
                out[i] = (val << (i % 16)) | (val >> (64 - (i % 16)));
                break;
            case 3:
                /* Complex function call */
                out[i] = complex_op(val, data[(i + 1) % size], 
                                   data[(i + 2) % size]);
                break;
            case 4:
                /* Memory intensive */
                out[i] = data[(i * 3) % size] + data[(i * 5) % size];
                break;
            case 5:
                /* Division heavy */
                out[i] = val / ((data[(i + 3) % size] & 0xFF) + 1);
                break;
            default:
                /* Mixed operations */
                out[i] = (val * 3) / 2 + (val % 100);
                break;
        }
        
        /* Inline assembly barrier */
        asm volatile ("" : : "r" (out[i]) : "memory");
    }
}

/* Pointer chasing pattern */
int64_t pointer_chase(int64_t* data, int size, int steps) {
    int64_t* ptr = data;
    int64_t sum = 0;
    volatile int vol_step = 0;
    
    for (int i = 0; i < steps; i++) {
        /* Pointer chasing with computation */
        int64_t val = *ptr;
        sum = sum * 6364136223846793005LL + val + 1;
        
        /* Non-linear access pattern */
        int next_idx = (val ^ sum) % size;
        ptr = &data[next_idx];
        
        /* Complex operation every 8 steps */
        if (i % 8 == 0) {
            sum = complex_op(sum, val, i);
        }
        
        /* Volatile prevents optimization */
        vol_step = i;
    }
    
    return sum;
}

int main(void) {
    const int DATA_SIZE = 10000;
    const int RESULT_SIZE = 1000;
    const int STRIDE = 10;
    
    /* Allocate with alignment for better scheduling */
    int64_t* data = aligned_alloc(64, DATA_SIZE * sizeof(int64_t));
    int64_t* result = aligned_alloc(64, RESULT_SIZE * sizeof(int64_t));
    int64_t* out = aligned_alloc(64, DATA_SIZE * sizeof(int64_t));
    
    if (!data || !result || !out) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < DATA_SIZE; i++) {
        data[i] = rand() ^ (rand() << 16) ^ ((int64_t)rand() << 32);
    }
    
    printf("Starting complex computations...\n");
    
    /* First kernel - nested loops with dependencies */
    compute_kernel(data, result, DATA_SIZE, STRIDE);
    
    /* Second kernel - switch-based computation */
    compute_switch_kernel(data, out, DATA_SIZE);
    
    /* Pointer chasing computation */
    int64_t chase_result = pointer_chase(data, DATA_SIZE, 5000);
    
    /* Final reduction to prevent optimization */
    int64_t final_result = 0;
    for (int i = 0; i < RESULT_SIZE; i++) {
        final_result ^= result[i];
    }
    for (int i = 0; i < DATA_SIZE; i += 13) {
        final_result += out[i];
    }
    final_result ^= chase_result;
    
    printf("Final result: %ld\n", (long)final_result);
    
    /* Cleanup */
    free(data);
    free(result);
    free(out);
    
    return 0;
}
