/* Test program to trigger DDG edge creation in GCC's instruction scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Helper function with always_inline to increase DDG construction scope */
static inline __attribute__((always_inline)) 
float process_element(float a, float b, float *restrict scalar) {
    return a * (*scalar) + b;
}

/* Main computation function with loop-carried dependencies */
void compute(int n, float *restrict a, float *restrict b, 
             float *restrict c, float *restrict d, float *restrict result) {
    int i, j;
    float sum = 0.0f;
    float tmp_reg = 0.0f;
    float scalar = 2.5f;
    
    /* Loop with multiple dependency types */
    for (i = 1; i < n; ++i) {
        /* 1. Flow/true dependency on sum (register edge) */
        sum += a[i] * scalar;
        
        /* 2. Memory flow dependency with one-element lag */
        b[i] = process_element(a[i], a[i-1], &scalar);
        
        /* 3. Anti-dependency and output dependency via swap */
        float tmp = c[i];
        c[i] = d[i];
        d[i] = tmp;
        tmp_reg = tmp;  /* Register anti-dependency on tmp */
        
        /* 4. Condition edge - depends on computed value */
        if (sum > tmp_reg) {
            /* Memory output dependency */
            a[i] = sum * 0.5f;
        }
        
        /* 5. Nested loop with varying bound for complexity */
        for (j = i; j < n && j < i + 3; ++j) {
            /* Memory flow with non-affine index */
            int idx = i + (j % 2);
            if (idx < n) {
                d[idx] = b[j] * 0.8f;
            }
        }
        
        /* 6. Pointer arithmetic creating memory dependencies */
        float *ptr = &a[i];
        *ptr = *ptr + 1.0f;
    }
    
    /* 7. Another loop with different characteristics */
    #pragma GCC ivdep
    for (i = 0; i < n - 1; ++i) {
        /* Assert no loop-carried dependencies (compiler will verify) */
        float diff = b[i+1] - b[i];
        c[i] = diff * scalar;
        
        /* Complex addressing */
        d[i + (i % 4)] = c[i] * 2.0f;
    }
    
    *result = sum;
}

/* Function with loop suitable for modulo scheduling */
void modulo_sched_candidate(int n, float *restrict x, float *restrict y) {
    float acc1 = 0.0f, acc2 = 0.0f, acc3 = 0.0f;
    
    /* Loop with independent chains - good for modulo scheduling */
    for (int i = 0; i < n; ++i) {
        /* Independent computation chains */
        float t1 = x[i] * 1.5f;
        float t2 = y[i] + 2.0f;
        float t3 = t1 * t2;
        float t4 = x[i] / 3.0f;
        float t5 = y[i] - 1.0f;
        float t6 = t4 + t5;
        
        /* Cross-iteration dependencies */
        acc1 = acc1 + t3;
        acc2 = acc2 * 0.9f + t6;
        acc3 = t3 - t6 + acc3 * 0.8f;
        
        /* Memory dependencies */
        x[i] = acc1;
        y[i] = acc2 + acc3;
    }
}

int main(int argc, char *argv[]) {
    const int max_size = 1000;
    float *a = (float*)malloc(max_size * sizeof(float));
    float *b = (float*)malloc(max_size * sizeof(float));
    float *c = (float*)malloc(max_size * sizeof(float));
    float *d = (float*)malloc(max_size * sizeof(float));
    float result1, result2;
    
    /* Initialize with sequential and random-ish values */
    for (int i = 0; i < max_size; ++i) {
        a[i] = (float)i * 0.1f;
        b[i] = (float)(i % 100) * 0.5f;
        c[i] = (float)(i * 2) * 0.3f;
        d[i] = (float)(i + 1) * 0.7f;
    }
    
    /* Call compute with different sizes to trigger various optimizations */
    for (int size = 100; size <= 500; size += 100) {
        compute(size, a, b, c, d, &result1);
        modulo_sched_candidate(size, a, b);
        
        /* Use results to prevent dead code elimination */
        printf("Size %d: result = %f\n", size, result1);
    }
    
    /* Additional test with compile-time unknown bounds */
    int dynamic_size = argc > 1 ? atoi(argv[1]) : 200;
    if (dynamic_size > 0 && dynamic_size < max_size) {
        compute(dynamic_size, a, b, c, d, &result2);
        printf("Dynamic size %d: result = %f\n", dynamic_size, result2);
    }
    
    /* Checksum to ensure all computations are used */
    float checksum = result1 + result2;
    for (int i = 0; i < 100; ++i) {
        checksum += a[i] + b[i] + c[i] + d[i];
    }
    printf("Final checksum: %f\n", checksum);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
