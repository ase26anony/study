/* Test program to trigger DDG edge creation in GCC's scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Helper function with always_inline to ensure loop body is visible */
static void __attribute__((always_inline)) 
process_element(float* restrict a, float* restrict b, float* restrict c, 
                 float* restrict d, int i, int prev, float* acc, float* tmp) {
    /* Memory flow dependency: uses previous iteration's value */
    b[i] = a[i] + a[prev];
    
    /* Register flow dependency: accumulation */
    *acc += c[i] * 1.5f;
    
    /* Anti and output dependencies: swap with temporary */
    *tmp = d[i];
    d[i] = d[prev];
    d[prev] = *tmp;
}

/* Main computation with loop-carried dependencies */
void __attribute__((noinline))
compute(int n, float* restrict a, float* restrict b, 
        float* restrict c, float* restrict d) {
    float acc = 0.0f;
    float tmp = 0.0f;
    
    /* Complex loop with multiple dependency types */
    #pragma GCC ivdep
    for (int i = 1; i < n; ++i) {
        int prev = i - 1;
        
        /* Process element with mixed dependencies */
        process_element(a, b, c, d, i, prev, &acc, &tmp);
        
        /* Conditional creating control dependency */
        if (acc > 100.0f) {
            /* Memory output dependency */
            a[i] = acc * 0.01f;
            acc = acc * 0.5f;  /* Register output dependency */
        }
        
        /* Non-linear access pattern for memory dependency analysis */
        int idx = i + (i % 3) - 1;
        if (idx >= 0 && idx < n) {
            /* Cross-iteration memory anti-dependency */
            float val = b[idx];
            c[i] = val * 2.0f + c[prev];
        }
    }
    
    /* Use result to prevent dead code elimination */
    a[0] = acc;
}

/* Nested loop with outer-loop dependent bounds */
void __attribute__((noinline))
nested_compute(int n, float* restrict arr1, float* restrict arr2) {
    for (int i = 0; i < n; ++i) {
        float local_acc = 0.0f;
        
        /* Inner loop bound depends on outer index */
        for (int j = i; j < n; ++j) {
            /* Flow dependency through local_acc */
            local_acc += arr1[j] * arr2[i];
            
            /* Memory flow with distance > 1 */
            if (j > 1) {
                arr2[j] = arr1[j] + arr1[j-2];  /* Distance 2 dependency */
            }
            
            /* Conditional with register dependency */
            if (local_acc > arr2[i]) {
                arr1[j] = local_acc;
                local_acc *= 0.9f;  /* Register output dependency */
            }
        }
        
        /* Cross-iteration dependency through arr2 */
        if (i > 0) {
            arr2[i] += arr2[i-1];
        }
    }
}

int main() {
    const int sizes[] = {500, 1000, 1500};
    const int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    for (int s = 0; s < num_sizes; ++s) {
        int n = sizes[s];
        
        /* Allocate with alignment for better optimization */
        float* a = (float*)aligned_alloc(16, n * sizeof(float));
        float* b = (float*)aligned_alloc(16, n * sizeof(float));
        float* c = (float*)aligned_alloc(16, n * sizeof(float));
        float* d = (float*)aligned_alloc(16, n * sizeof(float));
        float* arr1 = (float*)aligned_alloc(16, n * sizeof(float));
        float* arr2 = (float*)aligned_alloc(16, n * sizeof(float));
        
        /* Initialize with pattern (not just zeros) */
        for (int i = 0; i < n; ++i) {
            a[i] = (float)(i % 100) * 0.1f;
            b[i] = (float)((i + 1) % 100) * 0.2f;
            c[i] = (float)((i * 2) % 100) * 0.3f;
            d[i] = (float)((i * 3) % 100) * 0.4f;
            arr1[i] = (float)(i % 50) * 0.5f;
            arr2[i] = (float)((i + 2) % 50) * 0.6f;
        }
        
        /* Call compute functions multiple times */
        for (int iter = 0; iter < 3; ++iter) {
            compute(n, a, b, c, d);
            nested_compute(n, arr1, arr2);
        }
        
        /* Compute checksum to prevent elimination */
        float checksum = 0.0f;
        for (int i = 0; i < n; ++i) {
            checksum += a[i] + b[i] + c[i] + d[i] + arr1[i] + arr2[i];
        }
        
        printf("Size %d: checksum = %f\n", n, checksum);
        
        /* Cleanup */
        free(a); free(b); free(c); free(d);
        free(arr1); free(arr2);
    }
    
    return 0;
}
