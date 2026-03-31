/* sel-sched-trigger.c
 * Program designed to trigger GCC selective scheduler debugging dumps
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-dump -std=gnu11 sel-sched-trigger.c -o sel-sched-trigger
 */

#include <stdio.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define OUTER_LOOP 8
#define INNER_LOOP 128

/* Simple LCG pseudo-random generator */
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Initialize arrays with pseudo-random data */
static void init_arrays(int32_t *a, int32_t *b, float *fa, double *db) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = (int32_t)(lcg_rand() % 1000) - 500;
        b[i] = (int32_t)(lcg_rand() % 1000) - 500;
        fa[i] = (float)(lcg_rand() % 1000) / 10.0f;
        db[i] = (double)(lcg_rand() % 1000) / 10.0;
    }
}

/* Function 1: Complex nested loop with data-dependent branches
 * Force selective scheduling with attribute */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-dump")))
static void test_function_1(int32_t *a, int32_t *b, float *fa, double *db) {
    volatile int volatile_flag = 1; /* Prevent optimization */
    int32_t sum = 0;
    float fsum = 0.0f;
    double dsum = 0.0;
    
    /* Outer loop with carried state */
    for (int j = 0; j < OUTER_LOOP && volatile_flag; j++) {
        int base = (j * 37) & 0xFF; /* Computation with outer loop index */
        
        /* Inner loop with mixed operations and data-dependent branches */
        for (int i = 0; i < INNER_LOOP; i++) {
            int idx = (j * INNER_LOOP + i) % ARRAY_SIZE;
            
            /* Multiple dependency types: flow, anti, output */
            int32_t temp = a[idx] * b[idx];
            sum = sum + temp;  /* Flow dependency */
            
            /* Data-dependent conditional branch */
            if (sum > 1000) {
                a[idx] = sum;  /* Anti dependency on a[idx] */
                sum = 0;
                /* Inline assembly barrier to create scheduling complexity */
                asm volatile("" ::: "memory");
            }
            
            /* Mixed floating-point operations */
            fsum = fsum + fa[idx] * 1.5f;
            dsum = dsum + db[idx] * 2.5;
            
            /* Another conditional with different data type */
            if (fsum > 500.0f) {
                db[idx] = dsum;  /* Output dependency on db[idx] */
                fsum = fsum * 0.5f;
                asm volatile("" ::: "memory");
            }
            
            /* Manual unrolling (2 iterations) */
            if (i + 1 < INNER_LOOP) {
                int idx2 = (j * INNER_LOOP + i + 1) % ARRAY_SIZE;
                int32_t temp2 = a[idx2] - b[idx2];
                sum = sum + temp2 + base;  /* Use outer loop carried state */
                
                if (temp2 & 1) {  /* Unpredictable branch */
                    b[idx2] = temp2;
                    asm volatile("" ::: "memory");
                }
                i++;  /* Increment for manual unroll */
            }
        }
        
        /* Modify outer loop carried state */
        base = (base * 3) % 256;
    }
}

/* Function 2: Volatile counters and inline assembly barriers */
static void test_function_2(int32_t *a, int32_t *b) {
    volatile int volatile_counter = INNER_LOOP; /* Volatile prevents optimization */
    int32_t acc = 0;
    
    while (volatile_counter > 0) {
        int idx = INNER_LOOP - volatile_counter;
        
        /* Complex dependency chain */
        int32_t x = a[idx];
        int32_t y = b[idx];
        
        /* Multiple operations with anti-dependencies */
        a[idx] = x * y + acc;
        b[idx] = y - x * 2;
        acc = a[idx] + b[idx];
        
        /* Inline assembly creating scheduling barrier */
        asm volatile("" ::: "memory");
        
        /* Data-dependent branch with volatile condition */
        if (acc > (int32_t)(lcg_rand() % 1000)) {
            a[idx] = acc >> 1;
            asm volatile("" ::: "memory");
        }
        
        volatile_counter--;
    }
}

/* Function 3: Outer-loop carried state pattern */
static void test_function_3(float *fa, double *db) {
    float outer_state = 10.0f;
    
    for (int j = 0; j < OUTER_LOOP; j++) {
        /* Compute base from outer loop state */
        float base = outer_state * (j + 1);
        double dbase = (double)base * 1.5;
        
        /* Inner loop using outer loop carried state */
        for (int i = 0; i < INNER_LOOP; i++) {
            int idx = (j * INNER_LOOP + i) % ARRAY_SIZE;
            
            /* Mixed float/double operations with outer state */
            fa[idx] = (fa[idx] + base) * 1.1f;
            db[idx] = db[idx] * 0.9 + dbase;
            
            /* Conditional that depends on outer state */
            if (fa[idx] > base * 2.0f) {
                db[idx] = db[idx] / 2.0;
                asm volatile("" ::: "memory");
            }
            
            /* Small manual unroll */
            if (i % 2 == 0 && i + 1 < INNER_LOOP) {
                int idx2 = (j * INNER_LOOP + i + 1) % ARRAY_SIZE;
                fa[idx2] = fa[idx2] * 0.8f + base;
                i++;
            }
        }
        
        /* Update outer loop carried state */
        outer_state = outer_state * 0.9f + (float)j;
    }
}

/* Compute checksum to prevent dead code elimination */
static int64_t compute_checksum(int32_t *a, int32_t *b, float *fa, double *db) {
    int64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += a[i] + b[i];
        checksum += (int64_t)(fa[i] * 100);
        checksum += (int64_t)(db[i] * 100);
    }
    return checksum;
}

int main(void) {
    /* Declare arrays */
    int32_t array_a[ARRAY_SIZE];
    int32_t array_b[ARRAY_SIZE];
    float array_fa[ARRAY_SIZE];
    double array_db[ARRAY_SIZE];
    
    /* Initialize with pseudo-random data */
    init_arrays(array_a, array_b, array_fa, array_db);
    
    /* Volatile flag for runtime variability */
    volatile int volatile_flag = 1;
    int64_t total_checksum = 0;
    
    /* Call test functions multiple times with runtime decisions */
    for (int iteration = 0; iteration < 3; iteration++) {
        /* Runtime decision based on pseudo-random value */
        uint32_t rand_val = lcg_rand();
        
        if (volatile_flag) {
            /* Force multiple calls to trigger scheduler */
            for (int rep = 0; rep < 2; rep++) {
                test_function_1(array_a, array_b, array_fa, array_db);
            }
        }
        
        if (rand_val % 2 == 0) {
            test_function_2(array_a, array_b);
        }
        
        if (rand_val % 3 != 0) {
            test_function_3(array_fa, array_db);
        }
        
        /* Update volatile flag based on array content */
        volatile_flag = (array_a[0] + array_b[0]) > 0;
    }
    
    /* Compute final checksum */
    total_checksum = compute_checksum(array_a, array_b, array_fa, array_db);
    
    /* Print result to prevent optimization */
    printf("Final checksum: %lld\n", (long long)total_checksum);
    
    return 0;
}
