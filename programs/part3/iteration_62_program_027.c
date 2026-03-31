/* sel-sched-trigger.c - Program to trigger selective scheduler debugging dumps */
#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Function 1: Complex nested loops with data-dependent branches */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
void test_complex_schedule(int* restrict a, int* restrict b, float* restrict c, 
                          int size, int threshold) {
    volatile int vol_size = size; /* Prevent constant propagation */
    float sum = 0.0f;
    float factor = 1.5f;
    
    /* Outer loop with carried state */
    for (int j = 0; j < 4; j++) {
        int base = (j * 17) & 0xFF; /* Varying base for inner loop */
        
        /* Manually unrolled inner loop with mixed operations */
        for (int i = 0; i < vol_size - 3; i += 4) {
            /* Create multiple dependency types */
            float temp1 = a[i] * factor + base;
            float temp2 = b[i] * 0.75f - base;
            
            /* Flow dependency through sum */
            sum = sum + temp1 * temp2;
            
            /* Data-dependent branch with anti-dependency */
            if (sum > (float)threshold) {
                c[i] = sum;
                sum = 0.0f; /* Output dependency on sum */
            }
            
            /* Inline assembly barrier to complicate scheduling */
            asm volatile("" ::: "memory");
            
            /* Second iteration of unrolled loop */
            temp1 = a[i+1] * factor + base;
            temp2 = b[i+1] * 0.75f - base;
            sum = sum + temp1 * temp2;
            if (sum > (float)threshold * 1.1f) {
                c[i+1] = sum;
                sum = sum * 0.5f; /* Different transformation */
            }
            
            /* Third iteration with different operations */
            double dtemp = (double)a[i+2] * 0.25 + (double)base;
            sum = sum + (float)dtemp;
            if ((a[i+2] & 1) == 0) { /* Bitwise condition */
                c[i+2] = sum * 2.0f;
            }
            
            /* Fourth iteration with more complex dependencies */
            sum = sum + a[i+3] * b[i+3] * 0.01f;
            float old_sum = sum; /* Anti-dependency */
            if (old_sum < -threshold) {
                c[i+3] = old_sum;
                sum = old_sum * -1.0f;
            }
            
            /* Another memory barrier */
            asm volatile("" ::: "memory");
        }
    }
}

/* Function 2: Volatile counters and assembly barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining", "funroll-loops")))
void test_volatile_barriers(float* restrict arr, int* restrict mask, int n) {
    volatile int vol_n = n; /* Volatile to prevent optimization */
    float acc1 = 0.0f, acc2 = 0.0f, acc3 = 0.0f;
    
    for (volatile int v = 0; v < vol_n; v++) {
        int i = v; /* Convert to non-volatile */
        
        /* Multiple accumulators with resource conflicts */
        acc1 = acc1 + arr[i] * 1.1f;
        asm volatile("" ::: "memory");
        
        acc2 = acc2 + arr[i] * 2.2f;
        if (mask[i] & 0x1) {
            acc3 = acc3 - arr[i];
        }
        
        asm volatile("" ::: "memory");
        
        /* Conditional with unpredictable pattern */
        if ((i % 7) == 0) {
            arr[i] = acc1 + acc2;
            acc1 = acc2 * 0.3f; /* Output dependency */
        } else if ((i % 13) == 0) {
            arr[i] = acc3 * 2.0f;
            acc2 = acc3; /* Flow dependency */
        }
        
        /* Mixed integer/floating point operations */
        int int_val = mask[i] * i;
        float float_val = (float)int_val * 0.01f;
        arr[i] = arr[i] + float_val;
        
        asm volatile("" ::: "memory");
    }
}

/* Function 3: Outer-loop carried state pattern */
void test_outer_carried_state(double* restrict data, int rows, int cols) {
    double outer_state = 1.0;
    
    for (int r = 0; r < rows; r++) {
        /* Outer loop modifies state used in inner loop */
        outer_state = outer_state * 1.05 + (double)r;
        volatile double vol_state = outer_state; /* Volatile copy */
        
        /* Inner loop with complex dependency on outer state */
        for (int c = 0; c < cols - 1; c += 2) {
            /* Two iterations processed together */
            double val1 = data[r * cols + c] * vol_state;
            double val2 = data[r * cols + c + 1] / vol_state;
            
            /* Cross-iteration dependency */
            data[r * cols + c] = val1 + val2;
            data[r * cols + c + 1] = val1 - val2;
            
            /* Data-dependent operation */
            if (val1 > val2) {
                data[r * cols + c] = data[r * cols + c] * 0.9;
                asm volatile("" ::: "memory");
            }
            
            /* Potential resource conflict */
            double temp = val1 * val2;
            data[r * cols + c] = data[r * cols + c] + temp * 0.1;
        }
    }
}

/* Main function with runtime variability */
int main(void) {
    const int SIZE = 1024;
    const int ROWS = 32;
    const int COLS = 32;
    
    /* Initialize arrays with pseudo-random data */
    int array_a[SIZE], array_b[SIZE];
    float array_c[SIZE];
    double array_d[ROWS * COLS];
    
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (int)lcg_rand() % 1000 - 500;
        array_b[i] = (int)lcg_rand() % 1000 - 500;
        array_c[i] = (float)(lcg_rand() % 1000) / 10.0f - 50.0f;
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        array_d[i] = (double)(lcg_rand() % 1000) / 10.0 - 50.0;
    }
    
    /* Volatile flag for runtime control flow */
    volatile int vol_flag = array_a[0] > 0;
    
    /* Call test functions with runtime variability */
    unsigned long checksum = 0;
    
    if (vol_flag) {
        for (int rep = 0; rep < 5; rep++) {
            test_complex_schedule(array_a, array_b, array_c, SIZE, 100);
        }
    }
    
    /* Always execute second test */
    for (int rep = 0; rep < 3; rep++) {
        test_volatile_barriers(array_c, array_a, SIZE);
    }
    
    /* Conditional execution based on array content */
    if (array_b[SIZE/2] % 2 == 0) {
        test_outer_carried_state(array_d, ROWS, COLS);
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        checksum += (unsigned long)(array_a[i] & 0xFFF);
        checksum += (unsigned long)(array_b[i] & 0xFFF);
        checksum ^= (unsigned long)(*(uint32_t*)&array_c[i]);
    }
    
    for (int i = 0; i < ROWS * COLS; i += 8) {
        checksum ^= (unsigned long)(*(uint64_t*)&array_d[i]);
    }
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
