/* sel-sched-trigger.c
 * Program designed to trigger GCC selective scheduler debugging dumps
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-dump -std=gnu11 sel-sched-trigger.c -o sel-sched-trigger
 */

#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 12345;
static inline uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Function 1: Complex nested loops with data-dependent branches
 * Force selective scheduling with attribute */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-dump")))
static void test_function_1(int* restrict a, int* restrict b, float* restrict c, 
                           int size, volatile int threshold) {
    float sum = 0.0f;
    float factor = 1.5f;
    
    /* Outer loop with carried state */
    for (int j = 0; j < 4; j++) {
        int base = (j * 17) & 0xFF;  /* Non-trivial base computation */
        
        /* Inner loop with mixed operations and data-dependent branches */
        for (int i = 0; i < size; i++) {
            /* Multiple dependency types */
            float temp = (float)a[i] * factor + (float)base;
            
            /* Flow dependency */
            sum = sum + temp;
            
            /* Anti dependency on sum */
            if (sum > (float)threshold) {
                /* Output dependency on c[i] */
                c[i] = sum;
                sum = 0.0f;
            }
            
            /* Control dependency with unpredictable branch */
            if (a[i] & 0x1) {
                b[i] = b[i] * 3 + base;
            } else {
                b[i] = b[i] / 2 - base;
            }
            
            /* Manual unrolling - 2 iterations */
            if (i + 1 < size) {
                float temp2 = (float)a[i+1] * factor * 0.5f + (float)base;
                sum = sum + temp2;
                if (sum > (float)(threshold * 2)) {
                    c[i+1] = sum * 0.5f;
                    sum = sum * 0.25f;
                }
                i++;  /* Skip next iteration */
            }
        }
        
        /* Inline assembly barrier to create scheduling boundary */
        asm volatile("" ::: "memory");
    }
}

/* Function 2: Volatile counters and assembly barriers */
static void test_function_2(double* restrict arr1, double* restrict arr2, 
                           int size, volatile int flag) {
    volatile int counter = 0;
    double acc = 0.0;
    
    for (volatile int i = 0; i < size; i++) {
        /* Mixed floating-point operations with resource conflicts */
        double x = arr1[i];
        double y = arr2[i];
        
        /* Multiple FP operations that could compete for FP units */
        double t1 = x * y;
        double t2 = x / (y + 1.0);
        double t3 = t1 - t2;
        double t4 = t1 + t2;
        
        /* Data-dependent conditional with volatile */
        if (counter++ & flag) {
            arr1[i] = t3 * t4;
            acc += t3;
        } else {
            arr2[i] = t4 - t3;
            acc -= t4;
        }
        
        /* Assembly barrier every 8 iterations */
        if ((i & 0x7) == 0) {
            asm volatile("" ::: "memory");
        }
        
        /* Nested conditional with unpredictable pattern */
        if ((i ^ (i >> 3)) & 0x1) {
            double tmp = arr1[i] * 0.7071;
            arr2[i] = arr2[i] + tmp;
            asm volatile("" ::: "memory");  /* Additional barrier */
        }
    }
}

/* Function 3: Outer-loop carried state pattern */
static void test_function_3(int* restrict data, float* restrict floats, 
                           int outer, int inner) {
    int state = 0;
    
    for (int j = 0; j < outer; j++) {
        /* Compute base from outer loop with non-trivial arithmetic */
        int base = ((state * 13 + j * 17) ^ 0x55AA) & 0xFFF;
        float fbase = (float)base * 0.01f;
        
        /* Inner loop with complex dependency on outer loop state */
        for (int i = 0; i < inner; i++) {
            /* Multiple operations with different latencies */
            int idx = (i + base) % inner;
            
            /* Integer arithmetic with flow dependency */
            state = state + data[idx] * 3;
            
            /* Floating-point with anti-dependency */
            float old_float = floats[idx];
            floats[idx] = old_float * fbase + (float)state * 0.5f;
            
            /* Conditional store with output dependency */
            if (state & 0x100) {
                data[idx] = state ^ base;
            }
            
            /* Manual unrolling - 3 iterations */
            if (i + 2 < inner) {
                int idx2 = (i + 1 + base) % inner;
                int idx3 = (i + 2 + base) % inner;
                
                state = state ^ data[idx2];
                floats[idx2] = floats[idx2] * 0.9f + (float)state;
                
                if ((state + base) & 0x200) {
                    data[idx3] = data[idx3] * 2 - base;
                }
                
                i += 2;  /* Skip two iterations */
            }
        }
        
        /* Memory barrier between outer loop iterations */
        asm volatile("" ::: "memory");
    }
}

int main(void) {
    const int SIZE = 1024;
    const int OUTER = 8;
    const int INNER = 128;
    
    /* Initialize arrays with pseudo-random data */
    int array_a[SIZE];
    int array_b[SIZE];
    float array_c[SIZE];
    double array_d1[SIZE];
    double array_d2[SIZE];
    int array_data[INNER];
    float array_floats[INNER];
    
    /* Fill arrays with non-uniform, non-constant data */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (int)lcg_rand() % 1000;
        array_b[i] = (int)lcg_rand() % 1000;
        array_c[i] = (float)(lcg_rand() % 1000) * 0.001f;
        array_d1[i] = (double)(lcg_rand() % 1000) * 0.01;
        array_d2[i] = (double)(lcg_rand() % 1000) * 0.01;
    }
    
    for (int i = 0; i < INNER; i++) {
        array_data[i] = lcg_rand() % 256;
        array_floats[i] = (float)(lcg_rand() % 256) * 0.1f;
    }
    
    /* Volatile flag to prevent optimization of control flow */
    volatile int flag = 0;
    
    /* Compute initial checksum to use in volatile condition */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= array_a[i] ^ array_b[i] ^ (int)array_c[i];
    }
    flag = checksum & 1;
    
    /* Variable threshold to prevent constant propagation */
    volatile int threshold = 500 + (lcg_rand() % 500);
    
    /* Call test functions based on runtime conditions */
    if (flag) {
        for (int rep = 0; rep < 5; rep++) {
            test_function_1(array_a, array_b, array_c, SIZE, threshold);
        }
    }
    
    /* Always call function 2 */
    test_function_2(array_d1, array_d2, SIZE, flag);
    
    /* Call function 3 multiple times if flag is set */
    if (flag) {
        for (int rep = 0; rep < 3; rep++) {
            test_function_3(array_data, array_floats, OUTER, INNER);
        }
    } else {
        /* Alternative path */
        test_function_3(array_data, array_floats, OUTER/2, INNER);
    }
    
    /* Final checksum computation to prevent dead code elimination */
    int final_checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_checksum += array_a[i] + array_b[i] + (int)array_c[i];
        final_checksum ^= (int)array_d1[i] ^ (int)array_d2[i];
    }
    
    for (int i = 0; i < INNER; i++) {
        final_checksum += array_data[i] + (int)array_floats[i];
    }
    
    printf("Final checksum: %d\n", final_checksum);
    return 0;
}
