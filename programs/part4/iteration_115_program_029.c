#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define CHECKSUM_MOD 10007

/* Test function 1: Nested loops with SIMD clause and conditional execution */
void test_simt_nested(int *A, int *B, int *C, int n, int m, int iter) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(tofrom: A[0:n*m]) map(to: B[0:n*m], C[0:n*m]) \
        num_teams(32) thread_limit(128)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            /* Conditional execution based on thread index and loop indices */
            if ((omp_get_thread_num() % 4) == 0) {
                /* Complex pattern to prevent optimization */
                A[idx] = B[idx] + C[idx] * (i % 16) + (j % 8);
            } else if ((i + j) % 3 == 0) {
                /* Another conditional path */
                A[idx] = B[idx] - C[idx] / ((j % 7) + 1);
            } else {
                /* Default path with SIMD-friendly operation */
                A[idx] = B[idx] * C[idx] + (i ^ j);
            }
            
            /* Additional conditional to create more control flow */
            if (A[idx] > 1000 && (j % 5) == 2) {
                A[idx] = A[idx] % 997;
            }
        }
    }
}

/* Test function 2: Mapped data with pointer-based accesses and safelen */
void test_simt_mapped(float *X, float *Y, int *perm, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: X[0:size]) map(to: Y[0:size], perm[0:size]) \
        safelen(32) num_teams(64) thread_limit(256)
    for (int i = 0; i < size; i += stride) {
        /* Indirect access pattern - harder to optimize */
        int src_idx = perm[i % size];
        int dst_idx = perm[(i + 1) % size];
        
        /* Conditional SIMD operation */
        if (src_idx != dst_idx) {
            X[dst_idx] = Y[src_idx] * 1.5f + sinf(i * 0.01f);
        } else {
            X[dst_idx] = Y[src_idx] * 0.5f + cosf(i * 0.02f);
        }
        
        /* Additional computation with thread-dependent condition */
        if ((omp_get_thread_num() % 8) < 4) {
            X[dst_idx] += 0.1f * (i % 32);
        }
    }
}

/* Test function 3: Separate parallel and simd regions with conditional */
void test_simt_conditional(int *data, int *mask, int rows, int cols) {
    #pragma omp target map(tofrom: data[0:rows*cols]) map(to: mask[0:rows*cols]) \
        num_teams(16)
    {
        #pragma omp parallel for simd collapse(2) \
            num_threads(omp_get_num_threads())
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                int idx = r * cols + c;
                
                /* Complex condition depending on multiple factors */
                int thread_mod = omp_get_thread_num() % 6;
                int warp_like = (idx / 32) % 4;
                
                if (thread_mod == warp_like) {
                    data[idx] = mask[idx] * 2 + thread_mod;
                } else if ((thread_mod + warp_like) % 2 == 0) {
                    data[idx] = mask[idx] / 2 + (thread_mod ^ warp_like);
                } else {
                    data[idx] = mask[idx] + (r * c) % 255;
                }
                
                /* Nested condition to create more basic blocks */
                if (data[idx] > 1000) {
                    data[idx] = (data[idx] * 13) % 997;
                } else if (data[idx] < -1000) {
                    data[idx] = (-data[idx] * 17) % 997;
                }
            }
        }
    }
}

/* Test function 4: Mixed SIMD and non-SIMD loops */
void test_mixed_simd(double *vec, int *indices, int n, int scale) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: vec[0:n]) map(to: indices[0:n]) \
        num_teams(8) thread_limit(64)
    for (int i = 0; i < n; i++) {
        int idx = indices[i % n];
        
        /* Switch-like conditional structure */
        switch (omp_get_thread_num() % 5) {
            case 0:
                vec[idx] = sin(vec[idx]) * scale;
                break;
            case 1:
                vec[idx] = cos(vec[idx]) / (scale + 1);
                break;
            case 2:
                vec[idx] = sqrt(fabs(vec[idx])) + i;
                break;
            case 3:
                vec[idx] = vec[idx] * vec[idx] - i;
                break;
            default:
                vec[idx] = (vec[idx] + i) * 0.5;
                break;
        }
        
        /* Additional conditional with early exit simulation */
        if (vec[idx] > 1e6) {
            vec[idx] = 1e6;
        } else if (vec[idx] < -1e6) {
            vec[idx] = -1e6;
        }
    }
}

/* Initialize arrays with pattern */
void init_arrays(int *A, int *B, int *C, int *mask, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        A[i] = 0;
        B[i] = (i * 3) % 97;
        C[i] = (i * 5) % 113;
        mask[i] = (i % 2 == 0) ? 1 : -1;
    }
}

void init_float_arrays(float *X, float *Y, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        X[i] = (i % 100) * 0.01f;
        Y[i] = ((i + 50) % 100) * 0.02f;
    }
}

void init_double_array(double *vec, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        vec[i] = (i % 200) * 0.005;
    }
}

void init_permutation(int *perm, int n) {
    for (int i = 0; i < n; i++) {
        perm[i] = (i * 17) % n;
    }
}

/* Compute checksum to ensure all code paths executed */
int compute_checksum(int *data, int n) {
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum = (sum + data[i]) % CHECKSUM_MOD;
    }
    return sum;
}

float compute_float_checksum(float *data, int n) {
    float sum = 0.0f;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += fabs(data[i]);
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int n = 1000, m = 200, iter = 10;
    
    /* Parse command line arguments */
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
    }
    if (argc >= 4) {
        iter = atoi(argv[3]);
    }
    
    int total_int = n * m;
    int total_small = n * 2;
    
    /* Dynamic allocation */
    int *A = (int *)malloc(total_int * sizeof(int));
    int *B = (int *)malloc(total_int * sizeof(int));
    int *C = (int *)malloc(total_int * sizeof(int));
    int *mask = (int *)malloc(total_int * sizeof(int));
    float *X = (float *)malloc(total_int * sizeof(float));
    float *Y = (float *)malloc(total_int * sizeof(float));
    double *vec = (double *)malloc(total_small * sizeof(double));
    int *perm = (int *)malloc(total_int * sizeof(int));
    int *indices = (int *)malloc(total_small * sizeof(int));
    
    if (!A || !B || !C || !mask || !X || !Y || !vec || !perm || !indices) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with patterns */
    init_arrays(A, B, C, mask, total_int);
    init_float_arrays(X, Y, total_int);
    init_double_array(vec, total_small);
    init_permutation(perm, total_int);
    init_permutation(indices, total_small);
    
    printf("Starting OpenMP target SIMT tests...\n");
    printf("Array sizes: %d x %d = %d elements\n", n, m, total_int);
    
    /* Execute multiple test functions with different constructs */
    for (int i = 0; i < iter; i++) {
        test_simt_nested(A, B, C, n, m, i);
        test_simt_mapped(X, Y, perm, total_int, 1 + (i % 4));
        test_simt_conditional(A, mask, n, m);
        test_mixed_simd(vec, indices, total_small, 2 + i);
        
        /* Modify some inputs for next iteration */
        #pragma omp parallel for simd
        for (int j = 0; j < total_int; j++) {
            B[j] = (B[j] + 1) % 100;
            C[j] = (C[j] + 2) % 150;
        }
    }
    
    /* Compute and print checksums */
    int checksum_A = compute_checksum(A, total_int);
    float checksum_X = compute_float_checksum(X, total_int);
    double sum_vec = 0.0;
    
    #pragma omp parallel for reduction(+:sum_vec)
    for (int i = 0; i < total_small; i++) {
        sum_vec += vec[i];
    }
    
    printf("Final checksums:\n");
    printf("  Integer array A: %d (mod %d)\n", checksum_A, CHECKSUM_MOD);
    printf("  Float array X: %.2f\n", checksum_X);
    printf("  Double array vec: %.4f\n", sum_vec);
    
    /* Cleanup */
    free(A); free(B); free(C); free(mask);
    free(X); free(Y); free(vec);
    free(perm); free(indices);
    
    return 0;
}
