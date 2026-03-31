/* Test program to trigger DDG edge creation in GCC's scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Helper function with always_inline to ensure loop body is visible */
static inline __attribute__((always_inline)) 
void process_inner(int i, float *restrict a, float *restrict b, 
                   float *restrict c, float *restrict d, 
                   float *restrict sum, float *restrict tmp_var) {
    /* Multiple operations creating various dependency types */
    
    /* 1. Flow/true dependency (register) - reduction pattern */
    *sum += a[i] * 1.5f;
    
    /* 2. Flow dependency (memory) - array with one-element lag */
    if (i > 0) {
        b[i] = a[i] + a[i-1];  /* Uses a[i-1] from previous iteration */
    } else {
        b[i] = a[i];
    }
    
    /* 3. Anti-dependency and output dependency (register) - swap pattern */
    float local_tmp = c[i];
    c[i] = d[i];
    d[i] = local_tmp;
    
    /* 4. Complex addressing for memory dependency analysis */
    int idx = i + (i % 3);  /* Non-affine index */
    if (idx < 1000) {
        *tmp_var = a[idx] * 0.5f;  /* Memory dependency */
    }
    
    /* 5. Condition dependency */
    if (*tmp_var > 100.0f) {
        b[i] *= 2.0f;  /* Depends on tmp_var computation */
    }
}

/* Main computation function with loop nest */
void compute(int n, float *restrict a, float *restrict b, 
             float *restrict c, float *restrict d) {
    float sum = 0.0f;
    float tmp_var = 0.0f;
    
    /* Outer loop with parameter bound */
    for (int i = 0; i < n; ++i) {
        /* Inner loop with dependent bound - increases scheduling complexity */
        for (int j = i; j < n && j < i + 5; ++j) {
            /* Mixed operations to create various DDG edge types */
            
            /* Memory dependency with pointer arithmetic */
            float *ptr = &a[j];
            float val = *ptr + *(ptr + 1);
            
            /* Register dependency chain */
            float r1 = val * 2.0f;
            float r2 = r1 + 3.0f;  /* Flow dependency on r1 */
            float r3 = r2 - r1;    /* Anti-dependency on r1 */
            
            /* Output dependency */
            tmp_var = r3;          /* Overwrites tmp_var */
            
            /* Conditional that creates control dependency */
            if (r3 > 0) {
                b[j] += val;
                sum += r3;
            }
        }
        
        /* Call inline function to add more complexity */
        process_inner(i, a, b, c, d, &sum, &tmp_var);
        
        /* Additional flow dependency across iterations */
        static float persistent = 0.0f;
        persistent += sum * 0.01f;
        if (i % 10 == 0) {
            d[i] = persistent;  /* Output to memory */
        }
    }
    
    /* Use result to prevent dead code elimination */
    a[0] = sum;
}

/* Secondary function with different loop structure */
void compute2(int m, int n, float *restrict x, float *restrict y) {
    /* Nested loops with cross-iteration dependencies */
    for (int i = 1; i < m; ++i) {
        #pragma GCC ivdep  /* Assert no loop-carried dependencies (may trigger DDG verification) */
        for (int j = 1; j < n; ++j) {
            /* Pattern that looks like it has dependencies but pragma says otherwise */
            y[i*n + j] = x[i*n + j] + x[(i-1)*n + j] + x[i*n + (j-1)];
        }
    }
    
    /* Reduction with recurrence */
    float acc = 0.0f;
    for (int i = 0; i < m*n; ++i) {
        acc = acc * 0.9f + x[i];  /* Strong flow dependency */
        x[i] = acc;
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant sizes to prevent full unrolling */
    int size1 = (argc > 1) ? atoi(argv[1]) : 500;
    int size2 = (argc > 2) ? atoi(argv[2]) : 300;
    
    if (size1 > 1000) size1 = 1000;
    if (size2 > 1000) size2 = 1000;
    
    /* Allocate and initialize arrays */
    float *a = (float*)malloc(size1 * sizeof(float));
    float *b = (float*)malloc(size1 * sizeof(float));
    float *c = (float*)malloc(size1 * sizeof(float));
    float *d = (float*)malloc(size1 * sizeof(float));
    float *x = (float*)malloc(size1 * size2 * sizeof(float));
    float *y = (float*)malloc(size1 * size2 * sizeof(float));
    
    /* Initialize with pattern (not just zeros) */
    for (int i = 0; i < size1; ++i) {
        a[i] = (float)i * 0.1f;
        b[i] = (float)(i % 10);
        c[i] = (float)(i * 2);
        d[i] = (float)(i * 3);
    }
    
    for (int i = 0; i < size1 * size2; ++i) {
        x[i] = (float)(i % 100) * 0.01f;
        y[i] = 0.0f;
    }
    
    /* Call compute functions multiple times with different sizes */
    compute(size1, a, b, c, d);
    compute2(size1, size2, x, y);
    
    /* Additional call with different parameters */
    if (size1 > 100) {
        compute(size1/2, a, b, c, d);
    }
    
    /* Compute checksum to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < size1; ++i) {
        checksum += a[i] + b[i] + c[i] + d[i];
    }
    for (int i = 0; i < size1 * size2; ++i) {
        checksum += x[i] + y[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(x); free(y);
    
    return 0;
}
