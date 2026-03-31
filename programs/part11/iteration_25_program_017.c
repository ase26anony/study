/* Target: Trigger DDG edge creation in GCC's instruction scheduler */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1000

/* Helper with always_inline to increase scheduling scope */
static inline __attribute__((always_inline)) 
void process_element(float *restrict a, float *restrict b, 
                     float *restrict c, float *restrict d,
                     int i, float *acc, float *tmp) {
    /* Multiple operations creating various dependencies */
    float t1 = a[i] * 2.5f;      /* Memory read */
    float t2 = b[i] + 1.0f;      /* Memory read */
    
    /* Flow dependency chain */
    *acc = *acc + t1 * t2;       /* Register flow dependency */
    
    /* Anti-dependency on tmp */
    float old_tmp = *tmp;        /* Read before write */
    *tmp = t1 - t2;              /* Write after read */
    
    /* Memory flow with one-element lag */
    if (i > 0) {
        c[i] = c[i-1] + old_tmp; /* Memory flow dependency */
    } else {
        c[i] = old_tmp;
    }
    
    /* Conditional update creating condition edges */
    if (*acc > 100.0f) {
        *acc = *acc * 0.9f;      /* Condition-dependent update */
    }
    
    /* Output dependency on d */
    d[i] = *tmp;                 /* Write */
    d[i] = d[i] + old_tmp;       /* Write after write to same location */
}

/* Main computation function with loop nest */
void compute(int n, float *restrict a, float *restrict b, 
             float *restrict c, float *restrict d) {
    float acc = 0.0f;
    float tmp = 0.0f;
    
    /* Outer loop with non-constant bound */
    for (int i = 0; i < n; ++i) {
        /* Inner loop with dependent bound */
        for (int j = i; j < n && j < i + 3; ++j) {
            /* Complex addressing to create memory dependencies */
            int idx = j + (i % 2);
            if (idx < n) {
                /* Mix of operations */
                float x = a[idx] * b[j];      /* Memory reads */
                float y = c[j] + d[idx];      /* More memory reads */
                
                /* Register dependencies */
                tmp = x - y;                  /* Output dependency on tmp */
                acc += tmp * (i + 1);         /* Flow dependency on acc */
                
                /* Anti-dependency swap pattern */
                float swap_tmp = a[idx];
                a[idx] = b[j];                /* Write after read */
                b[j] = swap_tmp;              /* Write after read */
            }
        }
        
        /* Call inline function */
        process_element(a, b, c, d, i, &acc, &tmp);
        
        /* Additional reduction with loop-carried dependency */
        if (i > 0) {
            d[i] += d[i-1] * 0.5f;           /* Memory flow dependency */
        }
    }
}

/* Another function with different pattern */
void compute2(int m, float *restrict x, float *restrict y) {
    float sum = 0.0f;
    
    #pragma GCC ivdep
    for (int i = 1; i < m; ++i) {
        /* Assert no loop-carried dependencies (but compiler must verify) */
        float diff = x[i] - x[i-1];          /* Potential memory flow */
        y[i] = y[i-1] + diff * 2.0f;         /* Memory flow */
        sum += y[i];                         /* Register flow */
        
        /* Non-affine access pattern */
        int idx = i + (sum > 0 ? 1 : 0);
        if (idx < m) {
            x[idx] = sum * 0.1f;             /* Memory write */
        }
    }
    
    /* Prevent dead code elimination */
    x[0] = sum;
}

int main() {
    /* Initialize arrays */
    float *a = malloc(SIZE * sizeof(float));
    float *b = malloc(SIZE * sizeof(float));
    float *c = malloc(SIZE * sizeof(float));
    float *d = malloc(SIZE * sizeof(float));
    float *x = malloc(SIZE * sizeof(float));
    float *y = malloc(SIZE * sizeof(float));
    
    if (!a || !b || !c || !d || !x || !y) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with sequential values */
    for (int i = 0; i < SIZE; ++i) {
        a[i] = (float)i;
        b[i] = (float)(i * 2);
        c[i] = (float)(i * 3);
        d[i] = (float)(i * 4);
        x[i] = (float)(i % 10);
        y[i] = (float)(i % 5);
    }
    
    /* Call compute with different sizes to trigger various optimizations */
    compute(SIZE, a, b, c, d);
    compute(SIZE / 2, a, b, c, d);
    compute(SIZE * 3 / 4, a, b, c, d);
    
    compute2(SIZE, x, y);
    compute2(SIZE / 3, x, y);
    
    /* Calculate checksum to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < SIZE; ++i) {
        checksum += a[i] + b[i] + c[i] + d[i] + x[i] + y[i];
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
