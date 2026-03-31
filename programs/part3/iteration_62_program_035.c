/* sel_sched_trigger.c - Program to trigger GCC selective scheduler debugging dumps */
#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 12345;
static uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Force selective scheduling with attribute */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
void test_selective_sched_1(int* arr_a, int* arr_b, float* arr_c, int size, int threshold) {
    volatile int vol_size = size; /* Prevent constant propagation */
    int sum = 0;
    float fsum = 0.0f;
    
    /* Nested loops with data-dependent branches */
    for (int outer = 0; outer < 4; outer++) {
        int base = (outer * 17) & 0xFF; /* Outer loop carried state */
        
        for (int i = 0; i < vol_size; i++) {
            /* Mixed data types and operations */
            int val_a = arr_a[i];
            int val_b = arr_b[i];
            
            /* Flow dependency chain */
            sum = sum + val_a * val_b;
            
            /* Data-dependent conditional branch */
            if (sum > threshold) {
                arr_c[i] = (float)sum * 0.5f;
                sum = sum / 2; /* Anti-dependency on sum */
            } else {
                arr_c[i] = (float)val_a * 0.25f + (float)val_b * 0.75f;
            }
            
            /* Floating point operations with output dependency */
            fsum = fsum + arr_c[i];
            arr_c[i] = fsum; /* Output dependency */
            
            /* Manual partial unrolling (2 iterations) */
            if (i + 1 < vol_size) {
                int next_val = arr_a[i+1] ^ base; /* Use outer loop state */
                arr_b[i+1] = arr_b[i+1] + next_val;
                
                /* Inline assembly barrier - creates scheduling complexity */
                asm volatile("" ::: "memory");
                
                /* Another conditional with different data type */
                if ((next_val & 3) == 0) {
                    arr_c[i+1] = arr_c[i+1] * 1.1f;
                }
                i++; /* Skip the unrolled iteration */
            }
        }
        
        /* Modify threshold based on outer loop */
        threshold = (threshold + base) & 0x3FF;
    }
}

/* Second test with volatile counters and more barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining")))
void test_selective_sched_2(double* arr_d, int* arr_e, int size) {
    volatile int vol_counter = 0;
    double acc = 0.0;
    int int_acc = 0;
    
    /* Loop with volatile condition */
    while (vol_counter < size) {
        /* Multiple dependency types in one iteration */
        double temp_d = arr_d[vol_counter];
        int temp_e = arr_e[vol_counter];
        
        /* Flow dependency with type conversion */
        acc = acc + temp_d * (double)temp_e;
        
        /* Anti-dependency chain */
        arr_d[vol_counter] = acc * 0.333;
        double old_acc = acc;
        
        /* Output dependency */
        acc = old_acc + sin(acc * 0.01); /* Simple approximation */
        
        /* Control dependency with bit operations */
        if (temp_e & (1 << (vol_counter & 7))) {
            int_acc = int_acc ^ temp_e;
            arr_e[vol_counter] = int_acc;
            
            /* Another scheduling barrier */
            asm volatile("" ::: "memory");
        }
        
        /* Mixed integer/floating point */
        arr_d[vol_counter] = arr_d[vol_counter] + (double)(int_acc % 100);
        
        vol_counter++;
        
        /* Additional barrier every 8 iterations */
        if ((vol_counter & 7) == 0) {
            asm volatile("" ::: "memory");
        }
    }
}

/* Third test with outer-loop carried state pattern */
void test_selective_sched_3(int* arr, int size, int iterations) {
    int state = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Outer loop modifies state used in inner loop */
        int base = (state * 13 + iter * 7) & 0xFF;
        state = base;
        
        /* Inner loop with complex dependency on outer state */
        for (int i = 0; i < size; i++) {
            /* Loop-carried dependency spanning iterations */
            int old_val = arr[i];
            
            /* Calculation using outer loop state */
            arr[i] = (old_val + base) * ((i & 1) ? 3 : 5);
            
            /* Data-dependent branch */
            if (arr[i] > 1000) {
                arr[i] = arr[i] % 1000;
                base = (base + 1) & 0xFF; /* Modify outer state */
            }
            
            /* Unrolled by 2 with different operations */
            if (i + 1 < size) {
                arr[i+1] = arr[i+1] + (base ^ old_val);
                i++;
            }
        }
    }
}

/* Approximation of sin for testing */
static double sin(double x) {
    /* Simple Taylor series approximation for small x */
    return x - (x*x*x)/6.0 + (x*x*x*x*x)/120.0;
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
        array_d[i] = (double)(lcg_rand() % 100) * 0.01;
        array_e[i] = (int)(lcg_rand() % 256);
    }
    
    volatile int checksum = 0;
    volatile int flag = 1;
    
    /* Runtime-variable control flow */
    if (flag) {
        for (int rep = 0; rep < 5; rep++) {
            test_selective_sched_1(array_a, array_b, array_c, SIZE, 5000 + rep * 100);
        }
    }
    
    /* Compute intermediate checksum */
    for (int i = 0; i < SIZE; i++) {
        checksum ^= array_a[i] ^ array_b[i] ^ (int)array_c[i];
    }
    
    /* More conditional execution */
    if (checksum & 1) {
        for (int rep = 0; rep < 3; rep++) {
            test_selective_sched_2(array_d, array_e, SIZE);
        }
    }
    
    /* Always execute third test */
    test_selective_sched_3(array_b, SIZE, 8);
    
    /* Final checksum computation to prevent dead code elimination */
    long long final_checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_checksum += array_a[i];
        final_checksum += array_b[i];
        final_checksum += (long long)(array_c[i] * 100);
        final_checksum += (long long)(array_d[i] * 1000);
        final_checksum += array_e[i];
        
        /* Mix operations to prevent optimization */
        if ((i & 31) == 0) {
            final_checksum = (final_checksum * 6364136223846793005ULL) >> 32;
        }
    }
    
    printf("Final checksum: %lld\n", final_checksum);
    return (int)(final_checksum & 0x7FFFFFFF);
}
