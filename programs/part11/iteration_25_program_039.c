/* Target: Trigger DDG edge creation in GCC's instruction scheduler */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1000

/* Helper function with always_inline to ensure loop body is visible */
static inline __attribute__((always_inline)) 
void process_inner(int j, int n, float *a, float *b, float *c, 
                   float *d, float *tmp_arr, float *sum) {
    /* Create register dependencies */
    float local_sum = *sum;
    float t1, t2;
    
    /* Memory flow dependency with offset */
    t1 = a[j] + a[j-1];  /* Flow dependency on a[j-1] from previous iteration */
    
    /* Anti-dependency: read before write */
    t2 = b[j];           /* Read b[j] */
    b[j] = t1 * 2.0f;    /* Write b[j] - anti-dependency on t2 */
    
    /* Output dependency */
    c[j] = t2 + local_sum;  /* Write c[j] */
    if (j % 3 == 0) {
        c[j] = c[j] * 1.5f; /* Write c[j] again - output dependency */
    }
    
    /* Register flow dependency */
    local_sum += t1 + t2;
    
    /* Memory anti-dependency with pointer aliasing possibility */
    d[j] = tmp_arr[j] * 0.5f;
    tmp_arr[j] = local_sum;  /* Anti-dependency on d[j] calculation */
    
    *sum = local_sum;
}

/* Main computation function with loop nest */
void compute(int n, float *restrict a, float *restrict b, 
             float *restrict c, float *restrict d) {
    int i, j;
    float sum = 0.0f;
    float tmp_arr[SIZE];
    
    /* Initialize temporary array */
    for (i = 0; i < n; i++) {
        tmp_arr[i] = (float)i;
    }
    
    /* Outer loop with non-constant bound */
    for (i = 1; i < n; i++) {
        /* Complex addressing to create memory dependencies */
        float *ptr_a = &a[i];
        float *ptr_b = &b[i];
        
        /* Inner loop with dependency on outer index */
        #pragma GCC ivdep  /* Assert no loop-carried dependencies (may trigger DDG verification) */
        for (j = i; j < n; j += 1 + (j % 2)) {  /* Non-unit stride */
            /* Mixed data types and dependencies */
            float temp;
            
            /* 1. Register flow dependency (accumulator) */
            sum += ptr_a[j-i] * 1.1f;
            
            /* 2. Memory flow dependency with one-element lag */
            if (j > 0) {
                b[j] = a[j] + a[j-1];  /* True/flow dependency on a[j-1] */
            }
            
            /* 3. Swap operation creating anti and output dependencies */
            temp = c[j];      /* Read c[j] */
            c[j] = d[j];      /* Write c[j] - output dependency if j repeats */
            d[j] = temp;      /* Write d[j] - anti-dependency on temp */
            
            /* 4. Conditional update based on computed value */
            if (sum > 100.0f) {  /* Condition edge */
                ptr_b[j-i] = ptr_b[j-i] * 0.9f;  /* Memory dependency */
                sum = sum * 0.5f;  /* Register dependency */
            }
            
            /* 5. Call to inline function with more dependencies */
            process_inner(j, n, a, b, c, d, tmp_arr, &sum);
            
            /* 6. Non-linear array access */
            int idx = j + (j % 3) - 1;
            if (idx >= 0 && idx < n) {
                a[idx] = b[j] * c[j];  /* Complex memory dependency */
            }
        }
        
        /* Cross-iteration dependency on scalar */
        a[i] = sum * 0.1f;
    }
    
    /* Final reduction to prevent dead code elimination */
    float final_sum = 0.0f;
    for (i = 0; i < n; i++) {
        final_sum += a[i] + b[i] + c[i] + d[i];
    }
    
    /* Use result to prevent optimization */
    if (final_sum > 0) {
        printf("Checksum: %f\n", final_sum);
    }
}

/* Alternate computation with different pattern */
void compute2(int n, float *restrict x, float *restrict y) {
    float acc1 = 0.0f, acc2 = 0.0f, acc3 = 0.0f;
    int i;
    
    /* Multiple accumulation chains with interdependencies */
    for (i = 1; i < n; i++) {
        /* Chain 1: Simple reduction with flow dependency */
        acc1 += x[i] * y[i];
        
        /* Chain 2: Dependent on chain 1 with different latency */
        acc2 = acc1 * 0.7f + y[i-1];  /* Flow on acc1, memory on y[i-1] */
        
        /* Chain 3: Independent computation */
        float temp = x[i] - x[i-1];  /* Memory flow on x[i-1] */
        
        /* Conditional merge of chains */
        if (acc2 > acc1) {  /* Condition edge */
            acc3 = temp * acc2;  /* Register dependencies */
        } else {
            acc3 = temp * acc1;
        }
        
        /* Write results creating anti-dependencies */
        x[i-1] = acc3;  /* Anti-dependency on next iteration's x[i-1] read */
        y[i] = acc1 + acc2;  /* Various dependencies */
    }
    
    /* Use results */
    if (acc1 + acc2 + acc3 > 0) {
        printf("Accumulators: %f, %f, %f\n", acc1, acc2, acc3);
    }
}

int main(int argc, char *argv[]) {
    int i;
    float *a, *b, *c, *d;
    float *x, *y;
    
    /* Use command line argument for variable loop bound */
    int base_size = SIZE;
    if (argc > 1) {
        base_size = atoi(argv[1]);
        if (base_size <= 0 || base_size > 10000) base_size = SIZE;
    }
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(base_size * sizeof(float));
    b = (float*)malloc(base_size * sizeof(float));
    c = (float*)malloc(base_size * sizeof(float));
    d = (float*)malloc(base_size * sizeof(float));
    x = (float*)malloc(base_size * sizeof(float));
    y = (float*)malloc(base_size * sizeof(float));
    
    srand(time(NULL));
    for (i = 0; i < base_size; i++) {
        a[i] = (float)(rand() % 100) / 10.0f;
        b[i] = (float)(rand() % 100) / 10.0f;
        c[i] = (float)(rand() % 100) / 10.0f;
        d[i] = (float)(rand() % 100) / 10.0f;
        x[i] = (float)i;
        y[i] = (float)(base_size - i);
    }
    
    /* Call compute multiple times with different sizes */
    compute(base_size, a, b, c, d);
    compute(base_size / 2, a, b, c, d);
    compute(base_size * 3 / 4, a, b, c, d);
    
    compute2(base_size, x, y);
    compute2(base_size / 2, x, y);
    
    /* Final checksum */
    float total = 0.0f;
    for (i = 0; i < base_size; i++) {
        total += a[i] + b[i] + c[i] + d[i] + x[i] + y[i];
    }
    printf("Total checksum: %f\n", total);
    
    free(a); free(b); free(c); free(d);
    free(x); free(y);
    
    return 0;
}
