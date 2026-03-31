/* Target: ddg.cc lines 749-757 - DDG edge creation */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1000

/* Helper with always_inline to ensure loop body is visible */
static inline __attribute__((always_inline)) 
void process_inner(int j, int n, float *a, float *b, float *c, float *d, 
                   float *acc, float *tmp_var) {
    /* Complex addressing to create memory dependencies */
    int idx1 = j + (j % 3);
    int idx2 = j + (j % 5);
    
    /* Flow dependency on *acc (register edge) */
    *acc = *acc + a[idx1 % n] * 1.5f;
    
    /* Anti-dependency: read then write to same location */
    float old = b[idx2 % n];
    b[idx2 % n] = a[idx1 % n] + old;
    
    /* Output dependency: multiple writes to *tmp_var */
    *tmp_var = c[j % n];
    *tmp_var = *tmp_var * 2.0f;
    
    /* Condition edge */
    if (*tmp_var > 100.0f) {
        d[j % n] = *tmp_var;
    }
}

/* Main computation function with loop nest */
void compute(int n, float *a, float *b, float *c, float *d) {
    float acc = 0.0f;
    float tmp = 0.0f;
    
    /* Outer loop with parameter bound */
    for (int i = 0; i < n; ++i) {
        /* Reduction with flow dependency on acc */
        acc += a[i] * 0.7f;
        
        /* Memory flow dependency with one-element lag */
        if (i > 0) {
            b[i] = a[i] + a[i-1];  /* True/flow dependency on a */
        } else {
            b[i] = a[i];
        }
        
        /* Swap creating anti and output dependencies */
        float tmp_swap = c[i];
        c[i] = d[i];
        d[i] = tmp_swap;
        
        /* Nested loop with dependent bounds */
        #pragma GCC ivdep  /* Assert no loop-carried dependencies */
        for (int j = i; j < n && j < i + 10; ++j) {
            process_inner(j, n, a, b, c, d, &acc, &tmp);
        }
        
        /* Conditional creating control dependency */
        if (acc > 1000.0f) {
            acc *= 0.9f;  /* Register output dependency */
        }
    }
    
    /* Prevent dead code elimination */
    a[0] = acc;
    b[0] = tmp;
}

/* Alternate computation with different pattern */
void compute2(int n, float *restrict x, float *restrict y, 
              float *restrict z, float *restrict w) {
    float sum1 = 0.0f, sum2 = 0.0f;
    
    /* Loop with pointer arithmetic */
    float *px = x;
    float *py = y;
    float *pz = z;
    
    for (int i = 0; i < n; ++i) {
        /* Independent chains that later merge */
        float t1 = *px * 1.1f;
        float t2 = *py * 1.2f;
        float t3 = *pz * 1.3f;
        
        /* Flow dependencies on temporaries */
        t1 = t1 + t2;
        t2 = t2 + t3;
        
        /* Merge creating register dependencies */
        sum1 += t1;
        sum2 += t2;
        
        /* Memory anti-dependency */
        float old_w = w[i];
        w[i] = t1 + t2;
        
        /* Use old_w to prevent elimination */
        sum1 += old_w * 0.1f;
        
        /* Pointer increment */
        px++;
        py++;
        pz++;
        
        /* Complex condition with data dependency */
        if ((i % 7) == 0) {
            sum2 = sum1 * 0.5f;  /* Output dependency on sum2 */
        }
    }
    
    /* Store results */
    x[0] = sum1;
    y[0] = sum2;
}

int main() {
    /* Initialize with different patterns */
    float a[SIZE], b[SIZE], c[SIZE], d[SIZE];
    float x[SIZE], y[SIZE], z[SIZE], w[SIZE];
    
    srand(time(NULL));
    
    for (int i = 0; i < SIZE; ++i) {
        a[i] = (float)(rand() % 100) / 10.0f;
        b[i] = (float)(rand() % 100) / 10.0f;
        c[i] = (float)(rand() % 100) / 10.0f;
        d[i] = (float)(rand() % 100) / 10.0f;
        
        x[i] = (float)i;
        y[i] = (float)(i * 2);
        z[i] = (float)(i * 3);
        w[i] = (float)(i * 4);
    }
    
    /* Call with different sizes to trigger various optimizations */
    compute(SIZE, a, b, c, d);
    compute(SIZE / 2, a, b, c, d);
    compute(SIZE * 3 / 4, a, b, c, d);
    
    compute2(SIZE, x, y, z, w);
    compute2(SIZE / 3, x, y, z, w);
    compute2(SIZE * 2 / 3, x, y, z, w);
    
    /* Calculate checksum to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < SIZE; ++i) {
        checksum += a[i] + b[i] + c[i] + d[i];
        checksum += x[i] + y[i] + z[i] + w[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    return 0;
}
