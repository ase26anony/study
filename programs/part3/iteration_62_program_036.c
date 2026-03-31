/* sel-sched-trigger.c
 * Program designed to trigger GCC selective scheduler debugging dumps
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-dump -std=gnu11 sel-sched-trigger.c -o sel-sched-trigger
 */

#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 12345;
static inline uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Force selective scheduling on this function */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-dump")))
void test_selective_scheduling_1(int *arr_a, int *arr_b, float *arr_c, int size) {
    volatile int vol_size = size; /* Prevent constant propagation */
    int sum = 0;
    float fsum = 0.0f;
    
    /* Nested loops with data-dependent branches and mixed operations */
    for (int outer = 0; outer < 4; outer++) {
        int base = (outer * 17) & 0xFF;
        
        for (int i = 0; i < vol_size; i++) {
            /* Create flow dependency chain */
            int val_a = arr_a[i];
            int val_b = arr_b[i];
            
            /* Mixed integer operations */
            int prod = val_a * val_b;
            sum = sum + prod;
            
            /* Data-dependent conditional branch */
            if (sum & 0x100) {
                /* Branch taken unpredictably */
                arr_a[i] = sum ^ base;
                sum = sum >> 1;
            } else {
                arr_a[i] = prod ^ base;
            }
            
            /* Floating-point operations interleaved */
            float fval = (float)val_a * 0.5f;
            fsum = fsum + fval;
            
            /* Control dependency */
            if (fsum > 1000.0f) {
                arr_c[i] = fsum;
                fsum = fsum * 0.9f;
            }
            
            /* Scheduling barrier */
            asm volatile("" ::: "memory");
        }
    }
}

/* Function with volatile counters and manual unrolling */
__attribute__((optimize("O3", "fsel-sched-pipelining")))
void test_selective_scheduling_2(double *arr_d, int *arr_e, int size) {
    volatile int vol_start = 0;
    volatile int vol_end = size;
    double acc = 0.0;
    
    for (int j = vol_start; j < vol_end - 3; j += 4) {
        /* Manually unrolled loop with different operations */
        double d1 = arr_d[j] * 1.1;
        double d2 = arr_d[j+1] * 1.2;
        double d3 = arr_d[j+2] * 1.3;
        double d4 = arr_d[j+3] * 1.4;
        
        /* Create anti-dependencies */
        arr_d[j] = d1 + acc;
        acc = d1;
        
        arr_d[j+1] = d2 + acc;
        acc = d2;
        
        arr_d[j+2] = d3 + acc;
        arr_e[j+2] = (int)d3;
        
        arr_d[j+3] = d4 + acc;
        arr_e[j+3] = (int)d4;
        
        /* Complex conditional with output dependency */
        if ((j & 7) == 0) {
            double temp = arr_d[j] * arr_d[j+1];
            arr_d[j] = temp;
            arr_d[j+1] = temp * 0.5;
        }
        
        /* Another scheduling barrier */
        asm volatile("" ::: "memory");
    }
}

/* Outer loop carried state pattern */
void test_outer_loop_carried_state(int *arr, int size) {
    int outer_state = 0;
    
    for (int phase = 0; phase < 8; phase++) {
        /* Outer loop modifies state used in inner loop */
        outer_state = (outer_state * 13 + phase * 7) & 0xFF;
        int factor = (outer_state & 0xF) + 1;
        
        for (int i = 0; i < size; i++) {
            /* Loop-carried dependency spanning nesting levels */
            int val = arr[i];
            val = (val + outer_state) * factor;
            
            /* Data-dependent operation */
            if (val > 1000) {
                val = val >> 2;
                factor = (factor + 1) & 0xF;
            }
            
            arr[i] = val;
            
            /* Mix of operations to create resource conflicts */
            if (i & 1) {
                float ftemp = (float)val * 0.3f;
                arr[i] += (int)ftemp;
            }
        }
        
        /* Prevent loop invariant code motion */
        asm volatile("" ::: "memory");
    }
}

int main(void) {
    const int SIZE = 1024;
    int array_a[SIZE];
    int array_b[SIZE];
    float array_c[SIZE];
    double array_d[SIZE];
    int array_e[SIZE];
    
    /* Initialize with pseudo-random but non-uniform data */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (int)(lcg_rand() & 0xFFF);
        array_b[i] = (int)(lcg_rand() & 0xFFF);
        array_c[i] = (float)(lcg_rand() & 0xFF) * 0.1f;
        array_d[i] = (double)(lcg_rand() & 0xFF) * 0.01;
        array_e[i] = 0;
    }
    
    volatile int run_flag = 1;
    int checksum = 0;
    
    /* Runtime-variable control flow to encourage scheduling */
    if (run_flag) {
        for (int rep = 0; rep < 5; rep++) {
            test_selective_scheduling_1(array_a, array_b, array_c, SIZE);
        }
    }
    
    /* Volatile condition to prevent optimization */
    volatile int cond = array_a[0] & 1;
    if (cond) {
        for (int rep = 0; rep < 3; rep++) {
            test_selective_scheduling_2(array_d, array_e, SIZE);
        }
    }
    
    /* Always run the outer loop test */
    test_outer_loop_carried_state(array_a, SIZE);
    
    /* Compute final checksum to prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        checksum += array_a[i];
        checksum += array_b[i];
        checksum += (int)array_c[i];
        checksum += (int)array_d[i];
        checksum += array_e[i];
        
        /* Prevent simple accumulation optimization */
        checksum = (checksum << 3) | (checksum >> 29);
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
