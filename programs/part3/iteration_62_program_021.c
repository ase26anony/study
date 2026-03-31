/* sel-sched-trigger.c - Program to trigger selective scheduler debugging dumps */
#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 12345;
static inline uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Force selective scheduling with optimization attributes */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-dump")))
void test_selective_scheduling_1(int* arr_a, int* arr_b, float* arr_c, int size) {
    volatile int threshold = 1000; /* Prevent constant propagation */
    int sum = 0;
    float fsum = 0.0f;
    
    /* Nested loops with data-dependent branches */
    for (int outer = 0; outer < 4; outer++) {
        int base = (outer * 17) & 0xFF; /* Outer loop carried state */
        
        for (int i = 0; i < size; i++) {
            /* Mixed data types and operations */
            int val_a = arr_a[i];
            int val_b = arr_b[i];
            
            /* Flow dependency chain */
            sum = sum + val_a * val_b;
            
            /* Anti-dependency: sum used then modified */
            if (sum > threshold) {
                arr_c[i] = (float)sum * 0.5f;
                sum = 0; /* Output dependency on sum */
            }
            
            /* Floating point operations with control dependency */
            fsum = fsum + arr_c[i];
            if (fsum > 500.0f) {
                fsum = fsum * 0.9f;
                /* Inline asm barrier to complicate scheduling */
                asm volatile("" ::: "memory");
            }
            
            /* Data-dependent branch with unpredictable pattern */
            if (val_a & (1 << (i % 8))) {
                arr_b[i] = val_b + base; /* Uses outer loop carried state */
            }
        }
    }
}

/* Function with volatile counters and manual unrolling */
__attribute__((optimize("O3", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
void test_selective_scheduling_2(double* arr_d, int* arr_e, int size) {
    volatile int v_counter = size; /* Volatile prevents optimization */
    double accum[4] = {0.0, 0.0, 0.0, 0.0};
    
    while (v_counter > 0) {
        int idx = size - v_counter;
        
        /* Manually unrolled loop with mixed operations */
        if (idx + 3 < size) {
            /* Unrolled iteration 0 */
            double temp0 = arr_d[idx] * 1.1;
            accum[0] += temp0;
            if (arr_e[idx] & 1) accum[0] *= 0.99;
            
            /* Unrolled iteration 1 */
            double temp1 = arr_d[idx+1] * 1.2;
            accum[1] += temp1;
            arr_e[idx+1] = (int)(accum[1] * 0.5);
            asm volatile("" ::: "memory"); /* Scheduling barrier */
            
            /* Unrolled iteration 2 */
            double temp2 = arr_d[idx+2] * 1.3;
            accum[2] += temp2;
            if (accum[2] > 1000.0) accum[2] -= 500.0;
            
            /* Unrolled iteration 3 */
            double temp3 = arr_d[idx+3] * 1.4;
            accum[3] += temp3;
            arr_e[idx+3] ^= (int)temp3; /* Bitwise operation */
        }
        
        v_counter -= 4;
        
        /* Complex conditional with volatile read */
        if (v_counter < (size / 2)) {
            asm volatile("" ::: "memory");
            /* Cross-iteration dependency */
            accum[0] = accum[0] + accum[1] * 0.1;
        }
    }
}

/* Outer loop carried state pattern */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
void test_selective_scheduling_3(int* arr, int size) {
    const int OUTER = 8;
    const int INNER = size / OUTER;
    
    for (int j = 0; j < OUTER; j++) {
        /* Outer loop modifies state used in inner loop */
        int base = (j * 73) % 256;
        int factor = 1 + (j % 3);
        
        for (int i = 0; i < INNER; i++) {
            int idx = j * INNER + i;
            if (idx >= size) break;
            
            /* Loop-carried dependency chain */
            int old = arr[idx];
            arr[idx] = (old + base) * factor;
            
            /* Data-dependent branch with irregular pattern */
            if ((old ^ base) & 0x0F) {
                arr[idx] += (i % 16);
                asm volatile("" ::: "memory");
            }
            
            /* Additional dependency through array */
            if (i > 0) {
                arr[idx] += arr[idx-1] & 0xFF;
            }
        }
        
        /* Inter-outer-loop dependency */
        if (j > 0) {
            asm volatile("" ::: "memory");
        }
    }
}

int main(void) {
    const int SIZE = 1024;
    int array_a[SIZE];
    int array_b[SIZE];
    float array_c[SIZE];
    double array_d[SIZE];
    int array_e[SIZE];
    
    /* Initialize with pseudo-random but bounded values */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (int)(lcg_rand() % 1000);
        array_b[i] = (int)(lcg_rand() % 1000);
        array_c[i] = (float)(lcg_rand() % 100) * 0.1f;
        array_d[i] = (double)(lcg_rand() % 1000) * 0.01;
        array_e[i] = (int)(lcg_rand() % 10000);
    }
    
    volatile int run_flag = 1; /* Prevent compile-time elimination */
    long long checksum = 0;
    
    /* Variable control flow to force runtime execution */
    if (run_flag) {
        for (int rep = 0; rep < 5; rep++) {
            test_selective_scheduling_1(array_a, array_b, array_c, SIZE);
        }
    }
    
    /* Alternate execution path */
    volatile int alt_flag = (array_a[0] > 500);
    if (alt_flag) {
        for (int rep = 0; rep < 3; rep++) {
            test_selective_scheduling_2(array_d, array_e, SIZE);
        }
    }
    
    /* Always execute third test */
    test_selective_scheduling_3(array_a, SIZE);
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        checksum += array_a[i] + array_b[i] + (int)array_c[i] 
                  + (int)array_d[i] + array_e[i];
    }
    
    /* Use checksum in output */
    printf("Final checksum: %lld\n", checksum);
    
    return 0;
}
