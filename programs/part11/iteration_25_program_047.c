/* Test program to cover DDG edge creation in GCC's modulo scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Helper function with always_inline to ensure loop body is visible */
static inline __attribute__((always_inline)) 
void process_chunk(float *restrict a, float *restrict b, 
                   float *restrict c, float *restrict d,
                   int start, int end, float *sum) {
    float local_sum = *sum;
    float tmp1, tmp2;
    int i;
    
    /* Loop with multiple dependency types */
    for (i = start; i < end; ++i) {
        /* 1. Flow/true dependency (register) - reduction pattern */
        local_sum += a[i] * 1.5f;
        
        /* 2. Flow dependency (memory) - array with one-element shift */
        if (i > 0) {
            b[i] = a[i] + a[i-1] * 0.7f;
        } else {
            b[i] = a[i];
        }
        
        /* 3. Anti and output dependencies - swap operation */
        tmp1 = c[i];
        c[i] = d[i];
        d[i] = tmp1;
        
        /* 4. Condition dependency */
        if (local_sum > 100.0f) {
            /* Output dependency on c[i] */
            c[i] *= 0.9f;
            local_sum *= 0.95f;  /* Flow dependency on local_sum */
        }
        
        /* 5. Complex addressing for memory dependency analysis */
        int idx = i + (i % 3) - 1;
        if (idx >= 0 && idx < end) {
            /* Potential flow dependency through memory */
            d[i] += b[idx] * 0.3f;
        }
    }
    
    *sum = local_sum;
}

/* Main computation function with nested loops */
void compute(int n, float *restrict a, float *restrict b, 
             float *restrict c, float *restrict d) {
    float total_sum = 0.0f;
    int i, j;
    
    /* Outer loop with parameterized bound */
    for (i = 0; i < n; ++i) {
        /* Initialize arrays with some values */
        a[i] = (float)i * 0.1f;
        c[i] = (float)(n - i) * 0.2f;
        d[i] = (float)(i * i) * 0.01f;
    }
    
    /* Try to hint no loop-carried dependencies for outer loop */
    #pragma GCC ivdep
    for (i = 0; i < n - 1; ++i) {
        /* Inner loop with dependent bound - increases scheduling complexity */
        for (j = i; j < n && j < i + 10; ++j) {
            /* Mixed dependencies in inner loop */
            float tmp;
            
            /* Flow dependency through array */
            a[j] = b[j] + c[j] * d[j];
            
            /* Anti-dependency: read then write */
            tmp = b[j];
            b[j] = a[j] * tmp;
            
            /* Output dependency */
            c[j] = tmp * 2.0f;
            
            /* Register flow dependency */
            total_sum += a[j];
            
            /* Condition with register dependency */
            if (total_sum > 50.0f) {
                d[j] = total_sum * 0.1f;
                total_sum *= 0.8f;
            }
        }
        
        /* Process chunks to create more DDG contexts */
        if (i % 4 == 0) {
            process_chunk(a, b, c, d, i, i + 8, &total_sum);
        }
    }
    
    /* Final reduction loop with carried dependency */
    for (i = 1; i < n; ++i) {
        /* Strong loop-carried flow dependency */
        a[i] = a[i] + a[i-1] * 0.5f;
        total_sum += a[i];
    }
    
    /* Use result to prevent dead code elimination */
    b[0] = total_sum;
}

/* Another function with different loop pattern */
void compute2(int m, int n, float *restrict arr1, float *restrict arr2) {
    int i, j;
    float acc = 0.0f;
    
    /* Double nested loop with non-linear index */
    for (i = 0; i < m; ++i) {
        #pragma GCC ivdep
        for (j = 0; j < n; ++j) {
            /* Complex addressing */
            int idx = (i * 3 + j * 2) % n;
            
            /* Multiple interleaved dependencies */
            float old = arr1[idx];
            arr1[idx] = arr2[j] * (float)(i + 1);
            arr2[j] = old + (float)idx;
            
            /* Register dependency chain */
            acc = acc * 0.9f + arr1[idx];
            
            /* Conditional with dependency */
            if (acc > arr2[j]) {
                arr1[idx] -= acc * 0.1f;
            }
        }
        
        /* Loop-carried dependency through acc */
        arr2[i % n] += acc;
    }
}

int main(int argc, char *argv[]) {
    const int size1 = 1000;
    const int size2 = 500;
    float *a, *b, *c, *d;
    float *arr1, *arr2;
    int i;
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(size1 * sizeof(float));
    b = (float*)malloc(size1 * sizeof(float));
    c = (float*)malloc(size1 * sizeof(float));
    d = (float*)malloc(size1 * sizeof(float));
    
    arr1 = (float*)malloc(size2 * sizeof(float));
    arr2 = (float*)malloc(size2 * sizeof(float));
    
    /* Initialize with some values */
    for (i = 0; i < size1; ++i) {
        a[i] = (float)i * 0.01f;
        b[i] = (float)(size1 - i) * 0.02f;
        c[i] = (float)(i % 100) * 0.5f;
        d[i] = (float)(i * i) * 0.001f;
    }
    
    for (i = 0; i < size2; ++i) {
        arr1[i] = (float)i * 0.03f;
        arr2[i] = (float)(size2 - i) * 0.04f;
    }
    
    /* Call compute functions multiple times with different sizes
       to increase chance of DDG construction */
    compute(size1, a, b, c, d);
    compute(size1 / 2, a, b, c, d);
    compute(size1 / 4, a, b, c, d);
    
    compute2(100, size2, arr1, arr2);
    compute2(50, size2 / 2, arr1, arr2);
    
    /* Calculate checksum to prevent dead code elimination */
    float checksum = 0.0f;
    for (i = 0; i < size1; ++i) {
        checksum += a[i] + b[i] + c[i] + d[i];
    }
    for (i = 0; i < size2; ++i) {
        checksum += arr1[i] + arr2[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(arr1); free(arr2);
    
    return 0;
}
