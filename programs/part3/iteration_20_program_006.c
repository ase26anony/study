/* Test program to cover SIMT transformation in omp-low.cc
 * Specifically targets lines 2941-2975 in omp-low.cc.gcov
 * Compile with: g++ -O2 -fopenmp -foffload=nvptx-none -fno-inline -fdump-tree-omplower simt_test.cc -o simt_test
 */

#include <cstdlib>
#include <iostream>
#include <cmath>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact */
__attribute__((noinline))
void target_simt_vector_scale(float* arr, int size, float scale) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) num_teams(4) thread_limit(256)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] * scale + 1.0f;
    }
}

__attribute__((noinline))
void target_simt_conditional_update(float* data, int size, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) num_teams(8)
    for (int i = 0; i < size; ++i) {
        /* Complex enough control flow for GIMPLE sequence */
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]) * 2.0f;
        } else {
            data[i] = data[i] * 0.5f;
        }
    }
}

__attribute__((noinline))
void target_simt_nested_if(float* a, float* b, int size) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:size]) map(from: b[0:size]) \
        num_teams(2) num_threads(128)
    for (int i = 0; i < size; ++i) {
        /* Multiple conditions to create interesting GIMPLE */
        if (i % 2 == 0) {
            b[i] = a[i] * 3.14f;
        } else if (i % 3 == 0) {
            b[i] = a[i] / 2.0f;
        } else {
            b[i] = a[i] + 100.0f;
        }
    }
}

__attribute__((noinline))
void target_mixed_clauses(int* arr, int size, int offset) {
    /* No simd clause - let compiler decide */
    #pragma omp target teams distribute parallel for \
        map(tofrom: arr[0:size]) collapse(1) \
        shared(offset) default(none)
    for (int i = 0; i < size; ++i) {
        arr[i] = (arr[i] + offset) * 2;
    }
}

int main(int argc, char* argv[]) {
    float* host_arr = new float[N];
    float* host_data = new float[N];
    float* host_b = new float[N];
    int* host_int_arr = new int[N];
    
    /* Initialize with test patterns */
    for (int i = 0; i < N; ++i) {
        host_arr[i] = static_cast<float>(i);
        host_data[i] = static_cast<float>(i * 10);
        host_b[i] = 0.0f;
        host_int_arr[i] = i % 100;
    }
    
    /* Use command-line arguments to vary execution paths */
    int test_mode = 1;
    if (argc > 1) {
        test_mode = atoi(argv[1]);
    }
    
    /* Execute target regions multiple times with different parameters */
    for (int iter = 0; iter < 3; ++iter) {
        switch (test_mode) {
            case 1:
                /* Basic SIMD clause with teams distribute parallel for simd */
                target_simt_vector_scale(host_arr, N, 2.5f);
                break;
            case 2:
                /* Conditional update without explicit simd clause */
                target_simt_conditional_update(host_data, N, THRESHOLD);
                break;
            case 3:
                /* Nested if-else with simd clause */
                target_simt_nested_if(host_arr, host_b, N);
                break;
            default:
                /* Mixed clauses, no simd */
                target_mixed_clauses(host_int_arr, N, iter * 10);
                break;
        }
        
        /* Alternate between modes if multiple iterations */
        test_mode = (test_mode % 4) + 1;
    }
    
    /* Verify computation to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < N; ++i) {
        checksum += host_arr[i] + host_data[i] + host_b[i] + host_int_arr[i];
    }
    
    std::cout << "Checksum: " << checksum << std::endl;
    
    /* Cleanup */
    delete[] host_arr;
    delete[] host_data;
    delete[] host_b;
    delete[] host_int_arr;
    
    return 0;
}
