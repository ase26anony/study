/* Test program to trigger DDG edge creation in GCC's scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Helper function with always_inline to ensure loop body is visible */
static void __attribute__((always_inline)) 
process_element(float* restrict a, float* restrict b, float* restrict c, 
                 float* restrict d, int i, float* tmp, float* sum) {
    /* Memory flow dependency: uses previous iteration's a[i-1] */
    b[i] = a[i] + a[i-1];
    
    /* Register flow dependency: accumulation */
    *sum += a[i] * 1.5f;
    
    /* Anti-dependency: read then write of same location */
    *tmp = c[i];
    c[i] = d[i];
    d[i] = *tmp;
    
    /* Output dependency: multiple writes to same location */
    c[i] = c[i] * 0.9f;
}

/* Main computation function with loop nest */
void __attribute__((noinline))
compute(int n, float* restrict a, float* restrict b, 
        float* restrict c, float* restrict d) {
    float sum = 0.0f;
    float tmp;
    
    /* Outer loop with non-constant bound */
    for (int i = 1; i < n; ++i) {
        /* Mixed data types and dependencies */
        
        /* Memory dependency chain */
        float t1 = a[i] * 2.0f;
        float t2 = a[i-1] * 1.5f;
        
        /* Independent computation for potential parallelism */
        float x = c[i] + d[i];
        float y = c[i] - d[i];
        
        /* Condition creating control dependency */
        if (t1 > t2) {
            /* Memory flow with non-affine index */
            b[i + (i%2)] = t1;
        } else {
            b[i + (i%2)] = t2;
        }
        
        /* Register anti-dependency */
        tmp = x;
        x = y;
        y = tmp;
        
        /* Call to inlined function for more complex pattern */
        process_element(a, b, c, d, i, &tmp, &sum);
        
        /* Nested loop with dependent bound */
        for (int j = i; j < n && j < i + 3; ++j) {
            /* Cross-iteration memory dependency */
            d[j] = d[j] + b[i] * 0.1f;
        }
    }
    
    /* Prevent dead code elimination */
    b[0] = sum;
}

/* Another computation with different pattern */
void __attribute__((noinline))
compute2(int m, int n, float* restrict arr1, float* restrict arr2) {
    float acc1 = 0.0f, acc2 = 0.0f;
    
    #pragma GCC ivdep
    for (int i = 0; i < n; ++i) {
        /* Multiple independent chains */
        float v1 = arr1[i] * 3.14f;
        float v2 = arr2[i] * 2.71f;
        float v3 = v1 + v2;
        float v4 = v1 - v2;
        
        /* Register dependencies */
        acc1 += v3;
        acc2 += v4;
        
        /* Memory output dependency */
        arr1[i] = acc1;
        arr2[i] = acc2;
        
        /* Pointer arithmetic creating complex memory references */
        float* p = arr1 + i;
        *p = *p * 0.99f;
    }
    
    /* Use results */
    arr1[0] = acc1 + acc2;
}

int main() {
    const int N = 1000;
    const int M = 500;
    
    /* Allocate and initialize arrays */
    float* a = (float*)malloc(N * sizeof(float));
    float* b = (float*)malloc(N * sizeof(float));
    float* c = (float*)malloc(N * sizeof(float));
    float* d = (float*)malloc(N * sizeof(float));
    float* arr1 = (float*)malloc(M * sizeof(float));
    float* arr2 = (float*)malloc(M * sizeof(float));
    
    /* Initialize with pattern */
    for (int i = 0; i < N; ++i) {
        a[i] = (float)i * 0.1f;
        b[i] = (float)(N - i) * 0.2f;
        c[i] = (float)(i % 100) * 0.3f;
        d[i] = (float)(i % 50) * 0.4f;
    }
    
    for (int i = 0; i < M; ++i) {
        arr1[i] = (float)i * 0.25f;
        arr2[i] = (float)(M - i) * 0.35f;
    }
    
    /* Call compute with different sizes to trigger various optimizations */
    compute(N, a, b, c, d);
    compute(N/2, a, b, c, d);
    compute(N/4, a, b, c, d);
    
    compute2(M, M, arr1, arr2);
    compute2(M/2, M/2, arr1, arr2);
    
    /* Calculate checksum to prevent elimination */
    float checksum = 0.0f;
    for (int i = 0; i < N; ++i) {
        checksum += a[i] + b[i] + c[i] + d[i];
    }
    for (int i = 0; i < M; ++i) {
        checksum += arr1[i] + arr2[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(arr1);
    free(arr2);
    
    return 0;
}
