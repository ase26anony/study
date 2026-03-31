/* Target: ddg.cc lines 749-757 - create_ddg_edge() field assignments */
#include <stdio.h>
#include <stdlib.h>

/* Helper with always_inline to increase DDG construction scope */
static inline __attribute__((always_inline)) 
float conditional_update(float val, float threshold) {
    return (val > threshold) ? val * 0.5f : val * 2.0f;
}

/* Main computation with loop-carried dependencies */
void compute(int n, float* restrict a, float* restrict b, 
             float* restrict c, float* restrict d, float* scalar) {
    int i, j;
    float acc = 0.0f;  /* Register flow dependency */
    float tmp1, tmp2;  /* Register anti/output dependencies */
    
    /* Loop with cross-iteration dependencies */
    for (i = 1; i < n; ++i) {
        /* 1. Flow dependency on acc (register edge) */
        acc += a[i] * (*scalar);
        
        /* 2. Memory flow dependency with one-element lag */
        b[i] = a[i] + a[i-1];  /* Uses a[i-1] from previous iteration */
        
        /* 3. Anti and output dependencies via swap pattern */
        tmp1 = c[i];
        c[i] = d[i];
        d[i] = tmp1;
        
        /* 4. Mixed index for memory edge complexity */
        int idx = i + (i % 3);
        if (idx < n) {
            /* Memory dependency with non-affine index */
            a[idx] = b[i] * 0.3f;
        }
        
        /* 5. Condition edge - depends on computed value */
        float threshold = acc * 0.01f;
        if (b[i] > threshold) {
            /* Conditional update creates control dependency */
            b[i] = conditional_update(b[i], threshold);
        }
        
        /* 6. Nested loop with dependent bounds for scheduling complexity */
        for (j = i; j < n && j < i + 4; ++j) {
            /* Additional memory dependencies */
            d[j] += a[i] * 0.1f;
        }
        
        /* 7. Pointer arithmetic for memory edge variation */
        float* ptr = &a[i];
        *(ptr + (i % 2)) = acc * 0.5f;
    }
    
    /* Use result to prevent dead code elimination */
    a[0] = acc;
}

/* Secondary function with different pattern */
void compute2(int m, int n, float* restrict x, float* restrict y) {
    int i, j;
    float local_acc = x[0];
    
    /* Double nested loop for more complex DDG */
    for (i = 0; i < m; ++i) {
        /* Output dependency on local_acc */
        local_acc = 0.0f;
        
        for (j = 0; j < n; ++j) {
            /* Flow dependency within inner loop */
            local_acc += x[i * n + j] * y[j];
            
            /* Anti-dependency: read then write */
            float old_val = y[j];
            y[j] = x[i * n + j] + old_val;
            x[i * n + j] = old_val * 0.7f;
            
            /* Memory dependency with stride */
            if (j > 0) {
                y[j] += y[j-1] * 0.3f;  /* Loop-carried flow dependency */
            }
        }
        
        /* Cross-iteration dependency */
        x[i * n] = local_acc;
    }
}

int main(int argc, char** argv) {
    const int size1 = 1000;
    const int size2 = 100;
    const int size3 = 50;
    
    /* Allocate and initialize arrays */
    float* a = (float*)malloc(size1 * sizeof(float));
    float* b = (float*)malloc(size1 * sizeof(float));
    float* c = (float*)malloc(size1 * sizeof(float));
    float* d = (float*)malloc(size1 * sizeof(float));
    float* x = (float*)malloc(size2 * size3 * sizeof(float));
    float* y = (float*)malloc(size3 * sizeof(float));
    
    float scalar = 1.5f;
    
    /* Initialize with pattern */
    for (int i = 0; i < size1; ++i) {
        a[i] = (float)i * 0.1f;
        b[i] = (float)(size1 - i) * 0.2f;
        c[i] = (float)i * 0.3f;
        d[i] = (float)(i * i) * 0.01f;
    }
    
    for (int i = 0; i < size2 * size3; ++i) {
        x[i] = (float)(i % 100) * 0.25f;
    }
    
    for (int i = 0; i < size3; ++i) {
        y[i] = (float)i * 0.33f;
    }
    
    /* Call compute multiple times with different sizes */
    compute(size1, a, b, c, d, &scalar);
    
    /* Try with smaller size for different compilation context */
    compute(500, a, b, c, d, &scalar);
    
    /* Call second compute function */
    compute2(size2, size3, x, y);
    
    /* Compute checksum to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < size1; ++i) {
        checksum += a[i] + b[i] + c[i] + d[i];
    }
    
    for (int i = 0; i < size2 * size3; ++i) {
        checksum += x[i];
    }
    
    for (int i = 0; i < size3; ++i) {
        checksum += y[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(x);
    free(y);
    
    return 0;
}
