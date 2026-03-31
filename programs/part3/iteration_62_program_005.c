/* sel-sched-trigger.c - Program to trigger selective scheduler debugging dumps */
#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 123456789;
static inline uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Force selective scheduling with optimization attributes */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-dump")))
void test_complex_schedule(int* restrict arr1, int* restrict arr2, 
                          float* restrict farr, int size, volatile int* vflag) {
    float sum_f = 0.0f;
    double sum_d = 0.0;
    int sum_i = 0;
    const float threshold = 1000.0f;
    
    /* Outer loop with carried state */
    for (int outer = 0; outer < 4; outer++) {
        int base = (outer * 17) & 0xFF;
        float factor = 1.0f + outer * 0.1f;
        
        /* Manually unrolled inner loop with mixed operations */
        for (int i = 0; i < size - 3; i += 4) {
            /* Unrolled iteration 1 - mixed data types and dependencies */
            int val1 = arr1[i] + base;
            float fval1 = farr[i] * factor;
            sum_f += fval1;
            
            /* Data-dependent branch with unpredictable pattern */
            if (val1 & 1) {
                arr2[i] = val1 * 3;
                sum_i += arr2[i];
            } else {
                arr2[i] = val1 / 2;
                sum_i -= arr2[i];
            }
            
            /* Inline assembly barrier to create scheduling complexity */
            asm volatile("" ::: "memory");
            
            /* Unrolled iteration 2 - floating point with control flow */
            int val2 = arr1[i+1] ^ base;
            double dval2 = (double)farr[i+1] * 1.5;
            sum_d += dval2;
            
            if (sum_f > threshold) {
                farr[i+1] = sum_f;
                sum_f = 0.0f;
            }
            
            /* Unrolled iteration 3 - more complex dependency chain */
            int val3 = arr1[i+2] - base;
            arr2[i+2] = val3 * val1;  /* Flow dependency from iteration 1 */
            sum_i += arr2[i+2] * 2;
            
            /* Anti-dependency: reading after writing */
            int temp = arr2[i+2];
            arr1[i+2] = temp + 1;
            
            /* Unrolled iteration 4 - resource conflict simulation */
            float fval4 = farr[i+3] * 2.0f;
            double dval4 = (double)fval4 * 0.5;
            sum_f += fval4;
            sum_d += dval4;
            
            /* Another conditional with volatile check */
            if (*vflag) {
                arr2[i+3] = (int)(sum_f + sum_d);
            } else {
                arr2[i+3] = (int)(sum_f - sum_d);
            }
            
            /* Output dependency: multiple writes to same variable */
            sum_i = sum_i + arr1[i+3];
            sum_i = sum_i - base;
        }
        
        /* Loop-carried dependency across outer iterations */
        base = (base + sum_i) & 0xFF;
    }
}

/* Second test with volatile counters and barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
void test_volatile_barriers(int* arr, int size, volatile int limit) {
    volatile int counter = 0;
    float accum = 0.0f;
    
    for (volatile int v = 0; v < limit; v++) {
        /* Nested loop with volatile condition */
        for (int i = 0; i < size; i++) {
            /* Multiple operations with different latencies */
            int val = arr[i];
            float fval = (float)val * 1.2345f;
            double dval = (double)fval * 0.9876;
            
            /* Complex conditional with side effects */
            if ((val ^ counter) & 0xF) {
                arr[i] = val * 2 + 1;
                accum += fval;
                
                /* Memory barrier forcing serialization */
                asm volatile("" ::: "memory");
                
                /* More operations after barrier */
                dval = dval * 2.0;
                accum -= (float)dval;
            } else {
                arr[i] = val / 2;
                accum += (float)dval;
            }
            
            /* Periodically reset accumulator to create control flow */
            if (counter % 16 == 0) {
                accum = accum * 0.5f;
            }
            
            counter++;
        }
        
        /* Outer loop computation affecting inner loop */
        limit = (limit * 13 + 7) & 0xFF;
    }
}

/* Third test: outer-loop carried state pattern */
void test_outer_carried_state(int* data, float* fdata, int rows, int cols) {
    int state = 0;
    
    for (int row = 0; row < rows; row++) {
        /* Compute base from outer loop state */
        int base = (state * row + 17) % 256;
        float factor = 1.0f + (row % 10) * 0.1f;
        
        for (int col = 0; col < cols; col++) {
            int idx = row * cols + col;
            
            /* Mixed computation with outer loop dependency */
            int val = data[idx];
            float fval = fdata[idx];
            
            /* Data-dependent operation */
            if ((val + base) & 0x1) {
                data[idx] = (val * 3 + base) & 0xFFF;
                fdata[idx] = fval * factor + 0.5f;
            } else {
                data[idx] = (val / 2 - base) & 0xFFF;
                fdata[idx] = fval / factor - 0.5f;
            }
            
            /* Update state carried across iterations */
            state = (state + data[idx]) & 0xFF;
            
            /* Create anti-dependency chain */
            int temp = data[idx];
            data[idx] = temp ^ state;
        }
        
        /* Outer loop update with inner loop result */
        base = (base + state) & 0xFF;
    }
}

int main(void) {
    const int SIZE = 1024;
    const int ROWS = 32;
    const int COLS = 32;
    
    /* Initialize arrays with pseudo-random data */
    int array1[SIZE];
    int array2[SIZE];
    float farray[SIZE];
    int matrix[ROWS * COLS];
    float fmatrix[ROWS * COLS];
    
    printf("Initializing arrays with pseudo-random data...\n");
    
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (int)(lcg_rand() % 1000);
        array2[i] = (int)(lcg_rand() % 1000);
        farray[i] = (float)(lcg_rand() % 1000) * 0.01f;
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        matrix[i] = (int)(lcg_rand() % 1000);
        fmatrix[i] = (float)(lcg_rand() % 1000) * 0.01f;
    }
    
    /* Volatile flag to prevent optimization */
    volatile int vflag = 1;
    volatile int limit = 8;
    
    /* Call test functions multiple times with runtime decisions */
    int checksum = 0;
    
    for (int iteration = 0; iteration < 3; iteration++) {
        printf("Iteration %d: Running scheduler tests...\n", iteration + 1);
        
        /* Runtime decision based on volatile flag */
        if (vflag) {
            for (int rep = 0; rep < 2; rep++) {
                test_complex_schedule(array1, array2, farray, SIZE, &vflag);
            }
        }
        
        /* Alternate between test functions */
        if (iteration & 1) {
            test_volatile_barriers(array1, SIZE, limit);
            limit = (limit * 5 + 3) & 0xF;
        } else {
            test_outer_carried_state(matrix, fmatrix, ROWS, COLS);
        }
        
        /* Toggle volatile flag */
        vflag = !vflag;
    }
    
    /* Compute final checksum to prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        checksum += array1[i] + array2[i] + (int)farray[i];
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        checksum += matrix[i] + (int)fmatrix[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Selective scheduler test completed.\n");
    
    return 0;
}
