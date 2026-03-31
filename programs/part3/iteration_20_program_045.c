/* Test program to cover SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact */
__attribute__((noinline))
void target_simt_vector_add(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        num_teams(4) thread_limit(128)
    for (int i = 0; i < n; ++i) {
        c[i] = a[i] + b[i];
    }
}

__attribute__((noinline))
void target_simt_conditional_update(float *data, int n, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:n]) map(to: threshold) \
        num_teams(8) thread_limit(256)
    for (int i = 0; i < n; ++i) {
        /* Complex enough control flow for GIMPLE sequence */
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]) * 2.0f;
        } else {
            data[i] = data[i] * data[i] / 3.14f;
        }
    }
}

__attribute__((noinline))
void target_simt_multi_clause(float *x, float *y, float *z, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: x[0:n], y[0:n]) map(from: z[0:n]) \
        reduction(+:z[0:n]) collapse(1) \
        num_teams(2) thread_limit(64)
    for (int i = 0; i < n; ++i) {
        z[i] = x[i] * y[i] + sinf(x[i]) * cosf(y[i]);
    }
}

__attribute__((noinline))
void target_simt_nested_if(float *arr, int n) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: arr[0:n]) \
        num_teams(1) thread_limit(512)
    for (int i = 0; i < n; ++i) {
        /* Multiple nested conditions to create complex GIMPLE */
        if (i % 2 == 0) {
            if (arr[i] > 0) {
                arr[i] = logf(fabsf(arr[i]) + 1.0f);
            } else {
                arr[i] = -arr[i] * 0.5f;
            }
        } else {
            arr[i] = arr[i] * 3.14f;
        }
    }
}

/* Helper function with variable loop bounds */
__attribute__((noinline))
void target_variable_bound(float *in, float *out, int start, int end) {
    int size = end - start;
    #pragma omp target teams distribute parallel for simd \
        map(to: in[start:size]) map(from: out[start:size]) \
        num_teams(4) thread_limit(128)
    for (int i = start; i < end; ++i) {
        out[i] = in[i] * 2.0f + 1.0f;
    }
}

float compute_checksum(float *arr, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; ++i) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    float *a = (float *)malloc(N * sizeof(float));
    float *b = (float *)malloc(N * sizeof(float));
    float *c = (float *)malloc(N * sizeof(float));
    float *d = (float *)malloc(N * sizeof(float));
    
    /* Initialize test data */
    for (int i = 0; i < N; ++i) {
        a[i] = (float)i;
        b[i] = (float)(i * 2);
        c[i] = 0.0f;
        d[i] = (float)(i % 100);
    }
    
    int test_case = 0;
    if (argc > 1) {
        test_case = atoi(argv[1]) % 5;
    }
    
    /* Execute different target regions based on test case */
    switch (test_case) {
        case 0:
            printf("Test case 0: Simple vector addition with simd clause\n");
            target_simt_vector_add(a, b, c, N);
            break;
            
        case 1:
            printf("Test case 1: Conditional update without explicit simd\n");
            target_simt_conditional_update(d, N, THRESHOLD);
            break;
            
        case 2:
            printf("Test case 2: Multi-clause with mathematical functions\n");
            target_simt_multi_clause(a, b, c, N);
            break;
            
        case 3:
            printf("Test case 3: Nested if statements in loop body\n");
            target_simt_nested_if(d, N);
            break;
            
        case 4:
            printf("Test case 4: Variable loop bounds\n");
            target_variable_bound(a, c, 100, 900);
            break;
    }
    
    /* Force multiple executions with different parameters */
    for (int iter = 0; iter < 3; ++iter) {
        int size = 256 + iter * 256;
        if (size > N) size = N;
        
        target_simt_vector_add(a, b, c, size);
        target_simt_conditional_update(d, size, THRESHOLD * (iter + 1));
    }
    
    /* Verify results aren't optimized away */
    float checksum_c = compute_checksum(c, N);
    float checksum_d = compute_checksum(d, N);
    
    printf("Checksum c: %f\n", checksum_c);
    printf("Checksum d: %f\n", checksum_d);
    
    /* Additional test with structured data */
    struct ComplexData {
        float values[512];
        int indices[512];
    };
    
    struct ComplexData *data = (struct ComplexData *)malloc(sizeof(struct ComplexData));
    
    #pragma omp target teams distribute parallel for \
        map(tofrom: data->values[0:512]) map(to: data->indices[0:512]) \
        num_teams(2) thread_limit(256)
    for (int i = 0; i < 512; ++i) {
        data->values[i] = data->indices[i] * 1.5f;
    }
    
    free(data);
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
