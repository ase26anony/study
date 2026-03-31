/* sel-sched-trigger.c
 * Program designed to trigger GCC selective scheduler debugging dumps
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-dump -std=gnu11 sel-sched-trigger.c -o sel-sched-trigger
 */

#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Function 1: Complex nested loop with data-dependent branches
 * Force selective scheduling with attribute */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
void test_function_1(int* restrict a, int* restrict b, int* restrict c, int size) {
    volatile int threshold = 1000; /* volatile to prevent constant propagation */
    int sum = 0;
    
    /* Outer loop with carried state */
    for (int j = 0; j < 4; j++) {
        int base = (j * 256) & 0xFF; /* Compute base from outer loop */
        
        /* Inner loop with mixed operations and data-dependent branches */
        for (int i = 0; i < size; i++) {
            /* Multiple dependency types: flow (sum), anti (a[i]), output (c[i]) */
            int temp = a[i] * b[i];
            sum = sum + temp;
            
            /* Data-dependent conditional branch - unpredictable pattern */
            if (sum > threshold) {
                c[i] = sum;
                sum = 0;
                /* Inline asm barrier to create scheduling boundary */
                asm volatile("" ::: "memory");
            } else {
                c[i] = temp;
            }
            
            /* Additional floating point operations to increase RTL complexity */
            float f_temp = (float)sum * 0.5f;
            if (f_temp > 50.0f) {
                /* Another scheduling barrier */
                asm volatile("" ::: "memory");
                sum = (int)(f_temp * 2.0f);
            }
            
            /* Manual unrolling - 2 iterations */
            if (i + 1 < size) {
                int temp2 = a[i+1] * b[i+1] + base;
                sum = sum + temp2;
                c[i+1] = (sum > threshold) ? sum : temp2;
                i++; /* Skip next iteration */
            }
        }
        
        /* Modify threshold based on outer loop state */
        threshold += base;
    }
}

/* Function 2: Volatile counters and explicit scheduling barriers */
void test_function_2(double* restrict arr1, double* restrict arr2, int size) {
    volatile int v_counter = size; /* volatile prevents optimization */
    double acc = 0.0;
    
    /* Loop with volatile condition */
    for (volatile int v = 0; v < v_counter; v = v + 1) {
        int i = v; /* Use non-volatile for computation */
        
        /* Mixed double and integer operations */
        double d1 = arr1[i];
        double d2 = arr2[i];
        
        /* Create resource conflicts with multiple FP operations */
        double prod = d1 * d2;
        double sum = d1 + d2;
        double diff = d1 - d2;
        
        /* Complex conditional with multiple dependencies */
        if ((i & 3) == 0) {
            acc += prod * 1.5;
            /* Scheduling barrier */
            asm volatile("" ::: "memory");
        } else if ((i & 3) == 1) {
            acc += sum * 0.75;
        } else if ((i & 3) == 2) {
            acc += diff * 2.0;
            asm volatile("" ::: "memory");
        } else {
            acc = acc * 0.9 + prod;
        }
        
        /* Write back with data-dependent pattern */
        arr1[i] = (acc > 1000.0) ? acc : prod;
        arr2[i] = (i % 5 == 0) ? sum : diff;
        
        /* Additional integer computation to mix types */
        int int_val = (int)acc;
        if (int_val & 1) {
            arr1[i] += 1.0;
        }
    }
}

/* Function 3: Outer-loop carried state pattern */
void test_function_3(float* restrict data, int outer, int inner) {
    float state = 0.0f;
    
    for (int j = 0; j < outer; j++) {
        /* Compute base from outer loop - creates loop-carried dependency */
        float base = (float)((j * 137) % 101) * 0.01f;
        float factor = 1.0f + base;
        
        /* Inner loop with dependency on outer loop state */
        for (int i = 0; i < inner; i++) {
            /* Multiple operations with flow dependencies */
            float val = data[i];
            val = (val + base) * factor;
            
            /* Data-dependent update with conditional */
            if (val > state) {
                state = val * 0.9f;
                asm volatile("" ::: "memory");
            } else {
                state = state * 0.95f + val * 0.05f;
            }
            
            /* Write back with potential anti-dependency */
            data[i] = val + state;
            
            /* Manual unrolling - 3 iterations */
            if (i + 2 < inner) {
                float val2 = data[i+1];
                float val3 = data[i+2];
                
                val2 = (val2 + base) * (factor * 0.5f);
                val3 = (val3 + base) * (factor * 1.5f);
                
                if (val2 > val3) {
                    data[i+1] = val2;
                    data[i+2] = val3;
                } else {
                    data[i+1] = val3;
                    data[i+2] = val2;
                }
                
                i += 2; /* Skip two iterations */
            }
        }
        
        /* Modify factor based on accumulated state */
        factor += state * 0.01f;
    }
}

int main(void) {
    const int SIZE = 1024;
    int array_a[SIZE];
    int array_b[SIZE];
    int array_c[SIZE];
    double array_d1[SIZE];
    double array_d2[SIZE];
    float array_f[SIZE];
    
    /* Initialize with pseudo-random but non-uniform data */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (int)(lcg_rand() % 1000);
        array_b[i] = (int)(lcg_rand() % 1000);
        array_c[i] = 0;
        array_d1[i] = (double)(lcg_rand() % 1000) * 0.1;
        array_d2[i] = (double)(lcg_rand() % 1000) * 0.1;
        array_f[i] = (float)(lcg_rand() % 1000) * 0.01f;
    }
    
    /* Volatile flag to introduce runtime variability */
    volatile int volatile_flag = 1;
    
    /* Call test functions multiple times with runtime decisions */
    for (int iteration = 0; iteration < 3; iteration++) {
        if (volatile_flag) {
            /* Force multiple calls to increase scheduling opportunities */
            for (int rep = 0; rep < 5; rep++) {
                test_function_1(array_a, array_b, array_c, SIZE);
            }
        }
        
        /* Modify volatile flag based on data */
        volatile_flag = (array_c[0] & 1);
        
        if (volatile_flag || (iteration % 2 == 0)) {
            test_function_2(array_d1, array_d2, SIZE);
        }
        
        /* Always call third function but with varying parameters */
        test_function_3(array_f, 4 + (iteration % 3), SIZE);
        
        /* Modify some array values to change dependencies */
        for (int i = 0; i < SIZE; i += 64) {
            array_a[i] += iteration;
            array_b[i] -= iteration;
        }
    }
    
    /* Compute final checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += array_c[i];
        checksum += (long long)(array_d1[i] * 100);
        checksum += (long long)(array_d2[i] * 100);
        checksum += (long long)(array_f[i] * 1000);
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    return 0;
}
