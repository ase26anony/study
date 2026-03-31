/* sel_sched_trigger.c - Program to trigger GCC selective scheduler debugging dumps */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024
#define OUTER_LOOP 8
#define INNER_LOOP 128

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 12345;
static inline uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Function 1: Complex nested loops with data-dependent branches and mixed operations
   Force selective scheduling with attribute */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-dump")))
void test_selective_scheduling(int *arr_a, int *arr_b, float *arr_c, volatile int *trigger) {
    float sum_f = 0.0f;
    double sum_d = 0.0;
    int sum_i = 0;
    
    /* Outer loop with carried state */
    for (int outer = 0; outer < OUTER_LOOP; outer++) {
        int base = (*trigger + outer) & 0xFF;
        float threshold = (base * 0.1f) + 10.0f;
        
        /* Manually unrolled inner loop with mixed operations */
        for (int i = 0; i < INNER_LOOP; i += 2) {
            /* Flow dependency chain with mixed types */
            float temp_f = arr_c[i] * 1.5f + base;
            sum_f += temp_f;
            
            /* Data-dependent branch creating control dependency */
            if (sum_f > threshold) {
                arr_c[i] = sum_f;
                sum_f = 0.0f;
                /* Inline asm barrier to prevent reordering */
                asm volatile("" ::: "memory");
            }
            
            /* Integer operations with anti-dependencies */
            int old_val = arr_a[i];
            arr_a[i] = arr_b[i] + base + old_val;
            arr_b[i] = old_val - base;
            
            /* Second unrolled iteration with different pattern */
            if (i + 1 < INNER_LOOP) {
                double temp_d = arr_c[i+1] * 2.5;
                sum_d += temp_d;
                
                /* Another conditional with output dependency */
                if (arr_a[i+1] & 1) {
                    arr_c[i+1] = (float)sum_d;
                    sum_d *= 0.5;
                }
                
                /* More complex integer math */
                sum_i += arr_a[i+1] * arr_b[i+1];
                arr_a[i+1] ^= (sum_i & 0xFF);
                
                /* Another memory barrier */
                asm volatile("" ::: "memory");
            }
        }
        
        /* Loop-carried dependency to outer loop */
        *trigger = (*trigger + sum_i) & 0xFFFF;
    }
}

/* Function 2: Volatile counters and explicit scheduling barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
void test_volatile_barriers(float *arr_f, double *arr_d, volatile int *counter) {
    volatile int v_limit = *counter % 64 + 32;
    float acc_f[4] = {0};  /* Small array for accumulation */
    
    for (volatile int v = 0; v < v_limit; v++) {
        /* Multiple accumulators to create register pressure */
        for (int i = 0; i < ARRAY_SIZE; i += 4) {
            /* Four parallel accumulation chains */
            acc_f[0] += arr_f[i] * 1.1f;
            acc_f[1] += arr_f[i+1] * 2.2f;
            acc_f[2] += arr_f[i+2] * 3.3f;
            acc_f[3] += arr_f[i+3] * 4.4f;
            
            /* Conditional store with barrier */
            if (i % 16 == 0) {
                arr_d[i/4] = acc_f[0] + acc_f[1] + acc_f[2] + acc_f[3];
                asm volatile("" ::: "memory");
            }
            
            /* Complex floating point with dependency chain */
            double d1 = arr_d[i/4] * 0.5;
            double d2 = d1 * d1;
            arr_f[i] = (float)(d2 * 0.25);
            
            /* Another barrier */
            asm volatile("" ::: "memory");
        }
        
        /* Rotate accumulators to create loop-carried dependencies */
        float temp = acc_f[0];
        acc_f[0] = acc_f[1];
        acc_f[1] = acc_f[2];
        acc_f[2] = acc_f[3];
        acc_f[3] = temp;
    }
    
    *counter = (int)(acc_f[0] + acc_f[1] + acc_f[2] + acc_f[3]);
}

/* Function 3: Outer-loop carried state with irregular access patterns */
void test_outer_carried_state(int *arr, volatile int *state, int iterations) {
    int local_state = *state;
    
    for (int iter = 0; iter < iterations; iter++) {
        int base = (local_state * 13 + 7) & 0xFF;
        int factor = (iter % 3) + 1;
        
        /* Inner loop with stride and condition */
        for (int i = 0; i < ARRAY_SIZE; i += factor) {
            /* Multiple dependency types in one expression */
            int old = arr[i];
            arr[i] = (old + base) * factor;
            
            /* Output dependency with conditional */
            if (old > base) {
                arr[i] ^= 0xAA;
                /* Memory clobber to force scheduling consideration */
                asm volatile("" ::: "memory");
            }
            
            /* Anti-dependency chain */
            int temp = arr[i];
            arr[i] = temp - base;
            base = (base + temp) & 0x7F;
        }
        
        /* Update carried state for next outer iteration */
        local_state = (local_state + base) & 0xFFFF;
        
        /* Every few iterations, add extra complexity */
        if (iter % 4 == 0) {
            for (int i = 0; i < 16; i++) {
                arr[i] = arr[i] * 3 - local_state;
            }
            asm volatile("" ::: "memory");
        }
    }
    
    *state = local_state;
}

int main(void) {
    /* Initialize arrays with non-uniform data */
    int array_a[ARRAY_SIZE];
    int array_b[ARRAY_SIZE];
    float array_c[ARRAY_SIZE];
    double array_d[ARRAY_SIZE/4];
    
    volatile int trigger = 42;
    volatile int counter = 1;
    volatile int state = 0xABCD;
    
    /* Fill arrays with pseudo-random but bounded values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array_a[i] = (int)(lcg_rand() % 1000);
        array_b[i] = (int)(lcg_rand() % 1000);
        array_c[i] = (float)(lcg_rand() % 100) * 0.1f;
    }
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        array_d[i] = (double)(lcg_rand() % 100) * 0.01;
    }
    
    /* Variable control flow to prevent static optimization */
    volatile int run_selector = array_a[0] & 3;
    
    /* Call test functions based on runtime conditions */
    for (int repeat = 0; repeat < 5; repeat++) {
        if (run_selector & 1) {
            test_selective_scheduling(array_a, array_b, array_c, &trigger);
        }
        
        if (run_selector & 2) {
            test_volatile_barriers(array_c, array_d, &counter);
        }
        
        /* Update selector based on array contents */
        run_selector = (run_selector + array_a[repeat % ARRAY_SIZE]) & 3;
    }
    
    /* More iterations with outer carried state */
    for (int batch = 0; batch < 3; batch++) {
        test_outer_carried_state(array_a, &state, 4 + (batch % 3));
        trigger = (trigger + state) & 0xFF;
    }
    
    /* Final checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += (uint64_t)array_a[i];
        checksum += (uint64_t)array_b[i];
        checksum += (uint64_t)(array_c[i] * 1000);
    }
    for (int i = 0; i < ARRAY_SIZE/4; i++) {
        checksum += (uint64_t)(array_d[i] * 10000);
    }
    
    printf("Final checksum: %llu\n", (unsigned long long)checksum);
    printf("Trigger: %d, Counter: %d, State: %d\n", trigger, counter, state);
    
    return 0;
}
