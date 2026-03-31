/* Test program to trigger DDG edge creation in GCC's instruction scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Helper function with always_inline to ensure loop body is visible */
static void __attribute__((always_inline)) 
process_element(float* restrict a, float* restrict b, float* restrict c, 
                 float* restrict d, int i, int prev, float* sum, float* tmp) {
    /* Memory flow dependency: b[i] depends on a[i] and a[i-1] */
    b[i] = a[i] + a[prev];
    
    /* Register flow dependency: sum accumulates across iterations */
    *sum += b[i] * 1.5f;
    
    /* Anti-dependency: tmp is read before being written */
    float old_tmp = *tmp;
    
    /* Output dependency: tmp is reassigned */
    *tmp = c[i] * 2.0f;
    
    /* Condition dependency: d update depends on comparison */
    if (old_tmp > *tmp) {
        /* Memory output dependency: d[i] is written */
        d[i] = old_tmp - *tmp;
    } else {
        d[i] = *tmp - old_tmp;
    }
    
    /* Complex memory access with potential aliasing */
    c[i + (i % 3)] = d[i] * 0.5f;
}

/* Main computation with loop-carried dependencies */
void __attribute__((noinline))
compute(int n, float* restrict a, float* restrict b, 
        float* restrict c, float* restrict d) {
    float sum = 0.0f;
    float tmp = 0.0f;
    
    /* Loop with cross-iteration dependencies */
    #pragma GCC ivdep  /* Assert no loop-carried memory deps (forces analysis) */
    for (int i = 1; i < n - 1; ++i) {
        int prev = i - 1;
        int next = i + 1;
        
        /* Multiple independent chains with dependencies */
        float x = a[i] * 3.14f;
        float y = b[prev] + 2.71f;
        float z = x + y;  /* Register dependency chain */
        
        /* Anti-dependency: a[i] read before written */
        float old_a = a[i];
        a[i] = z * old_a;
        
        /* Process element creates various edge types */
        process_element(a, b, c, d, i, prev, &sum, &tmp);
        
        /* Write-after-write dependency on c */
        c[next] = c[i] * 0.9f;
        
        /* Nested loop-like pattern with dependency */
        for (int j = i; j < i + 2 && j < n; ++j) {
            /* Memory flow dependency across outer iterations */
            d[j] += a[i] * 0.1f;
        }
        
        /* Conditional creating control dependencies */
        if (sum > 100.0f) {
            tmp = sum * 0.01f;  /* Register output dependency */
        }
    }
    
    /* Final reduction to prevent dead code elimination */
    a[0] = sum;
}

/* Secondary computation with different pattern */
void __attribute__((noinline))
compute2(int m, int n, float* restrict arr1, float* restrict arr2) {
    /* Nested loops with dependencies */
    for (int i = 0; i < m; ++i) {
        float acc = arr1[i];
        for (int j = i; j < n; ++j) {
            /* Flow dependency on acc across inner iterations */
            acc += arr2[j] * (i + 1);
            /* Anti-dependency: arr2 read before written */
            float temp = arr2[j];
            arr2[j] = acc;
            arr1[i] = temp;  /* Output dependency on arr1[i] */
        }
        /* Cross-iteration dependency through arr1 */
        if (i > 0) {
            arr1[i] += arr1[i - 1];
        }
    }
}

int main(int argc, char** argv) {
    /* Use non-constant sizes to prevent full unrolling */
    int size1 = (argc > 1) ? atoi(argv[1]) : 1000;
    int size2 = (argc > 2) ? atoi(argv[2]) : 500;
    
    if (size1 < 10) size1 = 1000;
    if (size2 < 10) size2 = 500;
    
    /* Allocate and initialize arrays */
    float* a = (float*)malloc(size1 * sizeof(float));
    float* b = (float*)malloc(size1 * sizeof(float));
    float* c = (float*)malloc(size1 * sizeof(float));
    float* d = (float*)malloc(size1 * sizeof(float));
    float* arr1 = (float*)malloc(size2 * sizeof(float));
    float* arr2 = (float*)malloc(size2 * sizeof(float));
    
    /* Initialize with pattern (not all zeros) */
    for (int i = 0; i < size1; ++i) {
        a[i] = (i % 7) * 1.1f;
        b[i] = (i % 5) * 1.2f;
        c[i] = (i % 3) * 1.3f;
        d[i] = (i % 11) * 1.4f;
    }
    
    for (int i = 0; i < size2; ++i) {
        arr1[i] = (i % 13) * 1.5f;
        arr2[i] = (i % 17) * 1.6f;
    }
    
    /* Call computations multiple times with different sizes */
    compute(size1, a, b, c, d);
    compute2(size2 / 2, size2, arr1, arr2);
    compute(size1 - 100, a + 50, b + 50, c + 50, d + 50);
    
    /* Calculate checksum to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < size1; ++i) {
        checksum += a[i] + b[i] + c[i] + d[i];
    }
    for (int i = 0; i < size2; ++i) {
        checksum += arr1[i] + arr2[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(arr1); free(arr2);
    
    return 0;
}
