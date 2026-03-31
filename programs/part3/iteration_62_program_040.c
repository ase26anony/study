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

/* Function 1: Complex nested loop with data-dependent branches
 * Force selective scheduling with attribute */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
void test_selective_schedule_1(int* restrict a, int* restrict b, float* restrict c, 
                               int size, volatile int threshold) {
    float sum = 0.0f;
    float factor = 1.5f;
    
    /* Outer loop with carried state */
    for (int j = 0; j < 4; j++) {
        int base = (j * 7) & 0xF;  /* Varying base for inner loop */
        
        /* Inner loop with mixed operations and data-dependent branch */
        for (int i = 0; i < size; i++) {
            /* Multiple dependency types */
            float temp = (float)a[i] * factor + (float)base;
            
            /* Flow dependency */
            sum = sum + temp * (float)b[i];
            
            /* Data-dependent conditional branch - unpredictable */
            if (sum > (float)threshold) {
                /* Anti dependency on c[i] */
                c[i] = sum;
                sum = 0.0f;
            }
            
            /* Output dependency simulation */
            a[i] = (int)(temp * 0.7f);
            
            /* Memory barrier to prevent over-optimization */
            asm volatile("" ::: "memory");
        }
        
        /* Manual unrolling for more scheduling complexity */
        for (int i = 0; i < 4 && i < size; i++) {
            double dval = (double)c[i] * 2.5;
            int ival = (int)dval;
            
            /* Another conditional with different data type */
            if (ival & 1) {
                b[i] = ival * 3;
            } else {
                b[i] = ival / 2;
            }
            
            /* Complex expression with multiple operations */
            c[i] = (float)((dval * 0.3) + (ival * 0.7));
        }
    }
}

/* Function 2: Volatile counters and inline assembly barriers */
void test_selective_schedule_2(double* restrict arr1, double* restrict arr2, 
                               int size, volatile int iter_count) {
    volatile int v_counter = iter_count;
    double acc = 0.0;
    
    while (v_counter-- > 0) {
        /* Nested loops with volatile condition */
        for (volatile int k = 0; k < 2; k++) {
            for (int i = 0; i < size; i += 2) {
                /* Multiple FP operations creating resource conflicts */
                double t1 = arr1[i] * 1.1;
                double t2 = arr2[i] * 2.2;
                double t3 = arr1[i+1] * 3.3;
                double t4 = arr2[i+1] * 4.4;
                
                /* Cross dependencies */
                arr1[i] = t1 + t2;
                arr2[i] = t3 - t4;
                arr1[i+1] = t1 * t3;
                arr2[i+1] = t2 / (t4 + 1.0);
                
                /* Accumulator with loop-carried dependency */
                acc += arr1[i] + arr2[i] + arr1[i+1] + arr2[i+1];
                
                /* Scheduling barrier */
                asm volatile("" ::: "memory");
            }
        }
        
        /* Conditional with volatile to prevent optimization */
        if (v_counter & 1) {
            for (int i = 0; i < size; i++) {
                arr1[i] = arr1[i] * 0.9;
            }
        }
    }
}

/* Function 3: Outer-loop carried state pattern */
void test_selective_schedule_3(int* restrict data, int size, int outer_iters) {
    int state = 0;
    
    for (int outer = 0; outer < outer_iters; outer++) {
        /* Compute state based on outer iteration */
        int base = (state * 13 + outer * 17) & 0xFF;
        int modifier = (outer % 3) + 1;
        
        /* Inner loop uses outer loop state */
        for (int i = 0; i < size; i++) {
            /* Complex addressing with multiple dependencies */
            int idx = (i + base) % size;
            
            /* Mixed operations creating anti and output dependencies */
            int old_val = data[idx];
            int new_val = (old_val * modifier + base) >> 1;
            
            /* Conditional update based on complex condition */
            if ((old_val ^ new_val) & 0x1) {
                data[idx] = new_val + i;
            } else {
                data[idx] = new_val - (i % 8);
            }
            
            /* Update state with loop-carried dependency */
            state = (state + data[idx]) & 0xFFFF;
            
            /* Another memory barrier */
            asm volatile("" ::: "memory");
        }
        
        /* Small unrolled section */
        for (int i = 0; i < 3 && i < size; i++) {
            data[i] = data[i] * 2 - state;
            data[i] = data[i] ^ (base & 0xFF);
        }
    }
}

int main(void) {
    const int SIZE = 1024;
    int array1[SIZE];
    int array2[SIZE];
    float farray[SIZE];
    double darray1[SIZE];
    double darray2[SIZE];
    
    /* Initialize with pseudo-random but bounded values */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (int)(lcg_rand() % 1000);
        array2[i] = (int)(lcg_rand() % 1000);
        farray[i] = (float)(lcg_rand() % 100) / 10.0f;
        darray1[i] = (double)(lcg_rand() % 100) / 5.0;
        darray2[i] = (double)(lcg_rand() % 100) / 5.0;
    }
    
    /* Volatile flag to prevent compile-time optimization */
    volatile int run_flag = 1;
    int checksum = 0;
    
    /* Call test functions multiple times with volatile control */
    if (run_flag) {
        for (int rep = 0; rep < 3; rep++) {
            /* Varying threshold to create different paths */
            volatile int threshold = 500 + rep * 100;
            test_selective_schedule_1(array1, array2, farray, SIZE, threshold);
        }
    }
    
    /* Another volatile condition */
    volatile int iter_control = 2;
    if (iter_control > 0) {
        for (int rep = 0; rep < 2; rep++) {
            test_selective_schedule_2(darray1, darray2, SIZE, iter_control + rep);
        }
    }
    
    /* Final test with outer loop iterations */
    volatile int outer_iters = 3;
    test_selective_schedule_3(array1, SIZE, outer_iters);
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        checksum += array1[i] + array2[i] + (int)farray[i];
        checksum += (int)(darray1[i] + darray2[i]);
        checksum &= 0xFFFFFF;  /* Keep it bounded */
    }
    
    /* Use checksum to affect control flow */
    volatile int final_flag = (checksum > 1000);
    if (final_flag) {
        /* One more call with different parameters */
        test_selective_schedule_1(array2, array1, farray, SIZE / 2, checksum % 1000);
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
