/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define ITERATIONS 100000
#define ARRAY_SIZE 1024

/* Volatile variables to prevent optimizations and create dependencies */
volatile int vol_var1 = 1;
volatile int vol_var2 = 2;
volatile float vol_float1 = 1.5f;
volatile float vol_float2 = 2.5f;

/* Function to create high register pressure with independent instructions */
__attribute__((noinline))
void high_pressure_loop(float* restrict a, float* restrict b, float* restrict c, 
                        float* restrict d, int size) {
    int i;
    /* Unrolled loop with many temporary variables to increase register pressure */
    for (i = 0; i < size - 3; i += 4) {
        /* Group 1: Independent floating point operations */
        float t1 = a[i] * b[i] + c[i];
        float t2 = a[i+1] / (b[i+1] + 0.001f);  /* Division for latency */
        float t3 = c[i+1] * d[i+1] - a[i+1];
        float t4 = b[i+2] + c[i+2] * d[i+2];
        
        /* Group 2: More independent operations */
        float t5 = t1 * t2 + vol_float1;
        float t6 = t3 / (t4 + 0.001f);
        float t7 = t5 - t6 * vol_float2;
        float t8 = t7 + t1 * t3;
        
        /* Group 3: Integer operations mixed with float */
        int it1 = (int)t1 * (int)t2 + vol_var1;
        int it2 = (int)t3 * (int)t4 - vol_var2;
        int it3 = it1 * it2 / (vol_var1 + 1);
        int it4 = it2 + it3 * (int)vol_float1;
        
        /* Store results creating dependencies */
        a[i] = t5 + (float)it1;
        b[i+1] = t6 * (float)it2;
        c[i+2] = t7 - (float)it3;
        d[i+3] = t8 / ((float)it4 + 0.001f);
        
        /* Inline assembly to clobber registers and force spills */
        __asm__ volatile (
            "mov $0, %%eax\n\t"
            "mov $0, %%ebx\n\t"
            "mov $0, %%ecx\n\t"
            "mov $0, %%edx\n\t"
            "mov $0, %%esi\n\t"
            "mov $0, %%edi\n\t"
            : /* no outputs */
            : /* no inputs */
            : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
    }
}

/* Function with artificial dependencies and resource conflicts */
__attribute__((noinline))
void mixed_dependency(int* restrict arr1, int* restrict arr2, 
                      float* restrict farr1, float* restrict farr2, int size) {
    int i;
    int dep1 = vol_var1;
    int dep2 = vol_var2;
    float fdep1 = vol_float1;
    float fdep2 = vol_float2;
    
    for (i = 0; i < size; i++) {
        /* Create long dependency chain */
        dep1 = dep1 * arr1[i] + dep2;
        dep2 = dep2 * arr2[i] - dep1;
        
        /* Floating point with different latency */
        fdep1 = fdep1 / (farr1[i] + 0.001f);  /* Slow division */
        fdep2 = fdep2 * farr2[i] - fdep1;
        
        /* Memory operations that might alias */
        arr1[i] = dep1 + (int)fdep1;
        arr2[(i + 1) % size] = dep2 * (int)fdep2;
        
        /* Conditional to create control flow and different priorities */
        if (arr1[i] > 1000) {
            farr1[i] = sqrtf(fdep1);  /* Slow sqrt operation */
            vol_var1 = arr1[i];  /* Volatile write creates barrier */
        } else {
            farr2[i] = fdep2 * 2.0f;
            vol_var2 = arr2[i];
        }
        
        /* More independent operations that can be scheduled in parallel */
        int tmp1 = arr1[i] * 3;
        int tmp2 = arr2[i] / 2;
        float ftmp1 = farr1[i] + 1.0f;
        float ftmp2 = farr2[i] - 0.5f;
        
        /* Use them to prevent dead code elimination */
        arr1[i] = tmp1 + (int)ftmp1;
        arr2[i] = tmp2 * (int)ftmp2;
    }
}

/* Function with many independent instructions in basic blocks */
__attribute__((hot, noinline))
void independent_instructions_block(float* data, int count) {
    int i;
    for (i = 0; i < count; i++) {
        /* Block of independent instructions - scheduler has many choices */
        float a = data[i] * 1.1f;
        float b = data[i+1] / 1.2f;
        float c = data[i+2] + 2.3f;
        float d = data[i+3] - 3.4f;
        float e = a * b;
        float f = c / d;
        float g = e + f;
        float h = e - f;
        float j = g * h;
        float k = g / (h + 0.001f);
        
        /* More independent integer operations */
        int ia = (int)a * 2;
        int ib = (int)b + 3;
        int ic = ia * ib;
        int id = ib - ia;
        int ie = ic / (id + 1);
        
        /* Store results creating output dependencies */
        data[i] = j + (float)ie;
        data[i+1] = k - (float)ic;
        data[i+2] = (float)ia * k;
        data[i+3] = (float)ib / (j + 0.001f);
    }
}

int main() {
    int i;
    clock_t start, end;
    
    /* Allocate and initialize arrays with random data */
    float* array1 = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float* array2 = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float* array3 = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float* array4 = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    
    int* iarray1 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* iarray2 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = (float)rand() / RAND_MAX * 100.0f;
        array2[i] = (float)rand() / RAND_MAX * 100.0f;
        array3[i] = (float)rand() / RAND_MAX * 100.0f;
        array4[i] = (float)rand() / RAND_MAX * 100.0f;
        iarray1[i] = rand() % 1000;
        iarray2[i] = rand() % 1000;
    }
    
    printf("Starting scheduling test...\n");
    start = clock();
    
    /* Perform many iterations to ensure scheduler sees hot code */
    for (i = 0; i < ITERATIONS; i++) {
        /* Call functions that create different scheduling scenarios */
        high_pressure_loop(array1, array2, array3, array4, ARRAY_SIZE);
        mixed_dependency(iarray1, iarray2, array1, array2, ARRAY_SIZE / 4);
        independent_instructions_block(array3, ARRAY_SIZE / 8);
        
        /* Modify volatile variables to change dependencies */
        if (i % 100 == 0) {
            vol_var1 = (vol_var1 * 3) % 100;
            vol_var2 = (vol_var2 + 5) % 200;
            vol_float1 = sinf(vol_float1 * 1.1f);
            vol_float2 = cosf(vol_float2 * 0.9f);
        }
    }
    
    end = clock();
    
    /* Compute checksum to prevent dead code elimination */
    float checksum = 0.0f;
    int intsum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += array1[i] + array2[i] + array3[i] + array4[i];
        intsum += iarray1[i] + iarray2[i];
    }
    
    printf("Checksum: %f, Int sum: %d\n", checksum, intsum);
    printf("Time: %f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    free(iarray1);
    free(iarray2);
    
    return 0;
}
