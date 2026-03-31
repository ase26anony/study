/* sel_sched_trigger.c - Program to trigger GCC selective scheduler debugging dumps */
#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 12345;
static inline uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Initialize arrays with pseudo-random data */
static void init_arrays(int *a, int *b, float *c, double *d, int size) {
    for (int i = 0; i < size; i++) {
        a[i] = (int)(lcg_rand() % 1000) - 500;
        b[i] = (int)(lcg_rand() % 1000) - 500;
        c[i] = (float)(lcg_rand() % 1000) / 10.0f;
        d[i] = (double)(lcg_rand() % 1000) / 10.0;
    }
}

/* Test function 1: Complex nested loops with data-dependent branches
   Force selective scheduling with attribute */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-dump")))
static void test_function_1(int *a, int *b, float *c, double *d, int size) {
    volatile int outer_counter = 3; /* Prevent constant propagation */
    double accumulator = 0.0;
    float threshold = 250.0f;
    
    /* Outer loop with carried state */
    for (int j = 0; j < outer_counter; j++) {
        int base = (j * 17) & 0xFF; /* Compute base from outer loop */
        
        /* Inner loop with mixed operations and data-dependent branches */
        for (int i = 0; i < size; i++) {
            /* Create flow dependency */
            accumulator += (double)a[i] * (double)b[i];
            
            /* Anti-dependency through accumulator reuse */
            float temp_f = c[i] * (float)accumulator;
            
            /* Output dependency through array write */
            if (temp_f > threshold) {
                c[i] = temp_f;
                accumulator = accumulator * 0.5; /* Modify accumulator */
            }
            
            /* Control dependency with unpredictable branch */
            if ((a[i] & 1) && (b[i] > 0)) {
                d[i] = d[i] + accumulator * 0.1;
                /* Inline asm barrier to prevent reordering */
                asm volatile("" ::: "memory");
            }
            
            /* Manual unrolling (2 iterations) */
            if (i + 1 < size) {
                int idx = i + 1;
                accumulator += (double)a[idx] * (double)b[idx] * 0.5;
                if ((a[idx] & 2) && (b[idx] < 0)) {
                    d[idx] = d[idx] - accumulator * 0.05;
                }
            }
            
            /* Loop-carried dependency with outer loop state */
            a[i] = (a[i] + base) * ((j % 3) + 1);
        }
        
        /* Resource conflict: multiple FP operations */
        for (int i = 0; i < size; i += 4) {
            c[i] = c[i] * 1.1f + (float)base;
            c[i+1] = c[i+1] * 1.2f - (float)base;
            c[i+2] = c[i+2] * 0.9f + (float)(base * 2);
            c[i+3] = c[i+3] * 0.8f - (float)(base * 2);
        }
    }
}

/* Test function 2: Volatile counters and inline assembly barriers */
static void test_function_2(int *a, float *c, int size) {
    volatile int v_limit = size; /* Volatile to prevent optimization */
    float sum = 0.0f;
    
    for (volatile int v_i = 0; v_i < v_limit; v_i++) {
        int i = v_i; /* Convert to non-volatile for array access */
        
        /* Complex dependency chain */
        float x = c[i] * 2.0f;
        float y = x + (float)a[i];
        float z = y - x * 0.5f;
        
        /* Memory barrier forcing scheduler to work harder */
        asm volatile("" ::: "memory");
        
        /* Conditional with side effects */
        if (z > 100.0f) {
            c[i] = z;
            sum += z;
            /* Another barrier inside conditional */
            asm volatile("" ::: "memory");
        } else if (z < -100.0f) {
            c[i] = z * 0.5f;
            sum -= z;
        } else {
            c[i] = 0.0f;
        }
        
        /* Nested condition with unpredictable pattern */
        if ((a[i] % 7) == 0) {
            sum = sum * 1.1f;
            asm volatile("" ::: "memory");
        }
    }
}

/* Test function 3: Outer-loop carried state pattern */
static void test_function_3(int *a, int *b, double *d, int size) {
    const int OUTER = 4;
    const int INNER = size / 4;
    
    for (int j = 0; j < OUTER; j++) {
        /* Outer loop modifies state used in inner loop */
        int base = (j * 13 + 7) & 0x3F;
        double factor = 1.0 + (j * 0.1);
        
        /* Inner loop with loop-carried dependencies */
        double carry = 0.0;
        for (int i = 0; i < INNER; i++) {
            int idx = j * INNER + i;
            if (idx >= size) break;
            
            /* Flow dependency through carry */
            carry = carry + (double)a[idx] * factor;
            
            /* Anti-dependency through array reuse */
            double old_d = d[idx];
            d[idx] = carry + (double)base;
            
            /* Output dependency through b[] */
            b[idx] = (int)(old_d - carry);
            
            /* Complex condition with data-dependent branch */
            if ((a[idx] + b[idx]) > (base * 2)) {
                carry = carry * 0.8;
                asm volatile("" ::: "memory");
            }
            
            /* Manual unrolling */
            if (i + 1 < INNER) {
                int idx2 = j * INNER + (i + 1);
                if (idx2 < size) {
                    carry = carry + (double)a[idx2] * factor * 0.5;
                    d[idx2] = d[idx2] + carry;
                }
            }
        }
    }
}

/* Compute checksum to prevent dead code elimination */
static int compute_checksum(int *a, int *b, float *c, double *d, int size) {
    int checksum = 0;
    for (int i = 0; i < size; i++) {
        checksum += a[i] ^ (i * 3);
        checksum += b[i] ^ (i * 5);
        checksum += (int)(c[i] * 100);
        checksum += (int)(d[i] * 100);
    }
    return checksum;
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    int array_a[ARRAY_SIZE];
    int array_b[ARRAY_SIZE];
    float array_c[ARRAY_SIZE];
    double array_d[ARRAY_SIZE];
    
    /* Initialize with pseudo-random data */
    init_arrays(array_a, array_b, array_c, array_d, ARRAY_SIZE);
    
    /* Volatile flag to introduce runtime variability */
    volatile int volatile_flag = (lcg_rand() % 10) > 3;
    
    /* Call test functions based on volatile condition */
    if (volatile_flag) {
        for (int rep = 0; rep < 5; rep++) {
            test_function_1(array_a, array_b, array_c, array_d, ARRAY_SIZE);
        }
    }
    
    /* Always call function 2 */
    for (int rep = 0; rep < 3; rep++) {
        test_function_2(array_a, array_c, ARRAY_SIZE);
    }
    
    /* Call function 3 based on another volatile condition */
    volatile int another_flag = (lcg_rand() % 10) > 5;
    if (another_flag) {
        for (int rep = 0; rep < 2; rep++) {
            test_function_3(array_a, array_b, array_d, ARRAY_SIZE);
        }
    }
    
    /* Compute and print final checksum */
    int final_checksum = compute_checksum(array_a, array_b, array_c, array_d, ARRAY_SIZE);
    printf("Final checksum: %d\n", final_checksum);
    
    return 0;
}
