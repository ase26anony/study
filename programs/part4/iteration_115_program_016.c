#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <math.h>

#define MAX_SIZE 10000

/* Test 1: Nested loops with collapse and conditional inside SIMD */
void test_simt_nested(int *A, int *B, int *C, int n, int m) {
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(to: n, m, B[0:n*m], C[0:n*m]) map(tofrom: A[0:n*m]) \
        default(none)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            
            /* Conditional that depends on loop indices - may trigger SIMT transformation */
            if ((i + j) % 3 == 0) {
                A[idx] = B[idx] * 2;
            } else if ((i + j) % 3 == 1) {
                A[idx] = B[idx] + C[idx];
            } else {
                A[idx] = B[idx] - C[idx];
            }
            
            /* Additional conditional with thread ID */
            if (omp_get_thread_num() % 4 == 0) {
                A[idx] += 1;
            }
        }
    }
}

/* Test 2: Complex pointer-based accesses with SIMD */
void test_simt_mapped(float *X, float *Y, int *indices, int size, int stride) {
    #pragma omp target teams distribute parallel for simd \
        map(to: size, stride, Y[0:size], indices[0:size]) \
        map(tofrom: X[0:size]) \
        simdlen(16) safelen(32) \
        default(none)
    for (int i = 0; i < size; i += stride) {
        /* Indirect access pattern - encourages SIMT for coalescing */
        int idx = indices[i % size];
        if (idx >= 0 && idx < size) {
            X[i] = Y[idx] * 2.0f;
        } else {
            X[i] = Y[i % size] * 1.5f;
        }
        
        /* Complex conditional with floating point */
        if (X[i] > 100.0f) {
            X[i] = sqrtf(X[i]);
        } else if (X[i] < -50.0f) {
            X[i] = fabsf(X[i]);
        }
    }
}

/* Test 3: Nested parallel for simd inside target region */
void test_simt_conditional(double *D, int *mask, int rows, int cols) {
    #pragma omp target map(to: rows, cols, mask[0:rows*cols]) \
        map(tofrom: D[0:rows*cols]) default(none)
    {
        #pragma omp teams distribute
        for (int i = 0; i < rows; i++) {
            #pragma omp parallel for simd
            for (int j = 0; j < cols; j++) {
                int idx = i * cols + j;
                
                /* Condition depending on thread number - may trigger IFN_GOMP_USE_SIMT */
                int tid = omp_get_thread_num();
                if (tid % 2 == 0) {
                    D[idx] = mask[idx] ? D[idx] * 3.14 : D[idx] / 2.0;
                } else {
                    D[idx] = mask[idx] ? D[idx] + 1.0 : D[idx] - 1.0;
                }
                
                /* Nested condition with loop variables */
                if (i > rows/2 && j < cols/3) {
                    D[idx] = sin(D[idx]);
                }
            }
        }
    }
}

/* Test 4: Multiple SIMD clauses with reduction */
void test_simt_reduction(int *data, int n, int *result) {
    int sum = 0;
    
    #pragma omp target teams distribute parallel for simd \
        map(to: n, data[0:n]) map(tofrom: sum) \
        reduction(+:sum) \
        default(none)
    for (int i = 0; i < n; i++) {
        /* Conditional that creates divergent execution paths */
        if (data[i] % 7 == 0) {
            sum += data[i] * 2;
        } else if (data[i] % 7 == 3) {
            sum += data[i] / 2;
        } else {
            sum += data[i];
        }
        
        /* Additional thread-dependent operation */
        if (omp_get_thread_num() % 3 == 0) {
            data[i] += 1;
        }
    }
    
    *result = sum;
}

/* Test 5: Complex nested loops with multiple conditions */
void test_simt_complex(unsigned char *img, int width, int height, int channels) {
    #pragma omp target teams distribute parallel for simd collapse(3) \
        map(to: width, height, channels) map(tofrom: img[0:width*height*channels]) \
        default(none)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            for (int c = 0; c < channels; c++) {
                int idx = (y * width + x) * channels + c;
                
                /* Multiple conditions creating complex control flow */
                if (y % 8 == 0) {
                    if (x % 4 == 0) {
                        img[idx] = (img[idx] * 2) % 256;
                    } else {
                        img[idx] = (img[idx] + 128) % 256;
                    }
                } else if (y % 5 == 0) {
                    img[idx] = 255 - img[idx];
                } else {
                    if (c == 0) {
                        img[idx] = (img[idx] + 64) % 256;
                    } else if (c == 1) {
                        img[idx] = (img[idx] * 3) % 256;
                    } else {
                        img[idx] = (img[idx] / 2) % 256;
                    }
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int n = 512, m = 256;
    int size = 10000;
    
    /* Parse command line arguments for sizes */
    if (argc >= 3) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        if (n <= 0) n = 512;
        if (m <= 0) m = 256;
    }
    if (argc >= 4) {
        size = atoi(argv[3]);
        if (size <= 0) size = 10000;
    }
    
    printf("Testing SIMT transformations with n=%d, m=%d, size=%d\n", n, m, size);
    
    /* Allocate and initialize arrays */
    int *A = (int *)malloc(n * m * sizeof(int));
    int *B = (int *)malloc(n * m * sizeof(int));
    int *C = (int *)malloc(n * m * sizeof(int));
    
    float *X = (float *)malloc(size * sizeof(float));
    float *Y = (float *)malloc(size * sizeof(float));
    int *indices = (int *)malloc(size * sizeof(int));
    
    double *D = (double *)malloc(n * m * sizeof(double));
    int *mask = (int *)malloc(n * m * sizeof(int));
    
    int *data = (int *)malloc(size * sizeof(int));
    int reduction_result = 0;
    
    int img_width = 256, img_height = 256, img_channels = 3;
    unsigned char *img = (unsigned char *)malloc(img_width * img_height * img_channels * sizeof(unsigned char));
    
    /* Initialize with pattern-based data */
    for (int i = 0; i < n * m; i++) {
        A[i] = 0;
        B[i] = i % 97;
        C[i] = (i * 3) % 113;
        D[i] = (double)(i % 100) / 10.0;
        mask[i] = (i % 5 == 0) ? 1 : 0;
    }
    
    for (int i = 0; i < size; i++) {
        X[i] = (float)(i % 100) * 1.5f;
        Y[i] = (float)((i * 7) % 100) * 0.7f;
        indices[i] = (i * 3) % size;
        data[i] = i % 1000;
    }
    
    for (int i = 0; i < img_width * img_height * img_channels; i++) {
        img[i] = (unsigned char)(i % 256);
    }
    
    /* Execute test functions with different OpenMP constructs */
    printf("Running test_simt_nested...\n");
    test_simt_nested(A, B, C, n, m);
    
    printf("Running test_simt_mapped...\n");
    test_simt_mapped(X, Y, indices, size, 2);
    
    printf("Running test_simt_conditional...\n");
    test_simt_conditional(D, mask, n, m);
    
    printf("Running test_simt_reduction...\n");
    test_simt_reduction(data, size, &reduction_result);
    
    printf("Running test_simt_complex...\n");
    test_simt_complex(img, img_width, img_height, img_channels);
    
    /* Compute checksums to ensure all code paths executed */
    long long checksum_A = 0;
    long long checksum_X = 0;
    double checksum_D = 0.0;
    long long checksum_img = 0;
    
    for (int i = 0; i < n * m; i++) {
        checksum_A += A[i];
        checksum_D += D[i];
    }
    
    for (int i = 0; i < size; i++) {
        checksum_X += (long long)X[i];
    }
    
    for (int i = 0; i < img_width * img_height * img_channels; i++) {
        checksum_img += img[i];
    }
    
    printf("\nResults:\n");
    printf("Checksum A: %lld\n", checksum_A);
    printf("Checksum X: %lld\n", checksum_X);
    printf("Checksum D: %f\n", checksum_D);
    printf("Reduction result: %d\n", reduction_result);
    printf("Checksum img: %lld\n", checksum_img);
    
    /* Cleanup */
    free(A); free(B); free(C);
    free(X); free(Y); free(indices);
    free(D); free(mask);
    free(data);
    free(img);
    
    return 0;
}
