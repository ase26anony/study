#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define CHECKSUM_MOD 10007

/* Function 1: Nested loops with collapse and SIMD clause */
void test_simt_nested(int *A, int *B, int *C, int n, int m, int iter) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: n, m, iter) map(tofrom: A[0:n*m], B[0:n*m]) map(to: C[0:n*m])
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            /* Conditional execution based on thread index and loop indices */
            if ((omp_get_thread_num() % 3 == 0) && (i % 2 == 0)) {
                A[idx] = B[idx] * C[idx] + iter;
            } else if ((j % 4 == 0) && (omp_get_team_num() % 2 == 0)) {
                A[idx] = B[idx] - C[idx] + (iter % 17);
            } else {
                A[idx] = (B[idx] + C[idx]) * (i + j + 1);
            }
            
            /* Additional conditional with early exit pattern */
            if (A[idx] > 1000 && j < m/2) {
                A[idx] = A[idx] % 1000;
            }
        }
    }
}

/* Function 2: Complex pointer-based accesses with SIMD */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(to: size, stride) map(tofrom: X[0:size]) map(to: Y[0:size], indices[0:size])
    for (int i = 0; i < size; i += stride) {
        int idx = indices[i % size];
        
        /* Multiple conditional paths with pointer arithmetic */
        float *ptr = &X[i];
        float *src = &Y[idx];
        
        if (omp_get_thread_num() % 5 == 0) {
            *ptr = *src * 2.0f + (i % 10) * 0.1f;
        } else if (omp_get_team_num() % 3 == 0) {
            *ptr = sinf(*src) + cosf(i * 0.01f);
        } else {
            *ptr = *src + *(src + (idx % 16)) * 0.5f;
        }
        
        /* SIMD-specific clause with safelen */
        #pragma omp simd safelen(16)
        for (int k = 0; k < 8; k++) {
            if (k % 2 == 0) {
                X[(i + k) % size] += Y[(idx + k) % size] * 0.25f;
            }
        }
    }
}

/* Function 3: Mixed parallel regions with explicit SIMD */
void test_simt_conditional(double *D, int *mask, int rows, int cols, int offset) {
    #pragma omp target map(to: rows, cols, offset) map(tofrom: D[0:rows*cols]) map(to: mask[0:rows*cols])
    {
        #pragma omp teams distribute
        for (int i = 0; i < rows; i++) {
            #pragma omp parallel for simd
            for (int j = 0; j < cols; j++) {
                int idx = i * cols + j;
                
                /* Complex condition depending on multiple factors */
                int thread_id = omp_get_thread_num();
                int team_id = omp_get_team_num();
                
                if ((thread_id % 2 == 0) && (team_id % 2 == 1)) {
                    D[idx] = (mask[idx] > 0) ? 
                             D[idx] * 1.5 + offset : 
                             D[idx] * 0.5 - offset;
                } else if ((i + j) % 3 == 0) {
                    D[idx] = sqrt(fabs(D[idx])) + thread_id * 0.01;
                } else {
                    D[idx] = D[idx] + sin(j * 0.1) * cos(i * 0.05);
                }
                
                /* Nested condition with early continue */
                if (D[idx] > 100.0) {
                    D[idx] = 100.0;
                    continue;
                }
                
                /* Additional computation with conditional store */
                double temp = D[idx] * (1.0 + (thread_id % 10) * 0.1);
                if (temp < D[idx] * 2.0) {
                    D[idx] = temp;
                }
            }
        }
    }
}

/* Function 4: Multiple SIMD clauses with reduction */
int test_simt_reduction(int *data, int n, int m) {
    int total = 0;
    
    #pragma omp target teams distribute parallel for simd \
        map(to: n, m) map(tofrom: data[0:n*m]) reduction(+:total) \
        collapse(2)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            /* Conditional modification with thread-dependent behavior */
            if (omp_get_thread_num() % 4 == 0) {
                data[idx] = (data[idx] * 3) % 997;
            } else if (j % 5 == 0) {
                data[idx] = data[idx] ^ (i << 3);
            }
            
            /* Conditional reduction update */
            if (data[idx] % 2 == 0) {
                total += data[idx] % 100;
            } else {
                total -= data[idx] % 50;
            }
        }
    }
    
    return total;
}

/* Helper function to compute checksum */
unsigned long compute_checksum(int *arr, int size) {
    unsigned long sum = 0;
    for (int i = 0; i < size; i++) {
        sum = (sum * 31 + arr[i]) % CHECKSUM_MOD;
    }
    return sum;
}

unsigned long compute_checksum_float(float *arr, int size) {
    unsigned long sum = 0;
    for (int i = 0; i < size; i++) {
        sum = (sum * 31 + (int)(arr[i] * 1000)) % CHECKSUM_MOD;
    }
    return sum;
}

unsigned long compute_checksum_double(double *arr, int size) {
    unsigned long sum = 0;
    for (int i = 0; i < size; i++) {
        sum = (sum * 31 + (int)(arr[i] * 1000)) % CHECKSUM_MOD;
    }
    return sum;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <size_n> <size_m>\n", argv[0]);
        return 1;
    }
    
    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    
    if (n <= 0 || m <= 0) {
        n = 100;
        m = 50;
    }
    
    int total_size = n * m;
    
    /* Allocate and initialize arrays with pattern-based data */
    int *A = (int *)malloc(total_size * sizeof(int));
    int *B = (int *)malloc(total_size * sizeof(int));
    int *C = (int *)malloc(total_size * sizeof(int));
    int *indices = (int *)malloc(total_size * sizeof(int));
    int *mask = (int *)malloc(total_size * sizeof(int));
    
    float *X = (float *)malloc(total_size * sizeof(float));
    float *Y = (float *)malloc(total_size * sizeof(float));
    
    double *D = (double *)malloc(total_size * sizeof(double));
    
    /* Initialize with non-constant patterns */
    for (int i = 0; i < total_size; i++) {
        A[i] = i % 97;
        B[i] = (i * 3) % 101;
        C[i] = (i * 5) % 103;
        indices[i] = (i * 7) % total_size;
        mask[i] = (i % 11) > 5 ? 1 : -1;
        
        X[i] = (i % 100) * 0.1f;
        Y[i] = (i % 50) * 0.2f;
        
        D[i] = (i % 80) * 0.05;
    }
    
    printf("Initial checksums:\n");
    printf("A: %lu\n", compute_checksum(A, total_size));
    printf("X: %lu\n", compute_checksum_float(X, total_size));
    printf("D: %lu\n", compute_checksum_double(D, total_size));
    
    /* Execute test functions with different OpenMP constructs */
    for (int iter = 0; iter < 3; iter++) {
        test_simt_nested(A, B, C, n, m, iter);
        
        if (iter % 2 == 0) {
            test_simt_mapped(X, Y, indices, total_size, 1 + iter);
        }
        
        test_simt_conditional(D, mask, n, m, iter * 10);
        
        int reduction_result = test_simt_reduction(C, n, m);
        printf("Iteration %d, reduction result: %d\n", iter, reduction_result);
    }
    
    printf("\nFinal checksums:\n");
    printf("A: %lu\n", compute_checksum(A, total_size));
    printf("B: %lu\n", compute_checksum(B, total_size));
    printf("C: %lu\n", compute_checksum(C, total_size));
    printf("X: %lu\n", compute_checksum_float(X, total_size));
    printf("Y: %lu\n", compute_checksum_float(Y, total_size));
    printf("D: %lu\n", compute_checksum_double(D, total_size));
    
    /* Cleanup */
    free(A);
    free(B);
    free(C);
    free(indices);
    free(mask);
    free(X);
    free(Y);
    free(D);
    
    return 0;
}
