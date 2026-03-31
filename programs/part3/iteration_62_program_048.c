/* sel-sched-trigger.c
 * Designed to trigger GCC selective scheduler debugging dumps
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
void test_function_1(int* arr_a, int* arr_b, float* arr_c, int size, volatile int threshold) {
    float sum_f = 0.0f;
    double acc_d = 0.0;
    int counter = 0;
    
    /* Nested loops with data-dependent branches and mixed operations */
    for (int outer = 0; outer < 4; outer++) {
        int base = (outer * 17) & 0xFF;  /* Outer loop carried state */
        
        /* Manually unrolled inner loop with scheduling barriers */
        for (int i = 0; i < size - 3; i += 4) {
            /* First iteration - integer arithmetic with flow dependency */
            int temp1 = arr_a[i] * base + counter;
            asm volatile("" ::: "memory");  /* Scheduling barrier */
            
            /* Data-dependent conditional branch */
            if (temp1 & 1) {
                arr_b[i] = temp1 >> 1;
                sum_f += (float)temp1 * 0.5f;
            } else {
                arr_b[i] = temp1 * 3;
                sum_f -= (float)temp1 * 0.25f;
            }
            
            /* Second iteration - floating point with anti-dependency */
            float ftemp = arr_c[i + 1] * 2.0f + sum_f;
            asm volatile("" ::: "memory");
            
            /* Complex condition with output dependency */
            if (sum_f > (float)threshold && ftemp < 100.0f) {
                arr_c[i + 1] = ftemp;
                acc_d += (double)ftemp;
                counter++;
            }
            
            /* Third iteration - mixed types and potential resource conflict */
            double dtemp = (double)arr_a[i + 2] * 1.5 + acc_d;
            int itemp = arr_b[i + 2] + (int)dtemp;
            
            /* Another conditional with control dependency */
            if (itemp > threshold && itemp < threshold * 2) {
                arr_a[i + 2] = itemp & 0xFF;
                asm volatile("" ::: "memory");
            }
            
            /* Fourth iteration - more complex dependency chain */
            int final = arr_a[i + 3] + arr_b[i + 3] + (int)arr_c[i + 3];
            arr_c[i + 3] = (final & 1) ? final * 0.33f : final * 0.66f;
            
            /* Loop-carried dependency */
            counter = (counter + final) & 0x7F;
        }
        
        /* Dependency across outer loop iterations */
        base = (base + counter) & 0xFF;
    }
}

/* Function with volatile counters and assembly barriers */
void test_function_2(volatile int* data, int size) {
    volatile int v_counter = 0;
    volatile int v_limit = size / 2;
    
    for (volatile int i = 0; i < size; i++) {
        /* Create artificial resource conflicts */
        float f1 = (float)data[i] * 1.1f;
        float f2 = (float)data[i] * 2.2f;
        float f3 = (float)data[i] * 3.3f;
        
        asm volatile("" ::: "memory");  /* Force scheduling barrier */
        
        /* Data-dependent computation with multiple dependency types */
        int idx = i & 0x3F;
        data[idx] = (int)(f1 + f2 - f3);
        
        /* Unpredictable branch pattern */
        if (v_counter++ > v_limit) {
            data[idx] *= 2;
            v_limit = (v_limit + 1) & 0x1F;
            asm volatile("" ::: "memory");
        }
        
        /* More floating point operations to stress FPU scheduler */
        double d1 = (double)data[i] * 0.5;
        double d2 = d1 * d1;
        data[i] = (int)(d2 * 100.0);
    }
}

/* Outer loop carried state pattern */
void test_function_3(int* arr, int outer_iter, int inner_size) {
    int state = 0;
    
    for (int j = 0; j < outer_iter; j++) {
        /* Compute base from outer loop state - creates loop-carried dependency */
        int base = (state * j + 0xABCD) & 0xFFF;
        float factor = 1.0f + (float)(j & 0xF) * 0.1f;
        
        /* Inner loop with dependency on outer loop state */
        for (int i = 0; i < inner_size - 1; i += 2) {
            /* Two iterations processed together for more ILP */
            int val1 = arr[i] + base;
            int val2 = arr[i + 1] + base;
            
            /* Mixed operations creating output dependencies */
            arr[i] = (int)((float)val1 * factor);
            arr[i + 1] = (int)((float)val2 * factor * 1.5f);
            
            /* Update state with data-dependent computation */
            state = (state + (arr[i] & 0x3) - (arr[i + 1] & 0x1)) & 0xFF;
            
            /* Conditional that depends on evolving state */
            if (state > 128) {
                arr[i] >>= 1;
                state >>= 1;
            }
        }
        
        /* Cross-iteration dependency */
        base = (base + state) & 0xFFF;
    }
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    int array_a[ARRAY_SIZE];
    int array_b[ARRAY_SIZE];
    float array_c[ARRAY_SIZE];
    
    /* Initialize with pseudo-random but bounded values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array_a[i] = (int)(lcg_rand() % 1000);
        array_b[i] = (int)(lcg_rand() % 1000);
        array_c[i] = (float)(lcg_rand() % 1000) * 0.1f;
    }
    
    volatile int threshold = 500;
    volatile int flag = 0;
    
    /* Compute initial checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += array_a[i] + array_b[i] + (int)array_c[i];
    }
    flag = (checksum > 0) ? 1 : 0;
    
    /* Variable control flow to prevent static optimization */
    if (flag) {
        for (int rep = 0; rep < 5; rep++) {
            test_function_1(array_a, array_b, array_c, ARRAY_SIZE, threshold);
        }
    }
    
    /* More runtime variability */
    volatile int dynamic_size = ARRAY_SIZE - (lcg_rand() % 100);
    test_function_2(array_b, dynamic_size);
    
    /* Nested loop pattern */
    test_function_3(array_a, 8, ARRAY_SIZE);
    
    /* Final checksum computation and output */
    checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += array_a[i] * 3 + array_b[i] * 5 + (int)(array_c[i] * 7);
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    return 0;
}
