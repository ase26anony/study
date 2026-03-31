/* sel-sched-trigger.c
 * Designed to trigger GCC selective scheduler debugging dumps
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-dump -std=gnu11 sel-sched-trigger.c -o sel-sched-test
 */

#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 12345;
static inline uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Test function 1: Complex nested loops with data-dependent branches */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-dump")))
void test_complex_schedule(int* restrict a, int* restrict b, float* restrict c, 
                          int size, volatile int threshold) {
    float sum = 0.0f;
    float factor = 1.5f;
    
    /* Outer loop with carried state */
    for (int j = 0; j < 4; j++) {
        int base = (j * 17) & 0xFF;  /* Non-trivial base computation */
        
        /* Inner loop with mixed operations and data-dependent branches */
        for (int i = 0; i < size; i++) {
            /* Create flow dependency */
            sum = sum + a[i] * b[i];
            
            /* Control dependency with unpredictable branch */
            if (sum > threshold) {
                c[i] = sum * factor;
                sum = 0.0f;
                /* Memory barrier to prevent reordering */
                asm volatile("" ::: "memory");
            } else {
                c[i] = (a[i] + b[i]) * 0.5f;
            }
            
            /* Anti-dependency: read after write to same location */
            float temp = c[i];
            a[i] = (int)(temp * 100.0f);
            
            /* Output dependency through sum variable */
            sum = sum + temp * 0.1f;
            
            /* Manual unrolling for more scheduling complexity */
            if (i + 1 < size) {
                sum = sum + a[i+1] * b[i+1] * 0.3f;
                if ((a[i+1] ^ b[i+1]) & 1) {  /* Data-dependent condition */
                    c[i+1] = sum * 2.0f;
                }
                i++;  /* Skip next iteration */
            }
        }
        
        /* Outer loop modifies inner loop's base */
        factor = factor * (1.0f + (base % 10) * 0.01f);
    }
}

/* Test function 2: Volatile counters and assembly barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining-outer-loops")))
void test_volatile_barriers(double* restrict arr1, double* restrict arr2, 
                           int size, volatile int* restrict counter) {
    volatile int v_limit = size;
    double acc1 = 0.0, acc2 = 0.0, acc3 = 0.0;
    
    for (volatile int v_i = 0; v_i < v_limit; v_i++) {
        int i = v_i;  /* Convert to non-volatile for computation */
        
        /* Multiple accumulators with different operations */
        acc1 = acc1 + arr1[i] * arr2[i];
        acc2 = acc2 - arr1[i] / (arr2[i] + 1.0);
        acc3 = acc3 * 0.99 + arr1[i] * 0.01;
        
        /* Assembly barrier creating scheduling boundary */
        asm volatile("" : "+r"(i) : : "memory");
        
        /* Conditional store with anti-dependency */
        if (acc1 > acc2) {
            arr1[i] = acc1;
            /* Another barrier inside branch */
            asm volatile("" ::: "memory");
        } else {
            arr2[i] = acc2;
        }
        
        /* Complex condition with multiple dependencies */
        if ((i & 3) == 0 && acc3 > 0.0) {
            *counter = *counter + 1;
            acc1 = acc1 * 0.5;
        }
        
        /* Manual unrolling with different operations */
        if (i + 3 < size) {
            double t1 = arr1[i+1] + arr2[i+1];
            double t2 = arr1[i+2] - arr2[i+2];
            double t3 = arr1[i+3] * arr2[i+3];
            
            acc1 += t1 * t2;
            acc2 -= t3 / (t1 + 1.0);
            acc3 = (acc3 + t1 + t2 + t3) * 0.333;
            
            /* Barrier between unrolled iterations */
            asm volatile("" ::: "memory");
        }
    }
}

/* Test function 3: Outer-loop carried state pattern */
void test_outer_carried_state(int* restrict data, int rows, int cols, 
                             volatile int init_val) {
    int state = init_val;
    
    for (int r = 0; r < rows; r++) {
        int row_base = (state * r) & 0xFF;
        int local_factor = (row_base % 16) + 1;
        
        /* Inner loop with dependency on outer loop state */
        for (int c = 0; c < cols; c++) {
            int idx = r * cols + c;
            
            /* Multiple dependency types */
            int old_val = data[idx];  /* Read creates anti-dependency */
            int computed = (old_val * local_factor + row_base) ^ state;
            
            /* Data-dependent branch */
            if (computed & 1) {
                data[idx] = computed * 3;
                state = (state + 1) & 0x7F;
            } else {
                data[idx] = computed / 2;
                state = (state - 1) & 0x7F;
            }
            
            /* Output dependency through state variable */
            local_factor = (local_factor + (state % 7)) & 0xF;
            
            /* Partial unrolling */
            if (c + 2 < cols) {
                int idx1 = idx + 1;
                int idx2 = idx + 2;
                
                data[idx1] = (data[idx1] + state) * local_factor;
                data[idx2] = (data[idx2] ^ state) - local_factor;
                
                c += 2;  /* Skip two iterations */
            }
        }
        
        /* Outer loop modifies carried state */
        state = (state * 13 + r) & 0xFF;
    }
}

int main(void) {
    const int SIZE = 1024;
    const int ROWS = 32;
    const int COLS = 32;
    
    /* Initialize arrays with non-uniform data */
    int array_a[SIZE];
    int array_b[SIZE];
    float array_c[SIZE];
    double array_d1[SIZE];
    double array_d2[SIZE];
    int matrix[ROWS * COLS];
    
    /* Fill with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (int)(lcg_rand() % 1000);
        array_b[i] = (int)(lcg_rand() % 1000);
        array_c[i] = (float)(lcg_rand() % 1000) * 0.001f;
        array_d1[i] = (double)(lcg_rand() % 1000) * 0.001;
        array_d2[i] = (double)(lcg_rand() % 1000) * 0.001;
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        matrix[i] = lcg_rand() % 1000;
    }
    
    volatile int threshold = 500;
    volatile int counter = 0;
    volatile int init_val = 42;
    
    /* Runtime decision making with volatile flags */
    volatile int flag1 = (array_a[0] > 400);
    volatile int flag2 = (array_b[0] < 600);
    volatile int flag3 = (array_c[0] > 0.5f);
    
    /* Call test functions multiple times based on runtime conditions */
    if (flag1) {
        for (int rep = 0; rep < 5; rep++) {
            test_complex_schedule(array_a, array_b, array_c, SIZE, threshold);
            threshold += 50;  /* Change threshold each iteration */
        }
    }
    
    if (flag2) {
        for (int rep = 0; rep < 3; rep++) {
            test_volatile_barriers(array_d1, array_d2, SIZE, &counter);
            counter = counter + rep;  /* Modify counter */
        }
    }
    
    if (flag3) {
        for (int rep = 0; rep < 2; rep++) {
            test_outer_carried_state(matrix, ROWS, COLS, init_val + rep);
        }
    }
    
    /* Alternate execution pattern */
    for (int alt = 0; alt < 4; alt++) {
        if (alt & 1) {
            test_complex_schedule(array_a, array_b, array_c, SIZE / 2, threshold);
        } else {
            test_volatile_barriers(array_d1, array_d2, SIZE / 2, &counter);
        }
    }
    
    /* Compute final checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += (uint64_t)array_a[i];
        checksum += (uint64_t)array_b[i];
        checksum += (uint64_t)(array_c[i] * 1000.0f);
        checksum += (uint64_t)(array_d1[i] * 1000.0);
        checksum += (uint64_t)(array_d2[i] * 1000.0);
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        checksum += (uint64_t)matrix[i];
    }
    
    checksum += (uint64_t)counter;
    
    printf("Final checksum: %lu\n", (unsigned long)checksum);
    return 0;
}
