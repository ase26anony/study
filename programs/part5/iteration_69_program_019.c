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

/* Function to create high register pressure with independent instructions */
__attribute__((noinline))
void high_pressure_loop(float* restrict a, float* restrict b, float* restrict c, 
                        float* restrict d, float* restrict e, int n) {
    /* Many independent calculations to create scheduling candidates */
    for (int i = 0; i < n; i++) {
        /* Group 1: Independent FP operations */
        float t1 = a[i] * b[i] + c[i];
        float t2 = b[i] * c[i] - d[i];
        float t3 = c[i] * d[i] + e[i];
        float t4 = d[i] * e[i] - a[i];
        float t5 = e[i] * a[i] + b[i];
        
        /* Group 2: More independent operations */
        float t6 = t1 * t2 + t3;
        float t7 = t2 * t3 - t4;
        float t8 = t3 * t4 + t5;
        float t9 = t4 * t5 - t1;
        float t10 = t5 * t1 + t2;
        
        /* Group 3: Mixed operations with dependencies */
        float t11 = t6 / (t7 + 1.0f);  /* FP divide - long latency */
        float t12 = t8 * t9 - t10;
        float t13 = t9 + t10 * t11;
        float t14 = t10 - t11 / t12;
        float t15 = t11 * t12 + t13;
        
        /* Store results creating register pressure */
        a[i] = t11 + t12;
        b[i] = t13 - t14;
        c[i] = t15 * t1;
        d[i] = t2 + t3 * t4;
        e[i] = t5 / (t6 + 0.5f);  /* Another FP divide */
    }
}

/* Function with artificial dependencies and delays */
__attribute__((noinline))
void mixed_dependency(int* restrict arr1, int* restrict arr2, 
                      float* restrict farr1, float* restrict farr2, int n) {
    /* Create varying priorities through control flow */
    for (int i = 0; i < n; i++) {
        int x = arr1[i];
        int y = arr2[i];
        float fx = farr1[i];
        float fy = farr2[i];
        
        /* Volatile accesses create memory dependencies */
        int v1 = vol_var1;
        int v2 = vol_var2;
        float fv1 = vol_float1;
        float fv2 = vol_float2;
        
        /* Independent integer operations (scheduling candidates) */
        int r1 = x + y * v1;
        int r2 = y - x / (v2 + 1);
        int r3 = x * y + v1 * v2;
        int r4 = y / (x + 1) - v1;
        int r5 = x - y + v2 * 2;
        
        /* Floating point with different latencies */
        float fr1 = fx * fy + fv1;      /* Fast */
        float fr2 = fx / (fy + 0.1f);   /* Slow FP divide */
        float fr3 = fy * fv2 - fx;
        float fr4 = fv1 / fv2;          /* Another slow divide */
        float fr5 = fr1 * fr2 + fr3;
        
        /* Conditional to create priority differences */
        if (r1 > r2) {
            fr1 = fr1 * 2.0f;
            r3 = r3 + r4;
        } else {
            fr2 = fr2 / 2.0f;
            r4 = r4 - r5;
        }
        
        /* More independent ops */
        int r6 = r1 * r2 + r3;
        int r7 = r2 - r3 * r4;
        float fr6 = fr3 * fr4 - fr5;
        float fr7 = fr4 / (fr5 + 0.01f);  /* Another divide */
        
        /* Store results */
        arr1[i] = r6 + r7;
        arr2[i] = r7 - r6;
        farr1[i] = fr6 * fr7;
        farr2[i] = fr7 / (fr6 + 0.001f);
        
        /* Update volatiles to create dependencies */
        vol_var1 = r6 & 0xFF;
        vol_var2 = r7 & 0xFF;
        vol_float1 = fr6 * 0.5f;
        vol_float2 = fr7 * 0.25f;
    }
}

/* Function with inline assembly to clobber registers */
__attribute__((noinline))
void asm_register_pressure(int* data, int n) {
    for (int i = 0; i < n; i++) {
        int x = data[i];
        int y = x * 2;
        int z = y + 1;
        
        /* Inline assembly that clobbers multiple registers */
        __asm__ volatile (
            "movl %0, %%eax\n\t"
            "movl %1, %%ebx\n\t"
            "addl %%ebx, %%eax\n\t"
            "imull $3, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "+r" (x)
            : "r" (y)
            : "%eax", "%ebx", "cc"
        );
        
        /* More operations to create scheduling candidates */
        int a = z * x;
        int b = a / (x + 1);
        int c = b + z;
        int d = c * a - b;
        int e = d / (c + 1);
        
        /* Independent group */
        int f = e * 2;
        int g = f + a;
        int h = g - b;
        int j = h * c;
        int k = j / (d + 1);
        
        data[i] = x + a + b + c + d + e + f + g + h + j + k;
    }
}

/* Main computational kernel */
__attribute__((noinline))
void compute_kernel(float* fa, float* fb, float* fc, 
                    float* fd, float* fe,
                    int* ia, int* ib, int n) {
    /* Mix all patterns to create complex scheduling scenario */
    high_pressure_loop(fa, fb, fc, fd, fe, n);
    mixed_dependency(ia, ib, fa, fb, n);
    asm_register_pressure(ia, n);
    
    /* Additional independent operations */
    for (int i = 0; i < n; i++) {
        /* Many independent instructions for scheduler to choose from */
        float t1 = fa[i] * 1.1f;
        float t2 = fb[i] + 2.2f;
        float t3 = fc[i] - 3.3f;
        float t4 = fd[i] / 4.4f;  /* FP divide */
        float t5 = fe[i] * 5.5f;
        
        int r1 = ia[i] * 2;
        int r2 = ib[i] + 3;
        int r3 = r1 - r2;
        int r4 = r2 * r3;
        int r5 = r4 / (r1 + 1);
        
        /* Cross-type operations */
        fa[i] = t1 + (float)r1;
        fb[i] = t2 - (float)r2;
        fc[i] = t3 * (float)r3;
        fd[i] = t4 / ((float)r4 + 1.0f);  /* Another divide */
        fe[i] = t5 + (float)r5;
        
        ia[i] = r1 + r3 + r5;
        ib[i] = r2 * r4 - r1;
    }
}

int main() {
    /* Allocate and initialize data */
    float* fa = malloc(ARRAY_SIZE * sizeof(float));
    float* fb = malloc(ARRAY_SIZE * sizeof(float));
    float* fc = malloc(ARRAY_SIZE * sizeof(float));
    float* fd = malloc(ARRAY_SIZE * sizeof(float));
    float* fe = malloc(ARRAY_SIZE * sizeof(float));
    
    int* ia = malloc(ARRAY_SIZE * sizeof(int));
    int* ib = malloc(ARRAY_SIZE * sizeof(int));
    
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fa[i] = (float)rand() / RAND_MAX;
        fb[i] = (float)rand() / RAND_MAX;
        fc[i] = (float)rand() / RAND_MAX;
        fd[i] = (float)rand() / RAND_MAX;
        fe[i] = (float)rand() / RAND_MAX;
        ia[i] = rand() % 1000;
        ib[i] = rand() % 1000;
    }
    
    /* Perform computation many times to ensure scheduler sees hot code */
    long long checksum = 0;
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        compute_kernel(fa, fb, fc, fd, fe, ia, ib, ARRAY_SIZE);
        
        /* Prevent dead code elimination */
        if (iter % 1000 == 0) {
            for (int i = 0; i < ARRAY_SIZE; i += 128) {
                checksum += (long long)(fa[i] * 1000) +
                           (long long)(fb[i] * 1000) +
                           ia[i] + ib[i];
            }
        }
    }
    
    /* Final checksum output */
    printf("Checksum: %lld\n", checksum);
    
    free(fa);
    free(fb);
    free(fc);
    free(fd);
    free(fe);
    free(ia);
    free(ib);
    
    return 0;
}
