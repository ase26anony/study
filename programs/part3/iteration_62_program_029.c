/* sel-sched-trigger.c - Program to trigger selective scheduler debugging dumps */
#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 12345;
static inline uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Initialize arrays with non-uniform data */
static void init_arrays(int *a, int *b, float *c, double *d, int size) {
    for (int i = 0; i < size; i++) {
        a[i] = (int)(lcg_rand() % 1000) - 500;
        b[i] = (int)(lcg_rand() % 1000) - 500;
        c[i] = (float)(lcg_rand() % 1000) / 10.0f;
        d[i] = (double)(lcg_rand() % 1000) / 10.0;
    }
}

/* Test function 1: Complex nested loops with data-dependent branches
   Force selective scheduling with attribute */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
static void test_function_1(int *a, int *b, float *c, double *d, int size) {
    volatile int threshold = 1000; /* Prevent constant propagation */
    double sum = 0.0;
    int count = 0;
    
    /* Outer loop with carried state */
    for (int j = 0; j < 4; j++) {
        int base = (j * 250) + 100;
        
        /* Inner loop with mixed operations and data-dependent branches */
        for (int i = 0; i < size; i++) {
            /* Multiple dependency types */
            double temp = d[i] * (double)base;
            sum = sum + a[i] * b[i] + temp;
            
            /* Data-dependent conditional branch */
            if (sum > (double)threshold) {
                c[i] = (float)sum;
                sum = 0.0;
                count++;
            }
            
            /* Anti-dependency: read after write */
            d[i] = temp * 0.9;
            
            /* Manual unrolling (2 iterations) */
            if (i + 1 < size) {
                double temp2 = d[i+1] * (double)(base + 1);
                sum = sum + a[i+1] * b[i+1] + temp2;
                
                if (sum > (double)(threshold * 2)) {
                    c[i+1] = (float)sum * 0.5f;
                    sum = sum * 0.5;
                    count++;
                }
                
                d[i+1] = temp2 * 0.8;
                i++; /* Skip next iteration */
            }
            
            /* Scheduling barrier */
            asm volatile("" ::: "memory");
        }
        
        /* Outer loop modification affecting inner loop */
        threshold += 250;
    }
    
    /* Prevent dead code elimination */
    a[0] = count;
}

/* Test function 2: Volatile counters and assembly barriers */
static void test_function_2(int *arr1, int *arr2, int size) {
    volatile int v_counter = size; /* Volatile prevents optimization */
    int sum1 = 0, sum2 = 0;
    
    for (volatile int v = 0; v < v_counter; v++) {
        int idx = v;
        
        /* Complex dependency chain */
        int val1 = arr1[idx];
        int val2 = arr2[idx];
        
        /* Multiple operations with different latencies */
        val1 = val1 * 3 + 7;
        val2 = val2 / 2 - 5;
        
        /* Conditional with side effects */
        if ((val1 ^ val2) & 1) {
            arr1[idx] = val1 + val2;
            sum1 += val1;
        } else {
            arr2[idx] = val1 - val2;
            sum2 += val2;
        }
        
        /* Floating point operations mixed in */
        float fval = (float)val1 * 0.3f;
        double dval = (double)val2 * 0.7;
        
        /* Scheduling barrier after FP ops */
        asm volatile("" ::: "memory");
        
        /* Use results to prevent elimination */
        arr1[idx] += (int)fval;
        arr2[idx] += (int)dval;
        
        /* Another barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Cross-iteration dependency */
    arr1[0] = sum1 + sum2;
}

/* Test function 3: Outer-loop carried state pattern */
static void test_function_3(double *data, int *indices, int outer, int inner) {
    double accumulator = 0.0;
    double factor = 1.05;
    
    for (int j = 0; j < outer; j++) {
        /* Outer loop modifies state used in inner loop */
        double base = (double)(j * 100) + accumulator * 0.1;
        factor = factor * 0.99; /* Slowly changing factor */
        
        for (int i = 0; i < inner; i++) {
            int idx = indices[i % 1024];
            
            /* Complex calculation with loop-carried dependency */
            double old = data[idx];
            data[idx] = (old + base) * factor;
            
            /* Accumulator creates flow dependency */
            accumulator += data[idx] - old;
            
            /* Conditional store with anti-dependency */
            if (accumulator > 1000.0) {
                double temp = data[idx];
                data[idx] = accumulator * 0.5;
                accumulator = temp;
            }
            
            /* Manual unrolling (4 iterations) */
            if (i + 3 < inner) {
                for (int k = 1; k <= 3; k++) {
                    int idx2 = indices[(i + k) % 1024];
                    double old2 = data[idx2];
                    data[idx2] = (old2 + base * (k + 1)) * (factor * 0.9);
                    accumulator += data[idx2] - old2;
                    
                    if ((k & 1) && accumulator < -500.0) {
                        data[idx2] = -accumulator;
                        accumulator *= 0.8;
                    }
                }
                i += 3;
            }
        }
        
        /* Outer loop dependency */
        accumulator *= 0.95;
    }
    
    /* Store final state */
    data[0] = accumulator;
}

int main(void) {
    const int SIZE = 1024;
    int array_a[SIZE], array_b[SIZE];
    float array_c[SIZE];
    double array_d[SIZE];
    int indices[SIZE];
    
    /* Initialize with pseudo-random data */
    init_arrays(array_a, array_b, array_c, array_d, SIZE);
    for (int i = 0; i < SIZE; i++) {
        indices[i] = lcg_rand() % SIZE;
    }
    
    /* Volatile flag for runtime variability */
    volatile int flag = 0;
    int checksum = 0;
    
    /* Call test functions multiple times with runtime decisions */
    for (int iteration = 0; iteration < 3; iteration++) {
        flag = (lcg_rand() & 1);
        
        if (flag) {
            for (int rep = 0; rep < 5; rep++) {
                test_function_1(array_a, array_b, array_c, array_d, SIZE);
            }
        }
        
        test_function_2(array_a, array_b, SIZE);
        
        if (!flag || (iteration & 1)) {
            test_function_3(array_d, indices, 8, SIZE / 2);
        }
    }
    
    /* Compute final checksum to prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        checksum += array_a[i] + (int)array_c[i] + (int)array_d[i];
        checksum ^= array_b[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
