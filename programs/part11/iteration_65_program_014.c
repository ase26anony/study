#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100
#define M 50

/* Pattern 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
inline void pattern1_flow_dependence(int *a, int *b, int n) {
    /* Flow dependence with distance 2 */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i];
    }
    
    /* Anti-dependence with distance 1 */
    for (int i = 1; i < n; i++) {
        int temp = b[i - 1];
        b[i - 1] = a[i];
        a[i] = temp + i;
    }
}

/* Pattern 2: Multiple dependence types in single loop */
__attribute__((always_inline))
inline void pattern2_mixed_dependencies(double *arr, float *farr, int n) {
    double t = 0.0;
    
    /* Contains flow, anti, and output dependencies */
    for (int i = 1; i < n; i++) {
        /* Flow dependence: read arr[i-1] before writing arr[i] */
        t = arr[i - 1];
        
        /* Anti-dependence: read farr[i] before writing arr[i] */
        arr[i] = t + (double)farr[i];
        
        /* Output dependence: write arr[i] then potentially arr[i] again */
        if (i % 3 == 0) {
            arr[i] *= 2.0;
        }
        
        /* Flow dependence through farr */
        farr[i] = farr[i - 1] * 1.5f;
    }
}

/* Pattern 3: Pointer aliasing with restrict and non-restrict */
void pattern3_aliasing(int * restrict p, int *q, int n) {
    /* With restrict, compiler knows p and q don't alias */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];
    }
}

void pattern3_no_restrict(int *p, int *q, int n) {
    /* Without restrict, compiler must assume p and q might alias */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 13) % n];
    }
}

/* Pattern 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
inline void pattern4_nested_loops(int arr[M][N]) {
    /* Inner loop has carried dependence with distance 2 */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 4
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + arr[i][j - 1];
        }
    }
    
    /* Outer loop carried dependence */
    for (int i = 2; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] += arr[i - 2][j];
        }
    }
}

/* Pattern 5: Volatile memory accesses to force dependencies */
void pattern5_volatile_deps(volatile int *v, int n) {
    /* Strong memory dependencies enforced by volatile */
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + i;
    }
    
    /* Volatile with distance 3 */
    for (int i = 3; i < n; i++) {
        v[i] = v[i - 3] * 2 - v[i - 1];
    }
}

/* Pattern 6: Complex addressing with mixed data types */
void pattern6_complex_addressing(char *carr, short *sarr, int *iarr, int n) {
    /* Different data types create different DDG edge data_type fields */
    for (int i = 4; i < n; i++) {
        /* Multiple interleaved dependencies */
        short temp = sarr[i - 2];
        sarr[i - 2] = (short)carr[i] * 2;
        carr[i] = (char)(temp / 3);
        iarr[i] = iarr[i - 4] + (int)sarr[i - 1];
    }
}

/* Pattern 7: Software pipelining candidate with long latency chain */
void pattern7_long_latency_chain(float *fa, float *fb, float *fc, int n) {
    /* Long dependency chain good for modulo scheduling */
    for (int i = 3; i < n; i++) {
        fa[i] = fb[i - 1] * 0.5f;
        fb[i] = fc[i - 2] + 1.0f;
        fc[i] = fa[i - 3] * fb[i - 1] - fc[i - 1];
    }
}

int main() {
    /* Initialize with deterministic but varied data */
    srand(42);
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    double *darr = (double*)malloc(SIZE * sizeof(double));
    float *farr = (float*)malloc(SIZE * sizeof(float));
    volatile int *varr = (volatile int*)malloc(SIZE * sizeof(int));
    char *carr = (char*)malloc(SIZE * sizeof(char));
    short *sarr = (short*)malloc(SIZE * sizeof(short));
    int *iarr = (int*)malloc(SIZE * sizeof(int));
    float *fa = (float*)malloc(SIZE * sizeof(float));
    float *fb = (float*)malloc(SIZE * sizeof(float));
    float *fc = (float*)malloc(SIZE * sizeof(float));
    int arr2d[M][N];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        darr[i] = (double)(rand() % 100) / 3.0;
        farr[i] = (float)(rand() % 100) / 2.0f;
        varr[i] = rand() % 50;
        carr[i] = (char)(rand() % 128);
        sarr[i] = (short)(rand() % 1000);
        iarr[i] = rand() % 200;
        if (i < SIZE) {
            fa[i] = (float)(rand() % 100) / 4.0f;
            fb[i] = (float)(rand() % 100) / 5.0f;
            fc[i] = (float)(rand() % 100) / 6.0f;
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr2d[i][j] = rand() % 100;
        }
    }
    
    volatile long long checksum = 0;
    
    /* Execute all patterns to create various DDG edges */
    pattern1_flow_dependence(a, b, SIZE);
    pattern2_mixed_dependencies(darr, farr, SIZE);
    pattern3_aliasing(a, b, SIZE);
    pattern3_no_restrict(b, a, SIZE);
    pattern4_nested_loops(arr2d);
    pattern5_volatile_deps(varr, SIZE);
    pattern6_complex_addressing(carr, sarr, iarr, SIZE);
    pattern7_long_latency_chain(fa, fb, fc, SIZE);
    
    /* Accumulate results to prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        checksum += a[i] + b[i] + (int)darr[i] + (int)farr[i] + varr[i] + 
                   carr[i] + sarr[i] + iarr[i] + (int)fa[i] + (int)fb[i] + (int)fc[i];
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            checksum += arr2d[i][j];
        }
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(darr); free(farr); free((int*)varr);
    free(carr); free(sarr); free(iarr); free(fa); free(fb); free(fc);
    
    return 0;
}
