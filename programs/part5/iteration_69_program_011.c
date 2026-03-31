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
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.0f, vol_f2 = 2.0f, vol_f3 = 3.0f;

/* Function to create high register pressure with independent instructions */
__attribute__((noinline))
void high_pressure_loop(float *restrict a, float *restrict b, float *restrict c, 
                        float *restrict d, int n) {
    int i;
    /* Many independent floating point operations to create register pressure */
    for (i = 0; i < n; i++) {
        /* Group 1: Independent FP operations */
        float t1 = a[i] * b[i] + c[i];
        float t2 = a[i] / (b[i] + 1.0f);  /* Division has longer latency */
        float t3 = c[i] * d[i] - a[i];
        float t4 = b[i] / (c[i] + 2.0f);
        float t5 = d[i] * a[i] + b[i];
        float t6 = a[i] - c[i] * d[i];
        float t7 = b[i] * c[i] / (d[i] + 0.5f);
        float t8 = d[i] + a[i] * b[i];
        
        /* Group 2: More independent operations mixing FP and integer */
        int it1 = (int)(t1 * 100.0f);
        int it2 = (int)(t2 * 200.0f);
        float t9 = t3 * vol_f1 + t4 * vol_f2;  /* Volatile creates memory dependency */
        float t10 = t5 / (vol_f3 + t6);
        
        /* Group 3: Cross dependencies to create priority differences */
        a[i] = t1 + t9 * 0.5f;
        b[i] = t2 - t10 * 0.3f;
        c[i] = t3 * a[i] + t4 * b[i];
        d[i] = t5 / (c[i] + 1.0f) + t6;
        
        /* Artificial dependency chain */
        vol_f1 = t7 * 0.1f;
        vol_f2 = t8 * 0.2f;
    }
}

/* Function with mixed operation types and resource conflicts */
__attribute__((noinline))
void mixed_dependency(int *restrict arr1, int *restrict arr2, 
                      float *restrict farr1, float *restrict farr2, int n) {
    int i;
    
    /* Inline assembly to clobber multiple registers and create pressure */
    for (i = 0; i < n; i++) {
        /* Clobber multiple registers to force spills */
        __asm__ volatile (
            "movl $0x1, %%eax\n\t"
            "movl $0x2, %%ebx\n\t"
            "movl $0x3, %%ecx\n\t"
            "movl $0x4, %%edx\n\t"
            "movl $0x5, %%esi\n\t"
            "movl $0x6, %%edi\n\t"
            :
            :
            : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
        
        /* Mixed integer operations with dependencies */
        int x = arr1[i];
        int y = arr2[i];
        int z = x * y + vol_a;  /* Volatile read creates memory dependency */
        int w = y / (vol_b + 1); /* Division with volatile operand */
        
        /* Floating point operations competing for FP units */
        float fx = farr1[i];
        float fy = farr2[i];
        float fz = fx * fy + (float)vol_c;
        float fw = fy / (fx + vol_d * 0.1f); /* Long latency divide */
        
        /* Cross-type operations */
        arr1[i] = z + (int)(fz * 10.0f);
        arr2[i] = w - (int)(fw * 5.0f);
        farr1[i] = fz * 0.9f + (float)arr1[i];
        farr2[i] = fw / 2.0f - (float)arr2[i];
        
        /* Memory barrier to force ordering */
        __asm__ volatile ("" ::: "memory");
    }
}

/* Function with control flow to create priority variations */
__attribute__((noinline))
int control_flow_priority(int *arr, int n) {
    int i, sum = 0;
    
    for (i = 0; i < n; i++) {
        /* Complex if-else chain to create different code paths */
        if (arr[i] > 1000) {
            /* Critical path operations */
            sum += arr[i] * 2;
            arr[i] = sum / (vol_a + 1);
        } else if (arr[i] > 500) {
            /* Less critical path */
            sum += arr[i];
            arr[i] = sum - vol_b;
        } else {
            /* Even less critical */
            sum += arr[i] / 2;
            arr[i] = sum + vol_c;
        }
        
        /* Switch-like structure for more variation */
        switch (arr[i] % 4) {
            case 0:
                sum += vol_d * 3;
                break;
            case 1:
                sum += vol_a * 2;
                break;
            case 2:
                sum += vol_b;
                break;
            case 3:
                sum += vol_c / 2;
                break;
        }
    }
    
    return sum;
}

/* Main computational kernel that gets scheduled */
__attribute__((hot))
void compute_kernel(float *fa, float *fb, float *fc, float *fd,
                    int *ia, int *ib, float *ffa, float *ffb) {
    /* Create independent instruction groups for candidate selection */
    high_pressure_loop(fa, fb, fc, fd, ARRAY_SIZE);
    
    /* Force a scheduling boundary */
    __asm__ volatile ("" ::: "memory");
    
    mixed_dependency(ia, ib, ffa, ffb, ARRAY_SIZE);
    
    /* Another scheduling boundary */
    __asm__ volatile ("" ::: "memory");
    
    int sum = control_flow_priority(ia, ARRAY_SIZE);
    
    /* Use result to prevent dead code elimination */
    vol_d = sum % 1000;
}

int main() {
    /* Allocate and initialize data */
    float *fa = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float *fb = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float *fc = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float *fd = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float *ffa = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    float *ffb = (float*)aligned_alloc(32, ARRAY_SIZE * sizeof(float));
    int *ia = (int*)aligned_alloc(32, ARRAY_SIZE * sizeof(int));
    int *ib = (int*)aligned_alloc(32, ARRAY_SIZE * sizeof(int));
    
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fa[i] = (float)rand() / RAND_MAX * 100.0f;
        fb[i] = (float)rand() / RAND_MAX * 100.0f;
        fc[i] = (float)rand() / RAND_MAX * 100.0f;
        fd[i] = (float)rand() / RAND_MAX * 100.0f;
        ffa[i] = (float)rand() / RAND_MAX * 100.0f;
        ffb[i] = (float)rand() / RAND_MAX * 100.0f;
        ia[i] = rand() % 2000;
        ib[i] = rand() % 2000;
    }
    
    /* Perform many iterations to ensure scheduler sees hot code */
    clock_t start = clock();
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        compute_kernel(fa, fb, fc, fd, ia, ib, ffa, ffb);
        
        /* Modify inputs slightly each iteration */
        if (iter % 100 == 0) {
            fa[iter % ARRAY_SIZE] += 0.1f;
            ia[iter % ARRAY_SIZE] += 1;
        }
    }
    
    clock_t end = clock();
    
    /* Compute checksum to prevent optimization */
    double checksum = 0.0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += fa[i] + fb[i] + fc[i] + fd[i] + 
                   ffa[i] + ffb[i] + ia[i] + ib[i];
    }
    
    printf("Checksum: %f\n", checksum);
    printf("Time: %f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Cleanup */
    free(fa); free(fb); free(fc); free(fd);
    free(ffa); free(ffb); free(ia); free(ib);
    
    return 0;
}
