/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test_sched_coverage
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test_sched_coverage_pressure
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ITERATIONS 100000
#define ARRAY_SIZE 1024

/* Volatile variables to prevent optimization and create artificial dependencies */
volatile int vol_var1 = 1;
volatile int vol_var2 = 2;
volatile float vol_float1 = 1.5f;
volatile float vol_float2 = 2.5f;

/* Function to create high register pressure with independent instruction groups */
__attribute__((noinline))
void high_pressure_loop(int *arr1, int *arr2, int *arr3, float *farr1, float *farr2) {
    int i;
    /* Many independent integer operations to create scheduling candidates */
    int a, b, c, d, e, f, g, h, j, k, l, m, n, o, p, q, r, s, t;
    float fa, fb, fc, fd, fe, ff, fg, fh, fi, fj;
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        /* Group 1: Independent integer operations */
        a = arr1[i] + vol_var1;
        b = arr2[i] * vol_var2;
        c = arr1[i] - arr2[i];
        d = arr3[i] & 0xFF;
        e = arr1[i] | arr2[i];
        f = arr3[i] ^ arr1[i];
        g = arr2[i] << 2;
        h = arr3[i] >> 1;
        
        /* Group 2: More independent operations */
        j = a * b;
        k = c + d;
        l = e - f;
        m = g ^ h;
        n = j & k;
        o = l | m;
        p = n * 3;
        q = o / 2;
        r = p + q;
        s = r - i;
        t = s * vol_var1;
        
        /* Group 3: Floating point operations (different functional units) */
        fa = farr1[i] * vol_float1;
        fb = farr2[i] / vol_float2;  /* Division has longer latency */
        fc = fa + fb;
        fd = farr1[i] - farr2[i];
        fe = fc * fd;
        ff = fe / vol_float1;
        fg = ff + 1.0f;
        fh = fg * 2.0f;
        fi = fh - 0.5f;
        fj = fi / vol_float2;
        
        /* Mix integer and float to create resource conflicts */
        arr1[i] = t + (int)fj;
        arr2[i] = (int)(fa * 100) + r;
        farr1[i] = fj + (float)t;
        farr2[i] = (float)r * fb;
        
        /* Artificial dependency chain with volatile */
        vol_var1 = (vol_var1 + i) & 0x7F;
        vol_var2 = vol_var2 ^ (i * 3);
    }
}

/* Function with mixed dependencies and control flow for priority variations */
__attribute__((noinline))
void mixed_dependency(int *arr, float *farr) {
    int i;
    int tmp1, tmp2, tmp3, tmp4, tmp5;
    float ftmp1, ftmp2, ftmp3, ftmp4;
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        /* Create data dependencies */
        tmp1 = arr[i] + 1;
        tmp2 = tmp1 * 2;      /* Depends on tmp1 */
        tmp3 = tmp2 - i;      /* Depends on tmp2 */
        
        /* Independent parallel operations */
        tmp4 = arr[i] ^ 0xAA;
        tmp5 = arr[i] | 0x55;
        
        /* Control flow creates priority differences */
        if (tmp3 > 1000) {
            ftmp1 = farr[i] * 2.0f;
            ftmp2 = ftmp1 / 3.14159f;  /* Longer latency divide */
            arr[i] = tmp3 + (int)ftmp2;
        } else {
            ftmp3 = farr[i] + 1.0f;
            ftmp4 = ftmp3 * 0.5f;
            arr[i] = tmp4 + tmp5 + (int)ftmp4;
        }
        
        /* Inline assembly to clobber registers and force spills */
        __asm__ volatile (
            "movl $0, %%eax\n\t"
            "movl $0, %%ebx\n\t"
            "movl $0, %%ecx\n\t"
            "movl $0, %%edx\n\t"
            "movl $0, %%esi\n\t"
            "movl $0, %%edi\n\t"
            : : : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
        
        /* Memory barrier to prevent reordering */
        __asm__ volatile ("" ::: "memory");
    }
}

/* Function with SIMD-like operations for modern architectures */
__attribute__((noinline))
void vector_ops(int *arr1, int *arr2, int *arr3) {
    int i;
    /* Unrolled loop creates many independent instructions */
    for (i = 0; i < ARRAY_SIZE - 3; i += 4) {
        /* Four independent parallel operations */
        arr1[i] = arr2[i] + arr3[i];
        arr1[i+1] = arr2[i+1] * arr3[i+1];
        arr1[i+2] = arr2[i+2] - arr3[i+2];
        arr1[i+3] = arr2[i+3] ^ arr3[i+3];
        
        /* More independent ops */
        arr2[i] = arr1[i] >> 1;
        arr2[i+1] = arr1[i+1] << 1;
        arr2[i+2] = arr1[i+2] & 0xFF;
        arr2[i+3] = arr1[i+3] | 0xAA;
        
        /* Cross-element dependencies */
        arr3[i] = arr1[i] + arr2[i+1];
        arr3[i+1] = arr1[i+1] - arr2[i+2];
        arr3[i+2] = arr1[i+2] * arr2[i+3];
        arr3[i+3] = arr1[i+3] ^ arr2[i];
    }
}

int main() {
    int *arr1, *arr2, *arr3;
    float *farr1, *farr2;
    int i, sum = 0;
    clock_t start, end;
    
    /* Allocate and initialize arrays */
    arr1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    arr2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    arr3 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    farr1 = (float*)malloc(ARRAY_SIZE * sizeof(float));
    farr2 = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        arr3[i] = rand() % 1000;
        farr1[i] = (float)(rand() % 1000) / 10.0f;
        farr2[i] = (float)(rand() % 1000) / 10.0f;
    }
    
    start = clock();
    
    /* Perform many iterations to ensure scheduler sees hot code */
    for (i = 0; i < ITERATIONS; i++) {
        high_pressure_loop(arr1, arr2, arr3, farr1, farr2);
        mixed_dependency(arr1, farr1);
        vector_ops(arr2, arr3, arr1);
        
        /* Modify volatile variables to affect scheduling */
        vol_float1 += 0.1f;
        vol_float2 -= 0.05f;
    }
    
    end = clock();
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < ARRAY_SIZE; i++) {
        sum += arr1[i] + arr2[i] + arr3[i] + (int)farr1[i] + (int)farr2[i];
    }
    
    printf("Checksum: %d\n", sum);
    printf("Time: %.2f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    free(arr1);
    free(arr2);
    free(arr3);
    free(farr1);
    free(farr2);
    
    return 0;
}
