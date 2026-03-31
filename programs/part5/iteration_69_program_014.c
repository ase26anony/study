/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test_sched_coverage
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test_sched_coverage_pressure
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

/* Function to create high register pressure with many live variables */
__attribute__((noinline))
void high_pressure_loop(float* restrict a, float* restrict b, float* restrict c, 
                        float* restrict d, float* restrict e, int n) {
    int i;
    /* Many independent variables to create register pressure */
    float t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    float t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    for (i = 0; i < n; i++) {
        /* Create many independent computations that can be scheduled */
        t1 = a[i] * b[i];
        t2 = a[i] + b[i];
        t3 = a[i] - b[i];
        t4 = a[i] / (b[i] + 0.001f);
        
        t5 = c[i] * d[i];
        t6 = c[i] + d[i];
        t7 = c[i] - d[i];
        t8 = c[i] / (d[i] + 0.001f);
        
        t9 = e[i] * t1;
        t10 = e[i] + t2;
        t11 = e[i] - t3;
        t12 = e[i] / (t4 + 0.001f);
        
        t13 = t5 * t6;
        t14 = t7 + t8;
        t15 = t9 - t10;
        t16 = t11 / (t12 + 0.001f);
        
        t17 = t13 * t14;
        t18 = t15 + t16;
        t19 = t17 - t18;
        t20 = t17 / (t18 + 0.001f);
        
        /* Mix with volatile accesses to create delays */
        a[i] = t19 + vol_float1;
        b[i] = t20 * vol_float2;
        
        /* More independent operations */
        c[i] = t1 + t5 + t9 + t13 + t17;
        d[i] = t2 + t6 + t10 + t14 + t18;
        e[i] = t3 + t7 + t11 + t15 + t19;
    }
}

/* Function with mixed dependencies and resource conflicts */
__attribute__((noinline))
void mixed_dependency(int* restrict arr1, int* restrict arr2, 
                      float* restrict farr1, float* restrict farr2, int n) {
    int i;
    
    for (i = 0; i < n; i++) {
        /* Long latency floating point operations */
        float f1 = farr1[i] / 3.14159f;  /* Division has higher latency */
        float f2 = farr2[i] / 2.71828f;
        
        /* Integer operations that can be scheduled independently */
        int i1 = arr1[i] * vol_var1;
        int i2 = arr2[i] + vol_var2;
        int i3 = arr1[i] - arr2[i];
        int i4 = arr1[i] & arr2[i];
        int i5 = arr1[i] | arr2[i];
        int i6 = arr1[i] ^ arr2[i];
        
        /* Memory accesses with potential aliasing */
        farr1[i] = f1 * f2 + (float)i1;
        farr2[i] = f1 / f2 - (float)i2;
        
        /* More independent computations */
        arr1[i] = i3 * i4 + i5;
        arr2[i] = i6 - i3 * i4;
        
        /* Inline assembly to clobber registers and force spills */
        __asm__ volatile (
            "movl $0, %%eax\n\t"
            "movl $1, %%ebx\n\t"
            "movl $2, %%ecx\n\t"
            "movl $3, %%edx\n\t"
            "movl $4, %%esi\n\t"
            "movl $5, %%edi\n\t"
            : /* no outputs */
            : /* no inputs */
            : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
    }
}

/* Function with control flow to create priority differences */
__attribute__((noinline))
int control_flow_test(int* data, int n) {
    int sum = 0;
    int i;
    
    for (i = 0; i < n; i++) {
        /* Branch creates different priority paths */
        if (data[i] > 1000) {
            /* Critical path - high priority */
            sum += data[i] * 2;
            sum -= vol_var1;  /* Volatile access creates delay */
        } else if (data[i] > 500) {
            /* Medium priority path */
            sum += data[i];
            sum ^= vol_var2;  /* Another volatile access */
        } else {
            /* Low priority path */
            sum += data[i] / 2;
        }
        
        /* Independent operations that can be scheduled around branches */
        int temp1 = data[i] << 2;
        int temp2 = data[i] >> 1;
        int temp3 = temp1 + temp2;
        int temp4 = temp1 - temp2;
        
        /* Mix with floating point to compete for functional units */
        float ftemp = (float)data[i] * 1.5f;
        if (ftemp > 1000.0f) {
            sum += (int)ftemp;
        }
        
        data[i] = temp3 + temp4 + (int)ftemp;
    }
    
    return sum;
}

/* Main function that creates the scheduling scenario */
int main() {
    int i;
    clock_t start, end;
    double cpu_time_used;
    
    /* Allocate and initialize arrays with random data */
    float* fa1 = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float* fa2 = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float* fa3 = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float* fa4 = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float* fa5 = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    
    int* ia1 = (int*)aligned_alloc(32, ARRAY_SIZE * sizeof(int));
    int* ia2 = (int*)aligned_alloc(32, ARRAY_SIZE * sizeof(int));
    int* idata = (int*)aligned_alloc(32, ARRAY_SIZE * sizeof(int));
    
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; i++) {
        fa1[i] = (float)rand() / RAND_MAX * 100.0f;
        fa2[i] = (float)rand() / RAND_MAX * 100.0f;
        fa3[i] = (float)rand() / RAND_MAX * 100.0f;
        fa4[i] = (float)rand() / RAND_MAX * 100.0f;
        fa5[i] = (float)rand() / RAND_MAX * 100.0f;
        
        ia1[i] = rand() % 2000;
        ia2[i] = rand() % 2000;
        idata[i] = rand() % 2000;
    }
    
    start = clock();
    
    /* Perform multiple iterations to ensure scheduler sees hot code */
    for (int iter = 0; iter < ITERATIONS / 1000; iter++) {
        /* Call functions that create different scheduling scenarios */
        high_pressure_loop(fa1, fa2, fa3, fa4, fa5, ARRAY_SIZE);
        mixed_dependency(ia1, ia2, fa1, fa2, ARRAY_SIZE);
        
        /* Use result to prevent dead code elimination */
        int result = control_flow_test(idata, ARRAY_SIZE);
        
        /* Prevent compiler from optimizing everything away */
        if (result == 0x12345678) {  /* Never true */
            printf("Unexpected result\n");
        }
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    /* Compute checksum to use all results */
    float fsum = 0.0f;
    int isum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        fsum += fa1[i] + fa2[i] + fa3[i] + fa4[i] + fa5[i];
        isum += ia1[i] + ia2[i] + idata[i];
    }
    
    printf("Checksum: float=%f, int=%d\n", fsum, isum);
    printf("Time used: %f seconds\n", cpu_time_used);
    
    /* Cleanup */
    free(fa1);
    free(fa2);
    free(fa3);
    free(fa4);
    free(fa5);
    free(ia1);
    free(ia2);
    free(idata);
    
    return 0;
}
