/* sel-sched-trigger.c
 * Designed to trigger GCC selective scheduler debugging dumps
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

/* Function 1: Annotated to force selective scheduling with complex ILP patterns */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
void test_selective_scheduling(int* restrict a, int* restrict b, float* restrict c, 
                               float* restrict d, int size, volatile int* threshold_ptr) {
    volatile int threshold = *threshold_ptr; /* Prevent constant propagation */
    float sum_f = 0.0f;
    double sum_d = 0.0;
    int sum_i = 0;
    
    /* Nested loops with data-dependent branches and mixed operations */
    for (int j = 0; j < OUTER_LOOP; j++) {
        int base = (j * 17) & 0xFF; /* Outer loop carried state */
        
        /* Manually unrolled inner loop with scheduling barriers */
        for (int i = 0; i < size - 3; i += 4) {
            /* First iteration - integer operations with flow dependency */
            int temp1 = a[i] * base;
            sum_i += temp1;
            asm volatile("" ::: "memory"); /* Scheduling barrier */
            
            /* Data-dependent branch with anti-dependency */
            if (sum_i > threshold) {
                b[i] = sum_i;
                sum_i = 0;
                threshold = (threshold * 3) / 2; /* Modify threshold */
            }
            
            /* Second iteration - floating point with output dependency */
            float f1 = c[i] * 1.5f;
            float f2 = d[i] * 0.75f;
            sum_f += f1 - f2;
            asm volatile("" ::: "memory");
            
            /* Mixed type operations creating complex RTL patterns */
            if ((a[i] & 1) && (sum_f > 100.0f)) {
                d[i] = sum_f;
                sum_f *= 0.9f;
            }
            
            /* Third iteration - double precision with control flow */
            double dval = (double)a[i + 2] / (base + 1);
            sum_d += dval;
            
            if (sum_d > 500.0) {
                c[i + 2] = (float)sum_d;
                sum_d = sum_d * 0.8;
            }
            
            /* Fourth iteration - more complex dependency chain */
            int idx = i + 3;
            int val = a[idx] ^ b[idx];
            sum_i ^= val;
            
            /* Resource conflict simulation: multiple FP operations */
            float fp_ops = c[idx] * 2.0f + d[idx] * 3.0f;
            sum_f += fp_ops;
            asm volatile("" ::: "memory");
            
            /* Unpredictable branch based on computation */
            if ((val & 0x7) == (base & 0x7)) {
                b[idx] = sum_i;
                sum_i = (sum_i >> 1) | (sum_i << 31); /* Rotate */
            }
        }
    }
    
    /* Store results to prevent elimination */
    a[0] = sum_i;
    c[0] = sum_f;
    *threshold_ptr = threshold;
}

/* Function 2: Volatile counters with assembly barriers */
void test_volatile_scheduling(int* arr, float* farr, int size) {
    volatile int v_counter = size; /* Volatile prevents optimization */
    volatile float v_threshold = 100.0f;
    
    for (volatile int i = 0; i < v_counter; i = i + 1) {
        /* Create artificial dependencies */
        int idx = i % size;
        int temp = arr[idx];
        
        /* Multiple scheduling barriers */
        asm volatile("" ::: "memory");
        
        /* Complex conditional with volatile */
        if (temp > (int)v_threshold) {
            farr[idx] = farr[idx] * 1.1f + (float)temp;
            v_threshold = v_threshold * 1.05f;
            asm volatile("" ::: "memory");
        } else {
            farr[idx] = farr[idx] * 0.95f - (float)temp;
            asm volatile("" ::: "memory");
        }
        
        /* Cross-iteration dependency */
        arr[(idx + 1) % size] = temp ^ arr[(idx + 1) % size];
        
        /* More barriers for scheduler complexity */
        asm volatile("" ::: "memory");
    }
}

/* Function 3: Outer-loop carried state pattern */
void test_outer_carried_state(int* data, int size, int iterations) {
    int outer_state = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        outer_state = (outer_state * 13 + 7) & 0xFF;
        int factor = (outer_state % 16) + 1;
        
        /* Inner loop with outer-state dependency */
        for (int i = 0; i < size; i++) {
            /* Flow dependency through outer_state */
            int val = data[i];
            
            /* Multiple operation types */
            if (val & 1) {
                data[i] = (val * factor) + outer_state;
            } else {
                data[i] = (val / factor) - outer_state;
            }
            
            /* Anti-dependency creation */
            int next_idx = (i + 1) % size;
            int temp = data[next_idx];
            data[next_idx] = temp ^ val;
            
            /* Occasional scheduling barrier */
            if ((i & 0x3F) == 0) {
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
    float array_d[ARRAY_SIZE];
    
    printf("Initializing arrays with pseudo-random data...\n");
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array_a[i] = (int)lcg_rand() % 1000;
        array_b[i] = (int)lcg_rand() % 1000;
        array_c[i] = (float)(lcg_rand() % 1000) / 10.0f;
        array_d[i] = (float)(lcg_rand() % 1000) / 10.0f;
    }
    
    /* Volatile flag to introduce runtime variability */
    volatile int v_flag = array_a[0] & 1;
    volatile int threshold = 500;
    
    /* Call test functions with runtime-dependent repetition */
    printf("Running selective scheduling tests...\n");
    
    if (v_flag) {
        for (int rep = 0; rep < 5; rep++) {
            test_selective_scheduling(array_a, array_b, array_c, array_d, 
                                     ARRAY_SIZE, (int*)&threshold);
        }
    } else {
        test_selective_scheduling(array_a, array_b, array_c, array_d, 
                                 ARRAY_SIZE, (int*)&threshold);
    }
    
    /* More conditional execution based on array state */
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= array_a[i] ^ array_b[i];
    }
    
    v_flag = (checksum & 0x10) != 0;
    
    if (v_flag) {
        for (int rep = 0; rep < 3; rep++) {
            test_volatile_scheduling(array_a, array_c, ARRAY_SIZE / 2);
        }
    }
    
    /* Always run outer carried state test */
    test_outer_carried_state(array_b, ARRAY_SIZE, 4);
    
    /* Compute final checksum to prevent dead code elimination */
    uint64_t final_checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_checksum += array_a[i];
        final_checksum += array_b[i];
        final_checksum += (uint64_t)(array_c[i] * 100);
        final_checksum += (uint64_t)(array_d[i] * 100);
        
        /* Mix in some non-linear operations */
        final_checksum = (final_checksum << 5) | (final_checksum >> 59);
        final_checksum ^= (uint64_t)i * 0x9e3779b97f4a7c15ULL;
    }
    
    printf("Final checksum: 0x%016llx\n", (unsigned long long)final_checksum);
    printf("Threshold final value: %d\n", threshold);
    
    return (final_checksum & 0xFFFF);
}
