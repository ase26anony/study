/* sel-sched-trigger.c - Program to trigger GCC selective scheduler debugging dumps */
#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static uint32_t lcg_seed = 12345;
static uint32_t lcg_rand(void) {
    lcg_seed = (lcg_seed * 1103515245 + 12345) & 0x7fffffff;
    return lcg_seed;
}

/* Initialize arrays with pseudo-random data */
static void init_arrays(int *a, int *b, float *fa, double *db, int size) {
    for (int i = 0; i < size; i++) {
        a[i] = (int)(lcg_rand() % 1000) - 500;
        b[i] = (int)(lcg_rand() % 1000) - 500;
        fa[i] = (float)(lcg_rand() % 1000) / 100.0f - 5.0f;
        db[i] = (double)(lcg_rand() % 1000) / 100.0 - 5.0;
    }
}

/* Test function 1: Complex nested loops with data-dependent branches */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-dump")))
static int test_function_1(int *a, int *b, float *fa, double *db, int size) {
    volatile int outer_volatile = size / 4;  /* Prevent constant propagation */
    int sum = 0;
    float fsum = 0.0f;
    double dsum = 0.0;
    
    /* Outer loop with carried state */
    for (int j = 0; j < outer_volatile; j++) {
        int base = (j * 37) & 0xFF;  /* Data-dependent base calculation */
        
        /* Inner loop with mixed operations and data-dependent branches */
        for (int i = 0; i < size; i++) {
            /* Multiple dependency types in one expression */
            int idx = (i + base) % size;
            
            /* Flow dependency: sum depends on previous sum */
            sum = sum + a[idx] * b[idx];
            
            /* Anti dependency: a[idx] read before write */
            a[idx] = a[idx] + (sum >> 3);
            
            /* Output dependency: fsum written multiple times */
            fsum = fsum + fa[idx] * 1.5f;
            
            /* Data-dependent conditional branch - unpredictable */
            if (sum & 0x1) {  /* Odd sum */
                dsum = dsum + db[idx] * 2.0;
                if (dsum > 1000.0) {
                    dsum = dsum * 0.5;
                    /* Inline asm barrier creates scheduling boundary */
                    asm volatile("" ::: "memory");
                }
            } else {  /* Even sum */
                dsum = dsum - db[idx] * 0.5;
                if (dsum < -1000.0) {
                    dsum = 0.0;
                }
            }
            
            /* Manual unrolling (2 iterations) */
            if (i + 1 < size) {
                int idx2 = (i + 1 + base) % size;
                sum = sum - (a[idx2] & b[idx2]);
                fsum = fsum - fa[idx2] * 0.75f;
                i++;  /* Increment counter for unrolled iteration */
            }
        }
        
        /* Outer loop modification affects inner loop */
        base = (base * 3 + 1) & 0x7F;
    }
    
    return sum + (int)fsum + (int)dsum;
}

/* Test function 2: Volatile counters and inline assembly barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
static int test_function_2(int *a, int *b, int size) {
    volatile int v_limit = size;  /* Volatile prevents optimization */
    int result = 0;
    int temp[4] = {0};  /* Small array for register pressure */
    
    for (volatile int v_i = 0; v_i < v_limit; v_i++) {
        int i = v_i;  /* Convert to non-volatile for computation */
        
        /* Complex expression with multiple operations */
        int val1 = a[i] * 3 + b[i] * 7;
        int val2 = (a[i] << 2) | (b[i] & 0xF);
        
        /* Inline assembly acts as scheduling barrier */
        asm volatile("" ::: "memory");
        
        /* Data-dependent array indexing */
        int idx = (val1 + i) % 4;
        temp[idx] = temp[idx] + val2;
        
        /* Nested conditional with floating point */
        if (val1 > 0) {
            float fval = (float)val1 * 0.25f;
            if (fval > 100.0f) {
                result += (int)fval;
                /* Another barrier inside conditional path */
                asm volatile("" ::: "memory");
            }
        }
        
        /* Loop with carried dependency chain */
        for (int k = 0; k < 3; k++) {
            temp[k] = temp[k] * 2 - temp[(k + 1) % 3];
        }
    }
    
    /* Aggregate results */
    for (int i = 0; i < 4; i++) {
        result += temp[i];
    }
    
    return result;
}

/* Test function 3: Outer loop carried state pattern */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
static int test_function_3(int *arr, int size, int outer_iters) {
    int state = 0;
    int checksum = 0;
    
    for (int j = 0; j < outer_iters; j++) {
        /* Outer loop modifies state used in inner loop */
        state = (state * 13 + j) & 0xFFF;
        int factor = (state >> 4) + 1;
        
        /* Inner loop with state-dependent computation */
        for (int i = 0; i < size; i++) {
            /* Multiple updates create complex dependency graph */
            int old = arr[i];
            arr[i] = (arr[i] + state) * factor;
            
            /* Conditional with floating conversion */
            if (arr[i] > 1000) {
                float f = (float)arr[i] / (float)factor;
                arr[i] = (int)f;
                checksum += arr[i];
            }
            
            /* Anti-dependency: old read after arr[i] write */
            checksum += old - arr[i];
            
            /* Manual unrolling - 4 iterations */
            if (i + 3 < size) {
                arr[i+1] = (arr[i+1] + state/2) * (factor + 1);
                arr[i+2] = (arr[i+2] + state/3) * (factor + 2);
                arr[i+3] = (arr[i+3] + state/4) * (factor + 3);
                i += 3;
            }
        }
        
        /* Modify state for next iteration */
        state = (state ^ checksum) & 0xFFF;
    }
    
    return checksum;
}

int main(void) {
    const int SIZE = 1024;
    const int OUTER_ITERS = 8;
    
    /* Allocate and initialize arrays */
    int array1[SIZE], array2[SIZE];
    float farray[SIZE];
    double darray[SIZE];
    
    init_arrays(array1, array2, farray, darray, SIZE);
    
    /* Volatile flag for runtime variability */
    volatile int volatile_flag = (lcg_rand() & 1);
    
    int total_checksum = 0;
    
    /* Call test functions based on volatile condition */
    if (volatile_flag) {
        for (int rep = 0; rep < 10; rep++) {
            total_checksum += test_function_1(array1, array2, farray, darray, SIZE);
        }
    } else {
        total_checksum += test_function_1(array1, array2, farray, darray, SIZE);
    }
    
    /* Always call function 2 */
    for (int rep = 0; rep < 5; rep++) {
        total_checksum += test_function_2(array1, array2, SIZE);
    }
    
    /* Call function 3 with different parameters */
    total_checksum += test_function_3(array1, SIZE, OUTER_ITERS);
    total_checksum += test_function_3(array2, SIZE, OUTER_ITERS / 2);
    
    /* Final checksum to prevent dead code elimination */
    int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += array1[i] + array2[i] + (int)farray[i] + (int)darray[i];
    }
    
    printf("Total checksum: %d, Final array sum: %d\n", total_checksum, final_sum);
    
    return 0;
}
