/* sel-sched-test.c - Program to trigger selective scheduler debugging output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile double g_volatile_double = 1.0;

/* Function with potential aliasing */
static inline double process_value(double* restrict dest, 
                                   double* src1, 
                                   double* src2, 
                                   int* counter) {
    double temp = *src1 * *src2;
    *dest = temp + (*counter * 0.5);
    return temp;
}

/* Hot loop function with complex dependencies */
static inline void compute_loop(int iterations, 
                                double* restrict arr1, 
                                double* arr2, 
                                float* farr, 
                                int* iarr) {
    double local_acc = 1.0;
    float float_acc = 2.0f;
    int int_acc = 3;
    
    /* Multiple carried dependencies */
    for (int i = 0; i < iterations; i++) {
        /* Integer operations with carried dependency */
        int_acc = int_acc * 1103515245 + 12345;
        
        /* Floating-point operations */
        float_acc = float_acc * 1.5f + (float)i * 0.1f;
        
        /* Memory operations with potential aliasing */
        arr1[i % 128] = arr2[(i + 1) % 128] * 2.0 + float_acc;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Different execution path */
            local_acc = local_acc / 1.1 + (double)int_acc * 0.01;
            farr[i % 64] = (float)local_acc * 0.5f;
            
            /* Call to function with restrict pointers */
            double temp = process_value(&arr1[(i + 3) % 128], 
                                       &arr2[i % 128], 
                                       &arr2[(i + 2) % 128], 
                                       &iarr[i % 32]);
            local_acc += temp;
        } else if (i % 13 == 0) {
            /* Another execution path */
            local_acc = local_acc * 0.99 - (double)float_acc;
            iarr[i % 32] = int_acc ^ (i * 17);
        } else {
            /* Default path */
            local_acc = local_acc + arr2[i % 128] - (double)float_acc;
        }
        
        /* More arithmetic diversity */
        if (i % 5 == 0) {
            double div_result = local_acc / (1.0 + (double)(i % 100));
            arr2[(i + 5) % 128] = div_result * 3.14159;
        }
        
        /* Volatile access to prevent reordering */
        g_volatile_counter++;
        g_volatile_double *= 1.000001;
        
        /* Inline assembly with memory clobber */
        asm volatile("" : : : "memory");
    }
    
    /* Store final results */
    arr1[0] = local_acc;
    arr2[0] = (double)float_acc;
    iarr[0] = int_acc;
}

/* Another hot loop with different pattern */
static inline void compute_loop2(int iterations, 
                                 double* restrict out, 
                                 const double* in1, 
                                 const double* in2) {
    double sum1 = 0.0, sum2 = 0.0, sum3 = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiple independent chains */
        double t1 = in1[i % 256] * 1.1;
        double t2 = in2[i % 256] * 0.9;
        double t3 = t1 + t2;
        double t4 = t1 - t2;
        
        /* Cross dependencies */
        sum1 = sum1 * 0.95 + t3;
        sum2 = sum2 * 0.97 + t4;
        sum3 = sum3 * 0.99 + (t1 * t2);
        
        /* Conditional store */
        if (i % 11 == 0) {
            out[i % 128] = sum1 + sum2 + sum3;
        }
        
        /* More operations to increase instruction count */
        sum1 = sum1 / (1.0 + (double)(i % 50) * 0.01);
        sum2 = sum2 + (double)(i % 100) * 0.001;
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
    }
    
    /* Reduce results */
    out[1] = sum1;
    out[2] = sum2;
    out[3] = sum3;
}

int main(void) {
    /* Allocate and initialize arrays */
    const int size = 256;
    double* arr1 = (double*)aligned_alloc(64, size * sizeof(double));
    double* arr2 = (double*)aligned_alloc(64, size * sizeof(double));
    float* farr = (float*)aligned_alloc(32, 64 * sizeof(float));
    int* iarr = (int*)aligned_alloc(32, 32 * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < size; i++) {
        arr1[i] = (double)(i + 1) * 0.1;
        arr2[i] = (double)(i * 2) * 0.05;
    }
    
    for (int i = 0; i < 64; i++) {
        farr[i] = (float)i * 0.25f;
    }
    
    for (int i = 0; i < 32; i++) {
        iarr[i] = i * 3;
    }
    
    /* Perform multiple computations to create hot regions */
    uint64_t checksum = 0;
    
    for (int rep = 0; rep < 1000; rep++) {
        /* Call hot loops multiple times */
        compute_loop(1000 + (rep % 100), arr1, arr2, farr, iarr);
        compute_loop2(800 + (rep % 80), arr1, arr2, arr1);
        
        /* Update checksum to prevent dead code elimination */
        checksum ^= *(uint64_t*)&arr1[rep % 8];
        checksum ^= *(uint64_t*)&arr2[rep % 8];
        checksum ^= (uint64_t)iarr[rep % 4];
        
        /* Modify inputs slightly */
        arr1[rep % 16] += 0.1;
        arr2[rep % 16] -= 0.05;
        iarr[rep % 8] ^= rep;
    }
    
    /* Final computation */
    compute_loop(5000, arr1, arr2, farr, iarr);
    
    /* Aggregate final checksum */
    for (int i = 0; i < 16; i++) {
        checksum ^= *(uint64_t*)&arr1[i];
        checksum ^= *(uint64_t*)&arr2[i];
    }
    
    for (int i = 0; i < 8; i++) {
        checksum ^= (uint64_t)iarr[i];
    }
    
    /* Use results to prevent optimization */
    printf("Result checksum: 0x%016llx\n", (unsigned long long)checksum);
    printf("Volatile counter: %d, double: %f\n", 
           g_volatile_counter, g_volatile_double);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr);
    free(iarr);
    
    return 0;
}
