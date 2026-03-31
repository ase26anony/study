/* Test program to trigger SIMT transformation in OpenMP target offloading */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact */
__attribute__((noinline))
void target_simple_vector_add(float *a, float *b, float *c, int n) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
        num_teams(4) thread_limit(128)
    for (int i = 0; i < n; ++i) {
        c[i] = a[i] + b[i];
    }
}

__attribute__((noinline))
void target_conditional_update(float *data, int n, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:n]) map(to: threshold) \
        num_teams(8) thread_limit(256)
    for (int i = 0; i < n; ++i) {
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]);
        } else {
            data[i] = data[i] * data[i];
        }
    }
}

__attribute__((noinline))
void target_complex_loop(float *x, float *y, float *z, int n, float alpha) {
    #pragma omp target teams distribute parallel for \
        map(to: x[0:n], alpha) map(tofrom: y[0:n], z[0:n]) \
        num_teams(16) thread_limit(64)
    for (int i = 0; i < n; ++i) {
        float temp = x[i] * alpha;
        y[i] = temp + sinf((float)i * 0.01f);
        z[i] = y[i] * cosf(temp);
    }
}

__attribute__((noinline))
void target_nested_if(float *arr, int n, int *mask) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: arr[0:n]) map(to: mask[0:n]) \
        num_teams(4) thread_limit(128)
    for (int i = 0; i < n; ++i) {
        if (mask[i] == 1) {
            arr[i] = arr[i] * 2.0f;
        } else if (mask[i] == 2) {
            arr[i] = arr[i] / 2.0f;
        } else {
            arr[i] = arr[i] + 1.0f;
        }
    }
}

/* Helper function to initialize arrays */
void init_arrays(float *a, float *b, float *c, int *mask, int n) {
    for (int i = 0; i < n; ++i) {
        a[i] = (float)i * 1.5f;
        b[i] = (float)(n - i) * 0.7f;
        c[i] = 0.0f;
        mask[i] = i % 3;  /* Values 0, 1, or 2 */
    }
}

/* Verification function */
float verify_sum(float *arr, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; ++i) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Allocate and initialize test data */
    float *a = (float *)malloc(N * sizeof(float));
    float *b = (float *)malloc(N * sizeof(float));
    float *c = (float *)malloc(N * sizeof(float));
    float *d = (float *)malloc(N * sizeof(float));
    float *x = (float *)malloc(N * sizeof(float));
    float *y = (float *)malloc(N * sizeof(float));
    float *z = (float *)malloc(N * sizeof(float));
    int *mask = (int *)malloc(N * sizeof(int));
    
    assert(a && b && c && d && x && y && z && mask);
    
    init_arrays(a, b, c, mask, N);
    
    /* Copy initial values for d, x, y, z */
    for (int i = 0; i < N; ++i) {
        d[i] = a[i];
        x[i] = (float)i * 0.5f;
        y[i] = (float)i * 1.2f;
        z[i] = 0.0f;
    }
    
    /* Use command-line arguments to select different test cases */
    int test_case = 0;
    if (argc > 1) {
        test_case = atoi(argv[1]) % 4;
    }
    
    /* Execute different target regions based on test case */
    printf("Running test case %d\n", test_case);
    
    switch (test_case) {
        case 0:
            /* Simple vector addition with simd clause */
            target_simple_vector_add(a, b, c, N);
            printf("Vector add checksum: %f\n", verify_sum(c, N));
            break;
            
        case 1:
            /* Conditional update without explicit simd clause */
            target_conditional_update(d, N, THRESHOLD);
            printf("Conditional update checksum: %f\n", verify_sum(d, N));
            break;
            
        case 2:
            /* Complex loop with mathematical functions */
            target_complex_loop(x, y, z, N, 2.0f);
            printf("Complex loop y checksum: %f\n", verify_sum(y, N));
            printf("Complex loop z checksum: %f\n", verify_sum(z, N));
            break;
            
        case 3:
            /* Nested if statements in loop body */
            target_nested_if(a, N, mask);
            printf("Nested if checksum: %f\n", verify_sum(a, N));
            break;
    }
    
    /* Additional coverage: loop over different sizes */
    for (int size = 256; size <= 512; size += 256) {
        float *temp_a = (float *)malloc(size * sizeof(float));
        float *temp_b = (float *)malloc(size * sizeof(float));
        float *temp_c = (float *)malloc(size * sizeof(float));
        
        if (temp_a && temp_b && temp_c) {
            for (int i = 0; i < size; ++i) {
                temp_a[i] = (float)i;
                temp_b[i] = (float)(size - i);
                temp_c[i] = 0.0f;
            }
            
            #pragma omp target teams distribute parallel for \
                map(to: temp_a[0:size], temp_b[0:size]) map(from: temp_c[0:size])
            for (int i = 0; i < size; ++i) {
                temp_c[i] = temp_a[i] * temp_b[i];
            }
            
            float sum = 0.0f;
            for (int i = 0; i < size; ++i) {
                sum += temp_c[i];
            }
            printf("Size %d product checksum: %f\n", size, sum);
        }
        
        free(temp_a);
        free(temp_b);
        free(temp_c);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(x);
    free(y);
    free(z);
    free(mask);
    
    return 0;
}
