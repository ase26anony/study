#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define MAX_SIZE 10000

/* Test function 1: Nested loops with SIMD clause and conditional execution */
void test_simt_nested(int *A, int *B, int n, int m, int iter) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(tofrom: A[0:n*m]) map(to: B[0:n*m]) \
        num_teams(16) thread_limit(128)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            /* Conditional execution that depends on thread index and loop indices */
            if ((i + j) % 3 == 0) {
                A[idx] = B[idx] * 2 + iter;
            } else if ((i * j) % 5 == 0) {
                A[idx] = B[idx] / 2 + omp_get_thread_num() % 4;
            } else {
                A[idx] = B[idx] + i - j + omp_get_team_num();
            }
            
            /* Additional control flow to increase complexity */
            if (A[idx] > 1000) {
                A[idx] = A[idx] % 1000;
            }
        }
    }
}

/* Test function 2: Mapped data with pointer-based accesses and SIMD */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: X[0:size]) map(to: Y[0:size], indices[0:size]) \
        simdlen(16) safelen(32)
    for (int i = 0; i < size; i += stride) {
        /* Complex pointer-based access pattern */
        int idx = indices[i % size];
        float temp = Y[idx];
        
        /* Conditional SIMD execution */
        if (omp_get_thread_num() % 8 < 4) {
            X[i] = temp * temp + sinf((float)i * 0.1f);
        } else {
            X[i] = sqrtf(fabsf(temp)) + cosf((float)i * 0.05f);
        }
        
        /* Nested condition to create more control flow */
        if (X[i] < 0.0f && (i % 7 == 0)) {
            X[i] = -X[i] + (omp_get_team_num() % 10) * 0.1f;
        }
    }
}

/* Test function 3: Target with nested parallel for simd and thread-dependent condition */
void test_simt_conditional(double *D, int *mask, int rows, int cols, int phase) {
    #pragma omp target map(tofrom: D[0:rows*cols]) map(to: mask[0:rows*cols]) \
        num_teams(8)
    {
        #pragma omp parallel for simd collapse(2) \
            simdlen(8) linear(i:1)
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                int idx = i * cols + j;
                
                /* Thread-dependent conditional execution */
                int tid = omp_get_thread_num();
                int team = omp_get_team_num();
                
                if ((tid % 2) == 0) {
                    D[idx] = (double)(tid + team) * 0.5 + sin((double)idx * 0.01);
                } else {
                    D[idx] = (double)(tid * team) * 0.25 + cos((double)idx * 0.02);
                }
                
                /* Additional mask-based conditional */
                if (mask[idx] > 0 && phase % 2 == 0) {
                    D[idx] = D[idx] * 2.0 - 1.0;
                } else if (mask[idx] < 0 || phase % 3 == 0) {
                    D[idx] = D[idx] * 0.5 + 0.1 * (double)(i + j);
                }
            }
        }
    }
}

/* Test function 4: Multiple SIMD constructs with varying parameters */
void test_simt_mixed(int *out, const int *in1, const int *in2, 
                     int dim1, int dim2, int dim3) {
    #pragma omp target teams distribute parallel for simd collapse(3) \
        map(tofrom: out[0:dim1*dim2*dim3]) \
        map(to: in1[0:dim1*dim2*dim3], in2[0:dim1*dim2*dim3]) \
        num_teams(32) thread_limit(64)
    for (int i = 0; i < dim1; i++) {
        for (int j = 0; j < dim2; j++) {
            for (int k = 0; k < dim3; k++) {
                int idx = (i * dim2 + j) * dim3 + k;
                
                /* Complex conditional chain */
                int tid = omp_get_thread_num();
                int cond1 = (i + j + k) % 11;
                int cond2 = (tid + idx) % 7;
                
                if (cond1 < cond2) {
                    out[idx] = in1[idx] * 3 - in2[idx];
                } else if (cond1 == cond2) {
                    out[idx] = (in1[idx] + in2[idx]) / 2 + tid;
                } else {
                    out[idx] = in1[idx] + in2[idx] * 2 + omp_get_team_num();
                }
                
                /* SIMD-width dependent operation */
                if (out[idx] % 4 == 0) {
                    out[idx] = out[idx] >> 2;
                } else if (out[idx] % 3 == 0) {
                    out[idx] = out[idx] << 1;
                }
            }
        }
    }
}

/* Helper function to compute checksum */
unsigned long long compute_checksum(int *data, int size) {
    unsigned long long sum = 0;
    for (int i = 0; i < size; i++) {
        sum += (unsigned long long)(data[i] % 1000000);
    }
    return sum;
}

/* Helper function to compute floating point checksum */
double compute_fp_checksum(float *data, int size) {
    double sum = 0.0;
    for (int i = 0; i < size; i++) {
        sum += (double)fabs(data[i]);
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Parse command line arguments */
    int base_size = 512;
    int iterations = 10;
    
    if (argc >= 2) base_size = atoi(argv[1]);
    if (argc >= 3) iterations = atoi(argv[2]);
    
    if (base_size <= 0) base_size = 512;
    if (iterations <= 0) iterations = 10;
    
    /* Limit sizes to prevent excessive memory usage */
    int n = base_size;
    int m = base_size / 2;
    int size1 = n * m;
    int size2 = base_size * 64;
    int rows = base_size / 4;
    int cols = base_size / 8;
    int size3 = rows * cols;
    int dim1 = base_size / 8;
    int dim2 = base_size / 16;
    int dim3 = base_size / 32;
    int size4 = dim1 * dim2 * dim3;
    
    printf("Running SIMT transformation tests with sizes:\n");
    printf("  test1: %d x %d = %d elements\n", n, m, size1);
    printf("  test2: %d elements\n", size2);
    printf("  test3: %d x %d = %d elements\n", rows, cols, size3);
    printf("  test4: %d x %d x %d = %d elements\n", dim1, dim2, dim3, size4);
    printf("  iterations: %d\n\n", iterations);
    
    /* Allocate and initialize arrays */
    int *A1 = (int *)malloc(size1 * sizeof(int));
    int *B1 = (int *)malloc(size1 * sizeof(int));
    float *X2 = (float *)malloc(size2 * sizeof(float));
    float *Y2 = (float *)malloc(size2 * sizeof(float));
    int *indices2 = (int *)malloc(size2 * sizeof(int));
    double *D3 = (double *)malloc(size3 * sizeof(double));
    int *mask3 = (int *)malloc(size3 * sizeof(int));
    int *out4 = (int *)malloc(size4 * sizeof(int));
    int *in14 = (int *)malloc(size4 * sizeof(int));
    int *in24 = (int *)malloc(size4 * sizeof(int));
    
    if (!A1 || !B1 || !X2 || !Y2 || !indices2 || !D3 || !mask3 || !out4 || !in14 || !in24) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern-based data */
    for (int i = 0; i < size1; i++) {
        A1[i] = 0;
        B1[i] = i % 97;
    }
    
    for (int i = 0; i < size2; i++) {
        X2[i] = 0.0f;
        Y2[i] = (float)(i % 113) * 0.1f;
        indices2[i] = (i * 31) % size2;
    }
    
    for (int i = 0; i < size3; i++) {
        D3[i] = 0.0;
        mask3[i] = (i % 5) - 2;  /* Values between -2 and 2 */
    }
    
    for (int i = 0; i < size4; i++) {
        out4[i] = 0;
        in14[i] = i % 79;
        in24[i] = (i * 17) % 89;
    }
    
    unsigned long long total_checksum = 0;
    double total_fp_checksum = 0.0;
    
    /* Execute test functions multiple times with different parameters */
    for (int iter = 0; iter < iterations; iter++) {
        printf("Iteration %d/%d:\n", iter + 1, iterations);
        
        /* Test 1: Nested loops with SIMD */
        test_simt_nested(A1, B1, n, m, iter);
        unsigned long long checksum1 = compute_checksum(A1, size1);
        total_checksum += checksum1;
        printf("  Test1 checksum: %llu\n", checksum1);
        
        /* Test 2: Mapped data with pointer accesses */
        test_simt_mapped(X2, Y2, indices2, size2, 1 + (iter % 4));
        double checksum2 = compute_fp_checksum(X2, size2);
        total_fp_checksum += checksum2;
        printf("  Test2 fp checksum: %.2f\n", checksum2);
        
        /* Test 3: Conditional execution with thread dependency */
        test_simt_conditional(D3, mask3, rows, cols, iter);
        
        /* Test 4: Multiple SIMD constructs */
        test_simt_mixed(out4, in14, in24, dim1, dim2, dim3);
        unsigned long long checksum4 = compute_checksum(out4, size4);
        total_checksum += checksum4;
        printf("  Test4 checksum: %llu\n", checksum4);
        
        /* Modify input data for next iteration */
        for (int i = 0; i < size1; i++) {
            B1[i] = (B1[i] + 1) % 100;
        }
        
        for (int i = 0; i < size2; i++) {
            Y2[i] = Y2[i] * 1.1f;
            indices2[i] = (indices2[i] + 7) % size2;
        }
        
        for (int i = 0; i < size4; i++) {
            in14[i] = (in14[i] + iter) % 100;
            in24[i] = (in24[i] + iter * 2) % 100;
        }
    }
    
    printf("\nFinal results:\n");
    printf("  Total integer checksum: %llu\n", total_checksum);
    printf("  Total floating point checksum: %.2f\n", total_fp_checksum);
    
    /* Verify some results */
    int final_A1 = A1[size1 / 2];
    float final_X2 = X2[size2 / 3];
    double final_D3 = D3[size3 / 4];
    int final_out4 = out4[size4 / 5];
    
    printf("  Sample values: A1[%d]=%d, X2[%d]=%.3f, D3[%d]=%.3f, out4[%d]=%d\n",
           size1 / 2, final_A1,
           size2 / 3, final_X2,
           size3 / 4, final_D3,
           size4 / 5, final_out4);
    
    /* Cleanup */
    free(A1);
    free(B1);
    free(X2);
    free(Y2);
    free(indices2);
    free(D3);
    free(mask3);
    free(out4);
    free(in14);
    free(in24);
    
    return 0;
}
