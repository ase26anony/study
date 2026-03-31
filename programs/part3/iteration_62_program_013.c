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
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-dump")))
void test_function_1(int *arr_a, int *arr_b, float *arr_c, int size) {
    volatile int threshold = 1000000; /* volatile to prevent constant propagation */
    int sum = 0;
    float fsum = 0.0f;
    
    /* Outer loop with carried state */
    for (int j = 0; j < OUTER_LOOP; j++) {
        int base = (j * 37) & 0xFF; /* Data-dependent base calculation */
        
        /* Inner loop with mixed operations and data-dependent branches */
        for (int i = 0; i < size; i++) {
            /* Multiple dependency types: flow, anti, output */
            int temp = arr_a[i] + base;
            
            /* Data-dependent conditional branch - unpredictable */
            if (temp & 1) {
                /* Branch taken path: floating point operations */
                fsum = fsum + arr_c[i] * 1.5f;
                arr_b[i] = temp * 3;
            } else {
                /* Branch not taken path: integer operations */
                sum = sum + arr_a[i] * arr_b[i];
                arr_b[i] = temp / 2;
            }
            
            /* Anti-dependency: reading arr_a after potential modification */
            arr_a[i] = (arr_a[i] + arr_b[i]) & 0xFFFF;
            
            /* Complex condition with floating point */
            if (fsum > (float)threshold) {
                arr_c[i] = fsum;
                fsum = 0.0f;
            }
            
            /* Manual unrolling for more scheduling opportunities */
            if (i + 1 < size) {
                int temp2 = arr_a[i+1] ^ base;
                arr_b[i+1] = (arr_b[i+1] + temp2) * 2;
                sum += temp2;
            }
        }
        
        /* Loop-carried dependency across outer iterations */
        base = (base + sum) & 0xFF;
        
        /* Inline assembly barrier to create scheduling boundary */
        asm volatile("" ::: "memory");
    }
}

/* Function 2: Volatile counters and assembly barriers */
void test_function_2(double *arr_d, int *arr_e, int size) {
    volatile int v_counter = size; /* volatile prevents optimization */
    double accumulator = 0.0;
    
    for (volatile int v = 0; v < v_counter; v = v + 1) {
        int idx = v % size;
        
        /* Mixed floating point operations creating resource conflicts */
        double d1 = arr_d[idx] * 1.234567;
        double d2 = arr_d[(idx + 1) % size] * 0.987654;
        
        /* Output dependency chain */
        arr_d[idx] = d1 + d2;
        accumulator += arr_d[idx];
        
        /* Integer operations interleaved */
        arr_e[idx] = arr_e[idx] + (int)(d1 * 1000.0);
        
        /* Assembly barrier every 4 iterations */
        if ((v & 3) == 0) {
            asm volatile("" ::: "memory");
        }
        
        /* Data-dependent array access */
        int next_idx = arr_e[idx] % size;
        if (next_idx >= 0 && next_idx < size) {
            arr_d[next_idx] = arr_d[next_idx] * 0.5;
        }
    }
}

/* Function 3: Outer-loop carried state pattern */
void test_function_3(int *arr_f, int *arr_g, int size) {
    int outer_state = 0;
    
    for (int outer = 0; outer < 16; outer++) {
        /* Compute state carried from outer loop */
        int base = (outer_state * 17 + outer * 13) & 0xFFF;
        outer_state = base;
        
        /* Inner loop using outer state */
        for (int inner = 0; inner < size; inner += 4) {
            /* Manually unrolled for ILP */
            for (int u = 0; u < 4 && (inner + u) < size; u++) {
                int idx = inner + u;
                
                /* Complex expression with multiple dependencies */
                int val = arr_f[idx] * base + arr_g[idx];
                
                /* Conditional with side effects */
                if (val > 1000) {
                    arr_g[idx] = val / 3;
                    base = (base + 1) & 0xFFF;
                } else {
                    arr_f[idx] = val * 2;
                }
                
                /* Cross-iteration dependency in unrolled loop */
                if (u > 0) {
                    arr_f[idx] += arr_f[idx-1] & 0xF;
                }
            }
            
            /* Dependency across unrolled chunks */
            if (inner > 0) {
                arr_g[inner] ^= arr_f[inner-1];
            }
        }
        
        /* Volatile write to prevent loop elimination */
        volatile int *dummy = &outer_state;
        *dummy = outer_state;
    }
}

int main(void) {
    /* Initialize arrays with pseudo-random data */
    int array_a[ARRAY_SIZE];
    int array_b[ARRAY_SIZE];
    float array_c[ARRAY_SIZE];
    double array_d[ARRAY_SIZE];
    int array_e[ARRAY_SIZE];
    int array_f[ARRAY_SIZE];
    int array_g[ARRAY_SIZE];
    
    printf("Initializing arrays with pseudo-random data...\n");
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        uint32_t r = lcg_rand();
        array_a[i] = (int)(r & 0xFFFF);
        array_b[i] = (int)((r >> 16) & 0xFFFF);
        array_c[i] = (float)(r % 1000) / 10.0f;
        array_d[i] = (double)(r % 2000) / 20.0;
        array_e[i] = (int)(r & 0xFFF);
        array_f[i] = (int)(r & 0x7FF);
        array_g[i] = (int)((r >> 8) & 0x7FF);
    }
    
    /* Volatile flag to introduce runtime variability */
    volatile int run_alternate = 0;
    int checksum = 0;
    
    /* Call test functions multiple times with runtime decisions */
    for (int iteration = 0; iteration < 5; iteration++) {
        run_alternate = (lcg_rand() & 1);
        
        if (run_alternate) {
            /* Execute function 1 multiple times */
            for (int rep = 0; rep < 3; rep++) {
                test_function_1(array_a, array_b, array_c, ARRAY_SIZE);
            }
        } else {
            /* Execute functions 2 and 3 */
            test_function_2(array_d, array_e, ARRAY_SIZE);
            test_function_3(array_f, array_g, ARRAY_SIZE);
        }
        
        /* Mix both paths occasionally */
        if (iteration == 2) {
            test_function_1(array_a, array_b, array_c, ARRAY_SIZE);
            test_function_2(array_d, array_e, ARRAY_SIZE);
        }
    }
    
    /* Compute final checksum to prevent dead code elimination */
    printf("Computing checksum...\n");
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += array_a[i] + array_b[i] + (int)array_c[i];
        checksum += (int)array_d[i] + array_e[i];
        checksum += array_f[i] + array_g[i];
        
        /* Prevent simple accumulation optimization */
        checksum = checksum & 0xFFFFFF;
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Selective scheduler test completed.\n");
    
    return 0;
}
