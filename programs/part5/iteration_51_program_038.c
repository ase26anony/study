/* Test program to trigger SIMT transformation in GCC's omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function containing the primary SIMT transformation target */
void compute_simt(int N, float *a, float *b, float *result) {
    /* This combined construct is a primary candidate for SIMT transformation */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:N], b[0:N]) map(from: result[0:N]) \
        private(i) shared(result) collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 16; j++) {  /* Inner loop for collapse(2) */
            int idx = i * 16 + j;
            if (idx < N) {
                result[idx] = a[idx] * b[idx] + (float)j;
            }
        }
    }
}

/* Alternative function with nested explicit constructs */
void compute_nested_simt(int N, float *a, float *b, float *result) {
    /* Explicit nesting: teams -> distribute -> parallel for simd */
    #pragma omp target teams map(to: a[0:N], b[0:N]) map(from: result[0:N])
    {
        #pragma omp distribute parallel for simd \
            private(i) shared(result) collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < 8; j++) {
                int idx = i * 8 + j;
                if (idx < N) {
                    result[idx] = a[idx] + b[idx] * (float)(i + j);
                }
            }
        }
    }
}

/* Function with target simd in parallel context */
void compute_target_simd(int N, float *a, float *b, float *result) {
    volatile int use_parallel = 1;  /* Prevent constant folding */
    
    if (use_parallel) {
        /* target simd with parallel execution context */
        #pragma omp target simd map(to: a[0:N], b[0:N]) map(from: result[0:N]) \
            linear(i:1)
        for (int i = 0; i < N; i++) {
            result[i] = a[i] / (b[i] + 1.0f);
        }
    }
}

/* Helper function to verify results */
int verify_results(float *cpu_result, float *gpu_result, int N, float tolerance) {
    int errors = 0;
    for (int i = 0; i < N; i++) {
        float diff = cpu_result[i] - gpu_result[i];
        if (diff < -tolerance || diff > tolerance) {
            errors++;
            if (errors < 5) {
                printf("Mismatch at %d: CPU=%f, GPU=%f\n", 
                       i, cpu_result[i], gpu_result[i]);
            }
        }
    }
    return errors;
}

/* CPU reference computation */
void cpu_compute(int N, float *a, float *b, float *result) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 16; j++) {
            int idx = i * 16 + j;
            if (idx < N) {
                result[idx] = a[idx] * b[idx] + (float)j;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant sizes to prevent loop elimination */
    int base_size = 1024;
    if (argc > 1) base_size = atoi(argv[1]);
    
    /* Dynamic sizes to prevent constant folding */
    int N1 = base_size;
    int N2 = base_size * 2;
    int N3 = base_size / 2;
    
    /* Allocate and initialize arrays */
    float *a1 = (float *)malloc(N1 * sizeof(float));
    float *b1 = (float *)malloc(N1 * sizeof(float));
    float *result1 = (float *)malloc(N1 * sizeof(float));
    float *cpu_result1 = (float *)malloc(N1 * sizeof(float));
    
    float *a2 = (float *)malloc(N2 * sizeof(float));
    float *b2 = (float *)malloc(N2 * sizeof(float));
    float *result2 = (float *)malloc(N2 * sizeof(float));
    
    float *a3 = (float *)malloc(N3 * sizeof(float));
    float *b3 = (float *)malloc(N3 * sizeof(float));
    float *result3 = (float *)malloc(N3 * sizeof(float));
    
    /* Initialize with non-trivial patterns */
    for (int i = 0; i < N1; i++) {
        a1[i] = (float)(i % 100) * 0.1f;
        b1[i] = (float)((i + 1) % 100) * 0.2f;
    }
    
    for (int i = 0; i < N2; i++) {
        a2[i] = (float)(i % 200) * 0.3f;
        b2[i] = (float)((i * 2) % 200) * 0.4f;
    }
    
    for (int i = 0; i < N3; i++) {
        a3[i] = (float)(i % 50) * 0.5f;
        b3[i] = (float)((i + 3) % 50) * 0.6f;
    }
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Primary SIMT construct - called multiple times */
    printf("Test 1: Primary SIMT construct\n");
    cpu_compute(N1, a1, b1, cpu_result1);
    
    compute_simt(N1, a1, b1, result1);
    int errors1 = verify_results(cpu_result1, result1, N1, 0.001f);
    
    /* Call again with different context */
    compute_simt(N1, b1, a1, result1);  /* Swapped arguments */
    
    /* Test 2: Nested explicit construct */
    printf("Test 2: Nested explicit construct\n");
    compute_nested_simt(N2, a2, b2, result2);
    
    /* Test 3: target simd in conditional context */
    printf("Test 3: target simd with conditional\n");
    compute_target_simd(N3, a3, b3, result3);
    
    /* Additional call from different control flow path */
    if (errors1 == 0) {
        printf("No errors in Test 1, running additional verification...\n");
        compute_simt(N1, a1, b1, result1);  /* Third call */
    } else {
        printf("Errors found, trying alternative...\n");
        compute_nested_simt(N1, a1, b1, result1);
    }
    
    /* Cleanup */
    free(a1); free(b1); free(result1); free(cpu_result1);
    free(a2); free(b2); free(result2);
    free(a3); free(b3); free(result3);
    
    printf("Tests completed.\n");
    return 0;
}
