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

/* Volatile variables to prevent optimizations and create dependencies */
volatile int vol_var1 = 1;
volatile int vol_var2 = 2;
volatile float vol_float1 = 1.5f;
volatile float vol_float2 = 2.5f;

/* Function to create high register pressure */
__attribute__((noinline))
void high_pressure_loop(float *restrict a, float *restrict b, float *restrict c, 
                        float *restrict d, int n) {
    int i;
    /* Many independent variables to create register pressure */
    float t1, t2, t3, t4, t5, t6, t7, t8;
    float t9, t10, t11, t12, t13, t14, t15, t16;
    
    for (i = 0; i < n; i++) {
        /* Group 1: Independent floating point operations */
        t1 = a[i] * b[i];
        t2 = a[i] + b[i];
        t3 = a[i] - b[i];
        t4 = a[i] / (b[i] + 1.0f);
        
        /* Group 2: More independent operations */
        t5 = c[i] * d[i];
        t6 = c[i] + d[i];
        t7 = c[i] - d[i];
        t8 = c[i] / (d[i] + 1.0f);
        
        /* Group 3: Cross dependencies to create priority variations */
        t9 = t1 + t5;
        t10 = t2 * t6;
        t11 = t3 - t7;
        t12 = t4 / (t8 + 0.1f);
        
        /* Group 4: More operations to increase pressure */
        t13 = t9 * vol_float1;
        t14 = t10 + vol_float2;
        t15 = t11 - vol_float1;
        t16 = t12 / vol_float2;
        
        /* Store results creating memory dependencies */
        a[i] = t13 + t14;
        b[i] = t15 * t16;
        c[i] = t13 - t14;
        d[i] = t15 / (t16 + 0.01f);
    }
}

/* Function with mixed dependencies and delays */
__attribute__((noinline))
void mixed_dependency_ops(int *restrict arr1, int *restrict arr2, 
                          float *restrict farr1, float *restrict farr2, int n) {
    int i;
    
    for (i = 0; i < n; i++) {
        /* Long latency operations mixed with fast ones */
        float fdiv = farr1[i] / (farr2[i] + 0.001f);  /* Slow float divide */
        int imul = arr1[i] * arr2[i];                 /* Fast integer multiply */
        float fsqrt = sqrtf(farr1[i]);                /* Slow sqrt */
        int iadd = vol_var1 + vol_var2;               /* Volatile dependency */
        
        /* Artificial dependencies via inline asm */
        int asm_dep;
        __asm__ volatile (
            "mov %1, %%eax\n\t"
            "add %2, %%eax\n\t"
            "mov %%eax, %0\n\t"
            : "=r" (asm_dep)
            : "r" (imul), "r" (iadd)
            : "%eax"
        );
        
        /* More operations with different latencies */
        float fmul = fdiv * fsqrt;
        int isub = asm_dep - vol_var1;
        
        /* Store with different patterns */
        arr1[i] = isub + (int)fmul;
        arr2[i] = imul / (vol_var2 + 1);
        farr1[i] = fmul + (float)asm_dep;
        farr2[i] = fsqrt * vol_float1;
        
        /* Conditional to create control flow and priority differences */
        if (arr1[i] > 1000) {
            arr1[i] = arr1[i] / 2;
            vol_var1++;  /* Volatile write creates memory barrier */
        } else {
            arr2[i] = arr2[i] * 3;
            vol_var2++;  /* Another volatile write */
        }
    }
}

/* Function with completely independent instruction groups */
__attribute__((noinline))
void independent_groups(double *restrict d1, double *restrict d2, 
                        double *restrict d3, double *restrict d4, int n) {
    int i;
    
    for (i = 0; i < n; i++) {
        /* Group A: 4 independent double operations */
        double a1 = d1[i] * 1.1;
        double a2 = d2[i] + 2.2;
        double a3 = d3[i] - 3.3;
        double a4 = d4[i] / 4.4;
        
        /* Group B: Another 4 independent operations */
        double b1 = sin(d1[i]);
        double b2 = cos(d2[i]);
        double b3 = d3[i] * d3[i];
        double b4 = sqrt(fabs(d4[i]));
        
        /* Group C: Mixed operations */
        double c1 = a1 + b1;
        double c2 = a2 * b2;
        double c3 = a3 - b3;
        double c4 = a4 / (b4 + 0.001);
        
        /* Store results - creates dependencies but scheduler has many candidates */
        d1[i] = c1;
        d2[i] = c2;
        d3[i] = c3;
        d4[i] = c4;
    }
}

/* Function that uses many registers with unrolled loops */
__attribute__((noinline))
void unrolled_high_pressure(int *restrict out, int *restrict in, int n) {
    int i;
    /* Unrolled loop creates many temporary variables */
    for (i = 0; i < n - 3; i += 4) {
        int r0 = in[i] * 3;
        int r1 = in[i+1] + 7;
        int r2 = in[i+2] - 5;
        int r3 = in[i+3] / 2;
        
        int s0 = r0 ^ r1;
        int s1 = r1 & r2;
        int s2 = r2 | r3;
        int s3 = r3 ^ r0;
        
        int t0 = s0 << 1;
        int t1 = s1 >> 2;
        int t2 = s2 << 3;
        int t3 = s3 >> 1;
        
        out[i] = t0 + vol_var1;
        out[i+1] = t1 - vol_var2;
        out[i+2] = t2 * vol_var1;
        out[i+3] = t3 / (vol_var2 + 1);
    }
}

int main() {
    int i;
    float *fa, *fb, *fc, *fd;
    int *ia, *ib;
    double *da, *db, *dc, *dd;
    int *res;
    
    /* Allocate and initialize arrays */
    fa = (float*)malloc(ARRAY_SIZE * sizeof(float));
    fb = (float*)malloc(ARRAY_SIZE * sizeof(float));
    fc = (float*)malloc(ARRAY_SIZE * sizeof(float));
    fd = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    ia = (int*)malloc(ARRAY_SIZE * sizeof(int));
    ib = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    da = (double*)malloc(ARRAY_SIZE * sizeof(double));
    db = (double*)malloc(ARRAY_SIZE * sizeof(double));
    dc = (double*)malloc(ARRAY_SIZE * sizeof(double));
    dd = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    res = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    srand(time(NULL));
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        fa[i] = (float)rand() / RAND_MAX * 100.0f;
        fb[i] = (float)rand() / RAND_MAX * 100.0f;
        fc[i] = (float)rand() / RAND_MAX * 100.0f;
        fd[i] = (float)rand() / RAND_MAX * 100.0f;
        
        ia[i] = rand() % 1000;
        ib[i] = rand() % 1000;
        
        da[i] = (double)rand() / RAND_MAX * 100.0;
        db[i] = (double)rand() / RAND_MAX * 100.0;
        dc[i] = (double)rand() / RAND_MAX * 100.0;
        dd[i] = (double)rand() / RAND_MAX * 100.0;
    }
    
    /* Perform many iterations to ensure scheduler analyzes hot code */
    for (i = 0; i < ITERATIONS; i++) {
        high_pressure_loop(fa, fb, fc, fd, ARRAY_SIZE);
        mixed_dependency_ops(ia, ib, fa, fb, ARRAY_SIZE);
        independent_groups(da, db, dc, dd, ARRAY_SIZE);
        unrolled_high_pressure(res, ia, ARRAY_SIZE);
        
        /* Modify volatile variables to create changing dependencies */
        vol_var1 = (vol_var1 * 3 + 1) % 100;
        vol_var2 = (vol_var2 * 5 + 2) % 100;
        vol_float1 = sinf(vol_float1) * 2.0f;
        vol_float2 = cosf(vol_float2) * 2.0f;
    }
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += fa[i] + fb[i] + fc[i] + fd[i];
        checksum += ia[i] + ib[i];
        checksum += da[i] + db[i] + dc[i] + dd[i];
        checksum += res[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(fa); free(fb); free(fc); free(fd);
    free(ia); free(ib);
    free(da); free(db); free(dc); free(dd);
    free(res);
    
    return 0;
}
