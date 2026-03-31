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
static uint32_t lcg_seed = 12345;
static inline uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Function 1: Complex nested loops with data-dependent branches
 * Force selective scheduling with attribute */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
void test_function_1(int *a, int *b, float *c, float *d, int size) {
    volatile int threshold = 1000; /* Prevent constant propagation */
    float sum_f = 0.0f;
    double sum_d = 0.0;
    
    /* Outer loop with carried state */
    for (int j = 0; j < OUTER_LOOP; j++) {
        int base = (j * 37) & 0xFF; /* Non-trivial base computation */
        
        /* Manually unrolled inner loop with mixed operations */
        for (int i = 0; i < size - 3; i += 4) {
            /* Create flow dependencies */
            float temp1 = c[i] * 1.5f + base;
            float temp2 = c[i+1] * 2.0f - base;
            
            /* Data-dependent branch with anti-dependencies */
            if ((a[i] & 1) && (sum_f > threshold)) {
                c[i] = sum_f;
                sum_f = 0.0f;
                asm volatile("" ::: "memory"); /* Scheduling barrier */
            }
            
            /* Mixed integer/float operations */
            sum_f += temp1 + temp2;
            sum_d += (double)a[i] * b[i] * 0.01;
            
            /* Second unrolled iteration with different pattern */
            temp1 = c[i+2] * 0.75f + base * 2;
            temp2 = c[i+3] * 1.25f - base / 2;
            
            /* Another conditional with output dependency */
            if ((b[i+1] % 3) == 0) {
                d[i] = sum_f * 0.5f;
                asm volatile("" ::: "memory");
            }
            
            sum_f += temp1 - temp2;
            sum_d += (double)a[i+1] * b[i+1] * 0.02;
            
            /* More operations to increase ILP */
            c[i] = c[i] * 0.9f + sum_f * 0.1f;
            c[i+1] = c[i+1] * 0.8f + sum_f * 0.2f;
            
            /* Resource conflict: multiple FP operations */
            d[i+2] = d[i+2] * 1.1f + sum_d;
            d[i+3] = d[i+3] * 0.9f - sum_d;
        }
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : "+m" (c), "+m" (d));
}

/* Function 2: Volatile counters with inline assembly barriers */
void test_function_2(float *arr1, double *arr2, int size) {
    volatile int counter = size; /* Volatile prevents optimization */
    float acc1 = 0.0f;
    double acc2 = 0.0;
    
    while (counter > 0) {
        int idx = size - counter;
        
        /* Complex dependency chain */
        float t1 = arr1[idx] * 1.234f;
        double t2 = arr2[idx] * 2.345;
        
        asm volatile("" ::: "memory"); /* Force scheduling barrier */
        
        /* Conditional with side effects */
        if (t1 > 500.0f) {
            acc1 += t1 * 0.5f;
            arr1[idx] = acc1;
            asm volatile("" ::: "memory");
        }
        
        /* Nested condition to create control flow complexity */
        if ((idx % 7) == 0) {
            t2 = t2 * 1.5;
            acc2 -= t2;
        } else if ((idx % 5) == 0) {
            t2 = t2 * 0.5;
            acc2 += t2;
        } else {
            acc2 = acc2 * 0.99 + t2;
        }
        
        arr2[idx] = acc2;
        
        /* Multiple memory operations */
        if (idx > 0) {
            arr1[idx-1] = (arr1[idx-1] + arr1[idx]) * 0.5f;
        }
        
        counter--;
    }
}

/* Function 3: Outer-loop carried state pattern */
void test_function_3(int *data, float *output, int size) {
    int state = lcg_rand() % 100;
    
    for (int phase = 0; phase < 4; phase++) {
        int phase_factor = (phase * 29) % 13 + 1;
        
        for (int i = 0; i < size; i++) {
            /* Loop-carried dependency through state */
            int val = data[i] ^ state;
            
            /* Data-dependent operation */
            if (val & 0x1) {
                output[i] = output[i] * 1.1f + val * phase_factor;
            } else {
                output[i] = output[i] * 0.9f - val * phase_factor;
            }
            
            /* Update state based on computation */
            state = (state * 31 + val) & 0xFF;
            
            /* Unrolled additional operations */
            if (i % 2 == 0) {
                output[i] = output[i] + sinf((float)state * 0.01f);
            }
        }
        
        /* Cross-iteration dependency */
        state = (state + phase_factor) & 0x7F;
    }
}

int main(void) {
    /* Initialize arrays with pseudo-random data */
    int array_a[ARRAY_SIZE];
    int array_b[ARRAY_SIZE];
    float array_c[ARRAY_SIZE];
    float array_d[ARRAY_SIZE];
    double array_e[ARRAY_SIZE];
    
    printf("Initializing arrays with pseudo-random data...\n");
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array_a[i] = (int)lcg_rand() % 1000;
        array_b[i] = (int)lcg_rand() % 1000;
        array_c[i] = (float)(lcg_rand() % 1000) * 0.01f;
        array_d[i] = (float)(lcg_rand() % 1000) * 0.02f;
        array_e[i] = (double)(lcg_rand() % 1000) * 0.005;
    }
    
    /* Volatile flag to introduce runtime variability */
    volatile int run_alternate = 0;
    uint32_t checksum = 0;
    
    /* Call test functions multiple times with volatile control */
    for (int iteration = 0; iteration < 5; iteration++) {
        run_alternate = (lcg_rand() & 0x1);
        
        if (run_alternate) {
            for (int rep = 0; rep < 3; rep++) {
                test_function_1(array_a, array_b, array_c, array_d, ARRAY_SIZE);
            }
        } else {
            test_function_2(array_d, array_e, ARRAY_SIZE);
        }
        
        test_function_3(array_a, array_c, ARRAY_SIZE);
    }
    
    /* Compute final checksum to prevent dead code elimination */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += (uint32_t)array_a[i];
        checksum += (uint32_t)array_b[i];
        checksum ^= *(uint32_t*)&array_c[i];
        checksum ^= *(uint32_t*)&array_d[i];
        checksum ^= (uint32_t)array_e[i];
    }
    
    printf("Final checksum: %u\n", checksum);
    printf("Program completed. Check stderr for selective scheduler dumps.\n");
    
    return 0;
}
