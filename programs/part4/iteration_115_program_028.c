#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define CHECKSUM_SEED 5381

/* Function 1: Nested loops with SIMD clause and conditional execution */
void test_simt_nested(int *A, int *B, int *C, int n, int m, int iter) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: n, m, iter) map(tofrom: A[0:n*m]) map(to: B[0:n*m], C[0:n*m]) \
        num_teams(32) thread_limit(256)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            /* Conditional execution based on thread index and loop indices */
            int thread_id = omp_get_thread_num();
            int team_id = omp_get_team_num();
            
            /* Complex conditional that may trigger SIMT transformation */
            if ((thread_id % 4 == 0) && (i % 3 == 0)) {
                A[idx] = B[idx] * C[idx] + thread_id + team_id;
            } else if ((j % 5 == 0) && (team_id % 2 == 0)) {
                A[idx] = B[idx] / (C[idx] + 1) + i - j;
            } else {
                A[idx] = B[idx] + C[idx] + (i * j) % 256;
            }
            
            /* Additional conditional with early exit pattern */
            if (A[idx] > 1000 && thread_id < 64) {
                A[idx] = A[idx] % 1000;
            }
        }
    }
}

/* Function 2: Mapped data with pointer-based accesses and safelen */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(to: size, stride) map(tofrom: X[0:size]) map(to: Y[0:size], indices[0:size]) \
        safelen(32) num_teams(64) thread_limit(128)
    for (int i = 0; i < size; i += stride) {
        /* Indirect memory access pattern - may influence SIMT transformation */
        int idx = indices[i % size];
        
        /* Conditional execution with floating point operations */
        if (idx % 8 == 0) {
            X[i] = Y[idx] * 2.0f + sinf((float)i * 0.01f);
        } else if (idx % 8 == 1) {
            X[i] = Y[idx] / 1.5f + cosf((float)i * 0.02f);
        } else {
            X[i] = sqrtf(fabsf(Y[idx])) + (float)(i % 16);
        }
        
        /* Nested conditional */
        if (X[i] > 100.0f && omp_get_thread_num() < 32) {
            X[i] = 100.0f;
        } else if (X[i] < -100.0f && omp_get_team_num() % 2 == 0) {
            X[i] = -100.0f;
        }
    }
}

/* Function 3: Target with nested parallel for simd and thread-dependent condition */
void test_simt_conditional(double *D, int *mask, int rows, int cols, int offset) {
    #pragma omp target map(to: rows, cols, offset) map(tofrom: D[0:rows*cols]) map(to: mask[0:rows*cols])
    {
        #pragma omp teams distribute collapse(2) num_teams(16)
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                #pragma omp parallel for simd num_threads(32) safelen(16)
                for (int k = 0; k < 8; k++) {
                    int idx = (i * cols + j) * 8 + k;
                    
                    /* Condition that depends on thread number - may trigger IFN_GOMP_USE_SIMT */
                    if (omp_get_thread_num() % 2 == 0) {
                        D[idx] = (mask[idx] % 2 == 0) ? 
                                 D[idx] * 1.5 + offset : 
                                 D[idx] * 0.5 - offset;
                    } else {
                        D[idx] = (mask[idx] % 3 == 0) ?
                                 D[idx] * 2.0 + k :
                                 D[idx] * 0.8 - k;
                    }
                    
                    /* Additional control flow */
                    switch (k % 4) {
                        case 0:
                            D[idx] += 1.0;
                            break;
                        case 1:
                            D[idx] -= 1.0;
                            break;
                        case 2:
                            D[idx] *= 1.1;
                            break;
                        case 3:
                            D[idx] /= 1.1;
                            break;
                    }
                }
            }
        }
    }
}

/* Function 4: Mixed constructs to explore different lowering paths */
void test_mixed_constructs(int *data, int size, int scale) {
    /* First target region with distribute parallel for simd */
    #pragma omp target teams distribute parallel for simd \
        map(to: size, scale) map(tofrom: data[0:size]) \
        num_teams(8) thread_limit(64)
    for (int i = 0; i < size; i++) {
        int tid = omp_get_thread_num();
        if (tid % 8 < 4) {
            data[i] = (data[i] * scale + tid) % 1024;
        } else {
            data[i] = (data[i] / (scale + 1) - tid) % 1024;
        }
    }
    
    /* Second target region with different structure */
    #pragma omp target teams distribute simd \
        map(to: size) map(tofrom: data[0:size]) \
        num_teams(4) simdlen(16)
    for (int i = 0; i < size; i += 2) {
        if (i % 16 == 0) {
            data[i] = data[i] ^ 0xAAAA;
        } else {
            data[i] = data[i] ^ 0x5555;
        }
    }
}

/* Compute checksum to verify execution */
unsigned long compute_checksum(void *data, size_t size) {
    unsigned long checksum = CHECKSUM_SEED;
    unsigned char *bytes = (unsigned char *)data;
    
    for (size_t i = 0; i < size; i++) {
        checksum = ((checksum << 5) + checksum) + bytes[i];
    }
    
    return checksum;
}

int main(int argc, char *argv[]) {
    /* Parse command line arguments */
    int n = 512;
    int m = 256;
    int iterations = 10;
    
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        if (argc >= 4) {
            iterations = atoi(argv[3]);
        }
    }
    
    printf("Running with n=%d, m=%d, iterations=%d\n", n, m, iterations);
    
    /* Dynamic allocation with pattern-based initialization */
    int total_size = n * m;
    int *A = (int *)malloc(total_size * sizeof(int));
    int *B = (int *)malloc(total_size * sizeof(int));
    int *C = (int *)malloc(total_size * sizeof(int));
    int *mask = (int *)malloc(total_size * 8 * sizeof(int));
    
    float *X = (float *)malloc(total_size * sizeof(float));
    float *Y = (float *)malloc(total_size * sizeof(float));
    
    double *D = (double *)malloc(total_size * 8 * sizeof(double));
    int *indices = (int *)malloc(total_size * sizeof(int));
    
    if (!A || !B || !C || !X || !Y || !D || !indices || !mask) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pattern-based data */
    #pragma omp parallel for simd
    for (int i = 0; i < total_size; i++) {
        A[i] = 0;
        B[i] = i % 97;
        C[i] = (i * 3) % 113;
        X[i] = (float)(i % 255) * 0.1f;
        Y[i] = (float)((i + 17) % 255) * 0.2f;
        indices[i] = (i * 7) % total_size;
    }
    
    #pragma omp parallel for simd
    for (int i = 0; i < total_size * 8; i++) {
        D[i] = (double)(i % 1024) * 0.01;
        mask[i] = i % 31;
    }
    
    /* Execute test functions multiple times with different parameters */
    for (int iter = 0; iter < iterations; iter++) {
        printf("Iteration %d/%d\n", iter + 1, iterations);
        
        /* Call test functions with varying parameters to trigger different paths */
        test_simt_nested(A, B, C, n, m, iter);
        
        int stride = 1 + (iter % 4);
        test_simt_mapped(X, Y, indices, total_size, stride);
        
        test_simt_conditional(D, mask, n, m / 2, iter * 10);
        
        int scale = 2 + (iter % 7);
        test_mixed_constructs(C, total_size, scale);
        
        /* Modify some parameters for next iteration */
        #pragma omp parallel for simd
        for (int i = 0; i < total_size; i++) {
            B[i] = (B[i] + 1) % 256;
            indices[i] = (indices[i] + i) % total_size;
        }
    }
    
    /* Compute and print checksums to verify execution */
    unsigned long checksum_A = compute_checksum(A, total_size * sizeof(int));
    unsigned long checksum_X = compute_checksum(X, total_size * sizeof(float));
    unsigned long checksum_D = compute_checksum(D, total_size * 8 * sizeof(double));
    unsigned long checksum_C = compute_checksum(C, total_size * sizeof(int));
    
    printf("Checksums:\n");
    printf("  Array A: %lu\n", checksum_A);
    printf("  Array X: %lu\n", checksum_X);
    printf("  Array D: %lu\n", checksum_D);
    printf("  Array C: %lu\n", checksum_C);
    
    /* Cleanup */
    free(A); free(B); free(C); free(X); free(Y); free(D); free(indices); free(mask);
    
    return 0;
}
