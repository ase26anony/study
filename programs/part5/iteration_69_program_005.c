/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test_sched_coverage
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test_sched_pressure
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define ITERATIONS 100000
#define ARRAY_SIZE 1024

/* Volatile variables to prevent optimization and create dependencies */
volatile int vol_var1 = 1;
volatile int vol_var2 = 2;
volatile float vol_float1 = 1.5f;
volatile float vol_float2 = 2.5f;

/* Function to create high register pressure with independent instruction groups */
__attribute__((noinline))
void high_pressure_loop(float* restrict a, float* restrict b, float* restrict c, 
                        float* restrict d, int n) {
    /* Many independent variables to create register pressure */
    float t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    float t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    
    for (int i = 0; i < n; i++) {
        /* Group 1: Independent floating point operations */
        t1 = a[i] * b[i];
        t2 = a[i] + b[i];
        t3 = a[i] - b[i];
        t4 = a[i] / (b[i] + 0.001f);  /* Division for latency */
        
        /* Group 2: More independent operations */
        t5 = c[i] * d[i];
        t6 = c[i] + d[i];
        t7 = c[i] - d[i];
        t8 = c[i] / (d[i] + 0.001f);
        
        /* Group 3: Integer operations mixed with float */
        i1 = (int)a[i];
        i2 = (int)b[i];
        i3 = i1 * i2;
        i4 = i1 + i2;
        i5 = i1 - i2;
        
        /* Group 4: Cross dependencies to create priority differences */
        t9 = t1 + t5;
        t10 = t2 * t6;
        t11 = t3 - t7;
        t12 = t4 / (t8 + 0.001f);
        
        /* Group 5: More operations with artificial dependencies */
        t13 = t9 * vol_float1;
        t14 = t10 + vol_float2;
        t15 = t11 - vol_float1;
        t16 = t12 / vol_float2;
        
        /* Store results to prevent elimination */
        a[i] = t13 + t14;
        b[i] = t15 * t16;
        c[i] = t9 + t10 + t11 + t12;
        d[i] = (float)i3 + (float)i4 + (float)i5;
        
        /* Additional independent groups for more scheduling candidates */
        t17 = sinf(a[i]);      /* High latency operation */
        t18 = cosf(b[i]);      /* Another high latency */
        t19 = sqrtf(c[i]);     /* More latency */
        t20 = logf(fabs(d[i]) + 1.0f);
        
        /* Mix them back */
        a[i] += t17 * 0.1f;
        b[i] += t18 * 0.2f;
        c[i] += t19 * 0.3f;
        d[i] += t20 * 0.4f;
    }
}

/* Function with mixed dependencies and resource conflicts */
__attribute__((noinline))
void mixed_dependency(int* restrict arr1, int* restrict arr2, 
                      float* restrict farr1, float* restrict farr2, int n) {
    /* Create artificial resource conflicts with inline assembly */
    for (int i = 0; i < n; i++) {
        int tmp1, tmp2, tmp3, tmp4;
        float ftmp1, ftmp2, ftmp3, ftmp4;
        
        /* Long latency chain 1 */
        ftmp1 = farr1[i] / 3.14159f;  /* Division for latency */
        ftmp2 = ftmp1 * ftmp1;
        ftmp3 = sqrtf(fabs(ftmp2));
        
        /* Independent chain 2 */
        tmp1 = arr1[i] * vol_var1;
        tmp2 = arr2[i] + vol_var2;
        
        /* Inline assembly to clobber registers and create pressure */
        asm volatile (
            "movl $0xAAAAAAAA, %%eax\n\t"
            "movl $0x55555555, %%ebx\n\t"
            "addl %%ebx, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (tmp3)
            : 
            : "eax", "ebx", "cc"
        );
        
        /* Dependency on volatile creates delay */
        ftmp4 = ftmp3 * vol_float1;
        
        /* Another independent group */
        tmp4 = tmp1 ^ tmp2;
        
        /* More inline assembly with different registers */
        asm volatile (
            "movl $0x33333333, %%ecx\n\t"
            "movl $0xCCCCCCCC, %%edx\n\t"
            "xorl %%edx, %%ecx\n\t"
            "movl %%ecx, %0\n\t"
            : "=r" (tmp4)
            : 
            : "ecx", "edx", "cc"
        );
        
        /* Store with dependencies */
        farr1[i] = ftmp4 + (float)tmp3;
        farr2[i] = ftmp2 - (float)tmp4;
        arr1[i] = tmp1 + tmp3;
        arr2[i] = tmp2 ^ tmp4;
        
        /* Control flow to affect priority */
        if (arr1[i] > 1000) {
            arr1[i] = arr1[i] / 2;  /* Division for delay */
        } else {
            arr1[i] = arr1[i] * 3;
        }
        
        /* Switch statement for more control flow */
        switch (arr2[i] & 0x3) {
            case 0:
                arr2[i] += tmp1;
                break;
            case 1:
                arr2[i] -= tmp2;
                break;
            case 2:
                arr2[i] *= tmp3;
                break;
            case 3:
                arr2[i] = tmp4;
                break;
        }
    }
}

/* Function with completely independent instruction groups */
__attribute__((noinline))
void independent_groups(double* restrict d1, double* restrict d2, 
                        double* restrict d3, double* restrict d4, int n) {
    /* Four completely independent chains */
    for (int i = 0; i < n; i++) {
        /* Group A - completely independent from others */
        double a1 = d1[i] * 1.1;
        double a2 = sin(d1[i]);
        double a3 = a1 + a2;
        double a4 = cos(a3);
        d1[i] = a4 * 0.5;
        
        /* Group B - independent */
        double b1 = d2[i] * 2.2;
        double b2 = exp(d2[i] * 0.01);
        double b3 = b1 - b2;
        double b4 = log(fabs(b3) + 1.0);
        d2[i] = b4 * 0.3;
        
        /* Group C - independent */
        double c1 = d3[i] * 3.3;
        double c2 = d3[i] / 4.4;  /* Division for latency */
        double c3 = c1 * c2;
        double c4 = sqrt(fabs(c3));
        d3[i] = c4 * 0.7;
        
        /* Group D - independent */
        double d1v = d4[i] * 4.4;
        double d2v = pow(d4[i], 1.5);
        double d3v = d1v + d2v;
        double d4v = d3v * d3v;
        d4[i] = d4v * 0.9;
    }
}

int main() {
    /* Allocate and initialize arrays */
    float* fa1 = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float* fa2 = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float* fa3 = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float* fa4 = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    
    int* ia1 = (int*)aligned_alloc(32, ARRAY_SIZE * sizeof(int));
    int* ia2 = (int*)aligned_alloc(32, ARRAY_SIZE * sizeof(int));
    
    double* da1 = (double*)aligned_alloc(32, ARRAY_SIZE * sizeof(double));
    double* da2 = (double*)aligned_alloc(32, ARRAY_SIZE * sizeof(double));
    double* da3 = (double*)aligned_alloc(32, ARRAY_SIZE * sizeof(double));
    double* da4 = (double*)aligned_alloc(32, ARRAY_SIZE * sizeof(double));
    
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fa1[i] = (float)rand() / RAND_MAX * 100.0f;
        fa2[i] = (float)rand() / RAND_MAX * 100.0f;
        fa3[i] = (float)rand() / RAND_MAX * 100.0f;
        fa4[i] = (float)rand() / RAND_MAX * 100.0f;
        
        ia1[i] = rand() % 1000;
        ia2[i] = rand() % 1000;
        
        da1[i] = (double)rand() / RAND_MAX * 100.0;
        da2[i] = (double)rand() / RAND_MAX * 100.0;
        da3[i] = (double)rand() / RAND_MAX * 100.0;
        da4[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    /* Performance-critical loop with multiple scheduling scenarios */
    float checksum = 0.0f;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Modify volatile to affect scheduling */
        vol_var1 = (iter % 256) + 1;
        vol_var2 = ((iter * 37) % 256) + 1;
        vol_float1 = (float)(iter % 100) * 0.1f + 0.5f;
        vol_float2 = (float)((iter * 17) % 100) * 0.1f + 0.5f;
        
        /* Call functions that create different scheduling scenarios */
        high_pressure_loop(fa1, fa2, fa3, fa4, ARRAY_SIZE / 4);
        mixed_dependency(ia1, ia2, fa1, fa2, ARRAY_SIZE / 8);
        independent_groups(da1, da2, da3, da4, ARRAY_SIZE / 16);
        
        /* Accumulate checksum to prevent dead code elimination */
        for (int i = 0; i < ARRAY_SIZE; i += 64) {
            checksum += fa1[i] + fa2[i] + fa3[i] + fa4[i];
            checksum += (float)ia1[i] + (float)ia2[i];
            checksum += (float)(da1[i] + da2[i] + da3[i] + da4[i]);
        }
    }
    
    /* Print result to prevent optimization */
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(fa1);
    free(fa2);
    free(fa3);
    free(fa4);
    free(ia1);
    free(ia2);
    free(da1);
    free(da2);
    free(da3);
    free(da4);
    
    return 0;
}
