/* simt_transformation_test.c
 * Designed to trigger IFN_GOMP_USE_SIMT generation in omp-low.cc lines 2941-2975
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1024
#define M 512

/* Function containing primary target region for SIMT transformation */
void compute_simt(int n, int m, float *a, float *b, float *c) {
    /* Use volatile to prevent constant folding of loop bounds */
    volatile int dynamic_n = n;
    volatile int dynamic_m = m;
    
    /* Primary target region with teams distribute parallel for simd */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: a[0:dynamic_n], b[0:dynamic_n]) \
        map(from: c[0:dynamic_n]) \
        private(i) shared(a,b,c) collapse(2)
    for (int i = 0; i < dynamic_n; i++) {
        for (int j = 0; j < dynamic_m; j++) {
            int idx = i * dynamic_m + j;
            if (idx < dynamic_n) {
                c[idx] = a[idx] + b[idx] * (i + 1) / (j + 1);
            }
        }
    }
}

/* Alternative function with nested explicit regions */
void compute_nested_simt(int n, float *x, float *y, float *z) {
    volatile int size = n;
    
    /* Nested teams and distribute parallel for simd */
    #pragma omp target teams map(tofrom: x[0:size], y[0:size]) map(from: z[0:size])
    {
        #pragma omp distribute parallel for simd \
            private(i) shared(x,y,z) simdlen(8)
        for (int i = 0; i < size; i++) {
            z[i] = x[i] * y[i] + (i % 16);
        }
    }
}

/* Function with target simd and parallel execution context */
void compute_target_simd(int n, float *in, float *out) {
    volatile int count = n;
    
    /* target simd with data mapping - may trigger SIMT in certain contexts */
    #pragma omp target simd map(to: in[0:count]) map(from: out[0:count]) \
        linear(i:1)
    for (int i = 0; i < count; i++) {
        out[i] = in[i] * 2.0f + (float)i / count;
    }
}

/* Helper verification function */
int verify_results(float *cpu, float *gpu, int n, float tolerance) {
    int errors = 0;
    for (int i = 0; i < n; i++) {
        if (fabs(cpu[i] - gpu[i]) > tolerance) {
            errors++;
            if (errors < 5) {
                printf("Mismatch at %d: CPU=%f, GPU=%f\n", i, cpu[i], gpu[i]);
            }
        }
    }
    return errors;
}

/* CPU reference computation */
void cpu_compute(int n, int m, float *a, float *b, float *c) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            if (idx < n) {
                c[idx] = a[idx] + b[idx] * (i + 1) / (j + 1);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use command line or environment for dynamic sizing */
    int test_size = (argc > 1) ? atoi(argv[1]) : N;
    int inner_size = (argc > 2) ? atoi(argv[2]) : M;
    
    if (test_size <= 0) test_size = N;
    if (inner_size <= 0) inner_size = M;
    
    printf("Testing SIMT transformation with size=%d, inner=%d\n", 
           test_size, inner_size);
    
    /* Dynamic allocation to prevent compile-time optimization */
    float *a = (float *)malloc(test_size * sizeof(float));
    float *b = (float *)malloc(test_size * sizeof(float));
    float *c_gpu = (float *)malloc(test_size * sizeof(float));
    float *c_cpu = (float *)malloc(test_size * sizeof(float));
    
    if (!a || !b || !c_gpu || !c_cpu) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < test_size; i++) {
        a[i] = (float)i / test_size;
        b[i] = (float)(i % 100) / 100.0f;
        c_gpu[i] = 0.0f;
        c_cpu[i] = 0.0f;
    }
    
    /* Test 1: Primary SIMT transformation pattern */
    printf("Test 1: target teams distribute parallel for simd\n");
    compute_simt(test_size, inner_size, a, b, c_gpu);
    
    /* CPU reference */
    cpu_compute(test_size, inner_size, a, b, c_cpu);
    
    int errors = verify_results(c_cpu, c_gpu, test_size, 1e-5f);
    printf("Test 1 errors: %d\n", errors);
    
    /* Re-initialize for next test */
    for (int i = 0; i < test_size; i++) {
        c_gpu[i] = 0.0f;
        c_cpu[i] = 0.0f;
    }
    
    /* Test 2: Nested regions */
    printf("\nTest 2: Nested teams + distribute parallel for simd\n");
    
    /* Create different data for variety */
    float *x = (float *)malloc(test_size * sizeof(float));
    float *y = (float *)malloc(test_size * sizeof(float));
    float *z = (float *)malloc(test_size * sizeof(float));
    
    for (int i = 0; i < test_size; i++) {
        x[i] = (float)(i % 50) * 0.1f;
        y[i] = (float)(i % 30) * 0.2f;
    }
    
    compute_nested_simt(test_size, x, y, z);
    
    /* Verify */
    int errors2 = 0;
    for (int i = 0; i < test_size; i++) {
        float expected = x[i] * y[i] + (i % 16);
        if (fabs(z[i] - expected) > 1e-5f) {
            errors2++;
        }
    }
    printf("Test 2 errors: %d\n", errors2);
    
    /* Test 3: Multiple calls from different contexts */
    printf("\nTest 3: Multiple invocations with different sizes\n");
    
    /* Call from conditional to vary context */
    for (int iter = 0; iter < 3; iter++) {
        int current_size = test_size / (iter + 1);
        if (current_size < 10) current_size = 10;
        
        float *temp_in = (float *)malloc(current_size * sizeof(float));
        float *temp_out = (float *)malloc(current_size * sizeof(float));
        
        for (int i = 0; i < current_size; i++) {
            temp_in[i] = (float)(i + iter) / current_size;
        }
        
        if (iter % 2 == 0) {
            compute_target_simd(current_size, temp_in, temp_out);
        } else {
            compute_simt(current_size, inner_size/(iter+1), temp_in, temp_in, temp_out);
        }
        
        free(temp_in);
        free(temp_out);
    }
    
    /* Test 4: Dead code with alternative construct (should still be parsed) */
    if (0) {  /* Never executed but parsed by compiler */
        printf("\nTest 4: Alternative construct (dead code)\n");
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: a[0:test_size]) simdlen(16)
        for (int i = 0; i < test_size; i++) {
            a[i] = a[i] * 2.0f;
        }
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c_gpu);
    free(c_cpu);
    free(x);
    free(y);
    free(z);
    
    printf("\nAll tests completed\n");
    return 0;
}
