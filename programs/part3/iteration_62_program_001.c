/* sel-sched-trigger.c - Program to trigger selective scheduler debugging dumps */
#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 12345;
static uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Force selective scheduling with optimization attributes */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
void test_complex_schedule(int* restrict a, int* restrict b, float* restrict c, 
                          int size, volatile int threshold) {
    /* Mixed data types and dependencies */
    float sum_f = 0.0f;
    double acc_d = 0.0;
    int sum_i = 0;
    
    /* Outer loop with carried state */
    for (int outer = 0; outer < 4; outer++) {
        int base = (outer * 17) & 0xFF;  /* Non-trivial base computation */
        
        /* Inner loop with data-dependent branches */
        for (int i = 0; i < size; i++) {
            /* Flow dependency: sum_i depends on previous iteration */
            sum_i += a[i] * (b[i] + base);
            
            /* Anti dependency: a[i] read before potential write */
            int temp = a[i];
            
            /* Control dependency with unpredictable branch */
            if (sum_i > threshold) {
                /* Output dependency: c[i] written */
                c[i] = (float)sum_i * 0.5f;
                sum_i = sum_i / 2;  /* Reset partially */
                
                /* Memory barrier to complicate scheduling */
                asm volatile("" ::: "memory");
            }
            
            /* Mixed floating-point operations creating resource conflicts */
            sum_f += (float)temp * 1.5f;
            acc_d += (double)sum_f * 0.25;
            
            /* Manual unrolling (2 iterations) */
            if (i + 1 < size) {
                int next = i + 1;
                sum_i += a[next] * (b[next] + (base ^ 0x55));
                
                /* Another conditional with different condition */
                if ((a[next] & 1) && (sum_i < threshold * 2)) {
                    c[next] = sum_f * 2.0f;
                    asm volatile("" ::: "memory");
                }
                
                /* More mixed operations */
                sum_f -= (float)b[next] * 0.75f;
                i++;  /* Advance counter for unrolled iteration */
            }
        }
        
        /* Loop-carried dependency to outer loop */
        threshold = (threshold + base) & 0x3FF;
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r"(sum_i), "+r"(sum_f) : : "memory");
}

/* Function with volatile counters and assembly barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining")))
void test_volatile_barriers(int* arr, int size) {
    volatile int v_counter = 0;
    volatile int v_limit = size / 2;
    
    /* Nested loops with volatile conditions */
    for (volatile int outer = 0; outer < 3; outer++) {
        int base = outer * 256;
        
        for (int i = 0; i < size; i++) {
            /* Multiple dependency types in one expression */
            arr[i] = (arr[i] * 3 + base) / 2;
            
            /* Data-dependent array access pattern */
            int idx = (arr[i] + i) % size;
            arr[idx] ^= 0xAAAAAAAA;
            
            /* Frequent memory barriers */
            asm volatile("" ::: "memory");
            
            /* Complex conditional with volatile check */
            if (v_counter++ > v_limit) {
                arr[i] = ~arr[i];
                v_limit = (v_limit + 1) % size;
                asm volatile("" ::: "memory");
            }
            
            /* Floating point operations mixed with integer */
            float fval = (float)arr[i] * 0.333f;
            arr[i] += (int)fval;
        }
        
        /* Outer loop modification */
        base = (base + 1) ^ 0xFF;
    }
}

/* Outer loop carried state pattern */
__attribute__((optimize("O2", "fsel-sched-pipelining-outer-loops")))
void test_outer_carried_state(double* data, int rows, int cols) {
    double row_accum = 0.0;
    
    for (int r = 0; r < rows; r++) {
        /* State carried from previous outer iteration */
        double col_base = row_accum * 0.1 + (double)r;
        int col_mod = (r * 13) % cols;
        
        for (int c = 0; c < cols; c++) {
            /* Complex addressing with outer loop dependency */
            int idx = (r * cols + c) % (rows * cols);
            
            /* Mixed operations with outer loop state */
            data[idx] = data[idx] * 1.1 + col_base;
            
            /* Conditional that depends on outer loop */
            if (c == col_mod) {
                data[idx] = -data[idx];
                asm volatile("" ::: "memory");
            }
            
            /* Update carried state */
            row_accum += data[idx] * 0.01;
            
            /* Additional dependency chain */
            if (c % 4 == 0) {
                data[idx] = data[idx] / (1.0 + (double)c);
            }
        }
        
        /* Cross-iteration dependency in outer loop */
        row_accum = row_accum * 0.9;
    }
}

int main(void) {
    const int SIZE = 1024;
    const int ROWS = 32;
    const int COLS = 32;
    
    /* Initialize arrays with pseudo-random data */
    int array_a[SIZE];
    int array_b[SIZE];
    float array_c[SIZE];
    double array_d[ROWS * COLS];
    
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (int)lcg_rand() % 1000;
        array_b[i] = (int)lcg_rand() % 1000;
        array_c[i] = (float)(lcg_rand() % 100) * 0.1f;
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        array_d[i] = (double)(lcg_rand() % 1000) * 0.01;
    }
    
    /* Volatile flag to prevent compile-time optimization */
    volatile int run_flag = 1;
    volatile int threshold = 500;
    
    long long checksum = 0;
    
    /* Variable execution path based on runtime values */
    if (run_flag) {
        for (int rep = 0; rep < 5; rep++) {
            test_complex_schedule(array_a, array_b, array_c, SIZE, threshold);
            threshold = (threshold + 100) % 800;
        }
    }
    
    /* Alternate execution path */
    volatile int alt_flag = array_a[0] > 250;
    if (alt_flag) {
        for (int rep = 0; rep < 3; rep++) {
            test_volatile_barriers(array_a, SIZE);
        }
    }
    
    /* Always execute the outer loop test */
    test_outer_carried_state(array_d, ROWS, COLS);
    
    /* Compute checksums to prevent elimination */
    for (int i = 0; i < SIZE; i++) {
        checksum += array_a[i] + (int)array_c[i];
        checksum += array_b[i];
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        checksum += (long long)array_d[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    return 0;
}
