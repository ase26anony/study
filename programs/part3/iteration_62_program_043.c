/* sel-sched-trigger.c
 * Program to trigger GCC selective scheduler debugging dumps
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
__attribute__((optimize("O2", "fsel-sched-pipelining")))
void test_selective_sched_1(int* arr_a, int* arr_b, float* arr_c, int size, volatile int* control) {
    volatile int threshold = *control;
    float sum_f = 0.0f;
    double acc_d = 0.0;
    int sum_i = 0;
    
    /* Nested loops with data-dependent branches and mixed operations */
    for (int outer = 0; outer < 4; outer++) {
        int base = (outer * 256) & 0xFF;
        
        /* Inner loop with complex dependency pattern */
        for (int i = 0; i < size; i++) {
            /* Multiple dependency types with inline assembly barriers */
            int val_a = arr_a[i];
            int val_b = arr_b[i];
            
            /* Flow dependency chain */
            sum_i = sum_i + val_a * val_b;
            
            /* Anti dependency */
            arr_a[i] = sum_i ^ base;
            
            /* Output dependency with control flow */
            if (sum_i > threshold) {
                arr_c[i] = sum_f + (float)sum_i * 0.5f;
                sum_f = arr_c[i];
                asm volatile("" ::: "memory");  /* Scheduling barrier */
            } else {
                arr_c[i] = sum_f - (float)val_b;
                asm volatile("" ::: "memory");
            }
            
            /* Mixed floating point operations creating resource pressure */
            acc_d = acc_d + (double)val_a * 0.25 + (double)val_b * 0.75;
            
            /* Data-dependent branch with unpredictable pattern */
            if (val_a & (1 << (i % 8))) {
                sum_i = sum_i >> 1;
                acc_d = acc_d * 0.99;
            }
            
            /* Manual unrolling (2 iterations) */
            if (i + 1 < size) {
                int next_a = arr_a[i + 1];
                int next_b = arr_b[i + 1];
                sum_i = sum_i - (next_a ^ next_b);
                arr_b[i + 1] = sum_i & 0xFFFF;
                
                /* Another control dependency */
                if (next_a > next_b) {
                    arr_c[i + 1] = (float)sum_i * 0.1f;
                    asm volatile("" ::: "memory");
                }
                i++;  /* Advance counter for unrolled iteration */
            }
        }
        
        /* Loop-carried state modification */
        threshold = (threshold + base) & 0x3FF;
        *control = threshold;  /* Write back to volatile */
    }
}

/* Second test with volatile counters and assembly barriers */
void test_selective_sched_2(double* arr_d, int* arr_i, int size) {
    volatile int v_counter = size;
    volatile float v_factor = 2.5f;
    
    for (volatile int j = 0; j < 8; j++) {
        double local_acc = 0.0;
        int base_mod = (j * 17) % 31;
        
        /* Inner loop with volatile reads and scheduling barriers */
        for (int i = 0; i < v_counter; i++) {
            /* Read-Modify-Write with volatile intermediate */
            double temp = arr_d[i];
            temp = temp * v_factor + (double)base_mod;
            
            /* Inline assembly creating serialization */
            asm volatile("" : "+r" (i) : : "memory");
            
            arr_d[i] = temp;
            arr_i[i] = arr_i[i] + (int)temp;
            
            /* Complex conditional with side effects */
            if ((arr_i[i] & 0xF) == (i & 0xF)) {
                arr_d[i] = arr_d[i] * 0.5;
                asm volatile("" ::: "memory");
            }
            
            /* Dependency chain with multiple uses */
            local_acc = local_acc + arr_d[i];
            if (local_acc > 1000.0) {
                local_acc = local_acc * 0.9;
                v_factor = v_factor * 0.95f;  /* Modify volatile */
            }
        }
        
        /* Cross-iteration dependency */
        v_counter = v_counter - (j + 1);
        if (v_counter < size / 2) {
            v_counter = size;
        }
    }
}

/* Third test: Outer-loop carried state pattern */
void test_outer_loop_carried(float* arr_f, int* arr_mask, int size) {
    int outer_state = 0;
    
    for (int outer = 0; outer < 16; outer++) {
        /* Compute state that affects inner loop */
        int base = (outer_state * 13 + outer * 7) % 256;
        float factor = 1.0f + (outer % 4) * 0.25f;
        
        /* Inner loop using outer state */
        for (int i = 0; i < size; i++) {
            /* Multiple operation types */
            float val = arr_f[i];
            
            /* Loop-carried dependency through outer_state */
            val = val * factor + (float)base;
            
            /* Control dependent on array content */
            if (arr_mask[i] & (1 << (outer % 8))) {
                val = val * 2.0f - (float)(i % 16);
                outer_state = outer_state + (i & 0x3);
            } else {
                val = val * 0.5f + (float)(base % 32);
                asm volatile("" ::: "memory");
            }
            
            arr_f[i] = val;
            
            /* Update mask based on computed value */
            arr_mask[i] = arr_mask[i] ^ ((int)val & 0xFF);
            
            /* Periodic scheduling barrier */
            if ((i % 16) == 0) {
                asm volatile("" ::: "memory");
            }
        }
        
        /* Modify outer state for next iteration */
        outer_state = (outer_state + base) & 0xFFF;
    }
}

int main(void) {
    const int SIZE = 1024;
    int array_a[SIZE];
    int array_b[SIZE];
    float array_c[SIZE];
    double array_d[SIZE];
    int array_mask[SIZE];
    
    volatile int control_flag = 500;
    volatile int checksum = 0;
    
    /* Initialize with pseudo-random but non-uniform data */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = lcg_rand() % 1000;
        array_b[i] = lcg_rand() % 1000;
        array_c[i] = (float)(lcg_rand() % 100) * 0.1f;
        array_d[i] = (double)(lcg_rand() % 100) * 0.01;
        array_mask[i] = lcg_rand() & 0xFF;
    }
    
    /* Runtime decision making with volatile */
    volatile int mode = array_a[0] % 3;
    
    /* Call test functions based on runtime condition */
    if (mode == 0) {
        for (int rep = 0; rep < 5; rep++) {
            test_selective_sched_1(array_a, array_b, array_c, SIZE, &control_flag);
        }
    } else if (mode == 1) {
        for (int rep = 0; rep < 3; rep++) {
            test_selective_sched_2(array_d, array_b, SIZE);
            test_outer_loop_carried(array_c, array_mask, SIZE);
        }
    } else {
        for (int rep = 0; rep < 4; rep++) {
            test_selective_sched_1(array_a, array_b, array_c, SIZE / 2, &control_flag);
            test_selective_sched_2(array_d, array_mask, SIZE);
        }
    }
    
    /* Additional calls to ensure all paths are exercised */
    test_outer_loop_carried(array_c, array_mask, SIZE);
    
    /* Compute final checksum to prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        checksum = checksum + array_a[i] + (int)array_c[i] + (int)array_d[i];
        checksum = checksum ^ array_b[i] ^ array_mask[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Control flag: %d\n", control_flag);
    
    return 0;
}
