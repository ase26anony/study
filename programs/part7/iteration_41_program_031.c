#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/* Function to create loop-carried dependency and complex control flow */
void compute_kernel(int *arr1, int *arr2, float *farr1, float *farr2, 
                    int n, int *carry_dep, float *fp_carry) {
    int local_carry = *carry_dep;
    float local_fp = *fp_carry;
    
    for (int i = 0; i < n; ++i) {
        /* Loop-carried integer dependency */
        local_carry += arr1[i] * arr2[i];
        
        /* Mixed integer/float operations with bitwise ops */
        int temp = (arr1[i] & 0xFF) | (arr2[i] << 8);
        float ftemp = (float)temp * 0.5f;
        
        /* Complex array indexing */
        int idx = (i * 3) % n;
        int idx2 = (i * 7) % n;
        
        /* Conditional control flow with data-dependent condition */
        if (farr1[idx] > local_fp) {
            /* Floating-point computation */
            farr2[i] = sqrtf(fabsf(farr1[idx] + ftemp));
            
            /* More integer computation in conditional path */
            arr1[idx2] = (arr1[idx2] ^ local_carry) + (int)(farr2[i] * 100.0f);
        } else {
            /* Alternative computation path */
            farr2[i] = farr1[idx] * ftemp;
            arr1[idx2] = (arr1[idx2] & 0xFFFF) - temp;
        }
        
        /* Another loop-carried dependency (floating-point) */
        local_fp = local_fp * 0.99f + farr2[i] * 0.01f;
        
        /* Additional conditional with unpredictable branch */
        if ((arr1[i] + arr2[i]) % 17 < 8) {
            local_carry ^= (temp << 3);
        }
    }
    
    *carry_dep = local_carry;
    *fp_carry = local_fp;
}

int main(int argc, char **argv) {
    /* Use argc to prevent compile-time constant propagation */
    int n = 500;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0 || n > 1000) n = 500;
    }
    
    /* Volatile variable to prevent optimization */
    volatile int volatile_n = n;
    
    /* Initialize arrays with pseudo-random data */
    int *arr1 = malloc(volatile_n * sizeof(int));
    int *arr2 = malloc(volatile_n * sizeof(int));
    float *farr1 = malloc(volatile_n * sizeof(float));
    float *farr2 = malloc(volatile_n * sizeof(float));
    
    srand(time(NULL));
    for (int i = 0; i < volatile_n; ++i) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX * 10.0f;
        farr2[i] = 0.0f;
    }
    
    int carry_dep = rand() % 100;
    float fp_carry = (float)rand() / RAND_MAX;
    
    /* Outer loop to give scheduler repeated region to analyze */
    for (int outer = 0; outer < 3; ++outer) {
        compute_kernel(arr1, arr2, farr1, farr2, volatile_n, &carry_dep, &fp_carry);
        
        /* Modify data slightly between outer iterations */
        for (int i = 0; i < volatile_n; i += 7) {
            arr1[i] += outer;
            farr1[i] += (float)outer * 0.1f;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long int_sum = 0;
    float float_sum = 0.0f;
    for (int i = 0; i < volatile_n; ++i) {
        int_sum += arr1[i] + arr2[i];
        float_sum += farr1[i] + farr2[i];
    }
    
    printf("Checksum: int=%lld float=%f carry=%d fp_carry=%f\n", 
           int_sum, float_sum, carry_dep, fp_carry);
    
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    
    return 0;
}
