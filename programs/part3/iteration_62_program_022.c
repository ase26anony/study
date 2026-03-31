/* sel_sched_trigger.c - Program to trigger GCC selective scheduler debugging dumps */
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
void test_selective_scheduling(int* arr_a, int* arr_b, float* arr_c, 
                               volatile int outer_iters, volatile int inner_size) {
    volatile int threshold = 1000;
    float accumulator = 0.0f;
    double double_acc = 0.0;
    
    /* Outer loop with carried state */
    for (int outer = 0; outer < outer_iters; outer++) {
        int base = (outer * 37) & 0xFF;  /* Data-dependent base calculation */
        
        /* Inner loop with mixed operations and data-dependent branches */
        for (int i = 0; i < inner_size - 3; i += 4) {
            /* Manually unrolled 4 iterations with different operations */
            
            /* Iteration 1: Integer arithmetic with flow dependency */
            int temp1 = arr_a[i] * base + arr_b[i];
            accumulator += (float)temp1 * 0.5f;
            
            /* Artificial scheduling barrier */
            asm volatile("" ::: "memory");
            
            /* Data-dependent conditional branch */
            if (accumulator > (float)threshold) {
                arr_c[i] = accumulator;
                accumulator = accumulator * 0.25f;  /* Anti-dependency on accumulator */
            }
            
            /* Iteration 2: Different data type (double) */
            double_acc += (double)arr_a[i+1] * 1.5;
            arr_b[i+1] = (int)(double_acc) + base;
            
            /* Resource conflict: multiple FP operations */
            float fp_temp = arr_c[i+1] * 2.0f + accumulator;
            
            /* Complex conditional with output dependency */
            if ((arr_a[i+1] & 1) && (fp_temp < 500.0f)) {
                arr_c[i+1] = fp_temp;
                double_acc = double_acc * 0.9;  /* Output dependency on double_acc */
            }
            
            /* Iteration 3: Mixed integer/float with control dependency */
            int temp3 = arr_a[i+2] - arr_b[i+2];
            if (temp3 > 0) {
                accumulator += (float)temp3 * 3.14f;
                arr_c[i+2] = accumulator;
            } else {
                double_acc -= (double)(-temp3) * 0.1;
            }
            
            /* Scheduling barrier */
            asm volatile("" ::: "memory");
            
            /* Iteration 4: Complex expression with all dependency types */
            int idx = i + 3;
            int old_val = arr_b[idx];  /* Anti-dependency source */
            arr_b[idx] = (arr_a[idx] * 2 + base) & 0x7FF;
            
            /* Flow dependency through accumulator */
            accumulator += (float)(arr_b[idx] - old_val) * 0.01f;
            
            /* Output dependency through arr_c */
            float old_c = arr_c[idx];
            arr_c[idx] = old_c + accumulator;
            
            /* Conditional that depends on multiple computed values */
            if ((arr_b[idx] & 0x3) == 0 || accumulator > 2000.0f) {
                double_acc += (double)old_c * 0.001;
            }
        }
        
        /* Loop-carried dependency to next outer iteration */
        base = (int)(accumulator + double_acc) % 256;
        asm volatile("" ::: "memory");  /* Barrier between outer iterations */
    }
}

/* Second test with volatile counters and more barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
void test_volatile_barriers(int* arr, volatile int size) {
    volatile int v_counter = size;
    int sum = 0;
    float fsum = 0.0f;
    
    while (v_counter > 0) {
        /* Unpredictable loop with volatile condition */
        for (volatile int i = 0; i < 8 && i < v_counter; i++) {
            /* Multiple operations with artificial dependencies */
            int idx = (int)i;
            int val = arr[idx];
            
            /* Complex data-dependent computation */
            if (val & 0x1) {
                sum += val * 3;
                fsum += (float)val * 0.33f;
                asm volatile("" ::: "memory");  /* Barrier in taken branch */
            } else {
                sum -= val / 2;
                fsum -= (float)val * 0.1f;
            }
            
            /* Output dependency */
            arr[idx] = sum & 0xFFF;
            
            /* Another barrier */
            asm volatile("" ::: "memory");
            
            /* Second conditional with different pattern */
            if ((val + idx) % 7 == 0) {
                fsum = fsum * 1.1f - (float)idx;
                asm volatile("" ::: "memory");
            }
        }
        
        v_counter -= 8;
        /* Volatile memory operation to prevent optimization */
        *(volatile int*)&v_counter = v_counter;
    }
}

/* Third test: Outer loop carried state pattern */
void test_outer_carried_state(double* darr, int* iarr, int outer, int inner) {
    double outer_state = 1.0;
    
    for (int j = 0; j < outer; j++) {
        /* Compute base from outer state - creates loop-carried dependency */
        int base = (int)(outer_state * 100.0) % 256;
        double factor = 1.0 + (j % 10) * 0.1;
        
        /* Inner loop with dependency on outer state */
        for (int i = 0; i < inner; i++) {
            /* Mixed operations with flow and anti dependencies */
            double old_d = darr[i];
            int old_i = iarr[i];
            
            /* Anti-dependency on darr[i] */
            darr[i] = (old_d + (double)base) * factor;
            
            /* Flow dependency through iarr[i] */
            iarr[i] = old_i + (int)(darr[i] * 0.01);
            
            /* Control dependency */
            if (iarr[i] % 5 == 0) {
                outer_state += darr[i] * 0.001;
            }
            
            /* Unrolled second iteration */
            if (i + 1 < inner) {
                double temp = darr[i+1] * factor - (double)base;
                darr[i+1] = temp;
                iarr[i+1] ^= (int)temp;  /* XOR creates data dependency */
                
                if ((iarr[i+1] & 0x3) == 0) {
                    asm volatile("" ::: "memory");  /* Barrier */
                }
            }
        }
        
        /* Outer loop update with dependency on inner computations */
        outer_state = outer_state * 0.99 + (double)(j % 100) * 0.01;
    }
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    int array_a[ARRAY_SIZE];
    int array_b[ARRAY_SIZE];
    float array_c[ARRAY_SIZE];
    double array_d[ARRAY_SIZE];
    
    /* Initialize with pseudo-random but bounded values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array_a[i] = (int)(lcg_rand() % 1000);
        array_b[i] = (int)(lcg_rand() % 1000);
        array_c[i] = (float)(lcg_rand() % 100) * 0.1f;
        array_d[i] = (double)(lcg_rand() % 100) * 0.01;
    }
    
    volatile int checksum_a = 0;
    volatile int checksum_b = 0;
    
    /* Runtime-variable loop control */
    volatile int outer_iters = 5;
    volatile int inner_size = ARRAY_SIZE - 16;  /* Ensure bounds */
    
    /* Call test functions multiple times with volatile conditions */
    for (int run = 0; run < 3; run++) {
        /* Volatile flag to prevent optimization */
        volatile int flag = (lcg_rand() % 2);
        
        if (flag || run == 0) {
            test_selective_scheduling(array_a, array_b, array_c, 
                                     outer_iters, inner_size);
        }
        
        if (!flag || run == 1) {
            test_volatile_barriers(array_b, inner_size);
        }
        
        /* Always run third test but with different parameters */
        test_outer_carried_state(array_d, array_a, 
                                (run + 2),  /* Vary outer iterations */
                                inner_size / (run + 1 + 1));
    }
    
    /* Compute checksums to prevent dead code elimination */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum_a += array_a[i] ^ i;
        checksum_b += (int)array_c[i] + array_b[i];
    }
    
    /* Mix in double array checksum */
    int final_checksum = checksum_a + checksum_b;
    for (int i = 0; i < ARRAY_SIZE; i += 8) {
        final_checksum += (int)(array_d[i] * 100.0);
    }
    
    printf("Final checksum: %d\n", final_checksum);
    return final_checksum & 0xFF;
}
