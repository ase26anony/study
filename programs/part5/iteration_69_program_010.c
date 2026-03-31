/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1024
#define ITERATIONS 100000

/* Volatile variables to prevent optimization and create dependencies */
volatile int vol_var1 = 1;
volatile int vol_var2 = 2;
volatile float vol_float1 = 1.5f;
volatile float vol_float2 = 2.5f;

/* Function to create high register pressure with independent instructions */
void high_pressure_loop(float *a, float *b, float *c, float *d, int n) {
    /* Many independent calculations to create scheduling candidates */
    for (int i = 0; i < n; i++) {
        /* Group 1: Independent FP operations */
        float t1 = a[i] * b[i];
        float t2 = c[i] / (d[i] + 0.1f);  /* FP divide for latency */
        float t3 = t1 + t2;
        float t4 = t1 - t2;
        
        /* Group 2: More independent operations */
        float t5 = b[i] * c[i];
        float t6 = a[i] / (d[i] + 0.2f);
        float t7 = t5 * t6;
        float t8 = t5 / (t6 + 0.001f);
        
        /* Group 3: Integer operations mixed with FP */
        int it1 = (int)(t1 * 100);
        int it2 = (int)(t2 * 100);
        int it3 = it1 + it2;
        int it4 = it1 - it2;
        
        /* Group 4: More FP with dependencies */
        float t9 = t3 * t4;
        float t10 = t7 * t8;
        float t11 = t9 + t10;
        float t12 = t9 - t10;
        
        /* Use results to prevent elimination */
        a[i] = t11 + t12;
        b[i] = t3 + t4;
        c[i] = t7 + t8;
        d[i] = (float)(it3 + it4);
    }
}

/* Function with artificial dependencies and delays */
void mixed_dependency(int *arr1, int *arr2, float *farr1, float *farr2, int n) {
    /* Create long dependency chains */
    int dep1 = vol_var1;
    int dep2 = vol_var2;
    float fdep1 = vol_float1;
    float fdep2 = vol_float2;
    
    for (int i = 0; i < n; i++) {
        /* Chain 1: Integer dependency chain */
        dep1 = dep1 * arr1[i] + dep2;
        dep2 = dep2 * arr2[i] + dep1;
        arr1[i] = dep1;
        arr2[i] = dep2;
        
        /* Chain 2: Floating point dependency chain with divides */
        fdep1 = fdep1 / (farr1[i] + 1.0f);  /* High latency operation */
        fdep2 = fdep2 / (farr2[i] + 1.0f);
        farr1[i] = fdep1 * fdep2;
        farr2[i] = fdep1 / (fdep2 + 0.0001f);
        
        /* Independent operations that can be scheduled in parallel */
        int ind1 = i * 3;
        int ind2 = i * 5;
        int ind3 = ind1 + ind2;
        int ind4 = ind1 - ind2;
        
        float find1 = (float)i * 0.3f;
        float find2 = (float)i * 0.7f;
        float find3 = find1 * find2;
        float find4 = find1 / (find2 + 0.001f);
        
        /* Mix with volatile to create memory barriers */
        if (i % 32 == 0) {
            dep1 += vol_var1;
            fdep1 += vol_float1;
        }
    }
    
    /* Store final values to volatiles */
    vol_var1 = dep1;
    vol_var2 = dep2;
    vol_float1 = fdep1;
    vol_float2 = fdep2;
}

/* Function with inline assembly to clobber registers and create pressure */
void asm_pressure(int *a, int *b, int n) {
    for (int i = 0; i < n; i++) {
        int x = a[i];
        int y = b[i];
        int res1, res2, res3, res4;
        
        /* Inline assembly that uses and clobbers multiple registers */
        __asm__ volatile (
            "movl %1, %%eax\n\t"
            "movl %2, %%ebx\n\t"
            "imull %%ebx, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (res1)
            : "r" (x), "r" (y)
            : "%eax", "%ebx", "memory"
        );
        
        __asm__ volatile (
            "movl %1, %%ecx\n\t"
            "movl %2, %%edx\n\t"
            "addl %%edx, %%ecx\n\t"
            "movl %%ecx, %0\n\t"
            : "=r" (res2)
            : "r" (x), "r" (y)
            : "%ecx", "%edx", "memory"
        );
        
        /* More operations to create scheduling candidates */
        res3 = res1 * res2;
        res4 = res1 + res2;
        
        a[i] = res3;
        b[i] = res4;
    }
}

/* Function with control flow to create priority differences */
void control_flow_priority(float *data, int n) {
    float acc1 = 0.0f, acc2 = 0.0f, acc3 = 0.0f, acc4 = 0.0f;
    float tmp1, tmp2, tmp3, tmp4;
    
    for (int i = 0; i < n; i++) {
        /* Basic block with independent instructions */
        tmp1 = data[i] * 1.1f;
        tmp2 = data[i] / 1.1f;  /* FP divide for latency */
        tmp3 = tmp1 + tmp2;
        tmp4 = tmp1 - tmp2;
        
        /* Control flow creates different priorities */
        if (tmp3 > 0.0f) {
            acc1 += tmp3;
            /* More operations in taken branch */
            float t = acc1 * 0.5f;
            acc1 = t + tmp4;
        } else {
            acc2 += tmp4;
            /* Different operations in else branch */
            float t = acc2 * 0.3f;
            acc2 = t - tmp3;
        }
        
        /* Loop with break creates additional control flow */
        if (i % 7 == 0) {
            acc3 += tmp1 * tmp2;
            if (acc3 > 1000.0f) {
                acc3 = 0.0f;
            }
        }
        
        /* Unpredictable branch */
        if ((i * 17) % 13 == 0) {
            acc4 = acc4 * 0.9f + tmp4;
        }
    }
    
    /* Use results */
    data[0] = acc1 + acc2 + acc3 + acc4;
}

/* Main function that creates the hot loops for scheduling analysis */
int main() {
    /* Allocate and initialize data */
    float *fa = (float*)malloc(SIZE * sizeof(float));
    float *fb = (float*)malloc(SIZE * sizeof(float));
    float *fc = (float*)malloc(SIZE * sizeof(float));
    float *fd = (float*)malloc(SIZE * sizeof(float));
    
    int *ia = (int*)malloc(SIZE * sizeof(int));
    int *ib = (int*)malloc(SIZE * sizeof(int));
    int *ic = (int*)malloc(SIZE * sizeof(int));
    int *id = (int*)malloc(SIZE * sizeof(int));
    
    srand(time(NULL));
    
    for (int i = 0; i < SIZE; i++) {
        fa[i] = (float)rand() / RAND_MAX * 100.0f;
        fb[i] = (float)rand() / RAND_MAX * 100.0f;
        fc[i] = (float)rand() / RAND_MAX * 100.0f;
        fd[i] = (float)rand() / RAND_MAX * 100.0f;
        
        ia[i] = rand() % 1000;
        ib[i] = rand() % 1000;
        ic[i] = rand() % 1000;
        id[i] = rand() % 1000;
    }
    
    printf("Starting scheduling test...\n");
    
    /* Perform many iterations to make code "hot" for scheduler */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call functions with different characteristics */
        high_pressure_loop(fa, fb, fc, fd, SIZE / 4);
        mixed_dependency(ia, ib, fa, fb, SIZE / 8);
        asm_pressure(ic, id, SIZE / 16);
        control_flow_priority(fc, SIZE / 32);
        
        /* Prevent loop elimination */
        if (iter % 1000 == 0) {
            printf(".");
            fflush(stdout);
        }
    }
    
    /* Compute checksum to use all results */
    float checksum = 0.0f;
    for (int i = 0; i < SIZE; i++) {
        checksum += fa[i] + fb[i] + fc[i] + fd[i];
        checksum += (float)(ia[i] + ib[i] + ic[i] + id[i]);
    }
    
    printf("\nChecksum: %f\n", checksum);
    
    /* Cleanup */
    free(fa); free(fb); free(fc); free(fd);
    free(ia); free(ib); free(ic); free(id);
    
    return 0;
}
