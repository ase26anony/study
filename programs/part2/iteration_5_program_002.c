/* haifa_sched_trigger.c
 * Designed to trigger GCC Haifa scheduler state save/restore mechanism
 * Compile with: gcc -O3 -march=nehalem -mtune=nehalem -funroll-loops=2 -fno-peel-loops -fdump-rtl-sched -fdump-rtl-sched2 -o haifa_test haifa_sched_trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force scheduler to work with complex control flow */
#define FORCE_SCHED_BARRIER() asm volatile("" ::: "memory")

/* Simple LCG for pseudo-random values */
static inline uint32_t lcg(uint32_t *state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

/* Function with high register pressure and complex scheduling */
__attribute__((noinline, optimize("O3")))
static int compute_kernel(int *arr1, int *arr2, int size, int threshold) {
    /* Many local variables to create register pressure */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f0, f1, f2, f3, f4, f5, f6, f7;
    int sum = 0;
    
    /* Initialize with volatile reads to prevent optimization */
    volatile int init = 1;
    v0 = init; v1 = init + 1; v2 = init + 2; v3 = init + 3;
    v4 = init + 4; v5 = init + 5; v6 = init + 6; v7 = init + 7;
    v8 = init + 8; v9 = init + 9; v10 = init + 10; v11 = init + 11;
    v12 = init + 12; v13 = init + 13; v14 = init + 14; v15 = init + 15;
    
    f0 = init * 0.1f; f1 = init * 0.2f; f2 = init * 0.3f; f3 = init * 0.4f;
    f4 = init * 0.5f; f5 = init * 0.6f; f6 = init * 0.7f; f7 = init * 0.8f;
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent branch - hard to predict */
        if (__builtin_expect((arr1[i] ^ arr2[i]) > threshold, 0)) {
            /* Path A: Integer-heavy computation */
            v0 = v0 + arr1[i];
            v1 = v1 - arr2[i];
            v2 = v2 * (arr1[i] & 0xFF);
            v3 = v3 ^ arr2[i];
            v4 = v4 | (arr1[i] << 3);
            v5 = v5 & ~arr2[i];
            v6 = v6 + (arr1[i] >> 2);
            v7 = v7 - (arr2[i] * 3);
            v8 = v8 * ((arr1[i] % 64) + 1);
            v9 = v9 ^ (arr2[i] >> 1);
            
            /* Mix in floating point ops */
            f0 = f0 + (arr1[i] * 0.01f);
            f1 = f1 - (arr2[i] * 0.02f);
            f2 = f2 * ((arr1[i] & 0xF) * 0.1f + 0.1f);
            
            /* Complex dependency chain */
            v10 = v0 + v1;
            v11 = v2 - v3;
            v12 = v4 * v5;
            v13 = v6 ^ v7;
            v14 = v8 | v9;
            v15 = v10 + v11;
            
            f3 = f0 + f1;
            f4 = f2 * f3;
            f5 = f4 - f0;
            
            /* Force serialization point */
            FORCE_SCHED_BARRIER();
            
            /* Jump to common code */
            goto merge_point;
        } else {
            /* Path B: Different computation pattern */
            v0 = v0 - arr2[i];
            v1 = v1 + arr1[i];
            v2 = v2 ^ (arr1[i] >> 1);
            v3 = v3 & arr2[i];
            v4 = v4 * ((arr2[i] & 0x7F) + 1);
            v5 = v5 | (arr1[i] << 2);
            v6 = v6 - (arr2[i] >> 3);
            v7 = v7 + (arr1[i] * 5);
            v8 = v8 ^ arr2[i];
            v9 = v9 & ~arr1[i];
            
            /* Different floating point mix */
            f0 = f0 - (arr2[i] * 0.015f);
            f1 = f1 + (arr1[i] * 0.025f);
            f2 = f2 / ((arr2[i] & 0x7) * 0.2f + 0.5f);
            
            /* Different dependency pattern */
            v10 = v1 - v0;
            v11 = v3 ^ v2;
            v12 = v5 * v4;
            v13 = v7 & v6;
            v14 = v9 | v8;
            v15 = v10 - v11;
            
            f3 = f1 - f0;
            f4 = f2 + f3;
            f5 = f4 * f0;
            
            /* Another serialization point */
            FORCE_SCHED_BARRIER();
            
            /* Fall through to merge point */
        }
        
merge_point:
        /* Common merging code with more operations */
        v0 = v0 ^ v15;
        v1 = v1 + v14;
        v2 = v2 * v13;
        v3 = v3 - v12;
        
        f6 = f5 + f4;
        f7 = f6 * 0.99f;
        
        /* Switch statement to create additional control flow complexity */
        switch (arr1[i] & 0x3) {
            case 0:
                v4 = v4 + (int)f7;
                f0 = f0 * 1.01f;
                break;
            case 1:
                v5 = v5 - (int)f6;
                f1 = f1 / 1.01f;
                break;
            case 2:
                v6 = v6 ^ (int)(f5 * 10);
                f2 = f2 + 0.5f;
                break;
            default:
                v7 = v7 & (int)(f4 * 100);
                f3 = f3 - 0.25f;
                /* Use goto to create loop within basic block */
                if ((arr2[i] & 0x80) && i > 0) {
                    /* Small backward jump creates scheduling complexity */
                    v8 = v8 + arr1[i-1];
                    goto small_loop;
                }
        }
        
small_loop:
        /* Final mixing */
        v8 = v8 + v0;
        v9 = v9 - v1;
        v10 = v10 * v2;
        v11 = v11 ^ v3;
        
        /* Prevent loop invariant code motion */
        if (i & 1) {
            f0 = f0 + 0.1f;
        } else {
            f1 = f1 - 0.1f;
        }
    }
    
    /* Combine all variables into checksum */
    sum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 +
          v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15 +
          (int)f0 + (int)f1 + (int)f2 + (int)f3 +
          (int)f4 + (int)f5 + (int)f6 + (int)f7;
    
    return sum;
}

/* Alternate version with different optimization attributes */
#ifdef __x86_64__
__attribute__((target("arch=core2")))
#endif
__attribute__((noinline))
static int compute_kernel_alt(int *arr1, int *arr2, int size, int threshold) {
    /* Similar but slightly different to create more scheduling opportunities */
    int vars[20];
    float fvars[10];
    
    for (int i = 0; i < 20; i++) vars[i] = i;
    for (int i = 0; i < 10; i++) fvars[i] = i * 0.5f;
    
    for (int i = 0; i < size; i++) {
        /* Unrolled inner loop */
        for (int j = 0; j < 2; j++) {
            int idx = i + j;
            if (idx >= size) break;
            
            if (arr1[idx] > arr2[idx]) {
                vars[0] += arr1[idx] * vars[1];
                vars[1] -= arr2[idx] * vars[2];
                vars[2] ^= arr1[idx] | vars[3];
                vars[3] &= arr2[idx] ^ vars[4];
                
                fvars[0] = fvars[0] * (arr1[idx] * 0.01f) + fvars[1];
                fvars[1] = fvars[1] / (arr2[idx] * 0.02f) - fvars[2];
                
                FORCE_SCHED_BARRIER();
            } else {
                vars[4] += arr2[idx] * vars[5];
                vars[5] -= arr1[idx] * vars[6];
                vars[6] ^= arr2[idx] | vars[7];
                vars[7] &= arr1[idx] ^ vars[8];
                
                fvars[2] = fvars[2] + (arr2[idx] * 0.015f) * fvars[3];
                fvars[3] = fvars[3] - (arr1[idx] * 0.025f) / fvars[4];
                
                FORCE_SCHED_BARRIER();
            }
        }
        
        /* Complex dependency web */
        vars[8] = vars[0] + vars[1] - vars[2] * vars[3];
        vars[9] = vars[4] ^ vars[5] | vars[6] & vars[7];
        vars[10] = (vars[8] << 3) + (vars[9] >> 2);
        
        fvars[4] = fvars[0] + fvars[1] * fvars[2] - fvars[3];
        fvars[5] = fvars[4] / (fvars[0] + 1.0f);
    }
    
    int sum = 0;
    for (int i = 0; i < 20; i++) sum += vars[i];
    for (int i = 0; i < 10; i++) sum += (int)fvars[i];
    
    return sum;
}

int main(void) {
    const int SIZE = 256;
    int array1[SIZE];
    int array2[SIZE];
    
    /* Initialize with pseudo-random values */
    uint32_t seed = 42;
    for (int i = 0; i < SIZE; i++) {
        array1[i] = lcg(&seed) % 1000;
        array2[i] = lcg(&seed) % 1000;
    }
    
    /* Varying thresholds to explore different paths */
    int results[4];
    
    /* Call kernel multiple times with different parameters */
    results[0] = compute_kernel(array1, array2, SIZE, 500);
    results[1] = compute_kernel(array1, array2, SIZE, 200);
    results[2] = compute_kernel_alt(array1, array2, SIZE, 500);
    results[3] = compute_kernel_alt(array1, array2, SIZE, 200);
    
    /* Compute final checksum to prevent dead code elimination */
    int final_sum = 0;
    for (int i = 0; i < 4; i++) {
        final_sum ^= results[i];
    }
    
    /* Use result to prevent optimization */
    volatile int output = final_sum;
    printf("Result: %d\n", output);
    
    return 0;
}
