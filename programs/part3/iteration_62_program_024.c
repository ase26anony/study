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
void test_complex_schedule(int* arr1, int* arr2, float* farr, int size, int threshold) {
    volatile int vol_size = size;  /* Prevent constant propagation */
    int sum = 0;
    float fsum = 0.0f;
    
    /* Nested loops with data-dependent branches */
    for (int j = 0; j < 4; j++) {  /* Outer loop */
        int base = (j * 17) & 0xFF;  /* Outer loop carried state */
        
        /* Manually unrolled inner loop with mixed operations */
        for (int i = 0; i < vol_size - 3; i += 4) {
            /* First iteration - integer arithmetic with barrier */
            int val1 = arr1[i] * base;
            asm volatile("" ::: "memory");  /* Scheduling barrier */
            sum += val1;
            
            /* Data-dependent branch */
            if (sum > threshold) {
                arr2[i] = sum;
                sum = 0;
            }
            
            /* Second iteration - floating point */
            float fval = farr[i + 1] * 1.5f;
            fsum += fval;
            asm volatile("" ::: "memory");
            
            /* Third iteration - mixed types with condition */
            int val3 = arr1[i + 2] & 0x7F;
            if (val3 & 1) {  /* Unpredictable branch */
                farr[i + 2] = fsum;
                fsum = fval * 0.5f;
            }
            
            /* Fourth iteration - complex dependency chain */
            int val4 = arr1[i + 3] + arr2[i + 3];
            arr2[i + 3] = val4 * base;
            sum += val4;
            
            /* Another barrier to create scheduling regions */
            asm volatile("" ::: "memory");
            
            /* Additional control flow with volatile check */
            volatile int check = (i & 0xF);
            if (check == 0) {
                fsum = fsum * 0.9f;
            }
        }
    }
    
    /* Store final results to prevent elimination */
    if (size > 0) {
        arr1[0] = sum;
        farr[0] = fsum;
    }
}

/* Function with volatile counters and assembly barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining")))
void test_volatile_barriers(double* darr, int* iarr, int size) {
    volatile int v_start = 0;
    volatile int v_end = size;
    double accum = 0.0;
    
    for (volatile int vi = v_start; vi < v_end; vi = vi + 1) {
        int idx = vi;
        
        /* Create multiple dependency types */
        double temp = darr[idx] * 2.0;
        asm volatile("" ::: "memory");  /* Flow dependency barrier */
        
        iarr[idx] = (int)temp;
        accum += temp;
        
        /* Anti-dependency: read after write with barrier */
        asm volatile("" ::: "memory");
        double read_back = darr[idx];
        
        /* Output dependency: same memory location */
        darr[idx] = accum + read_back;
        
        /* Complex condition with volatile */
        volatile int mod_check = idx % 8;
        if (mod_check == 0) {
            accum = accum * 0.5;
            asm volatile("" ::: "memory");
        } else if (mod_check == 1) {
            iarr[idx] = iarr[idx] ^ 0xAAAA;
        }
        
        /* Nested short loop for additional ILP */
        for (int k = 0; k < 2; k++) {
            double ktemp = darr[idx] * (k + 1);
            accum += ktemp;
            asm volatile("" ::: "memory");
        }
    }
    
    /* Prevent dead code elimination */
    if (size > 0) {
        darr[0] = accum;
    }
}

/* Outer loop carried state pattern */
__attribute__((optimize("O2", "fsel-sched-pipelining-outer-loops")))
void test_outer_carried_state(int* data, int rows, int cols) {
    int state = 0;
    
    for (int r = 0; r < rows; r++) {
        /* Outer loop modifies state used in inner loop */
        int base = (state + r * 3) & 0xFF;
        int factor = 1 + (r & 0x3);
        
        /* Inner loop with carried dependency */
        for (int c = 0; c < cols; c++) {
            int idx = r * cols + c;
            
            /* Multiple operations with dependencies */
            int val = data[idx];
            val = (val + base) * factor;
            
            /* Data-dependent update */
            if (val > 1000) {
                state = val & 0xFF;
                val = val / 2;
            }
            
            /* Store with potential anti-dependency */
            data[idx] = val;
            
            /* Insert barrier every 8 iterations */
            if ((c & 0x7) == 0) {
                asm volatile("" ::: "memory");
            }
        }
        
        /* Modify state for next outer iteration */
        state = (state + base) & 0xFF;
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
    double darray[SIZE];
    
    printf("Initializing arrays with pseudo-random data...\n");
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (int)lcg_rand() % 1000;
        array2[i] = (int)lcg_rand() % 1000;
        farray[i] = (float)(lcg_rand() % 1000) / 10.0f;
        darray[i] = (double)(lcg_rand() % 1000) / 10.0;
    }
    
    int matrix[ROWS * COLS];
    for (int i = 0; i < ROWS * COLS; i++) {
        matrix[i] = lcg_rand() % 500;
    }
    
    /* Volatile flag for runtime variability */
    volatile int flag = 0;
    int checksum = 0;
    
    /* Call test functions multiple times with volatile control */
    for (int iteration = 0; iteration < 5; iteration++) {
        flag = (iteration & 1);
        
        if (flag) {
            for (int rep = 0; rep < 3; rep++) {
                test_complex_schedule(array1, array2, farray, SIZE, 5000);
            }
        }
        
        test_volatile_barriers(darray, array1, SIZE);
        
        if (!flag || (iteration % 2 == 0)) {
            test_outer_carried_state(matrix, ROWS, COLS);
        }
    }
    
    /* Compute final checksum to prevent elimination */
    for (int i = 0; i < SIZE; i++) {
        checksum += array1[i] + array2[i] + (int)farray[i];
        checksum += (int)darray[i];
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        checksum += matrix[i];
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("(This value varies based on scheduling decisions)\n");
    
    return 0;
}
