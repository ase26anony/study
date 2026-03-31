/* sel-sched-trigger.c
 * Program to trigger GCC selective scheduler debugging dumps
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-dump -std=gnu11 sel-sched-trigger.c -o sel-sched-trigger
 */

#include <stdio.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define OUTER_LOOP 8
#define INNER_LOOP 128

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 12345;
static inline uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Function 1: Complex nested loop with data-dependent branches
 * Force selective scheduling with attribute */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
static void test_function_1(int* restrict a, int* restrict b, float* restrict c, 
                           float* restrict d, int size) {
    volatile int threshold = 1000;  /* Prevent constant propagation */
    float sum_f = 0.0f;
    double sum_d = 0.0;
    
    /* Outer loop with carried state */
    for (int j = 0; j < OUTER_LOOP; j++) {
        int base = (j * 37) & 0xFF;  /* Varying base value */
        
        /* Inner loop with mixed operations and data-dependent branches */
        for (int i = 0; i < size; i++) {
            /* Multiple dependency types */
            int val_a = a[i];
            int val_b = b[i];
            float val_c = c[i];
            
            /* Flow dependency chain */
            sum_f += val_c * (val_a & 0xF) + (val_b >> 4);
            
            /* Anti dependency */
            c[i] = sum_f * 0.5f;
            
            /* Output dependency */
            d[i] = d[i] + sum_f;
            
            /* Data-dependent branch with unpredictable pattern */
            if ((val_a ^ val_b) & 0x1) {
                /* Complex operation on branch path */
                sum_d += (double)val_a * (double)val_b * 0.01;
                asm volatile("" ::: "memory");  /* Scheduling barrier */
            }
            
            /* Another conditional with resource conflict */
            if (sum_f > (float)threshold) {
                sum_f = sum_f * 0.25f;
                threshold = (threshold + base) & 0x3FF;
                asm volatile("" ::: "memory");
            }
            
            /* Manual unrolling - 2 iterations */
            if (i + 1 < size) {
                int val_a2 = a[i + 1];
                int val_b2 = b[i + 1];
                sum_f += (val_a2 & 0xF0) - (val_b2 & 0x0F);
                sum_d += (double)(val_a2 | val_b2) * 0.02;
            }
        }
        
        /* Loop-carried dependency across outer iterations */
        base = (base * 3 + 1) & 0xFF;
        asm volatile("" ::: "memory");
    }
    
    /* Prevent dead code elimination */
    a[0] = (int)sum_f;
    b[0] = (int)sum_d;
}

/* Function 2: Volatile counters and inline assembly barriers */
static void test_function_2(double* restrict arr1, double* restrict arr2, 
                           volatile int* restrict counter) {
    volatile int limit = *counter;  /* Volatile prevents optimization */
    double acc1 = 0.0, acc2 = 0.0;
    
    for (volatile int i = 0; i < limit; i = i + 1) {
        /* Multiple floating-point operations with dependencies */
        double temp1 = arr1[i] * 1.5;
        double temp2 = arr2[i] * 0.75;
        
        /* Create anti-dependency */
        arr1[i] = temp1 + temp2;
        arr2[i] = temp1 - temp2;
        
        /* Complex dependency chain */
        acc1 = acc1 + arr1[i] * arr2[i];
        acc2 = acc2 - arr1[i] / (arr2[i] + 1.0);
        
        /* Inline assembly barrier every 4 iterations */
        if ((i & 0x3) == 0) {
            asm volatile("" ::: "memory");
        }
        
        /* Data-dependent operation */
        if ((i ^ (i >> 2)) & 0x1) {
            acc1 = acc1 * 0.99;
            asm volatile("" ::: "memory");
        }
    }
    
    /* Cross-iteration dependency */
    arr1[0] = acc1;
    arr2[0] = acc2;
}

/* Function 3: Outer-loop carried state pattern */
static void test_function_3(int* restrict data, float* restrict accum, 
                           int outer_iter, int inner_size) {
    int state = 0;
    
    for (int outer = 0; outer < outer_iter; outer++) {
        /* Outer loop modifies state used in inner loop */
        int base = (state * 7 + outer * 13) & 0xFF;
        float factor = 1.0f + (outer & 0x3) * 0.25f;
        
        /* Inner loop with complex addressing */
        for (int inner = 0; inner < inner_size; inner += 2) {
            /* Unrolled by 2 with different operations */
            int idx1 = (inner + base) % inner_size;
            int idx2 = (inner + 1 + base) % inner_size;
            
            /* Mixed integer/float operations */
            int val1 = data[idx1];
            int val2 = data[idx2];
            
            /* Loop-carried dependency on accum */
            float old_acc = accum[inner % 16];
            accum[inner % 16] = old_acc + (val1 + val2) * factor;
            
            /* Modify data array with dependency */
            data[idx1] = val1 ^ (int)(accum[inner % 16]);
            data[idx2] = val2 & (int)(factor * 100.0f);
            
            /* Periodic scheduling barrier */
            if ((inner & 0x7) == 0) {
                asm volatile("" ::: "memory");
            }
        }
        
        /* Update outer loop state */
        state = (state + base) & 0xFF;
    }
}

int main(void) {
    /* Initialize arrays with pseudo-random data */
    int array_a[ARRAY_SIZE];
    int array_b[ARRAY_SIZE];
    float array_c[ARRAY_SIZE];
    float array_d[ARRAY_SIZE];
    double array_e[ARRAY_SIZE];
    double array_f[ARRAY_SIZE];
    
    printf("Initializing arrays with pseudo-random data...\n");
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array_a[i] = (int)lcg_rand();
        array_b[i] = (int)lcg_rand();
        array_c[i] = (float)(lcg_rand() % 1000) * 0.001f;
        array_d[i] = (float)(lcg_rand() % 1000) * 0.001f;
        array_e[i] = (double)(lcg_rand() % 1000) * 0.001;
        array_f[i] = (double)(lcg_rand() % 1000) * 0.001;
    }
    
    /* Volatile flag to introduce runtime variability */
    volatile int flag = array_a[0] & 0x1;
    
    /* Call test functions multiple times with varying patterns */
    printf("Running selective scheduler test patterns...\n");
    
    /* First test - force selective scheduling */
    if (flag) {
        for (int rep = 0; rep < 5; rep++) {
            test_function_1(array_a, array_b, array_c, array_d, ARRAY_SIZE / 2);
        }
    }
    
    /* Second test with volatile counter */
    volatile int counter = (array_b[0] % 50) + 10;
    for (int rep = 0; rep < 3; rep++) {
        test_function_2(array_e, array_f, &counter);
        counter = (counter * 3 + 1) % 100;
    }
    
    /* Third test with outer-loop carried state */
    if (!flag) {
        for (int rep = 0; rep < 4; rep++) {
            test_function_3(array_a, array_d, OUTER_LOOP, INNER_LOOP);
        }
    } else {
        test_function_3(array_b, array_c, OUTER_LOOP / 2, INNER_LOOP * 2);
    }
    
    /* Compute final checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += (uint64_t)array_a[i];
        checksum += (uint64_t)array_b[i];
        checksum += (uint64_t)(array_c[i] * 1000.0f);
        checksum += (uint64_t)(array_d[i] * 1000.0f);
        checksum += (uint64_t)(array_e[i] * 1000.0);
        checksum += (uint64_t)(array_f[i] * 1000.0);
    }
    
    printf("Final checksum: %llu\n", (unsigned long long)checksum);
    printf("Test completed. Check stderr for selective scheduler dumps.\n");
    
    return 0;
}
