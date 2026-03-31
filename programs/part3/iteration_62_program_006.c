/* sel-sched-trigger.c - Program to trigger selective scheduler debugging dumps */
#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Function 1: Complex nested loops with data-dependent branches */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-dump")))
void test_complex_schedule(int *restrict a, int *restrict b, float *restrict c, 
                           int n, volatile int threshold) {
    float sum_f = 0.0f;
    double sum_d = 0.0;
    int sum_i = 0;
    
    /* Outer loop with carried state */
    for (int j = 0; j < 4; j++) {
        int base = (j * 17) & 0xFF;
        
        /* Inner loop with mixed operations and data-dependent branches */
        for (int i = 0; i < n; i++) {
            /* Create flow dependency chain */
            int val1 = a[i] + base;
            int val2 = b[i] * 3;
            
            /* Mixed floating point operations */
            float fval = (float)val1 * 0.5f;
            double dval = (double)val2 * 1.5;
            
            /* Data-dependent conditional branch */
            if ((val1 & 1) && (val2 > threshold)) {
                c[i] = fval + (float)dval;
                sum_f += c[i];
                /* Inline asm barrier to prevent reordering */
                asm volatile("" ::: "memory");
            } else {
                c[i] = fval - (float)dval;
                sum_d += dval;
            }
            
            /* Manual unrolling (2 iterations) */
            if (i + 1 < n) {
                int val3 = a[i+1] ^ base;
                int val4 = b[i+1] + 7;
                
                if ((val3 & 2) || (val4 < threshold)) {
                    sum_i += val3 * val4;
                    /* Another scheduling barrier */
                    asm volatile("" ::: "memory");
                }
                
                /* Anti-dependency: reuse variables */
                val1 = val3;
                val2 = val4;
                i++;  /* Increment loop counter */
            }
            
            /* Output dependency simulation */
            {
                static int counter = 0;
                counter = (counter + 1) & 0xF;
                if (counter == 0) {
                    sum_i = sum_i >> 1;
                }
            }
        }
        
        /* Loop-carried dependency to outer loop */
        base = (base + sum_i) & 0xFF;
        threshold = (threshold + 1) & 0x3FF;
    }
    
    /* Prevent dead code elimination */
    volatile float v_sum_f = sum_f;
    volatile double v_sum_d = sum_d;
    volatile int v_sum_i = sum_i;
    (void)v_sum_f; (void)v_sum_d; (void)v_sum_i;
}

/* Function 2: Volatile counters with inline assembly barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
void test_volatile_barriers(float *arr, int size, volatile int start) {
    volatile int v_counter = start;
    float acc1 = 0.0f, acc2 = 0.0f, acc3 = 0.0f;
    
    for (volatile int v_i = 0; v_i < size; v_i = v_i + 1) {
        /* Multiple accumulators with different operations */
        float val = arr[v_i];
        
        acc1 = acc1 + val * 1.1f;
        asm volatile("" ::: "memory");  /* Barrier 1 */
        
        acc2 = acc2 - val * 0.9f;
        if (v_counter & 1) {
            acc3 = acc3 + val * 1.5f;
            asm volatile("" ::: "memory");  /* Barrier 2 */
        } else {
            acc3 = acc3 - val * 0.5f;
        }
        
        /* Complex conditional with volatile */
        if (v_counter > (v_i * 2)) {
            arr[v_i] = acc1 + acc2;
            v_counter = v_counter - 1;
        } else {
            arr[v_i] = acc3;
            v_counter = v_counter + 2;
        }
        
        /* Nested short loop */
        for (int k = 0; k < 3; k++) {
            float tmp = arr[v_i] * (k + 1);
            asm volatile("" ::: "memory");  /* Barrier 3 */
            arr[v_i] = tmp * 0.333f;
        }
    }
    
    /* Use results */
    volatile float v_acc1 = acc1, v_acc2 = acc2, v_acc3 = acc3;
    (void)v_acc1; (void)v_acc2; (void)v_acc3;
}

/* Function 3: Outer-loop carried state pattern */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
void test_outer_carried_state(int *data, int rows, int cols) {
    int state = 0;
    
    for (int r = 0; r < rows; r++) {
        /* Outer loop modifies state */
        int base = (state + r * 7) & 0xFF;
        int factor = 1 + (r & 3);
        
        /* Inner loop with dependency on outer state */
        for (int c = 0; c < cols; c++) {
            int idx = r * cols + c;
            
            /* Multiple dependency types */
            int old_val = data[idx];           /* Flow dependency */
            data[idx] = (old_val + base) * factor;
            
            /* Anti-dependency through temporary */
            int temp = data[idx] ^ base;
            data[idx] = temp;
            
            /* Output dependency */
            state = state + (data[idx] & 1);
            
            /* Manual unrolling (4 iterations) */
            if (c + 3 < cols) {
                for (int u = 1; u <= 3; u++) {
                    int idx2 = r * cols + c + u;
                    data[idx2] = (data[idx2] + base + u) * (factor - (u & 1));
                    state = state ^ data[idx2];
                }
                c += 3;
            }
        }
        
        /* Cross-iteration dependency */
        if (state > 1000) {
            state = state >> 1;
            asm volatile("" ::: "memory");
        }
    }
    
    volatile int v_state = state;
    (void)v_state;
}

int main(void) {
    const int SIZE = 1024;
    const int ROWS = 32;
    const int COLS = 32;
    
    /* Initialize arrays with pseudo-random data */
    int array_a[SIZE], array_b[SIZE];
    float array_c[SIZE];
    int matrix[ROWS * COLS];
    
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (int)lcg_rand() % 1000;
        array_b[i] = (int)lcg_rand() % 1000;
        array_c[i] = (float)(lcg_rand() % 100) * 0.1f;
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        matrix[i] = (int)lcg_rand() % 500;
    }
    
    /* Volatile flag to prevent optimization */
    volatile int flag = array_a[0] & 1;
    
    /* Call test functions based on runtime conditions */
    if (flag) {
        for (int rep = 0; rep < 5; rep++) {
            test_complex_schedule(array_a, array_b, array_c, SIZE, 250);
        }
    }
    
    volatile int start_val = array_b[0] % 100;
    for (int rep = 0; rep < 3; rep++) {
        test_volatile_barriers(array_c, SIZE, start_val + rep);
    }
    
    if (!flag) {
        test_outer_carried_state(matrix, ROWS, COLS);
    } else {
        for (int rep = 0; rep < 2; rep++) {
            test_outer_carried_state(matrix, ROWS / 2, COLS);
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += (uint32_t)array_a[i];
        checksum += (uint32_t)array_b[i];
        checksum += (uint32_t)(array_c[i] * 1000.0f);
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        checksum += (uint32_t)matrix[i];
    }
    
    printf("Final checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
