/* sel-sched-test.c - Program to trigger selective scheduler debugging output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000000

/* Volatile variables to prevent optimization */
static volatile int g_volatile_counter = 0;

/* Function with potential aliasing */
static inline double compute_chunk(double *restrict arr1, 
                                   double *arr2, 
                                   float *restrict farr,
                                   int start, 
                                   int end, 
                                   int stride) {
    double sum = 0.0;
    float fsum = 0.0f;
    int isum = 0;
    
    /* Complex loop with multiple dependencies and operations */
    for (int i = start; i < end; i += stride) {
        /* Integer operations with carried dependency */
        isum = (isum * 1103515245 + 12345) & 0x7fffffff;
        g_volatile_counter += (isum & 1);
        
        /* Floating-point operations mixing precision */
        double temp = arr1[i] * 1.234567;
        float ftemp = (float)temp / 3.14159265f;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Division operation - expensive and hard to schedule */
            arr2[i] = temp / (ftemp + 0.000001f);
            farr[i] = ftemp * 2.71828182f;
        } else if (i % 13 == 0) {
            /* Different computation path */
            arr2[i] = temp * ftemp;
            farr[i] = ftemp / 2.71828182f;
        } else {
            /* Default path with memory store */
            arr2[i] = temp + ftemp;
            farr[i] = ftemp - 0.5f;
        }
        
        /* More arithmetic diversity */
        sum += arr2[i] * (i % 5 + 1);
        fsum += farr[i] * ((i % 3) + 0.5f);
        
        /* Additional integer operations */
        int mod_val = i % 17;
        if (mod_val > 8) {
            isum += mod_val * 3;
        } else {
            isum -= mod_val;
        }
        
        /* Memory access with potential aliasing (arr2 not restrict) */
        if (i > 0) {
            arr2[i] += arr2[i-1] * 0.1;
        }
        
        /* Inline assembly to create memory clobber */
        asm volatile("" : : "r"(arr1[i]), "r"(arr2[i]) : "memory");
    }
    
    /* Mix results */
    return sum + fsum + isum;
}

/* Another hot function to encourage inlining */
static inline uint64_t process_array(double *restrict darr, 
                                     float *restrict farr,
                                     int *iarr,
                                     int n) {
    uint64_t checksum = 0;
    double acc1 = 1.0, acc2 = 0.0;
    float facc1 = 0.0f, facc2 = 1.0f;
    
    for (int i = 0; i < n; i++) {
        /* Complex dependency chain */
        acc1 = acc1 * darr[i] + iarr[i];
        acc2 = acc2 / (darr[i] + 1.0) - iarr[i];
        
        facc1 = facc1 + farr[i] * (i % 11);
        facc2 = facc2 - farr[i] / (i % 7 + 1);
        
        /* Cross-type operations */
        darr[i] = acc1 + facc1;
        farr[i] = (float)acc2 * facc2;
        
        /* Integer computation with branching */
        if (iarr[i] > 0) {
            iarr[i] = (iarr[i] * 16807) % 2147483647;
        } else {
            iarr[i] = (iarr[i] * 48271) % 2147483647;
        }
        
        /* Update checksum */
        checksum ^= *(uint64_t*)&darr[i];
        checksum += *(uint32_t*)&farr[i];
        checksum ^= (uint64_t)iarr[i] << 32;
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    
    return checksum;
}

int main(void) {
    /* Allocate and initialize arrays */
    double *arr1 = (double*)aligned_alloc(64, SIZE * sizeof(double));
    double *arr2 = (double*)aligned_alloc(64, SIZE * sizeof(double));
    float *farr = (float*)aligned_alloc(64, SIZE * sizeof(float));
    int *iarr = (int*)aligned_alloc(64, SIZE * sizeof(int));
    
    if (!arr1 || !arr2 || !farr || !iarr) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (i % 23) * 0.12345;
        arr2[i] = (i % 17) * 0.67890;
        farr[i] = (i % 29) * 0.24680f;
        iarr[i] = i * 1103515245 + 12345;
    }
    
    uint64_t total_checksum = 0;
    
    /* Perform multiple iterations to create hot loop */
    for (int iter = 0; iter < ITERATIONS / SIZE + 1; iter++) {
        /* Call compute_chunk multiple times with different strides */
        double result1 = compute_chunk(arr1, arr2, farr, 0, SIZE, 1);
        double result2 = compute_chunk(arr1, arr2, farr, 1, SIZE, 2);
        double result3 = compute_chunk(arr1, arr2, farr, 2, SIZE, 3);
        
        /* Process arrays */
        uint64_t checksum = process_array(arr1, farr, iarr, SIZE);
        
        /* Accumulate results to prevent optimization */
        total_checksum ^= *(uint64_t*)&result1;
        total_checksum += *(uint64_t*)&result2;
        total_checksum ^= *(uint64_t*)&result3;
        total_checksum += checksum;
        
        /* Modify inputs slightly */
        for (int i = 0; i < SIZE; i += 8) {
            arr1[i] *= 1.0001;
            iarr[i] += iter;
        }
        
        /* Memory clobber */
        asm volatile("" : : : "memory");
    }
    
    /* Use results to prevent dead code elimination */
    printf("Checksum: %016llx\n", (unsigned long long)total_checksum);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr);
    free(iarr);
    
    return 0;
}
