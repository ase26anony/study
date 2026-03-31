/* Target: Trigger DDG edge creation in GCC's instruction scheduler */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1000

/* Helper function with always_inline to increase DDG scope */
static inline __attribute__((always_inline)) 
float process_element(float a, float b, float* restrict tmp) {
    float local = a * 0.5f;
    *tmp = local + b;
    return local - b;
}

/* Main computation with mixed dependencies */
void compute(int n, float* restrict a, float* restrict b, 
             float* restrict c, float* restrict d) {
    int i, j;
    float acc = 0.0f;          /* Register accumulator - flow dependency */
    float tmp_reg;             /* Temporary register - anti/output dependencies */
    
    if (n <= 0) return;
    
    /* Initialize with first element dependency */
    b[0] = a[0];
    d[0] = c[0];
    
    #pragma GCC ivdep
    for (i = 1; i < n; ++i) {
        /* 1. Flow dependency on acc (register edge) */
        acc += a[i] * 0.3f;
        
        /* 2. Memory flow dependency with one-element lag */
        b[i] = a[i] + a[i-1] * 0.7f;
        
        /* 3. Anti and output dependencies via swap pattern */
        tmp_reg = c[i];
        c[i] = d[i];
        d[i] = tmp_reg;
        
        /* 4. Mixed address calculation for memory edges */
        int idx = i + (i % 3) - 1;
        if (idx >= 0 && idx < n) {
            /* Memory dependency with non-affine index */
            a[idx] = b[i] * 0.9f;
        }
        
        /* 5. Condition edge - depends on computed value */
        float threshold = acc * 0.01f;
        if (b[i] > threshold) {
            d[i] = process_element(a[i], b[i], &tmp_reg);
        }
        
        /* 6. Nested loop with varying bounds for complexity */
        for (j = i; j < n && j < i + 5; ++j) {
            /* Cross-iteration memory dependency */
            c[j] += a[i] * 0.2f;
        }
    }
    
    /* Final reduction with dependency */
    a[0] = acc;
}

/* Secondary function with different pattern */
void compute2(int m, int n, float* restrict x, float* restrict y) {
    int i, j;
    float sum = 0.0f;
    
    for (i = 0; i < m; ++i) {
        float local_sum = 0.0f;
        
        /* Triangular loop nest */
        for (j = i; j < n; ++j) {
            /* Multiple dependencies */
            float t1 = x[j] * y[i];
            float t2 = x[i] * y[j];
            local_sum += t1 - t2;
            
            /* Output dependency */
            x[j] = t1;
            y[j] = t2;
        }
        
        /* Flow dependency across outer loop */
        sum += local_sum * (i + 1);
        
        /* Conditional with register dependency */
        if (sum > 100.0f) {
            sum *= 0.99f;
        }
    }
    
    /* Prevent dead code elimination */
    x[0] = sum;
}

int main() {
    /* Allocate and initialize arrays */
    float* a = (float*)malloc(SIZE * sizeof(float));
    float* b = (float*)malloc(SIZE * sizeof(float));
    float* c = (float*)malloc(SIZE * sizeof(float));
    float* d = (float*)malloc(SIZE * sizeof(float));
    float* x = (float*)malloc(SIZE * sizeof(float));
    float* y = (float*)malloc(SIZE * sizeof(float));
    
    srand(time(NULL));
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; ++i) {
        a[i] = (float)(rand() % 100) * 0.1f;
        b[i] = (float)(rand() % 100) * 0.1f;
        c[i] = (float)(rand() % 100) * 0.1f;
        d[i] = (float)(rand() % 100) * 0.1f;
        x[i] = (float)(rand() % 100) * 0.1f;
        y[i] = (float)(rand() % 100) * 0.1f;
    }
    
    /* Call compute with different sizes to trigger various DDG constructions */
    compute(SIZE, a, b, c, d);
    compute(SIZE/2, a, b, c, d);
    compute(SIZE*2/3, a, b, c, d);
    
    compute2(SIZE/4, SIZE/2, x, y);
    compute2(SIZE/3, SIZE/3, x, y);
    
    /* Calculate checksum to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < SIZE; ++i) {
        checksum += a[i] + b[i] + c[i] + d[i] + x[i] + y[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d); free(x); free(y);
    
    return 0;
}
