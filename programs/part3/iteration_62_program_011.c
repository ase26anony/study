/* sel_sched_trigger.c - Program to trigger GCC selective scheduler debugging dumps */
#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 12345;
static inline uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Force selective scheduler with optimization attribute */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
void test_selective_scheduler_1(int *arr_a, int *arr_b, float *arr_c, int size) {
    volatile int threshold = 1000; /* Prevent constant propagation */
    int sum = 0;
    float fsum = 0.0f;
    
    /* Nested loops with data-dependent branches and mixed operations */
    for (int outer = 0; outer < 4; outer++) {
        int base = (outer * 17) & 0xFF; /* Outer loop carried state */
        
        /* Inner loop with high ILP and unpredictable control flow */
        for (int i = 0; i < size; i++) {
            /* Multiple dependency types and arithmetic operations */
            int temp = arr_a[i] * base;
            sum += temp;
            
            /* Data-dependent conditional branch */
            if (sum > threshold) {
                arr_b[i] = sum;
                sum = 0;
                /* Inline assembly barrier to create scheduling complexity */
                asm volatile("" ::: "memory");
            }
            
            /* Floating point operations mixed with integer */
            fsum += arr_c[i] * 0.5f;
            arr_c[i] = fsum;
            
            /* Manual unrolling (2 iterations) */
            if (i + 1 < size) {
                int temp2 = arr_a[i + 1] * (base ^ 0x55);
                sum += temp2;
                fsum += arr_c[i + 1] * 0.25f;
                
                /* Another conditional with different condition */
                if ((arr_a[i + 1] & 3) == 0) {
                    arr_b[i + 1] = (int)fsum;
                    asm volatile("" ::: "memory");
                }
                i++; /* Skip next iteration */
            }
        }
        
        /* Modify threshold based on outer loop state */
        threshold += base * 10;
    }
}

/* Second test with volatile counters and assembly barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining")))
void test_selective_scheduler_2(double *arr_d, int *arr_e, int size) {
    volatile int v_counter = size; /* Volatile prevents optimization */
    double acc = 0.0;
    int int_acc = 0;
    
    /* Loop with volatile control and mixed operations */
    while (v_counter > 0) {
        int idx = size - v_counter;
        
        /* Complex dependency chain */
        double dval = arr_d[idx];
        acc += dval * dval;
        
        /* Conditional with side effects */
        if (acc > 1000000.0) {
            arr_e[idx] = (int)(acc / 1000.0);
            acc = acc * 0.1;
            /* Memory barrier */
            asm volatile("" ::: "memory");
        }
        
        /* Integer operations with output dependency */
        int old_val = arr_e[idx];
        arr_e[idx] = old_val + int_acc;
        int_acc = old_val;
        
        /* Mixed type computation */
        arr_d[idx] = acc + (double)int_acc;
        
        /* Another barrier */
        asm volatile("" ::: "memory");
        
        v_counter--; /* Volatile decrement */
    }
}

/* Third test: Outer loop carried state pattern */
void test_outer_loop_carried(int *arr, int size) {
    int outer_state = 0;
    
    for (int j = 0; j < 8; j++) {
        /* Compute base from outer loop state */
        int base = (outer_state * j + 73) % 256;
        outer_state = base;
        
        /* Inner loop with dependency on outer state */
        for (int i = 0; i < size; i += 2) {
            /* Flow dependency within inner loop */
            int val1 = arr[i];
            val1 = (val1 + base) * 3;
            
            /* Anti-dependency */
            arr[i] = val1 ^ 0xAA;
            
            /* Output dependency */
            int val2 = arr[i + 1];
            val2 = (val2 - base) / 2;
            arr[i + 1] = val2;
            
            /* Control dependency */
            if ((val1 + val2) & 1) {
                arr[i] = arr[i] * 2;
                asm volatile("" ::: "memory");
            }
        }
        
        /* Modify base for next iteration */
        base = (base * 13) & 0xFF;
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
        array_a[i] = (int)(lcg_rand() % 1000);
        array_b[i] = (int)(lcg_rand() % 1000);
        array_c[i] = (float)(lcg_rand() % 1000) / 10.0f;
        array_d[i] = (double)(lcg_rand() % 1000) / 5.0;
        array_e[i] = (int)(lcg_rand() % 1000);
    }
    
    /* Volatile flag to introduce runtime variability */
    volatile int run_test_1 = 1;
    volatile int run_test_2 = 1;
    
    /* Call test functions multiple times with volatile control */
    if (run_test_1) {
        for (int rep = 0; rep < 5; rep++) {
            test_selective_scheduler_1(array_a, array_b, array_c, SIZE);
        }
    }
    
    if (run_test_2) {
        for (int rep = 0; rep < 3; rep++) {
            test_selective_scheduler_2(array_d, array_e, SIZE);
        }
    }
    
    /* Always run the third test */
    test_outer_loop_carried(array_a, SIZE);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += array_a[i];
        checksum += array_b[i];
        checksum += (long long)array_c[i];
        checksum += (long long)array_d[i];
        checksum += array_e[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    return 0;
}
