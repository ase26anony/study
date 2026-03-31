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

/* Function 1: Complex nested loop with data-dependent branches
 * Force selective scheduling with attribute */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
void test_function_1(int *a, int *b, float *c, float *d, int size) {
    volatile int threshold = 1000; /* Prevent constant propagation */
    float sum_f = 0.0f;
    double sum_d = 0.0;
    
    /* Outer loop with carried state */
    for (int j = 0; j < OUTER_LOOP; j++) {
        int base = (j * 37) & 0xFF; /* Data-dependent base */
        
        /* Inner loop with mixed operations and control flow */
        for (int i = 0; i < size; i++) {
            /* Multiple dependency types */
            int idx = (i + base) % size;
            int val_a = a[idx];
            int val_b = b[idx];
            
            /* Flow dependency chain */
            sum_f = sum_f + val_a * 0.5f;
            
            /* Anti-dependency: read then write */
            float old_c = c[idx];
            
            /* Output dependency */
            c[idx] = sum_f + old_c;
            
            /* Control dependency with branch */
            if (val_b & 1) { /* Unpredictable branch */
                /* Complex expression with multiple operations */
                d[idx] = (d[idx] * 1.1f) + (sum_f * 0.3f);
                sum_d += d[idx];
                
                /* Another conditional inside */
                if (sum_d > threshold) {
                    sum_d *= 0.5;
                    asm volatile("" ::: "memory"); /* Scheduling barrier */
                }
            } else {
                d[idx] = (d[idx] * 0.9f) - (sum_f * 0.2f);
                sum_d -= d[idx];
            }
            
            /* Manual unrolling - 2 iterations */
            if (i + 1 < size) {
                int idx2 = (i + 1 + base) % size;
                float temp = c[idx2] * 0.7f;
                c[idx2] = temp + d[idx] * 0.4f;
                sum_f += temp;
                
                /* Another memory barrier */
                asm volatile("" ::: "memory");
            }
        }
        
        /* Outer loop update with dependency on inner loop */
        threshold += (int)(sum_f * 0.01f);
    }
}

/* Function 2: Volatile counters and assembly barriers */
void test_function_2(double *arr1, double *arr2, int size) {
    volatile int v_counter = size; /* Volatile prevents optimization */
    int seed = 42;
    
    while (v_counter > 0) {
        int i = size - v_counter;
        
        /* Multiple FP operations with potential resource conflicts */
        double x = arr1[i];
        double y = arr2[i];
        
        /* Chain of dependent FP operations */
        x = x * 1.234567 + y * 0.987654;
        y = y * 0.543210 - x * 0.123456;
        
        /* Inline assembly barrier creates scheduling boundary */
        asm volatile("" : "+r"(x), "+r"(y) : : "memory");
        
        /* More operations after barrier */
        x = x * x + y * y;
        y = x * y - y * x;
        
        /* Pseudo-random condition */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        if (seed & 0x100) {
            arr1[i] = x;
            arr2[i] = y;
        } else {
            arr1[i] = y;
            arr2[i] = x;
        }
        
        /* Another barrier */
        asm volatile("" ::: "memory");
        
        v_counter--;
    }
}

/* Function 3: Outer-loop carried state pattern */
void test_function_3(int *data, float *output, int size) {
    int state = 0;
    
    for (int outer = 0; outer < 4; outer++) {
        /* Outer loop modifies state used in inner loop */
        state = (state * 73 + 17) & 0xFF;
        float factor = 1.0f + (state * 0.01f);
        
        /* Inner loop with dependency on outer state */
        for (int i = 0; i < size; i += 4) {
            /* Manual unrolling - 4 iterations */
            for (int u = 0; u < 4 && (i + u) < size; u++) {
                int idx = i + u;
                
                /* Complex expression with outer loop dependency */
                float val = data[idx] * factor;
                
                /* Data-dependent update */
                if (val > 0) {
                    output[idx] = output[idx] + val * (1.0f + u * 0.25f);
                } else {
                    output[idx] = output[idx] - val * (0.5f - u * 0.1f);
                }
                
                /* Update state based on result */
                state ^= (int)output[idx];
            }
            
            /* Dependency between unrolled iterations */
            factor *= 0.99f;
        }
        
        /* Outer loop update with inner loop dependency */
        if (state & 1) {
            asm volatile("" ::: "memory"); /* Force memory barrier */
        }
    }
}

int main(void) {
    /* Initialize arrays with pseudo-random data */
    int array_a[ARRAY_SIZE];
    int array_b[ARRAY_SIZE];
    float array_c[ARRAY_SIZE];
    float array_d[ARRAY_SIZE];
    double array_e[ARRAY_SIZE];
    double array_f[ARRAY_SIZE];
    int array_g[ARRAY_SIZE];
    float array_h[ARRAY_SIZE];
    
    printf("Initializing arrays...\n");
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array_a[i] = (int)lcg_rand() % 1000;
        array_b[i] = (int)lcg_rand() % 1000;
        array_c[i] = (float)(lcg_rand() % 100) * 0.1f;
        array_d[i] = (float)(lcg_rand() % 100) * 0.2f;
        array_e[i] = (double)(lcg_rand() % 100) * 0.3;
        array_f[i] = (double)(lcg_rand() % 100) * 0.4;
        array_g[i] = (int)lcg_rand() % 500;
        array_h[i] = (float)(lcg_rand() % 50) * 0.5f;
    }
    
    /* Volatile flag to prevent optimization of control flow */
    volatile int run_flag = 1;
    
    /* Call test functions multiple times with runtime decisions */
    for (int iteration = 0; iteration < 3; iteration++) {
        if (run_flag) {
            /* Force selective scheduler activation */
            test_function_1(array_a, array_b, array_c, array_d, ARRAY_SIZE);
        }
        
        /* Compute simple checksum to prevent dead code elimination */
        int checksum = 0;
        for (int i = 0; i < ARRAY_SIZE; i += 64) {
            checksum += array_a[i] + (int)array_c[i];
        }
        
        /* Use checksum to decide next step */
        if (checksum & 1) {
            test_function_2(array_e, array_f, ARRAY_SIZE);
        } else {
            test_function_3(array_g, array_h, ARRAY_SIZE);
        }
        
        /* Alternate between patterns */
        if (iteration & 1) {
            for (int rep = 0; rep < 2; rep++) {
                test_function_1(array_a, array_b, array_c, array_d, ARRAY_SIZE / 2);
            }
        }
    }
    
    /* Final checksum computation and output */
    uint64_t final_checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_checksum += array_a[i];
        final_checksum += (uint64_t)(array_c[i] * 100);
        final_checksum += (uint64_t)(array_e[i] * 1000);
        final_checksum += array_g[i];
    }
    
    printf("Final checksum: %lu\n", (unsigned long)final_checksum);
    printf("Selective scheduler test completed.\n");
    
    return 0;
}
