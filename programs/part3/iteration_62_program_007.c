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
/* Force selective scheduling with attribute */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-dump")))
static void test_complex_schedule(int *arr1, int *arr2, float *farr, int size, int threshold) {
    volatile int outer_volatile = threshold; /* Prevent constant propagation */
    int sum = 0;
    float fsum = 0.0f;
    
    /* Outer loop with carried state */
    for (int j = 0; j < outer_volatile % 8; j++) {
        int base = (j * 37) & 0xFF;
        
        /* Inner loop with mixed operations and data-dependent branches */
        for (int i = 0; i < size; i++) {
            /* Create flow dependency */
            int val1 = arr1[i] + base;
            
            /* Create anti dependency */
            arr1[i] = val1 * 3;
            
            /* Data-dependent branch - unpredictable pattern */
            if (val1 & 1) {
                /* Multiple operations in taken path */
                float fval = farr[i] * 2.5f;
                fsum += fval;
                arr2[i] = (arr2[i] + (int)fval) & 0xFFFF;
                
                /* Inline asm barrier to prevent reordering */
                asm volatile("" ::: "memory");
            } else {
                /* Different operations in not-taken path */
                double dval = (double)arr2[i] * 0.75;
                sum += (int)dval;
                farr[i] = (float)(dval * 0.5);
            }
            
            /* Manual unrolling - 2 iterations */
            if (i + 1 < size) {
                int val2 = arr1[i + 1] ^ base;
                arr1[i + 1] = val2 - 7;
                
                /* Another conditional with different condition */
                if ((val2 % 3) == 0) {
                    farr[i + 1] = farr[i + 1] * 1.1f;
                    asm volatile("" ::: "memory");
                }
                i++; /* Skip the unrolled element */
            }
        }
        
        /* Loop-carried output dependency */
        base = (base + sum) & 0x3FF;
        asm volatile("" ::: "memory");
    }
    
    /* Prevent dead code elimination */
    arr1[0] = sum;
    farr[0] = fsum;
}

/* Function 2: Volatile counters and inline assembly barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining", "funroll-loops")))
static void test_volatile_barriers(int *arr, float *farr, int size) {
    volatile int vcounter = size;
    int acc1 = 0, acc2 = 0;
    float facc1 = 0.0f, facc2 = 0.0f;
    
    /* Loop with volatile termination condition */
    for (volatile int i = 0; i < vcounter; i = i + 1) {
        /* Multiple independent chains with barriers */
        int idx = i & (size - 1);
        
        /* Chain 1: Integer operations */
        int temp1 = arr[idx] * 3;
        asm volatile("" ::: "memory");
        temp1 = temp1 + 7;
        acc1 ^= temp1;
        
        /* Chain 2: Floating point operations */
        float ftemp = farr[idx] * 1.5f;
        asm volatile("" ::: "memory");
        ftemp = ftemp - 0.25f;
        facc1 += ftemp;
        
        /* Chain 3: Mixed operations */
        if (acc1 > 1000) {
            double dtemp = (double)acc1 * 0.01;
            arr[idx] = (int)(dtemp * ftemp);
            asm volatile("" ::: "memory");
        }
        
        /* Chain 4: Another dependency chain */
        acc2 = acc2 * 2 + arr[idx];
        facc2 = facc2 * 0.9f + farr[idx];
        
        /* Nested conditional with unpredictable pattern */
        if ((lcg_rand() & 0x3F) == 0) {
            asm volatile("" ::: "memory");
            arr[idx] = acc1 + acc2;
            farr[idx] = facc1 + facc2;
        }
    }
    
    /* Store results to prevent elimination */
    arr[size-1] = acc1 + acc2;
    farr[size-1] = facc1 + facc2;
}

/* Function 3: Outer-loop carried state pattern */
static void test_outer_carried_state(int *arr1, int *arr2, int size) {
    int outer_state = 0;
    
    /* Outer loop modifies state used in inner loop */
    for (int outer = 0; outer < 4; outer++) {
        /* Compute base from outer state - creates dependency */
        int base = (outer_state * 17 + outer * 13) & 0xFF;
        outer_state = base;
        
        /* Inner loop with complex dependency on base */
        for (int i = 0; i < size; i++) {
            /* Multiple dependency types */
            int val = arr1[i];
            
            /* Flow dependency chain */
            val = val + base;
            val = val * 3;
            val = val - 7;
            
            /* Output dependency */
            arr1[i] = val;
            
            /* Anti dependency */
            int old_val = arr2[i];
            arr2[i] = (old_val + val) & 0xFFFF;
            
            /* Control dependency with data-dependent condition */
            if ((val + old_val) > 1000) {
                /* More operations in taken path */
                arr1[i] = arr1[i] / 2;
                asm volatile("" ::: "memory");
                
                /* Nested condition */
                if ((val & 0xF) == 0) {
                    arr2[i] = arr2[i] ^ 0xAAAA;
                }
            }
            
            /* Manual unrolling - 3 iterations */
            if (i + 2 < size) {
                /* Different operations for unrolled iterations */
                arr1[i+1] = (arr1[i+1] * base) & 0xFFF;
                arr2[i+1] = arr2[i+1] + (base >> 2);
                
                arr1[i+2] = arr1[i+2] ^ base;
                arr2[i+2] = arr2[i+2] - (base & 0xF);
                
                i += 2; /* Skip unrolled elements */
            }
        }
        
        /* Modify outer state for next iteration */
        outer_state = (outer_state + arr1[size-1]) & 0x3FF;
        asm volatile("" ::: "memory");
    }
}

int main(void) {
    const int SIZE = 1024;
    int array1[SIZE];
    int array2[SIZE];
    float farray[SIZE];
    
    /* Initialize with pseudo-random but bounded values */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (int)(lcg_rand() & 0xFFF);
        array2[i] = (int)(lcg_rand() & 0xFFF);
        farray[i] = (float)(lcg_rand() & 0xFF) / 256.0f;
    }
    
    /* Volatile flag to introduce runtime variability */
    volatile int flag = array1[0] & 0x3;
    
    /* Call test functions multiple times with runtime decisions */
    for (int rep = 0; rep < 3; rep++) {
        if (flag & 0x1) {
            test_complex_schedule(array1, array2, farray, SIZE, 100);
        }
        
        if (flag & 0x2) {
            for (int i = 0; i < 2; i++) {
                test_volatile_barriers(array1, farray, SIZE);
            }
        }
        
        /* Toggle flag based on array contents */
        flag = (array1[SIZE/2] + array2[SIZE/2]) & 0x3;
    }
    
    /* Always call the outer carried state test */
    test_outer_carried_state(array1, array2, SIZE);
    
    /* Compute final checksum to prevent dead code elimination */
    int checksum = 0;
    float fchecksum = 0.0f;
    for (int i = 0; i < SIZE; i++) {
        checksum ^= array1[i];
        checksum += array2[i];
        fchecksum += farray[i];
    }
    
    printf("Checksum: int=%d, float=%.2f\n", checksum, fchecksum);
    return 0;
}
