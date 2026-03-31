/* sel-sched-trigger.c
 * Program designed to trigger GCC selective scheduler debugging dumps
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-dump -std=gnu11 sel-sched-trigger.c -o sel-sched-trigger
 */

#include <stdio.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define OUTER_LOOP 8
#define INNER_LOOP 128

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 12345;
static inline uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Function 1: Complex nested loop with data-dependent branches */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-dump")))
void test_complex_schedule(int *arr_a, int *arr_b, float *arr_c, volatile int *trigger) {
    float sum_f = 0.0f;
    double sum_d = 0.0;
    int sum_i = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int limit = *trigger;
    
    for (int j = 0; j < OUTER_LOOP; j++) {
        /* Outer loop carried state */
        int base = (j * 37) & 0xFF;
        float factor = 1.0f + (j * 0.1f);
        
        /* Manually unrolled inner loop with mixed operations */
        for (int i = 0; i < INNER_LOOP; i += 2) {
            /* Flow dependency chain with anti and output dependencies */
            int idx = (i + base) % ARRAY_SIZE;
            
            /* Multiple data types and operations */
            float temp_f = arr_c[idx];
            int temp_i = arr_a[idx];
            
            /* Data-dependent conditional branch */
            if (temp_i & 1) {
                sum_f = sum_f + temp_f * factor;
                /* Inline asm barrier to create scheduling complexity */
                asm volatile("" ::: "memory");
                arr_b[idx] = (int)(sum_f) + base;
            } else {
                sum_d = sum_d + (double)temp_f * 0.5;
                arr_b[idx] = (int)(sum_d) - base;
            }
            
            /* Second iteration of unrolled loop */
            idx = (i + 1 + base) % ARRAY_SIZE;
            temp_i = arr_a[idx];
            temp_f = arr_c[idx];
            
            /* Different conditional pattern */
            if (temp_i & 2) {
                sum_i = sum_i + temp_i * 3;
                arr_b[idx] = sum_i ^ base;
                /* Another scheduling barrier */
                asm volatile("" ::: "memory");
            } else {
                sum_f = sum_f - temp_f * 0.25f;
                arr_b[idx] = (int)(sum_f * 100.0f);
            }
            
            /* Complex dependency: mix all sums */
            if ((sum_i + (int)sum_f) > limit) {
                arr_c[idx] = (float)(sum_i % 100);
                sum_i = sum_i / 2;
                asm volatile("" ::: "memory");
            }
        }
        
        /* Cross-iteration dependency */
        limit = (limit + base) & 0x3FF;
    }
}

/* Function 2: Volatile counters with inline assembly barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining-outer-loops")))
void test_volatile_schedule(double *arr_d, int *arr_i, volatile int counter) {
    volatile int v_limit = counter;
    double acc[4] = {0.0, 0.0, 0.0, 0.0};
    
    for (volatile int v = 0; v < v_limit; v++) {
        int idx = v % (ARRAY_SIZE - 3);
        
        /* Four parallel accumulation chains with barriers */
        acc[0] = acc[0] + arr_d[idx] * 1.1;
        asm volatile("" ::: "memory");
        
        acc[1] = acc[1] + arr_d[idx + 1] * 0.9;
        asm volatile("" ::: "memory");
        
        acc[2] = acc[2] + arr_d[idx + 2] * 1.05;
        /* Conditional barrier placement */
        if (idx & 4) {
            asm volatile("" ::: "memory");
        }
        
        acc[3] = acc[3] + arr_d[idx + 3] * 0.95;
        
        /* Data-dependent store with output dependency */
        if (acc[0] > acc[1]) {
            arr_i[idx] = (int)(acc[0] - acc[1]);
            asm volatile("" ::: "memory");
        } else {
            arr_i[idx] = (int)(acc[1] - acc[0]);
        }
        
        /* Complex condition with all accumulators */
        if ((acc[0] + acc[1]) > (acc[2] + acc[3])) {
            arr_d[idx] = acc[0] * 0.5;
            asm volatile("" ::: "memory");
        }
    }
}

/* Function 3: Outer-loop carried state pattern */
void test_outer_carried_state(int *arr, float *farr, int outer_iter) {
    int carried_state = 0;
    
    for (int j = 0; j < outer_iter; j++) {
        /* Outer loop modifies carried state */
        carried_state = (carried_state * 73 + j * 59) & 0xFFF;
        float factor = 1.0f + (carried_state % 100) * 0.01f;
        
        /* Inner loop uses carried state */
        for (int i = 0; i < INNER_LOOP; i++) {
            int idx = (i + j) % ARRAY_SIZE;
            
            /* Mixed operations with carried dependency */
            int val = arr[idx];
            float fval = farr[idx];
            
            /* Data-dependent operation */
            if (val > carried_state) {
                farr[idx] = fval * factor + (float)carried_state;
                arr[idx] = val - carried_state;
                asm volatile("" ::: "memory");
            } else {
                farr[idx] = fval / factor - (float)carried_state;
                arr[idx] = val + carried_state;
            }
            
            /* Additional complexity with periodic reset */
            if ((i & 15) == 0) {
                carried_state = (carried_state + 1) & 0xFF;
                asm volatile("" ::: "memory");
            }
        }
    }
}

int main(void) {
    /* Initialize arrays with pseudo-random data */
    int array_a[ARRAY_SIZE];
    int array_b[ARRAY_SIZE];
    float array_c[ARRAY_SIZE];
    double array_d[ARRAY_SIZE];
    
    printf("Initializing arrays with pseudo-random data...\n");
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array_a[i] = (int)lcg_rand() % 1000;
        array_b[i] = 0;
        array_c[i] = (float)(lcg_rand() % 1000) * 0.1f;
        array_d[i] = (double)(lcg_rand() % 1000) * 0.01;
    }
    
    /* Volatile flag to introduce runtime variability */
    volatile int run_flag = 1;
    volatile int trigger_val = 50;
    
    /* Call test functions multiple times with volatile conditions */
    if (run_flag) {
        for (int rep = 0; rep < 5; rep++) {
            printf("Running complex schedule test %d...\n", rep + 1);
            test_complex_schedule(array_a, array_b, array_c, &trigger_val);
            trigger_val = (trigger_val + 17) & 0x7F;
        }
    }
    
    volatile int counter = 100;
    if (run_flag) {
        for (int rep = 0; rep < 3; rep++) {
            printf("Running volatile schedule test %d...\n", rep + 1);
            test_volatile_schedule(array_d, array_a, counter);
            counter = (counter + 25) % 150;
        }
    }
    
    if (run_flag) {
        printf("Running outer carried state test...\n");
        test_outer_carried_state(array_b, array_c, 4);
    }
    
    /* Compute final checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += (uint64_t)array_a[i];
        checksum += (uint64_t)array_b[i];
        checksum += (uint64_t)(array_c[i] * 100.0f);
        checksum += (uint64_t)(array_d[i] * 1000.0);
    }
    
    printf("Final checksum: %llu\n", (unsigned long long)checksum);
    
    return 0;
}
