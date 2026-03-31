#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define CHECKSUM_MOD 10007

/* Function 1: Nested loops with collapse and conditional inside SIMD */
void test_simt_nested(int *A, int *B, int n, int m, int iter) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(tofrom: A[0:n*m]) map(to: B[0:n*m]) \
        num_teams(n/32) thread_limit(256)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            /* Conditional execution inside SIMD loop - may trigger SIMT transformation */
            if ((i + j) % 3 == 0) {
                A[idx] = B[idx] * 2 + iter;
            } else if ((i + j) % 3 == 1) {
                A[idx] = B[idx] / 2 + iter;
            } else {
                A[idx] = B[idx] + i - j + iter;
            }
            
            /* Additional control flow with thread-dependent condition */
            int tid = omp_get_thread_num();
            if (tid % 4 == 0) {
                A[idx] += 1;
            }
        }
    }
}

/* Function 2: Complex pointer-based accesses with SIMD */
void test_simt_mapped(float *X, int *indices, float *results, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(to: X[0:size*stride], indices[0:size]) \
        map(tofrom: results[0:size]) \
        safelen(32) num_teams(64) thread_limit(128)
    for (int i = 0; i < size; i++) {
        /* Indirect access pattern - may influence SIMT memory coalescing decisions */
        int idx = indices[i] % size;
        float temp = X[idx * stride];
        
        /* Conditional with floating point operations */
        if (temp > 0.5f) {
            results[i] = sinf(temp) * cosf((float)i);
        } else {
            results[i] = logf(fabsf(temp) + 1.0f) * (float)(i % 8);
        }
        
        /* SIMD-unfriendly pattern to potentially trigger special handling */
        for (int k = 0; k < (i % 4); k++) {
            results[i] += 0.1f * k;
        }
    }
}

/* Function 3: Nested parallel for simd with thread-dependent conditions */
void test_simt_conditional(double *data, int *mask, int rows, int cols, int phase) {
    #pragma omp target map(tofrom: data[0:rows*cols]) map(to: mask[0:rows*cols]) \
        num_teams(rows/16)
    {
        #pragma omp parallel for simd collapse(2) \
            simdlen(16) aligned(data: 64)
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                int idx = r * cols + c;
                
                /* Complex condition depending on thread ID and loop indices */
                int tid = omp_get_thread_num();
                int team_id = omp_get_team_num();
                
                if ((tid % 2 == 0) && (team_id % 2 == phase % 2)) {
                    data[idx] = data[idx] * 2.0 + (double)(r - c);
                } else if (mask[idx] > 0) {
                    data[idx] = sqrt(fabs(data[idx])) + (double)tid * 0.01;
                } else {
                    data[idx] = data[idx] / 3.0 - (double)(r * c) * 0.001;
                }
                
                /* Additional branching based on computed value */
                if (data[idx] > 100.0) {
                    data[idx] = 100.0;
                } else if (data[idx] < -100.0) {
                    data[idx] = -100.0;
                }
            }
        }
    }
}

/* Function 4: Mixed directives to explore different lowering paths */
void test_mixed_patterns(int *out, const int *in, int n, int offset) {
    /* First a simple target teams */
    #pragma omp target teams distribute \
        map(to: in[0:n]) map(from: out[0:n]) \
        num_teams(n/64)
    for (int i = 0; i < n; i++) {
        out[i] = in[i] + offset;
    }
    
    /* Then a target with parallel for simd */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: out[0:n]) \
        num_teams(n/32) thread_limit(64)
    for (int i = 0; i < n; i++) {
        /* Conditional that depends on i in non-linear way */
        if ((i * i) % 7 == offset % 7) {
            out[i] = out[i] * 3 - 1;
        } else {
            out[i] = out[i] / 2 + 5;
        }
    }
}

/* Helper function to compute checksum */
unsigned long long compute_checksum(int *arr, int size) {
    unsigned long long sum = 0;
    for (int i = 0; i < size; i++) {
        sum = (sum * 31 + (arr[i] % CHECKSUM_MOD)) % CHECKSUM_MOD;
    }
    return sum;
}

unsigned long long compute_float_checksum(float *arr, int size) {
    unsigned long long sum = 0;
    for (int i = 0; i < size; i++) {
        int val = (int)(fabs(arr[i]) * 1000);
        sum = (sum * 31 + (val % CHECKSUM_MOD)) % CHECKSUM_MOD;
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Parse command line arguments */
    int base_size = 512;
    int iterations = 10;
    
    if (argc >= 3) {
        base_size = atoi(argv[1]);
        iterations = atoi(argv[2]);
    }
    printf("Running with base_size=%d, iterations=%d\n", base_size, iterations);
    
    /* Dynamic allocations with varying sizes */
    int n = base_size;
    int m = base_size / 2;
    int total_int = n * m;
    int total_float = base_size * 32;
    
    int *A = (int *)malloc(total_int * sizeof(int));
    int *B = (int *)malloc(total_int * sizeof(int));
    int *indices = (int *)malloc(base_size * sizeof(int));
    int *mask = (int *)malloc(total_int * sizeof(int));
    float *X = (float *)malloc(total_float * sizeof(float));
    float *results = (float *)malloc(base_size * sizeof(float));
    double *data = (double *)malloc(total_int * sizeof(double));
    int *mixed_out = (int *)malloc(base_size * sizeof(int));
    int *mixed_in = (int *)malloc(base_size * sizeof(int));
    
    if (!A || !B || !indices || !mask || !X || !results || !data || !mixed_out || !mixed_in) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pattern-based data */
    for (int i = 0; i < total_int; i++) {
        A[i] = 0;
        B[i] = (i * 17) % 97;
        mask[i] = (i % 3 == 0) ? 1 : 0;
        data[i] = (double)(i % 100) * 0.5 - 25.0;
    }
    
    for (int i = 0; i < base_size; i++) {
        indices[i] = (i * 23) % base_size;
        mixed_in[i] = (i * 11) % 79;
        mixed_out[i] = 0;
    }
    
    for (int i = 0; i < total_float; i++) {
        X[i] = (float)((i * 13) % 100) / 99.0f;
    }
    
    for (int i = 0; i < base_size; i++) {
        results[i] = 0.0f;
    }
    
    /* Execute test functions multiple times with different parameters */
    unsigned long long total_checksum = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        printf("Iteration %d/%d\n", iter + 1, iterations);
        
        /* Call test functions - each with different OpenMP constructs */
        test_simt_nested(A, B, n, m, iter);
        
        test_simt_mapped(X, indices, results, base_size, 32);
        
        test_simt_conditional(data, mask, n, m, iter);
        
        test_mixed_patterns(mixed_out, mixed_in, base_size, iter);
        
        /* Compute and accumulate checksums */
        total_checksum = (total_checksum + compute_checksum(A, total_int)) % CHECKSUM_MOD;
        total_checksum = (total_checksum + compute_float_checksum(results, base_size)) % CHECKSUM_MOD;
        total_checksum = (total_checksum + compute_checksum(mixed_out, base_size)) % CHECKSUM_MOD;
        
        /* Modify input data slightly for next iteration */
        for (int i = 0; i < total_int; i++) {
            B[i] = (B[i] + 1) % 97;
        }
    }
    
    printf("Final checksum: %llu\n", total_checksum);
    
    /* Verify some results */
    int verify_sum = 0;
    for (int i = 0; i < 100 && i < total_int; i++) {
        verify_sum += A[i];
    }
    printf("Sample verification sum (first 100 elements of A): %d\n", verify_sum);
    
    /* Cleanup */
    free(A);
    free(B);
    free(indices);
    free(mask);
    free(X);
    free(results);
    free(data);
    free(mixed_out);
    free(mixed_in);
    
    return 0;
}
