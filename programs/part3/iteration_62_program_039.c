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
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Initialize arrays with non-uniform data */
static void init_arrays(int32_t *a, int32_t *b, float *fa, double *da) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = (int32_t)(lcg_rand() % 1000) - 500;
        b[i] = (int32_t)(lcg_rand() % 1000) - 500;
        fa[i] = (float)(lcg_rand() % 1000) / 100.0f;
        da[i] = (double)(lcg_rand() % 1000) / 100.0;
    }
}

/* Function 1: Complex nested loops with data-dependent branches
 * Force selective scheduling with attribute */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
static int32_t test_function_1(int32_t *a, int32_t *b, float *fa, double *da) {
    volatile int32_t threshold = 1000;  /* Prevent constant propagation */
    int32_t sum = 0;
    double dsum = 0.0;
    
    /* Outer loop with carried state */
    for (int j = 0; j < OUTER_LOOP; j++) {
        int32_t base = (j * 37) & 0xFF;  /* Compute varying base */
        
        /* Inner loop with mixed operations and data-dependent branches */
        for (int i = 0; i < INNER_LOOP; i++) {
            int idx = (j * INNER_LOOP + i) % ARRAY_SIZE;
            
            /* Multiple dependency types: flow, anti, output */
            int32_t temp = a[idx] * b[idx];
            sum = sum + temp;  /* Flow dependency */
            
            /* Data-dependent branch with unpredictable pattern */
            if (sum > threshold) {
                a[idx] = sum;  /* Anti dependency on a[idx] */
                sum = 0;
                /* Inline asm barrier to prevent reordering */
                asm volatile("" ::: "memory");
            }
            
            /* Mixed floating-point operations */
            float ftemp = fa[idx] * 1.5f;
            dsum += (double)ftemp + da[idx];
            
            /* Manual unrolling (2 iterations) */
            if (i + 1 < INNER_LOOP) {
                int idx2 = (j * INNER_LOOP + i + 1) % ARRAY_SIZE;
                int32_t temp2 = a[idx2] + base;  /* Use outer loop carried state */
                b[idx2] = temp2 * 3;
                
                /* Another conditional with bitwise test */
                if (temp2 & 1) {
                    fa[idx2] = (float)temp2 / 2.0f;
                    /* Another memory barrier */
                    asm volatile("" ::: "memory");
                }
                i++;  /* Increment for manual unroll */
            }
        }
        
        /* Modify threshold based on outer loop state */
        threshold += base * 2;
    }
    
    return (int32_t)(sum + dsum);
}

/* Function 2: Volatile counters and inline assembly barriers */
static int32_t test_function_2(int32_t *a, int32_t *b) {
    volatile int32_t v_counter = 0;  /* Volatile prevents optimization */
    int32_t result = 0;
    
    for (volatile int i = 0; i < ARRAY_SIZE / 2; i++) {
        /* Complex addressing with volatile */
        int idx1 = v_counter % ARRAY_SIZE;
        int idx2 = (v_counter * 7) % ARRAY_SIZE;
        
        /* Multiple operations with artificial dependencies */
        int32_t t1 = a[idx1] * 3;
        int32_t t2 = b[idx2] * 5;
        
        /* Inline asm barrier between dependent operations */
        asm volatile("" ::: "memory");
        
        result += t1 - t2;
        
        /* Data-dependent array update */
        if (result > 0) {
            a[idx1] = result & 0xFF;
            /* Another barrier */
            asm volatile("" ::: "memory");
        }
        
        /* More mixed operations */
        float f1 = (float)t1 / 2.0f;
        double d1 = (double)t2 * 1.5;
        result += (int32_t)(f1 + d1);
        
        v_counter = (v_counter + 1) & 0x3FF;
        
        /* Unpredictable branch */
        if ((v_counter & 0xF) == 0) {
            b[idx2] = result;
            asm volatile("" ::: "memory");
        }
    }
    
    return result;
}

/* Function 3: Outer-loop carried state pattern */
static double test_function_3(float *fa, double *da) {
    double outer_state = 1.0;
    double total = 0.0;
    
    for (int j = 0; j < OUTER_LOOP * 2; j++) {
        /* Compute base from outer loop state */
        double base = outer_state * (j + 1) * 0.5;
        
        /* Inner loop with outer-loop carried dependency */
        for (int i = 0; i < INNER_LOOP; i++) {
            int idx = (j * INNER_LOOP + i) % ARRAY_SIZE;
            
            /* Complex calculation with outer state */
            double val = (double)fa[idx] * base + da[idx];
            
            /* Conditional update with multiple operations */
            if (val > 0.0) {
                da[idx] = val * 0.9;
                total += val;
                
                /* Manual unrolling */
                if (i % 3 == 0 && i + 1 < INNER_LOOP) {
                    int idx2 = (j * INNER_LOOP + i + 1) % ARRAY_SIZE;
                    double val2 = (double)fa[idx2] * base * 0.5;
                    fa[idx2] = (float)val2;
                    total -= val2;
                    i++;
                }
            }
        }
        
        /* Update outer state for next iteration */
        outer_state = total * 0.01 + 1.0;
        
        /* Memory barrier between outer loop iterations */
        asm volatile("" ::: "memory");
    }
    
    return total;
}

int main(void) {
    /* Declare arrays */
    int32_t array_a[ARRAY_SIZE];
    int32_t array_b[ARRAY_SIZE];
    float array_fa[ARRAY_SIZE];
    double array_da[ARRAY_SIZE];
    
    /* Initialize with pseudo-random data */
    init_arrays(array_a, array_b, array_fa, array_da);
    
    /* Volatile flag to introduce runtime variability */
    volatile int volatile_flag = (array_a[0] > 0) ? 1 : 0;
    
    int32_t checksum1 = 0;
    double checksum3 = 0.0;
    
    /* Call test functions based on volatile condition */
    if (volatile_flag) {
        for (int rep = 0; rep < 10; rep++) {
            checksum1 += test_function_1(array_a, array_b, array_fa, array_da);
        }
    }
    
    /* Always call function 2 */
    int32_t checksum2 = test_function_2(array_a, array_b);
    
    /* Call function 3 multiple times */
    for (int rep = 0; rep < 5; rep++) {
        checksum3 += test_function_3(array_fa, array_da);
    }
    
    /* Compute final aggregate checksum */
    int64_t final_checksum = (int64_t)checksum1 + (int64_t)checksum2 + (int64_t)checksum3;
    
    /* Use results to prevent dead code elimination */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_checksum += array_a[i] + array_b[i];
    }
    
    printf("Final checksum: %ld\n", (long)final_checksum);
    
    return 0;
}
