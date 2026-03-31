#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define CHECKSUM_MOD 10007

/* Test 1: Nested loops with collapse and conditional inside SIMD */
void test_simt_nested(int *A, int *B, int *C, int n, int m) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: n, m) map(tofrom: A[0:n*m], B[0:n*m]) map(to: C[0:n*m])
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            /* Conditional execution path that depends on thread/loop indices */
            if ((i + j) % 3 == 0) {
                A[idx] = B[idx] * 2 + C[idx];
            } else if ((i + j) % 3 == 1) {
                A[idx] = B[idx] / 2 + C[idx];
            } else {
                A[idx] = B[idx] + C[idx] * 3;
            }
            
            /* Additional control flow with thread index check */
            if (omp_get_thread_num() % 4 == 0) {
                A[idx] += 1;
            }
        }
    }
}

/* Test 2: Complex data mapping with pointer indirection and safelen */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(to: size, stride) map(tofrom: X[0:size]) map(to: Y[0:size], indices[0:size]) \
        simd safelen(16)
    for (int i = 0; i < size; i += stride) {
        /* Complex memory access pattern with indirection */
        int idx = indices[i % size];
        if (idx >= 0 && idx < size) {
            /* Conditional transformation based on value */
            if (Y[idx] > 0.5f) {
                X[i] = Y[idx] * 2.0f + sinf((float)i * 0.1f);
            } else {
                X[i] = Y[idx] * 0.5f + cosf((float)i * 0.05f);
            }
            
            /* Thread-dependent operation */
            int tid = omp_get_thread_num();
            if (tid % 8 < 4) {
                X[i] += 0.1f * (tid % 8);
            }
        }
    }
}

/* Test 3: Separate target and parallel for simd with conditional */
void test_simt_conditional(double *D, int *mask, int rows, int cols) {
    #pragma omp target map(to: rows, cols) map(tofrom: D[0:rows*cols]) map(to: mask[0:rows*cols])
    {
        #pragma omp parallel for simd collapse(2)
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                int idx = r * cols + c;
                
                /* Complex condition depending on multiple factors */
                int condition = (mask[idx] + omp_get_thread_num() + r + c) % 5;
                
                switch (condition) {
                    case 0:
                        D[idx] = D[idx] * 2.0 + r * 0.01;
                        break;
                    case 1:
                        D[idx] = sqrt(fabs(D[idx])) + c * 0.02;
                        break;
                    case 2:
                        D[idx] = D[idx] * D[idx] * 0.5;
                        break;
                    case 3:
                        D[idx] = (D[idx] > 0) ? log1p(D[idx]) : D[idx];
                        break;
                    default:
                        D[idx] = sin(D[idx] * 0.1) * 100.0;
                }
                
                /* Additional SIMT-relevant branching */
                if (omp_get_team_num() % 2 == 0) {
                    D[idx] += 0.5;
                } else {
                    D[idx] -= 0.5;
                }
            }
        }
    }
}

/* Test 4: Multiple nested directives to stress the lowering pass */
void test_multi_nest(int *out, const int *in1, const int *in2, int dim1, int dim2, int dim3) {
    #pragma omp target teams distribute parallel for simd collapse(3) \
        map(to: dim1, dim2, dim3) map(tofrom: out[0:dim1*dim2*dim3]) \
        map(to: in1[0:dim1*dim2*dim3], in2[0:dim1*dim2*dim3])
    for (int i = 0; i < dim1; i++) {
        for (int j = 0; j < dim2; j++) {
            for (int k = 0; k < dim3; k++) {
                int idx = (i * dim2 + j) * dim3 + k;
                
                /* Multiple conditional paths */
                int t = omp_get_thread_num();
                int cond1 = (i + j + k) % 7;
                int cond2 = t % 3;
                
                if (cond1 == cond2) {
                    out[idx] = in1[idx] * 3 + in2[idx];
                } else if (cond1 > cond2) {
                    out[idx] = in1[idx] + in2[idx] * 2;
                } else {
                    out[idx] = in1[idx] - in2[idx];
                }
                
                /* SIMD lane dependent operation */
                if ((k % 16) < 8) {
                    out[idx] += 1;
                }
            }
        }
    }
}

/* Helper function to compute checksum */
int compute_checksum(int *data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum = (sum + data[i]) % CHECKSUM_MOD;
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Parse command line arguments for sizes */
    int base_size = 1000;
    int iterations = 100;
    
    if (argc >= 3) {
        base_size = atoi(argv[1]);
        iterations = atoi(argv[2]);
    }
    
    printf("Running with base_size=%d, iterations=%d\n", base_size, iterations);
    
    /* Dynamically allocate arrays with varying sizes */
    int n = base_size;
    int m = base_size / 2;
    int size1 = n * m;
    int size2 = base_size * 2;
    int rows = base_size / 4;
    int cols = base_size / 4;
    int size3 = rows * cols;
    int dim1 = base_size / 10;
    int dim2 = base_size / 10;
    int dim3 = 32;  /* SIMD width multiple */
    int size4 = dim1 * dim2 * dim3;
    
    /* Allocate and initialize integer arrays */
    int *A = (int *)malloc(size1 * sizeof(int));
    int *B = (int *)malloc(size1 * sizeof(int));
    int *C = (int *)malloc(size1 * sizeof(int));
    int *indices = (int *)malloc(size2 * sizeof(int));
    int *mask = (int *)malloc(size3 * sizeof(int));
    int *out_multi = (int *)malloc(size4 * sizeof(int));
    int *in1_multi = (int *)malloc(size4 * sizeof(int));
    int *in2_multi = (int *)malloc(size4 * sizeof(int));
    
    /* Allocate and initialize float/double arrays */
    float *X = (float *)malloc(size2 * sizeof(float));
    float *Y = (float *)malloc(size2 * sizeof(float));
    double *D = (double *)malloc(size3 * sizeof(double));
    
    if (!A || !B || !C || !indices || !mask || !out_multi || !in1_multi || !in2_multi ||
        !X || !Y || !D) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern-based data */
    for (int i = 0; i < size1; i++) {
        A[i] = 0;
        B[i] = i % 97;
        C[i] = (i * 3) % 101;
    }
    
    for (int i = 0; i < size2; i++) {
        X[i] = 0.0f;
        Y[i] = (float)(i % 100) / 100.0f;
        indices[i] = (i * 7) % size2;
    }
    
    for (int i = 0; i < size3; i++) {
        D[i] = (double)(i % 50) * 0.1;
        mask[i] = i % 11;
    }
    
    for (int i = 0; i < size4; i++) {
        out_multi[i] = 0;
        in1_multi[i] = i % 89;
        in2_multi[i] = (i * 5) % 73;
    }
    
    /* Execute test functions multiple times with different parameters */
    for (int iter = 0; iter < iterations; iter++) {
        int offset = iter % 10;
        
        /* Modify parameters slightly each iteration */
        test_simt_nested(A, B, C, n + offset, m);
        test_simt_mapped(X, Y, indices, size2 - offset * 10, 1 + (iter % 4));
        test_simt_conditional(D, mask, rows, cols);
        test_multi_nest(out_multi, in1_multi, in2_multi, 
                       dim1 + (iter % 3), dim2, dim3);
        
        /* Occasionally resize */
        if (iter % 20 == 19) {
            free(A); free(B); free(C);
            size1 = (n + iter) * (m + iter/2);
            A = (int *)malloc(size1 * sizeof(int));
            B = (int *)malloc(size1 * sizeof(int));
            C = (int *)malloc(size1 * sizeof(int));
            
            for (int i = 0; i < size1; i++) {
                A[i] = 0;
                B[i] = (i + iter) % 97;
                C[i] = (i * 3 + iter) % 101;
            }
        }
    }
    
    /* Compute and print checksums to ensure execution */
    int checksum_A = compute_checksum(A, size1 > 1000 ? 1000 : size1);
    int checksum_out = compute_checksum(out_multi, size4 > 1000 ? 1000 : size4);
    
    printf("Checksum A: %d\n", checksum_A);
    printf("Checksum out_multi: %d\n", checksum_out);
    
    /* Verify some values */
    int verify_count = 0;
    for (int i = 0; i < 100 && i < size1; i++) {
        if (A[i] != 0) verify_count++;
    }
    printf("Non-zero values in first 100 of A: %d\n", verify_count);
    
    /* Cleanup */
    free(A); free(B); free(C);
    free(indices); free(mask);
    free(out_multi); free(in1_multi); free(in2_multi);
    free(X); free(Y); free(D);
    
    return 0;
}
