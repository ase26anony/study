/* sel-sched-trigger.c
 * Program designed to trigger GCC selective scheduler debugging dumps
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

/* Force selective scheduling on this function */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
void test_selective_sched_1(int* restrict a, int* restrict b, float* restrict c, 
                           int size, int threshold) {
    volatile int vol_size = size; /* Prevent constant propagation */
    int sum = 0;
    float fsum = 0.0f;
    
    /* Nested loops with data-dependent branches */
    for (int j = 0; j < 4; j++) {  /* Outer loop with carried state */
        int base = (j * 17) & 0xFF;
        
        for (int i = 0; i < vol_size; i++) {
            /* Mixed data types and operations */
            int temp = a[i] * b[i] + base;
            sum += temp;
            
            /* Data-dependent conditional branch */
            if (sum > threshold) {
                c[i] = (float)sum * 0.5f;
                sum = sum / 2;
                asm volatile("" ::: "memory"); /* Scheduling barrier */
            } else {
                c[i] = (float)temp * 0.25f;
            }
            
            /* Floating point operations with dependencies */
            fsum += c[i];
            if (fsum > 1000.0f) {
                fsum = fsum * 0.9f;
                asm volatile("" ::: "memory"); /* Another barrier */
            }
            
            /* Manual unrolling - 2 iterations */
            if (i + 1 < vol_size) {
                int temp2 = a[i+1] * b[i+1] + (base ^ 0x55);
                sum += temp2;
                c[i+1] = (float)temp2 * 0.3f;
                fsum += c[i+1];
                i++; /* Skip next iteration */
            }
        }
        
        /* Outer loop modification affecting inner loop */
        threshold = (threshold + base) & 0x3FF;
    }
}

/* Second test function with volatile counters and assembly barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining")))
void test_selective_sched_2(double* restrict d, int* restrict counters, int n) {
    volatile int vol_n = n;
    double acc1 = 0.0, acc2 = 0.0, acc3 = 0.0;
    
    for (volatile int i = 0; i < vol_n; i++) {
        /* Multiple accumulators with flow dependencies */
        acc1 = acc1 + d[i] * 1.1;
        asm volatile("" : "+r"(acc1) : : "memory");
        
        acc2 = acc2 + d[i] * 2.2;
        if (i & 1) {
            acc3 = acc3 - d[i] * 0.5;
            asm volatile("" ::: "memory");
        } else {
            acc3 = acc3 + d[i] * 1.5;
        }
        
        /* Complex conditional with resource conflicts */
        if (counters[i] > 100) {
            d[i] = acc1 * acc2;
            counters[i] = counters[i] - 50;
        } else if (counters[i] < 0) {
            d[i] = acc3 * 0.8;
            counters[i] = counters[i] + 100;
        } else {
            d[i] = (acc1 + acc2 + acc3) / 3.0;
        }
        
        /* Additional unrolled computation */
        if (i + 3 < vol_n) {
            double t1 = d[i+1] * d[i+2];
            double t2 = d[i+2] * d[i+3];
            d[i+1] = t1 + t2;
            asm volatile("" ::: "memory");
        }
    }
}

/* Third function with outer-loop carried state pattern */
void test_outer_loop_carried(int* arr, float* farr, int outer, int inner) {
    volatile int vol_outer = outer;
    int state = 0;
    
    for (int j = 0; j < vol_outer; j++) {
        /* Compute base value that depends on outer loop iteration */
        int base = (j * 73 + state) & 0xFFF;
        float factor = 1.0f + (j % 10) * 0.1f;
        
        /* Inner loop with carried state from outer loop */
        for (int i = 0; i < inner; i++) {
            /* Mixed integer/float operations with dependencies */
            arr[i] = (arr[i] + base) * (i % 7 + 1);
            farr[i] = farr[i] * factor + (float)arr[i];
            
            /* Data-dependent update to state */
            if (farr[i] > 500.0f) {
                state = (state + i) & 0xFF;
                farr[i] = farr[i] * 0.5f;
            }
            
            /* Small unrolled section */
            if (i % 4 == 0 && i + 2 < inner) {
                farr[i+1] = (farr[i] + farr[i+2]) * 0.3f;
                arr[i+2] = arr[i] ^ arr[i+1];
            }
        }
        
        /* Outer loop modifies data for next iteration */
        base = (base * 3 + 1) & 0xFF;
    }
}

int main(void) {
    const int SIZE = 1024;
    int array1[SIZE];
    int array2[SIZE];
    float farray[SIZE];
    double darray[SIZE];
    int counters[SIZE];
    
    /* Initialize with pseudo-random but bounded values */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (int)(lcg_rand() % 1000) - 500;
        array2[i] = (int)(lcg_rand() % 1000) - 500;
        farray[i] = (float)(lcg_rand() % 1000) * 0.1f;
        darray[i] = (double)(lcg_rand() % 1000) * 0.01;
        counters[i] = (int)(lcg_rand() % 200) - 100;
    }
    
    volatile int checksum = 0;
    volatile int flag = (array1[0] > 0) ? 1 : 0;
    
    /* Call test functions multiple times with runtime variability */
    for (int iter = 0; iter < 3; iter++) {
        if (flag || iter == 0) {
            test_selective_sched_1(array1, array2, farray, SIZE, 1000 + iter * 100);
        }
        
        if (flag || iter == 1) {
            for (int rep = 0; rep < 2; rep++) {
                test_selective_sched_2(darray, counters, SIZE);
            }
        }
        
        if (flag || iter == 2) {
            test_outer_loop_carried(array1, farray, 8, SIZE);
        }
        
        /* Modify flag based on array contents */
        flag = (array1[iter % SIZE] + array2[iter % SIZE]) & 1;
    }
    
    /* Compute final checksum to prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        checksum += array1[i] + (int)farray[i] + (int)darray[i] + counters[i];
        checksum = (checksum * 31 + 17) & 0x7FFFFFFF;
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
