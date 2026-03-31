/* test_modulo_sched.c - Designed to trigger GCC's modulo scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Loop with multiple carried dependencies and array accesses */
int loop_carried_deps(int n, int *restrict a, int *restrict b, int *restrict c) {
    volatile int sum = 0;  /* volatile to preserve dependencies */
    int i;
    
    /* Core pattern: a[i] depends on a[i-1] (distance-1 dependence) */
    a[0] = b[0] + c[0];
    for (i = 1; i < n; i++) {
        /* Multiple operations to create complex schedule */
        int temp1 = a[i-1] * b[i];      /* distance-1 use */
        int temp2 = temp1 + c[i];
        a[i] = temp2 - b[i-1];          /* another distance-1 use */
        sum += a[i];                    /* accumulator with carried dependency */
        
        /* Additional operations for instruction-level parallelism */
        b[i] = (b[i] << 2) | (c[i] & 0xF);
        c[i] = c[i-1] ^ (i * 7);        /* yet another distance-1 dependence */
        
        /* Memory barrier to prevent over-optimization */
        asm volatile("" : : "r"(sum) : "memory");
    }
    return sum;
}

/* Function 2: Nested loop with inner loop having carried dependency */
int nested_loop_carried(int n, int m, int *restrict x, int *restrict y) {
    volatile int total = 0;
    int i, j;
    
    for (i = 0; i < n; i++) {
        int acc = x[i];  /* Initialize with outer loop value */
        for (j = 1; j < m; j++) {
            /* Inner loop with carried dependency across j */
            acc = acc * 3 + y[(i * m) + j - 1];  /* distance-1 in inner loop */
            y[(i * m) + j] = acc >> 1;
            total ^= acc;  /* Complex accumulator */
        }
        x[i] = acc;
        
        /* Prevent outer loop unrolling from eliminating inner loop */
        asm volatile("" : : "r"(total) : "memory");
    }
    return total;
}

/* Function 3: Multiple interleaved accumulators with different distances */
int multi_accumulator(int n, int *restrict arr1, int *restrict arr2) {
    volatile int sum1 = 0, sum2 = 0, sum3 = 0;
    int i;
    
    /* Initialize first elements */
    arr1[0] = 1;
    arr2[0] = 2;
    
    for (i = 1; i < n; i++) {
        /* Three different carried dependency patterns */
        sum1 = sum1 + arr1[i-1] * 5;        /* Simple accumulator */
        sum2 = sum2 ^ (arr2[i-1] + i);      /* XOR accumulator with distance-1 */
        sum3 = sum3 - (sum1 & sum2);        /* Dependent on both accumulators */
        
        /* Array updates with carried dependencies */
        arr1[i] = (arr1[i-1] + arr2[i-1]) * 3;  /* Uses both previous values */
        arr2[i] = arr1[i] ^ arr2[i-1];           /* Another distance-1 use */
        
        /* Complex expression to create many dependence edges */
        int temp = (sum1 << 3) | (sum2 & 0xFF);
        arr1[i] += temp;
        arr2[i] -= sum3;
        
        /* Force dependence preservation */
        asm volatile("" : : "r"(sum1), "r"(sum2), "r"(sum3) : "memory");
    }
    return sum1 + sum2 * 2 + sum3 * 3;
}

/* Function 4: Loop with variable trip count and complex index calculations */
int variable_loop(int n, int *restrict data, int seed) {
    volatile int hash = seed;
    int i;
    
    /* Initialize first two elements */
    data[0] = seed;
    if (n > 1) data[1] = seed * 2;
    
    for (i = 2; i < n; i++) {
        /* Multiple distance-1 dependencies */
        int prev1 = data[i-1];      /* distance = 1 */
        int prev2 = data[i-2];      /* distance = 2 (for contrast) */
        
        /* Complex calculation with carried dependencies */
        hash = hash ^ prev1;
        data[i] = (prev1 * 7 + prev2 * 3) ^ hash;
        
        /* Additional operations to increase ILP opportunities */
        int rot = (hash << 4) | (hash >> 28);
        data[i] += rot;
        hash = (hash * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Memory clobber to ensure dependencies are visible to scheduler */
        asm volatile("" : : "r"(hash) : "memory");
    }
    return hash;
}

/* Function 5: Software pipelining candidate with high trip count */
void pipeline_candidate(int n, float *restrict out, 
                       const float *restrict in1, 
                       const float *restrict in2) {
    volatile float acc = 0.0f;
    int i;
    
    /* FIR-like filter with carried dependency */
    out[0] = in1[0] * in2[0];
    for (i = 1; i < n; i++) {
        /* This pattern is classic for modulo scheduling:
           out[i] depends on out[i-1] */
        float prev = out[i-1];                     /* distance-1 load */
        float prod = in1[i] * in2[i];
        out[i] = prev * 0.7f + prod * 0.3f;       /* distance-1 store */
        acc += out[i];                             /* accumulator */
        
        /* Additional floating point ops for schedule complexity */
        float tmp = in1[i-1] + in2[i-1];           /* another distance-1 */
        out[i] = out[i] * (1.0f - tmp * 0.01f);
        
        /* Prevent optimization */
        asm volatile("" : : "r"(acc) : "memory");
    }
    
    /* Use result to prevent dead code elimination */
    out[n-1] += acc;
}

/* Main driver that calls all test functions */
int main(int argc, char *argv[]) {
    const int N = 1000;
    const int M = 100;
    int i;
    
    /* Allocate and initialize test arrays */
    int *a = (int *)malloc(N * sizeof(int));
    int *b = (int *)malloc(N * sizeof(int));
    int *c = (int *)malloc(N * sizeof(int));
    int *x = (int *)malloc(N * M * sizeof(int));
    int *y = (int *)malloc(N * M * sizeof(int));
    int *arr1 = (int *)malloc(N * sizeof(int));
    int *arr2 = (int *)malloc(N * sizeof(int));
    int *data = (int *)malloc(N * sizeof(int));
    float *out = (float *)malloc(N * sizeof(float));
    float *in1 = (float *)malloc(N * sizeof(float));
    float *in2 = (float *)malloc(N * sizeof(float));
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (i = 0; i < N; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        data[i] = rand() % 100;
        out[i] = 0.0f;
        in1[i] = (float)(rand() % 100) / 10.0f;
        in2[i] = (float)(rand() % 100) / 10.0f;
    }
    for (i = 0; i < N * M; i++) {
        x[i] = rand() % 100;
        y[i] = rand() % 100;
    }
    
    /* Call all test functions to ensure they're compiled */
    int result1 = loop_carried_deps(N, a, b, c);
    int result2 = nested_loop_carried(10, M, x, y);
    int result3 = multi_accumulator(N, arr1, arr2);
    int result4 = variable_loop(N, data, 42);
    pipeline_candidate(N, out, in1, in2);
    
    /* Compute final checksum to use all results */
    int final_result = result1 ^ result2 ^ result3 ^ result4;
    final_result += (int)out[N-1];
    
    printf("Result: %d\n", final_result);
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(x); free(y);
    free(arr1); free(arr2);
    free(data);
    free(out); free(in1); free(in2);
    
    return 0;
}
