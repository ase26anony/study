/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test_sched_coverage
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test_sched_pressure
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1024
#define ITERATIONS 100000

/* Volatile variables to prevent optimization and create dependencies */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.0f, vol_f2 = 2.0f, vol_f3 = 3.0f;

/* Function to create high register pressure with independent instructions */
void high_pressure_loop(int *arr1, int *arr2, int *arr3, float *farr1, float *farr2) {
    int i;
    /* Many independent integer operations to create scheduling candidates */
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    float ft1, ft2, ft3, ft4, ft5, ft6, ft7, ft8, ft9, ft10;
    
    /* Unrolled loop with many live variables */
    for (i = 0; i < SIZE; i += 10) {
        /* Group 1: Independent integer operations */
        t1 = arr1[i] + arr2[i];
        t2 = arr1[i+1] * arr2[i+1];
        t3 = arr1[i+2] - arr2[i+2];
        t4 = arr1[i+3] ^ arr2[i+3];
        t5 = arr1[i+4] | arr2[i+4];
        
        /* Group 2: More independent operations */
        t6 = arr3[i] << 2;
        t7 = arr3[i+1] >> 1;
        t8 = arr3[i+2] & 0xFF;
        t9 = arr3[i+3] * vol_a;  /* Volatile dependency */
        t10 = arr3[i+4] + vol_b;
        
        /* Group 3: Floating point operations (different functional units) */
        ft1 = farr1[i] * farr2[i];
        ft2 = farr1[i+1] / farr2[i+1];  /* Division has longer latency */
        ft3 = farr1[i+2] + farr2[i+2];
        ft4 = farr1[i+3] - farr2[i+3];
        ft5 = sqrtf(farr1[i+4] * farr1[i+4] + farr2[i+4] * farr2[i+4]);
        
        /* Group 4: Mixed operations with dependencies */
        t11 = t1 + t2;
        t12 = t3 - t4;
        t13 = t5 ^ t6;
        t14 = t7 | t8;
        t15 = t9 * t10;
        
        /* More floating point */
        ft6 = ft1 + vol_f1;  /* Volatile dependency creates delay */
        ft7 = ft2 / vol_f2;
        ft8 = ft3 * ft4;
        ft9 = ft5 + 1.0f;
        ft10 = ft6 * ft7;
        
        /* Final computations with all values to prevent dead code elimination */
        arr1[i] = t11 + t12 + t13 + t14 + t15;
        farr1[i] = ft8 + ft9 + ft10 + ft1 + ft2;
        
        /* Use inline assembly to clobber registers and increase pressure */
        asm volatile ("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4), "r"(t5),
                         "r"(t6), "r"(t7), "r"(t8), "r"(t9), "r"(t10) : );
        asm volatile ("" : : "f"(ft1), "f"(ft2), "f"(ft3), "f"(ft4), "f"(ft5) : );
    }
}

/* Function with artificial resource conflicts and delays */
void mixed_dependency(int *arr, float *farr) {
    int i;
    int a, b, c, d, e, f, g, h;
    float fa, fb, fc, fd, fe, ff, fg, fh;
    
    /* Create long dependency chains mixed with independent ops */
    for (i = 0; i < SIZE; i++) {
        /* Long latency chain 1 */
        a = arr[i] + vol_c;      /* Volatile read - potential delay */
        b = a * a;
        c = b / (vol_d + 1);     /* Another volatile read */
        d = c << 3;
        e = d ^ 0xAAAA;
        
        /* Long latency chain 2 (floating point) */
        fa = farr[i] * vol_f3;   /* Volatile */
        fb = fa / 2.0f;          /* Division */
        fc = fb * fb;
        fd = sqrtf(fc);          /* Square root - long latency */
        fe = fd + 1.0f;
        
        /* Independent operations that can be scheduled in parallel */
        f = arr[SIZE - i - 1] * 3;
        g = arr[i] & 0xFF;
        h = f ^ g;
        
        ff = farr[SIZE - i - 1] * 1.5f;
        fg = ff + 0.5f;
        fh = fg * fg;
        
        /* Cross-type dependencies to create interesting scheduling */
        arr[i] = e + h + (int)fe;
        farr[i] = fh + fe;
        
        /* Memory barrier to force ordering sometimes */
        if (i % 128 == 0) {
            asm volatile ("" ::: "memory");
        }
    }
}

/* Function with control flow to create priority differences */
void control_flow_priority(int *arr, int threshold) {
    int i;
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    
    for (i = 0; i < SIZE; i++) {
        /* Different paths create different instruction priorities */
        if (arr[i] > threshold) {
            /* Critical path */
            sum1 += arr[i] * 2;
            sum2 += arr[i] >> 1;
            
            /* Inline assembly with specific constraints */
            asm volatile ("/* constraint */" : "+r"(sum1), "+r"(sum2) : : );
        } else if (arr[i] < -threshold) {
            /* Less critical path */
            sum3 -= arr[i];
            sum4 = sum4 ^ arr[i];
        } else {
            /* Neutral path - independent operations */
            int t1 = arr[i] + 1;
            int t2 = arr[i] - 1;
            int t3 = t1 * t2;
            int t4 = t3 / 2;
            sum1 += t4;
            sum3 ^= t4;
            
            /* More independent ops */
            t1 = arr[i] << 2;
            t2 = arr[i] >> 2;
            t3 = t1 | t2;
            t4 = t3 & 0xFF;
            sum2 += t4;
            sum4 -= t4;
        }
        
        /* Loop-carried dependency */
        arr[i] = sum1 + sum2 + sum3 + sum4;
    }
}

int main() {
    int *arr1, *arr2, *arr3;
    float *farr1, *farr2;
    int i;
    clock_t start, end;
    double cpu_time_used;
    
    /* Allocate and initialize arrays */
    arr1 = (int*)malloc(SIZE * sizeof(int));
    arr2 = (int*)malloc(SIZE * sizeof(int));
    arr3 = (int*)malloc(SIZE * sizeof(int));
    farr1 = (float*)malloc(SIZE * sizeof(float));
    farr2 = (float*)malloc(SIZE * sizeof(float));
    
    srand(time(NULL));
    for (i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        arr3[i] = rand() % 1000;
        farr1[i] = (float)(rand() % 1000) / 100.0f;
        farr2[i] = (float)(rand() % 1000) / 100.0f;
    }
    
    start = clock();
    
    /* Perform multiple iterations to ensure hot code scheduling */
    for (i = 0; i < ITERATIONS / 100; i++) {
        high_pressure_loop(arr1, arr2, arr3, farr1, farr2);
        mixed_dependency(arr1, farr1);
        control_flow_priority(arr2, 500);
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    float fchecksum = 0.0f;
    for (i = 0; i < SIZE; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i];
        fchecksum += farr1[i] + farr2[i];
    }
    
    printf("Checksum: %lld, Float checksum: %f\n", checksum, fchecksum);
    printf("Time used: %f seconds\n", cpu_time_used);
    
    free(arr1);
    free(arr2);
    free(arr3);
    free(farr1);
    free(farr2);
    
    return 0;
}
