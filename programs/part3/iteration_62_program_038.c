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
void test_complex_schedule(int* restrict arr1, int* restrict arr2, 
                          float* restrict farr, int size, volatile int* vflag) {
    /* Mixed data types and dependencies */
    float sum_f = 0.0f;
    double acc_d = 0.0;
    int carry = 0;
    
    /* Outer loop with carried state */
    for (int outer = 0; outer < 4; outer++) {
        int base = (outer * 17) & 0xFF;
        float threshold = (outer + 1) * 100.0f;
        
        /* Inner loop with data-dependent branches and mixed operations */
        for (int i = 0; i < size; i++) {
            /* Flow dependency chain */
            int val1 = arr1[i] ^ base;
            int val2 = arr2[i] + carry;
            
            /* Anti dependency: reuse of val1 */
            arr1[i] = val1 * val2;
            
            /* Output dependency on farr */
            float fval = farr[i] * 1.5f;
            
            /* Complex conditional with unpredictable branch */
            if ((val1 & 0x3) == 0) {
                /* Branch taken ~25% of time */
                sum_f += fval;
                if (sum_f > threshold) {
                    /* Nested conditional creates control dependency */
                    farr[i] = sum_f;
                    sum_f = 0.0f;
                    carry = (carry + 1) & 0x7;
                }
            } else {
                /* Different execution path */
                acc_d += (double)val1 * 0.01;
                farr[i] = (float)acc_d;
            }
            
            /* Manual unrolling - 2 iterations */
            if (i + 1 < size) {
                int next_val = arr1[i+1] ^ (base >> 1);
                arr2[i+1] = next_val + (val1 & 0xF);
                
                /* Inline assembly barrier creates scheduling boundary */
                asm volatile("" ::: "memory");
                
                /* Floating point operation with dependency */
                farr[i+1] = farr[i+1] * 2.0f - fval;
            }
        }
        
        /* Loop-carried dependency across outer iterations */
        carry = (carry + base) & 0xF;
        
        /* Memory barrier between outer loop iterations */
        asm volatile("" ::: "memory");
    }
    
    /* Prevent dead code elimination */
    *vflag = (int)sum_f + (int)acc_d + carry;
}

/* Function with volatile counters and assembly barriers */
__attribute__((optimize("O3", "fsel-sched-pipelining", "funroll-loops")))
void test_volatile_barriers(int* data, int size, volatile int init) {
    volatile int vcounter = init;
    int local_sum = 0;
    
    /* Nested loops with volatile condition */
    for (int j = 0; j < 8; j++) {
        int mod_base = vcounter % 256;
        
        for (int i = 0; i < size; i += 4) {
            /* Process 4 elements with manual unrolling */
            int idx0 = i;
            int idx1 = i + 1;
            int idx2 = i + 2;
            int idx3 = i + 3;
            
            /* Independent computations that can be parallelized */
            int val0 = data[idx0] * mod_base;
            int val1 = data[idx1] + (mod_base >> 1);
            int val2 = data[idx2] ^ mod_base;
            int val3 = data[idx3] - mod_base;
            
            /* Data-dependent conditional */
            if ((val0 + val1) > (val2 + val3)) {
                data[idx0] = val0 >> 1;
                data[idx1] = val1 << 1;
                
                /* Assembly barrier in conditional path */
                asm volatile("" ::: "memory");
            } else {
                data[idx2] = val2 | 0x1;
                data[idx3] = val3 & 0xFE;
            }
            
            /* Mixed-type computation */
            float ftmp = (float)val0 * 0.5f + (float)val1 * 0.25f;
            double dtmp = (double)val2 * 0.125 + (double)val3 * 0.0625;
            
            /* Complex expression with multiple dependencies */
            local_sum += (int)ftmp + (int)dtmp + 
                        ((val0 & 1) ? val1 : val2) + 
                        ((val3 > 0) ? val0 : val1);
            
            /* Update volatile counter - prevents optimization */
            vcounter = (vcounter * 1103515245 + 12345) & 0x7FFF;
        }
        
        /* Outer loop update with volatile dependency */
        mod_base = (mod_base + vcounter) & 0xFF;
        
        /* Barrier between outer iterations */
        asm volatile("" ::: "memory");
    }
    
    /* Use results to prevent elimination */
    data[0] = local_sum;
}

/* Outer loop carried state pattern */
__attribute__((optimize("O2", "fsel-sched-pipelining-outer-loops")))
void test_outer_carried_state(float* fdata, int* idata, int size) {
    float outer_acc = 0.0f;
    int outer_mod = 1;
    
    /* Outer loop with carried state */
    for (int outer = 0; outer < 16; outer++) {
        float factor = 1.0f + (outer % 5) * 0.25f;
        int base = (outer_mod * 13) % 29;
        
        /* Inner loop with dependency on outer state */
        for (int i = 0; i < size; i++) {
            /* Multiple dependency chains */
            float fval = fdata[i];
            int ival = idata[i];
            
            /* Flow dependency on outer_acc */
            fval = fval * factor + outer_acc;
            
            /* Anti dependency on ival */
            idata[i] = ival + base;
            
            /* Output dependency on fdata */
            fdata[i] = fval;
            
            /* Update carried state */
            outer_acc = fval * 0.9f;
            
            /* Data-dependent branch */
            if (ival & (1 << (outer % 5))) {
                /* Complex operation in taken branch */
                fdata[i] = fdata[i] * 2.0f - (float)base;
                outer_mod = (outer_mod * 3) & 0xF;
                
                /* Memory barrier */
                asm volatile("" ::: "memory");
            }
            
            /* Manual unrolling - second element */
            if (i + 1 < size) {
                float fval2 = fdata[i+1];
                fdata[i+1] = fval2 * 0.5f + fval;
                idata[i+1] = idata[i+1] ^ base;
            }
        }
        
        /* Outer loop update with non-linear pattern */
        outer_mod = (outer_mod + outer * 7) & 0xFF;
        
        /* Prevent optimization across iterations */
        asm volatile("" ::: "memory");
    }
    
    /* Store final state */
    fdata[0] = outer_acc;
    idata[0] = outer_mod;
}

int main(void) {
    const int SIZE = 1024;
    int array1[SIZE];
    int array2[SIZE];
    float farray[SIZE];
    
    /* Initialize with pseudo-random but bounded values */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = (int)(lcg_rand() % 1000);
        array2[i] = (int)(lcg_rand() % 1000);
        farray[i] = (float)(lcg_rand() % 1000) * 0.01f;
    }
    
    volatile int flag1 = 0;
    volatile int flag2 = lcg_rand() % 100;
    volatile int flag3 = 1;
    
    /* Variable execution based on runtime values */
    int checksum = 0;
    
    /* Call test functions multiple times with volatile conditions */
    for (int run = 0; run < 3; run++) {
        if (flag3 || (run % 2 == 0)) {
            test_complex_schedule(array1, array2, farray, SIZE - 64, &flag1);
        }
        
        if (flag1 > 0 || run < 2) {
            test_volatile_barriers(array2, SIZE - 128, flag2);
        }
        
        if ((flag2 % 3) == run) {
            test_outer_carried_state(farray, array1, SIZE - 256);
        }
        
        /* Update volatile flags with pseudo-random pattern */
        flag1 = (flag1 + run * 17) & 0xFF;
        flag2 = (flag2 * 1103515245 + 12345) & 0x7FFF;
        flag3 = !flag3;
    }
    
    /* Compute final checksum to prevent elimination */
    for (int i = 0; i < SIZE; i += 16) {
        checksum += array1[i] + array2[i] + (int)farray[i];
        checksum = (checksum * 31) & 0xFFFF;
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Flag states: %d, %d, %d\n", flag1, flag2, flag3);
    
    return checksum & 0xFF;
}
