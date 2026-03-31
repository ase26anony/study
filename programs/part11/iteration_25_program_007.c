/* Target: ddg.cc lines 749-757 in create_ddg_edge() */
/* Compile with: -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves */
/* Alternative: -O3 -funroll-loops -fno-peel-loops */

#include <stdio.h>
#include <stdlib.h>

/* Helper with always_inline to increase DDG construction scope */
static inline __attribute__((always_inline)) 
float conditional_update(float val, float threshold) {
    return (val > threshold) ? val * 0.5f : val * 2.0f;
}

/* Main computation with various dependency patterns */
void compute(int n, float* restrict a, float* restrict b, 
             float* restrict c, float* restrict d, float* scalar) {
    int i, j;
    float tmp_reg;  /* Register for anti/output dependencies */
    float accum = 0.0f;  /* Reduction accumulator - register flow dep */
    
    /* Loop with multiple dependency types */
    for (i = 1; i < n - 1; ++i) {
        /* 1. FLOW/TRUE DEPENDENCY (register) - reduction pattern */
        accum += a[i] * (*scalar);
        
        /* 2. FLOW DEPENDENCY (memory) - array copy with one-element shift */
        b[i] = a[i] + a[i-1];  /* Uses a[i-1] from previous iteration */
        
        /* 3. ANTI and OUTPUT DEPENDENCIES (register) - swap operation */
        tmp_reg = c[i];
        c[i] = d[i];
        d[i] = tmp_reg;
        
        /* 4. CONDITION DEPENDENCY - creates condition edges */
        float threshold = (float)i / n;
        if (accum > threshold) {
            /* Memory flow with non-affine index */
            int idx = i + (i % 3);  /* Non-linear index calculation */
            if (idx < n) {
                a[idx] = conditional_update(a[idx], threshold);
            }
        }
        
        /* 5. Nested loop with flow dependency */
        for (j = i; j < n && j < i + 3; ++j) {
            /* Memory anti-dependency: read b[j] before potentially writing */
            float read_val = b[j];
            /* Memory flow dependency with pointer arithmetic */
            *(c + j) = read_val * 0.7f + *(d + j) * 0.3f;
        }
        
        /* 6. OUTPUT DEPENDENCY (memory) - write to same location */
        d[i] = d[i] * 1.1f;  /* Overwrites value from swap operation */
    }
    
    /* Store final accumulation result */
    a[0] = accum;
}

/* Secondary function with different loop structure */
void compute2(int m, int n, float* restrict x, float* restrict y) {
    int i, j;
    
    #pragma GCC ivdep
    for (i = 0; i < m; ++i) {
        float local_acc = 0.0f;
        
        /* Triangular loop nest - harder to analyze */
        for (j = 0; j < n - i; ++j) {
            /* Multiple independent arithmetic chains */
            float t1 = x[j] * 1.5f;
            float t2 = y[j] + 2.0f;
            float t3 = t1 + t2;
            
            /* Cross-iteration flow dependency */
            if (j > 0) {
                t3 += x[j-1] * 0.5f;
            }
            
            local_acc += t3;
            y[j] = t3;
            
            /* Conditional store with output dependency */
            if (local_acc > 100.0f) {
                x[j] = local_acc * 0.01f;
                local_acc = 0.0f;  /* Reset creates register anti-dep */
            }
        }
        
        /* Store with memory flow dependency */
        x[i] = local_acc;
    }
}

int main() {
    const int size1 = 1000;
    const int size2 = 500;
    float scalar = 1.5f;
    
    /* Allocate and initialize arrays */
    float* a = (float*)malloc(size1 * sizeof(float));
    float* b = (float*)malloc(size1 * sizeof(float));
    float* c = (float*)malloc(size1 * sizeof(float));
    float* d = (float*)malloc(size1 * sizeof(float));
    float* x = (float*)malloc(size2 * sizeof(float));
    float* y = (float*)malloc(size2 * sizeof(float));
    
    /* Initialize with sequential and random-ish values */
    for (int i = 0; i < size1; ++i) {
        a[i] = (float)i;
        b[i] = (float)(i * 2);
        c[i] = (float)(i % 100);
        d[i] = (float)((i * 3) % 150);
    }
    
    for (int i = 0; i < size2; ++i) {
        x[i] = (float)(i % 50);
        y[i] = (float)((i * 7) % 80);
    }
    
    /* Call compute multiple times with different sizes */
    compute(size1, a, b, c, d, &scalar);
    
    /* Change scalar to create different execution pattern */
    scalar = 2.0f;
    compute(size1 - 100, a + 50, b + 50, c + 50, d + 50, &scalar);
    
    /* Second computation with different structure */
    compute2(100, size2, x, y);
    compute2(50, size2 - 200, x + 100, y + 100);
    
    /* Calculate checksum to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < size1; ++i) {
        checksum += a[i] + b[i] + c[i] + d[i];
    }
    for (int i = 0; i < size2; ++i) {
        checksum += x[i] + y[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(x); free(y);
    
    return 0;
}
