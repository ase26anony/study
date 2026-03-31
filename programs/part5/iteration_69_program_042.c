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
volatile int vol_a = 1, vol_b = 2, vol_c = 3;
volatile float vol_f1 = 1.0f, vol_f2 = 2.0f, vol_f3 = 3.0f;

/* Function to create high register pressure and independent instructions */
void high_pressure_loop(float *restrict a, float *restrict b, float *restrict c, 
                        float *restrict d, int n) {
    /* Many independent variables to create register pressure */
    float t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    float t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    
    for (int i = 0; i < n; i++) {
        /* Group 1: Independent floating point operations */
        t1 = a[i] * b[i];
        t2 = a[i] + b[i];
        t3 = a[i] - b[i];
        t4 = a[i] / (b[i] + 0.001f);  /* Division creates longer latency */
        
        /* Group 2: More independent operations */
        t5 = c[i] * d[i];
        t6 = c[i] + d[i];
        t7 = c[i] - d[i];
        t8 = c[i] / (d[i] + 0.001f);
        
        /* Group 3: Cross dependencies to create priority differences */
        t9 = t1 + t5;   /* Depends on group 1 and 2 */
        t10 = t2 * t6;
        t11 = t3 - t7;
        t12 = t4 / t8;  /* Division - longer latency */
        
        /* Integer operations mixed in */
        i1 = (int)t1;
        i2 = (int)t2;
        i3 = i1 * i2;
        i4 = i1 + i2;
        i5 = i3 - i4;
        
        /* More floating point with volatile to force ordering */
        t13 = t9 * vol_f1;
        t14 = t10 + vol_f2;
        t15 = t11 - vol_f3;
        
        /* Artificial resource conflict via inline asm */
        asm volatile ("" : : : "memory", "eax", "ebx", "ecx", "edx");
        
        /* Store results creating write-back pressure */
        a[i] = t13 + t14;
        b[i] = t15 * t12;
        c[i] = (float)i5 + t9;
        d[i] = t10 / (t11 + 0.001f);
        
        /* Additional independent group */
        t16 = a[i] * 1.1f;
        t17 = b[i] * 1.2f;
        t18 = c[i] * 1.3f;
        t19 = d[i] * 1.4f;
        t20 = t16 + t17 + t18 + t19;
        
        /* Use volatile to prevent dead code elimination */
        vol_f1 = t20 * 0.5f;
    }
}

/* Function with mixed dependencies and artificial delays */
void mixed_dependency(int *restrict arr1, int *restrict arr2, 
                      float *restrict farr1, float *restrict farr2, int n) {
    /* Variables with varying lifetimes to create different priorities */
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    float facc1 = 0.0f, facc2 = 0.0f, facc3 = 0.0f, facc4 = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Critical path 1: Chain of dependencies */
        int x1 = arr1[i] + vol_a;
        int x2 = x1 * arr2[i];
        int x3 = x2 - vol_b;
        int x4 = x3 / (vol_c + 1);
        acc1 += x4;
        
        /* Critical path 2: Independent but similar operations */
        int y1 = arr2[i] - vol_b;
        int y2 = y1 * arr1[i];
        int y3 = y2 + vol_a;
        int y4 = y3 / (vol_a + 1);
        acc2 += y4;
        
        /* Floating point with longer latency operations */
        float f1 = farr1[i] * 3.14159f;  /* Multiplication */
        float f2 = farr2[i] / 2.71828f;  /* Division - longer latency */
        float f3 = f1 + f2;
        float f4 = f1 - f2;
        float f5 = f3 * f4;
        float f6 = f3 / (f4 + 0.0001f);  /* Another division */
        
        /* Mix integer and float */
        facc1 += f5;
        facc2 += f6;
        
        /* Conditional to create control flow and priority differences */
        if (i % 3 == 0) {
            acc3 += x1;
            facc3 += f1;
        } else if (i % 3 == 1) {
            acc4 += y1;
            facc4 += f2;
        } else {
            /* Complex expression with many operations */
            int z = (x1 * y1) + (x2 / (y2 + 1)) - (x3 % (y3 + 1));
            float fz = f3 * f4 / (f5 + 0.001f);
            acc3 += z;
            facc3 += fz;
        }
        
        /* Memory barrier to force ordering */
        asm volatile ("" : : : "memory");
        
        /* Use results to prevent elimination */
        arr1[i] = acc1 + acc2;
        arr2[i] = acc3 + acc4;
        farr1[i] = facc1 + facc2;
        farr2[i] = facc3 + facc4;
    }
    
    /* Final volatile store to ensure all computations are used */
    vol_a = acc1;
    vol_b = acc2;
    vol_f1 = facc1;
    vol_f2 = facc2;
}

/* Unrolled loop to create many independent instructions */
void unrolled_computation(float *restrict out, const float *restrict in, int n) {
    /* Manual unrolling creates many independent instructions */
    for (int i = 0; i < n; i += 8) {
        /* Eight independent computation chains */
        float r0 = in[i] * 0.1f + in[i] / 1.1f;
        float r1 = in[i+1] * 0.2f + in[i+1] / 1.2f;
        float r2 = in[i+2] * 0.3f + in[i+2] / 1.3f;
        float r3 = in[i+3] * 0.4f + in[i+3] / 1.4f;
        float r4 = in[i+4] * 0.5f + in[i+4] / 1.5f;
        float r5 = in[i+5] * 0.6f + in[i+5] / 1.6f;
        float r6 = in[i+6] * 0.7f + in[i+6] / 1.7f;
        float r7 = in[i+7] * 0.8f + in[i+7] / 1.8f;
        
        /* Cross dependencies between some chains */
        float s0 = r0 + r4;
        float s1 = r1 + r5;
        float s2 = r2 + r6;
        float s3 = r3 + r7;
        
        /* More operations with different latencies */
        float t0 = s0 * s1;  /* Multiplication */
        float t1 = s2 / s3;  /* Division - longer latency */
        float t2 = t0 + t1;
        float t3 = t0 - t1;
        float t4 = t2 * t3;
        float t5 = t2 / (t3 + 0.001f);  /* Another division */
        
        /* Store results */
        out[i] = t4;
        out[i+1] = t5;
        out[i+2] = r0 + r1;
        out[i+3] = r2 + r3;
        out[i+4] = r4 + r5;
        out[i+5] = r6 + r7;
        out[i+6] = s0 + s1;
        out[i+7] = s2 + s3;
    }
}

int main() {
    /* Allocate and initialize arrays */
    float *a = malloc(ARRAY_SIZE * sizeof(float));
    float *b = malloc(ARRAY_SIZE * sizeof(float));
    float *c = malloc(ARRAY_SIZE * sizeof(float));
    float *d = malloc(ARRAY_SIZE * sizeof(float));
    int *arr1 = malloc(ARRAY_SIZE * sizeof(int));
    int *arr2 = malloc(ARRAY_SIZE * sizeof(int));
    float *out = malloc(ARRAY_SIZE * sizeof(float));
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = (float)rand() / RAND_MAX;
        b[i] = (float)rand() / RAND_MAX;
        c[i] = (float)rand() / RAND_MAX;
        d[i] = (float)rand() / RAND_MAX;
        arr1[i] = rand();
        arr2[i] = rand();
    }
    
    printf("Starting scheduling test...\n");
    
    /* Perform multiple iterations to ensure hot code scheduling */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call functions with different patterns */
        high_pressure_loop(a, b, c, d, ARRAY_SIZE);
        mixed_dependency(arr1, arr2, a, b, ARRAY_SIZE);
        unrolled_computation(out, a, ARRAY_SIZE);
        
        /* Modify inputs slightly each iteration */
        a[iter % ARRAY_SIZE] += 0.01f;
        b[iter % ARRAY_SIZE] += 0.02f;
        vol_a = (vol_a + 1) % 100;
        vol_b = (vol_b + 1) % 100;
    }
    
    /* Compute checksum to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += a[i] + b[i] + c[i] + d[i] + out[i] + 
                   (float)arr1[i] + (float)arr2[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Volatile values: %d %d %f %f %f\n", 
           vol_a, vol_b, vol_f1, vol_f2, vol_f3);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(arr1); free(arr2);
    free(out);
    
    return 0;
}
