/* sel-sched-trigger.c
 * Program designed to trigger GCC selective scheduler debugging dumps
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-dump -std=gnu11 sel-sched-trigger.c -o sel-sched-test
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

/* Function 1: Annotated to force selective scheduling with complex ILP patterns */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-dump")))
static void test_function_1(int *arr_a, int *arr_b, float *arr_c, volatile int *trigger) {
    float sum_f = 0.0f;
    double acc_d = 0.0;
    int sum_i = 0;
    
    /* Outer loop with carried state */
    for (int j = 0; j < OUTER_LOOP; j++) {
        int base = (*trigger + j) * 73;  /* Data-dependent base calculation */
        float threshold = (j % 3 == 0) ? 100.0f : 200.0f;
        
        /* Manually unrolled inner loop with mixed operations */
        for (int i = 0; i < INNER_LOOP; i += 4) {
            /* Unrolled iteration 1 */
            int idx1 = (i + 0) % ARRAY_SIZE;
            int val1 = arr_a[idx1] + base;
            sum_i += val1;
            arr_b[idx1] = val1 * 3;
            
            /* Complex floating-point operation with condition */
            sum_f += arr_c[idx1] * 0.5f;
            if (sum_f > threshold) {
                arr_c[idx1] = sum_f;
                sum_f = 0.0f;
                asm volatile("" ::: "memory");  /* Scheduling barrier */
            }
            
            /* Unrolled iteration 2 */
            int idx2 = (i + 1) % ARRAY_SIZE;
            int val2 = arr_a[idx2] ^ base;  /* Different operation */
            sum_i -= val2;
            arr_b[idx2] = val2 / 2;
            
            /* Double precision operation */
            acc_d += (double)val2 * 0.25;
            if (acc_d > 500.0) {
                acc_d = acc_d * 0.9;
            }
            
            /* Unrolled iteration 3 - with bitwise condition */
            int idx3 = (i + 2) % ARRAY_SIZE;
            if (arr_a[idx3] & 1) {  /* Data-dependent branch */
                arr_b[idx3] = arr_a[idx3] * 7;
                sum_i += arr_b[idx3];
            } else {
                arr_b[idx3] = arr_a[idx3] >> 1;
                sum_i -= arr_b[idx3];
            }
            
            /* Unrolled iteration 4 - mixed types and memory barrier */
            int idx4 = (i + 3) % ARRAY_SIZE;
            float temp_f = arr_c[idx4] + (float)base;
            arr_c[idx4] = temp_f;
            asm volatile("" ::: "memory");  /* Another scheduling barrier */
            
            /* Anti-dependency: read after write to same location */
            sum_i += arr_b[idx4];
            arr_b[idx4] = sum_i & 0xFF;
        }
        
        /* Loop-carried dependency across outer iterations */
        base = (base * 2) % 256;
    }
    
    /* Prevent dead code elimination */
    *trigger = sum_i + (int)sum_f + (int)acc_d;
}

/* Function 2: Uses volatile counters and inline assembly barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining-outer-loops")))
static void test_function_2(double *arr_d, int *arr_i, volatile int loop_limit) {
    volatile int v_counter = 0;
    double sum1 = 0.0, sum2 = 0.0;
    
    for (volatile int v = 0; v < loop_limit; v++) {
        int idx = v % ARRAY_SIZE;
        
        /* Multiple floating-point operations with potential resource conflicts */
        double temp1 = arr_d[idx] * 1.1;
        double temp2 = arr_d[(idx + 1) % ARRAY_SIZE] * 0.9;
        double temp3 = arr_d[(idx + 2) % ARRAY_SIZE] * 1.05;
        
        /* Create output dependencies */
        arr_d[idx] = temp1 + temp2;
        arr_d[(idx + 1) % ARRAY_SIZE] = temp2 - temp3;
        arr_d[(idx + 2) % ARRAY_SIZE] = temp1 * temp3;
        
        /* Integer operations mixed with FP */
        int int_val = arr_i[idx];
        if (int_val > 0) {
            sum1 += (double)int_val;
            asm volatile("" ::: "memory");  /* Barrier in conditional path */
        } else {
            sum2 += (double)(-int_val);
        }
        
        /* Complex condition with side effects */
        v_counter += (int_val & 3) - 1;
        
        /* Nested conditional with different operation types */
        if (v_counter > 10) {
            arr_i[idx] = (int)(sum1 - sum2);
            v_counter = 0;
            asm volatile("" ::: "memory");
        } else if (v_counter < -10) {
            arr_i[idx] = (int)(sum2 - sum1);
            v_counter = 5;
        }
    }
}

/* Function 3: Outer-loop carried state pattern */
static void test_function_3(int *arr_a, int *arr_b, int outer_iter) {
    int state = 0;
    
    for (int j = 0; j < outer_iter; j++) {
        /* Compute base from outer loop state */
        int base = (state * 13 + j * 17) % 97;
        int factor = 1 + (j % 7);
        
        /* Inner loop with data-dependent computation */
        for (int i = 0; i < INNER_LOOP; i++) {
            int idx = (i + j) % ARRAY_SIZE;
            
            /* Flow dependency through arr_a and arr_b */
            int val_a = arr_a[idx];
            int val_b = arr_b[idx];
            
            /* Complex expression with multiple dependencies */
            int result = (val_a + base) * factor - val_b;
            
            /* Anti-dependency: read old value before write */
            state += val_b;
            arr_b[idx] = result;
            
            /* Output dependency: write to arr_a */
            arr_a[idx] = state % 256;
            
            /* Control dependency */
            if (result > 1000) {
                factor = factor * 2 % 11;
                asm volatile("" ::: "memory");
            }
        }
        
        /* Outer loop update with conditional */
        state = (state + base) & 0xFFFF;
        if (j % 4 == 0) {
            state = state ^ 0x5555;
        }
    }
}

int main(void) {
    /* Initialize arrays with pseudo-random data */
    int array_a[ARRAY_SIZE];
    int array_b[ARRAY_SIZE];
    float array_c[ARRAY_SIZE];
    double array_d[ARRAY_SIZE];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array_a[i] = (int)lcg_rand() % 1000;
        array_b[i] = (int)lcg_rand() % 1000;
        array_c[i] = (float)(lcg_rand() % 1000) / 10.0f;
        array_d[i] = (double)(lcg_rand() % 1000) / 5.0;
    }
    
    /* Volatile flag to prevent optimization */
    volatile int run_flag = 1;
    volatile int loop_limit = 50;
    
    int checksum = 0;
    
    /* Runtime-variable control flow */
    if (run_flag) {
        for (int rep = 0; rep < 5; rep++) {
            test_function_1(array_a, array_b, array_c, &run_flag);
        }
    }
    
    /* More calls based on data-dependent condition */
    int temp_sum = 0;
    for (int i = 0; i < 100; i++) {
        temp_sum += array_a[i % ARRAY_SIZE];
    }
    
    if (temp_sum > 25000) {
        for (int rep = 0; rep < 3; rep++) {
            test_function_2(array_d, array_b, loop_limit);
        }
    } else {
        test_function_3(array_a, array_b, OUTER_LOOP / 2);
    }
    
    /* Alternate between patterns */
    volatile int alt_flag = 1;
    for (int cycle = 0; cycle < 4; cycle++) {
        if (alt_flag) {
            test_function_1(array_a, array_b, array_c, &run_flag);
            test_function_2(array_d, array_b, loop_limit + cycle);
        } else {
            test_function_3(array_a, array_b, OUTER_LOOP);
        }
        alt_flag = !alt_flag;
    }
    
    /* Compute final checksum to prevent dead code elimination */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += array_a[i] + array_b[i] + (int)array_c[i] + (int)array_d[i];
        checksum = (checksum * 31 + i) & 0x7FFFFFFF;
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
