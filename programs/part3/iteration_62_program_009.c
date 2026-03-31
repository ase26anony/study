/* sel-sched-trigger.c - Program to trigger GCC selective scheduler debugging dumps */
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
void test_selective_sched_1(int* restrict arr1, int* restrict arr2, 
                           float* restrict farr, int size, volatile int* vflag) {
    float sum_f = 0.0f;
    double prod_d = 1.0;
    int sum_i = 0;
    
    /* Outer loop with carried state */
    for (int outer = 0; outer < 4; outer++) {
        int base = (outer * 17) & 0xFF;
        float fbase = (float)base * 0.1f;
        
        /* Inner loop with high ILP and data-dependent branches */
        for (int i = 0; i < size; i++) {
            /* Multiple dependency types with mixed operations */
            int val1 = arr1[i] ^ base;
            float fval = farr[i] * fbase;
            double dval = (double)val1 * 0.5;
            
            /* Flow dependency chain */
            sum_i += val1;
            sum_f += fval;
            prod_d *= (dval + 1.0);
            
            /* Data-dependent conditional branch - unpredictable */
            if (sum_i & 0x100) {
                /* Anti-dependency: read arr2[i], write to arr1[i] */
                int temp = arr2[i];
                arr1[i] = temp + (sum_i & 0xFF);
                /* Output dependency on farr */
                farr[i] = (float)temp * 0.25f;
            }
            
            /* Manual unrolling (2 iterations) */
            if (i + 1 < size) {
                int val2 = arr1[i+1] | base;
                float fval2 = farr[i+1] / (fbase + 0.001f);
                
                sum_i -= val2;  /* Create anti-dependency on sum_i */
                sum_f -= fval2;
                
                /* Another conditional with different condition */
                if ((val2 * val1) > 1000) {
                    arr2[i+1] = sum_i & 0xFFFF;
                    asm volatile("" ::: "memory"); /* Scheduling barrier */
                }
            }
            
            /* Volatile check prevents over-optimization */
            if (*vflag & 0x1) {
                prod_d *= 0.99;
            }
        }
        
        /* Loop-carried dependency to next outer iteration */
        base = (sum_i & 0xFF) + outer;
    }
    
    /* Prevent dead code elimination */
    arr1[0] = (int)sum_f;
    arr2[0] = sum_i;
    farr[0] = (float)prod_d;
}

/* Second test with volatile counters and assembly barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
void test_selective_sched_2(double* darr, int* iarr, int size) {
    volatile int vcounter = size;
    double acc1 = 0.0, acc2 = 0.0, acc3 = 0.0;
    
    while (vcounter-- > 0) {
        int idx = size - vcounter - 1;
        
        /* Multiple parallel FP operations with dependencies */
        double x = darr[idx];
        double y = x * 1.6180339887;  /* Golden ratio */
        double z = y / 2.7182818284;  /* e */
        
        acc1 += x * y;
        acc2 += y * z;
        acc3 += z * x;
        
        /* Complex conditional with mixed types */
        int ival = iarr[idx];
        if ((ival & 0x3) == 0) {
            darr[idx] = acc1 - acc2;
            asm volatile("" ::: "memory");
        } else if ((ival & 0x3) == 1) {
            darr[idx] = acc2 - acc3;
            asm volatile("" ::: "memory");
        } else {
            darr[idx] = acc3 - acc1;
        }
        
        /* Create output dependency chain */
        iarr[idx] = (int)(acc1 + acc2 + acc3);
        
        /* Unrolled section */
        if (idx + 3 < size) {
            double t1 = darr[idx+1] * 0.5;
            double t2 = darr[idx+2] * 0.25;
            double t3 = darr[idx+3] * 0.125;
            
            acc1 += t1; acc2 += t2; acc3 += t3;
            asm volatile("" ::: "memory");
        }
    }
    
    /* Cross-iteration dependency */
    darr[0] = acc1 + acc2 + acc3;
}

/* Third test: nested loops with outer-loop carried state */
void test_nested_carried_state(int* arr, int rows, int cols) {
    int outer_state = 0;
    
    for (int r = 0; r < rows; r++) {
        /* Outer loop computation affects inner loop */
        int base = (outer_state + r * 7919) & 0x7FF;  /* Prime multiplier */
        float factor = (float)base / 1024.0f + 0.5f;
        
        for (int c = 0; c < cols; c++) {
            int idx = r * cols + c;
            
            /* Mixed operations with loop-carried dependency */
            int val = arr[idx];
            val = (val ^ base) + (int)(val * factor);
            
            /* Data-dependent branch */
            if (val > 1000000) {
                val = val >> 2;
                asm volatile("" ::: "memory");
            } else if (val < -1000000) {
                val = val << 1;
                asm volatile("" ::: "memory");
            }
            
            arr[idx] = val;
            
            /* Update carried state */
            outer_state = (outer_state + val) & 0xFFF;
        }
        
        /* Dependency between outer loop iterations */
        base = outer_state;
    }
}

int main(void) {
    const int SIZE = 1024;
    const int ROWS = 32;
    const int COLS = 32;
    
    /* Initialize arrays with non-uniform data */
    int arr1[SIZE], arr2[SIZE];
    float farr[SIZE];
    double darr[SIZE];
    int matrix[ROWS * COLS];
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (int)lcg_rand() - 0x40000000;
        arr2[i] = (int)lcg_rand() - 0x40000000;
        farr[i] = (float)lcg_rand() / 1000.0f;
        darr[i] = (double)lcg_rand() / 1000.0;
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        matrix[i] = (int)lcg_rand() & 0xFFFF;
    }
    
    /* Volatile flags for runtime variability */
    volatile int vflag1 = lcg_rand() & 0x3;
    volatile int vflag2 = lcg_rand() & 0x3;
    
    /* Call test functions with runtime-decided repetitions */
    if (vflag1 & 0x1) {
        for (int rep = 0; rep < 5; rep++) {
            test_selective_sched_1(arr1, arr2, farr, SIZE, &vflag1);
        }
    }
    
    if (vflag2 & 0x2) {
        for (int rep = 0; rep < 3; rep++) {
            test_selective_sched_2(darr, arr1, SIZE);
        }
    }
    
    /* Always run nested test */
    test_nested_carried_state(matrix, ROWS, COLS);
    
    /* Compute final checksum to prevent elimination */
    uint64_t checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += (uint32_t)arr1[i];
        checksum += (uint32_t)arr2[i];
        checksum += (uint32_t)farr[i];
        checksum += (uint64_t)darr[i];
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        checksum += (uint32_t)matrix[i];
    }
    
    printf("Final checksum: %llu\n", (unsigned long long)checksum);
    return 0;
}
