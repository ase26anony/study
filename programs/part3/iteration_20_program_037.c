/* test_simt_lowering.c
 * 
 * This program is designed to trigger the SIMT lowering transformation
 * in GCC's omp-low.cc, specifically the uncovered block that generates
 * IFN_GOMP_USE_SIMT and restructures loops for GPU offloading.
 * 
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fno-inline -o test_simt test_simt_lowering.c
 * 
 * For coverage analysis, add appropriate GCOV flags and run with different arguments.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact for lowering */
__attribute__((noinline))
void target_simple_vector_add(float *arr, int size) {
    #pragma omp target teams distribute parallel for simd map(tofrom: arr[0:size])
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] * 3.14f + 1.0f;
    }
}

__attribute__((noinline))
void target_conditional_update(float *data, int size, float threshold) {
    #pragma omp target teams distribute parallel for map(tofrom: data[0:size])
    for (int i = 0; i < size; ++i) {
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]);
        } else {
            data[i] = data[i] * 0.5f;
        }
    }
}

__attribute__((noinline))
void target_nested_control_flow(int *a, int *b, int *c, int size) {
    #pragma omp target teams distribute parallel for simd map(tofrom: a[0:size], b[0:size], c[0:size])
    for (int i = 0; i < size; ++i) {
        int val = a[i] + b[i];
        if (val % 2 == 0) {
            c[i] = val * 2;
        } else {
            c[i] = val / 2;
        }
    }
}

__attribute__((noinline))
void target_multiple_arrays(float *x, float *y, float *z, int size) {
    #pragma omp target teams distribute parallel for simd \
        map(to: x[0:size], y[0:size]) map(from: z[0:size])
    for (int i = 0; i < size; ++i) {
        z[i] = x[i] * y[i] + sinf(x[i]) * cosf(y[i]);
    }
}

float compute_checksum(float *arr, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; ++i) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int test_size = N;
    int test_case = 0;
    
    /* Use command-line arguments to select test case and size */
    if (argc > 1) {
        test_case = atoi(argv[1]) % 4;
    }
    if (argc > 2) {
        test_size = atoi(argv[2]);
        if (test_size <= 0) test_size = N;
        if (test_size > 10000) test_size = 10000; /* Reasonable limit */
    }
    
    printf("Running test case %d with size %d\n", test_case, test_size);
    
    /* Allocate and initialize test data */
    float *data1 = (float *)malloc(test_size * sizeof(float));
    float *data2 = (float *)malloc(test_size * sizeof(float));
    float *data3 = (float *)malloc(test_size * sizeof(float));
    int *idata1 = (int *)malloc(test_size * sizeof(int));
    int *idata2 = (int *)malloc(test_size * sizeof(int));
    int *idata3 = (int *)malloc(test_size * sizeof(int));
    
    if (!data1 || !data2 || !data3 || !idata1 || !idata2 || !idata3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with deterministic but varying values */
    for (int i = 0; i < test_size; ++i) {
        data1[i] = (float)(i % 100) * 1.5f;
        data2[i] = (float)((i + 1) % 100) * 0.7f;
        data3[i] = 0.0f;
        idata1[i] = i;
        idata2[i] = test_size - i;
        idata3[i] = 0;
    }
    
    /* Run multiple iterations to increase coverage chances */
    for (int iter = 0; iter < 3; ++iter) {
        printf("Iteration %d: ", iter);
        
        switch (test_case) {
            case 0:
                target_simple_vector_add(data1, test_size);
                printf("Simple vector add - checksum: %.2f\n", 
                       compute_checksum(data1, test_size));
                break;
                
            case 1:
                target_conditional_update(data1, test_size, THRESHOLD);
                printf("Conditional update - checksum: %.2f\n",
                       compute_checksum(data1, test_size));
                break;
                
            case 2:
                target_nested_control_flow(idata1, idata2, idata3, test_size);
                {
                    int sum = 0;
                    for (int i = 0; i < test_size; ++i) sum += idata3[i];
                    printf("Nested control flow - sum: %d\n", sum);
                }
                break;
                
            case 3:
                target_multiple_arrays(data1, data2, data3, test_size);
                printf("Multiple arrays - checksum: %.2f\n",
                       compute_checksum(data3, test_size));
                break;
                
            default:
                break;
        }
        
        /* Rotate test cases to exercise different paths */
        test_case = (test_case + 1) % 4;
    }
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    free(idata1);
    free(idata2);
    free(idata3);
    
    printf("Test completed successfully.\n");
    return 0;
}
