/* sel-sched-trigger.c - Program to trigger GCC selective scheduler debugging dumps */
#include <stdio.h>
#include <stdint.h>

#define SIZE 1024
#define OUTER_ITER 8
#define INNER_ITER 128

/* Simple LCG pseudo-random generator */
static uint32_t lcg_seed = 12345;
static inline uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Initialize arrays with pseudo-random data */
static void init_arrays(int32_t *a, int32_t *b, float *fa, double *db) {
    for (int i = 0; i < SIZE; i++) {
        a[i] = (int32_t)(lcg_rand() % 1000) - 500;
        b[i] = (int32_t)(lcg_rand() % 1000) - 500;
        fa[i] = (float)(lcg_rand() % 1000) / 10.0f;
        db[i] = (double)(lcg_rand() % 1000) / 10.0;
    }
}

/* Test function 1: Complex nested loops with data-dependent branches
   Force selective scheduling with attribute */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-dump")))
static int32_t test_complex_schedule(int32_t *arr_a, int32_t *arr_b, 
                                     float *arr_f, double *arr_d) {
    volatile int32_t threshold = 10000; /* Prevent constant propagation */
    int32_t sum = 0;
    int32_t temp_acc = 0;
    float f_acc = 0.0f;
    double d_acc = 0.0;
    
    /* Outer loop with carried state */
    for (int j = 0; j < OUTER_ITER; j++) {
        int32_t base = (j * 37) & 0xFF; /* Computation with outer loop index */
        
        /* Inner loop with mixed operations and data-dependent branches */
        for (int i = 0; i < INNER_ITER; i++) {
            int idx = (j * INNER_ITER + i) % SIZE;
            
            /* Multiple dependency types: flow, anti, output */
            int32_t val_a = arr_a[idx];
            int32_t val_b = arr_b[idx];
            float f_val = arr_f[idx];
            double d_val = arr_d[idx];
            
            /* Flow dependency chain */
            sum = sum + val_a * val_b;
            
            /* Anti-dependency: read then write same variable */
            temp_acc = temp_acc + base;
            arr_a[idx] = temp_acc; /* Write creates anti-dependency */
            
            /* Control dependency with unpredictable branch */
            if (sum > threshold) {
                /* Complex operation when condition met */
                arr_b[idx] = sum & 0xFF;
                sum = sum >> 1; /* Modify carried state */
                
                /* Mixed floating-point operations */
                f_acc = f_acc + f_val * 1.5f;
                d_acc = d_acc + d_val * 2.5;
                
                /* Inline assembly barrier - creates scheduling boundary */
                asm volatile("" ::: "memory");
            } else if (sum < -threshold) {
                /* Another branch path */
                arr_b[idx] = (-sum) & 0xFF;
                f_acc = f_acc - f_val;
                
                /* Another scheduling barrier */
                asm volatile("" ::: "memory");
            }
            
            /* Manual unrolling - creates more ILP opportunities */
            if (i & 1) {
                /* Different operations on odd iterations */
                arr_f[idx] = f_acc * 0.9f;
                arr_d[idx] = d_acc * 0.8;
            } else {
                /* Even iterations do something else */
                arr_f[idx] = f_val + 1.0f;
                arr_d[idx] = d_val - 1.0;
            }
        }
        
        /* Outer loop modification of inner loop variable */
        threshold = (threshold + base) % 20000;
    }
    
    return sum + (int32_t)f_acc + (int32_t)d_acc;
}

/* Test function 2: Volatile counters and explicit scheduling barriers */
static int32_t test_volatile_barriers(int32_t *arr_a, int32_t *arr_b) {
    volatile int v_counter = 0; /* Volatile prevents optimization */
    int32_t result = 0;
    
    for (volatile int v_i = 0; v_i < 256; v_i++) {
        /* Multiple memory barriers create complex dependency graph */
        asm volatile("" ::: "memory");
        
        /* Unrolled loop with volatile access */
        for (int j = 0; j < 4; j++) {
            int idx = (v_i * 4 + j) % SIZE;
            
            /* Read-modify-write with volatile influence */
            int32_t old = arr_a[idx];
            arr_a[idx] = old + v_counter + (j * 17);
            
            /* Complex conditional with volatile */
            if (v_counter > (idx & 0x3F)) {
                arr_b[idx] = arr_b[idx] * 3;
                result += arr_b[idx];
                
                /* Scheduling barrier in conditional path */
                asm volatile("" ::: "memory");
            } else {
                arr_b[idx] = arr_b[idx] / 2;
                result -= arr_b[idx];
            }
            
            /* Update volatile based on computation */
            v_counter = (result & 0xFF) + v_i;
        }
        
        /* Another barrier at loop end */
        asm volatile("" ::: "memory");
    }
    
    return result;
}

/* Test function 3: Outer loop carried state with nested dependencies */
static double test_outer_carried_state(float *arr_f, double *arr_d) {
    double outer_acc = 0.0;
    
    for (int outer = 0; outer < 16; outer++) {
        /* Compute base value that depends on outer loop */
        double base = 1.0;
        for (int k = 0; k < (outer & 3); k++) {
            base = base * 1.2345; /* Small computation */
        }
        
        /* Inner loop uses outer loop state */
        for (int inner = 0; inner < 64; inner++) {
            int idx = (outer * 64 + inner) % SIZE;
            
            /* Multiple floating-point operations with dependencies */
            double d1 = arr_d[idx];
            float f1 = arr_f[idx];
            
            /* Cross-type operations create complex RTL */
            double temp = d1 * base + (double)f1;
            
            /* Conditional update with carried state */
            if (temp > outer_acc) {
                arr_d[idx] = temp;
                outer_acc = temp * 0.99;
                
                /* Memory barrier in floating-point path */
                asm volatile("" ::: "memory");
            } else {
                arr_f[idx] = (float)(temp * 1.01);
                outer_acc = outer_acc - temp * 0.01;
            }
            
            /* Additional operation with anti-dependency */
            double d2 = arr_d[(idx + 1) % SIZE];
            arr_d[idx] = arr_d[idx] + d2 * 0.5;
        }
    }
    
    return outer_acc;
}

int main(void) {
    /* Declare and initialize arrays */
    int32_t array_a[SIZE];
    int32_t array_b[SIZE];
    float array_f[SIZE];
    double array_d[SIZE];
    
    init_arrays(array_a, array_b, array_f, array_d);
    
    /* Volatile flag to introduce runtime variability */
    volatile int volatile_flag = (array_a[0] > 0);
    
    int32_t total_checksum = 0;
    double fp_checksum = 0.0;
    
    /* Call test functions based on volatile condition */
    if (volatile_flag) {
        for (int rep = 0; rep < 5; rep++) {
            total_checksum += test_complex_schedule(array_a, array_b, 
                                                   array_f, array_d);
        }
    }
    
    /* Always call the second function */
    total_checksum += test_volatile_barriers(array_a, array_b);
    
    /* Call third function multiple times */
    for (int rep = 0; rep < 3; rep++) {
        fp_checksum += test_outer_carried_state(array_f, array_d);
    }
    
    /* Compute final checksum from all arrays */
    int64_t final_checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_checksum += array_a[i];
        final_checksum += array_b[i];
        final_checksum += (int64_t)(array_f[i] * 1000);
        final_checksum += (int64_t)(array_d[i] * 1000);
    }
    
    final_checksum += total_checksum;
    final_checksum += (int64_t)(fp_checksum * 1000);
    
    printf("Final checksum: %ld\n", (long)final_checksum);
    
    return 0;
}
