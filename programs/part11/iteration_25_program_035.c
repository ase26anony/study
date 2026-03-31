/* Target: Trigger DDG edge creation in GCC's instruction scheduler */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1000

/* Helper function with always_inline to increase DDG scope */
static inline __attribute__((always_inline)) 
float process_element(float a, float b, float *restrict tmp) {
    float local = a * 2.5f;
    *tmp = local + b;
    return local - b;
}

/* Main computation with loop-carried dependencies */
void compute(int n, float *restrict a, float *restrict b, 
             float *restrict c, float *restrict d) {
    if (n <= 1) return;
    
    float acc = 0.0f;  /* Register accumulator - creates register flow edges */
    float tmp;         /* Temporary for swap - creates anti/output dependencies */
    
    /* #pragma GCC ivdep  /* Could be uncommented to influence dependency analysis */
    
    /* Outer loop with non-constant bound */
    for (int i = 0; i < n; ++i) {
        /* 1. Reduction pattern with flow dependency on acc */
        acc += a[i] * 1.5f;
        
        /* 2. Memory flow dependency with one-element lag */
        if (i > 0) {
            b[i] = a[i] + a[i-1];  /* True/flow dependency on a[i-1] */
        } else {
            b[i] = a[i];
        }
        
        /* 3. Swap operation creating anti and output dependencies */
        tmp = c[i];
        c[i] = d[i];    /* Anti-dependency on original d[i] */
        d[i] = tmp;     /* Output dependency on tmp */
        
        /* 4. Mixed address calculation for memory edges */
        int idx = i + (i % 3);  /* Non-affine index */
        if (idx < n) {
            /* Memory dependency through pointer */
            float val = process_element(a[i], b[i], &tmp);
            c[idx] = val * 0.7f;
        }
        
        /* 5. Conditional update based on computed value */
        if (acc > 100.0f) {  /* Condition edge from acc computation */
            acc *= 0.9f;     /* Register output dependency */
        }
        
        /* 6. Nested loop pattern to increase complexity */
        for (int j = i; j < n && j < i + 5; ++j) {
            /* Cross-iteration dependency through d */
            d[j] += c[i] * 0.3f;
        }
    }
    
    /* Use result to prevent dead code elimination */
    a[0] = acc;
}

/* Secondary computation with different pattern */
void compute2(int m, float *restrict x, float *restrict y) {
    float sum1 = 0.0f, sum2 = 0.0f;
    
    /* Loop with multiple independent chains */
    for (int i = 1; i < m; ++i) {
        /* Independent computation chains */
        float t1 = x[i] * 2.0f;
        float t2 = y[i] + 3.0f;
        float t3 = t1 - t2;
        
        /* Cross-iteration flow dependency */
        sum1 += t3;
        sum2 = sum1 * 1.1f;  /* Anti-dependency on sum1 */
        
        /* Memory output dependency */
        x[i-1] = sum2;
        
        /* Pointer arithmetic for memory edges */
        float *ptr = y + (i % 4);
        *ptr = t1 + t2;
    }
    
    x[0] = sum1 + sum2;
}

int main() {
    /* Initialize with different sizes to trigger various DDG constructions */
    int sizes[] = {100, 500, 1000};
    float checksum = 0.0f;
    
    srand(time(NULL));
    
    for (int s = 0; s < 3; ++s) {
        int n = sizes[s];
        
        /* Allocate and initialize arrays */
        float *a = (float*)malloc(n * sizeof(float));
        float *b = (float*)malloc(n * sizeof(float));
        float *c = (float*)malloc(n * sizeof(float));
        float *d = (float*)malloc(n * sizeof(float));
        float *x = (float*)malloc(n * sizeof(float));
        float *y = (float*)malloc(n * sizeof(float));
        
        for (int i = 0; i < n; ++i) {
            a[i] = (float)(rand() % 100) / 10.0f;
            b[i] = (float)(rand() % 100) / 10.0f;
            c[i] = (float)(rand() % 100) / 10.0f;
            d[i] = (float)(rand() % 100) / 10.0f;
            x[i] = (float)(rand() % 100) / 10.0f;
            y[i] = (float)(rand() % 100) / 10.0f;
        }
        
        /* Call compute functions - non-constant bounds force dependency analysis */
        compute(n, a, b, c, d);
        compute2(n, x, y);
        
        /* Calculate checksum to prevent elimination */
        for (int i = 0; i < n; ++i) {
            checksum += a[i] + b[i] + c[i] + d[i] + x[i] + y[i];
        }
        
        free(a); free(b); free(c); free(d); free(x); free(y);
    }
    
    printf("Checksum: %f\n", checksum);
    return 0;
}
