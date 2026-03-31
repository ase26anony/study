/* Test program to cover SIMT transformation in omp-low.cc
 * Specifically targets lines 2941-2975 in omp-low.cc.gcov
 * Compile with: g++ -O2 -fopenmp -foffload=nvptx-none -fno-inline -fdump-tree-omplower -o simt_test simt_test.cpp
 */

#include <iostream>
#include <cstdlib>
#include <cmath>

#define N 1024
#define THRESHOLD 500.0f

// Prevent inlining to keep target regions intact
__attribute__((noinline))
void target_simt_vector_scale(float* arr, int size, float scale) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) num_teams(4) thread_limit(256)
    for (int i = 0; i < size; ++i) {
        // Simple vectorizable operation
        arr[i] = arr[i] * scale + 1.0f;
    }
}

__attribute__((noinline))
void target_simt_conditional(float* data, int size, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) num_teams(8)
    for (int i = 0; i < size; ++i) {
        // Conditional inside loop - creates more complex GIMPLE
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]);
        } else {
            data[i] = data[i] * data[i];
        }
    }
}

__attribute__((noinline))
void target_simt_nested_control(float* a, float* b, float* c, int size) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:size], b[0:size]) map(from: c[0:size]) \
        num_teams(16) thread_limit(128)
    for (int i = 0; i < size; ++i) {
        // Multiple conditions to create complex control flow
        float val = a[i] + b[i];
        if (val < 0.0f) {
            c[i] = -val;
        } else if (val > 100.0f) {
            c[i] = val / 10.0f;
        } else {
            c[i] = val * 2.0f;
        }
    }
}

__attribute__((noinline))
void target_multi_clause(float* x, float* y, int size, int offset) {
    #pragma omp target teams distribute parallel for \
        map(to: x[0:size]) map(from: y[0:size]) \
        private(offset) firstprivate(size)
    for (int i = 0; i < size; ++i) {
        // Use offset to ensure private/firstprivate clauses are used
        y[i] = x[i] + static_cast<float>(offset + i);
    }
}

int main(int argc, char* argv[]) {
    // Initialize test data
    float* array1 = new float[N];
    float* array2 = new float[N];
    float* array3 = new float[N];
    float* array4 = new float[N];
    
    for (int i = 0; i < N; ++i) {
        array1[i] = static_cast<float>(i);
        array2[i] = static_cast<float>(i * 2);
        array3[i] = static_cast<float>(i * 3);
        array4[i] = 0.0f;
    }
    
    // Use command-line arguments to select different paths
    int test_mode = 0;
    if (argc > 1) {
        test_mode = atoi(argv[1]) % 4;
    }
    
    // Call target functions multiple times with different parameters
    // to increase chance of hitting the uncovered transformation
    for (int iter = 0; iter < 3; ++iter) {
        switch ((test_mode + iter) % 4) {
            case 0:
                target_simt_vector_scale(array1, N, 2.5f);
                break;
            case 1:
                target_simt_conditional(array2, N, THRESHOLD);
                break;
            case 2:
                target_simt_nested_control(array1, array2, array3, N);
                break;
            case 3:
                target_multi_clause(array1, array4, N, iter * 100);
                break;
        }
    }
    
    // Verify computation by computing checksum
    float checksum = 0.0f;
    for (int i = 0; i < N; ++i) {
        checksum += array1[i] + array2[i] + array3[i] + array4[i];
    }
    
    std::cout << "Computation checksum: " << checksum << std::endl;
    
    // Additional test with dynamic loop bounds
    int dynamic_size = 512;
    if (argc > 2) {
        dynamic_size = atoi(argv[2]) % 2048;
    }
    
    float* dynamic_arr = new float[dynamic_size];
    for (int i = 0; i < dynamic_size; ++i) {
        dynamic_arr[i] = static_cast<float>(i);
    }
    
    #pragma omp target teams distribute parallel for \
        map(tofrom: dynamic_arr[0:dynamic_size])
    for (int i = 0; i < dynamic_size; ++i) {
        dynamic_arr[i] = sinf(dynamic_arr[i]) * cosf(dynamic_arr[i]);
    }
    
    // Cleanup
    delete[] array1;
    delete[] array2;
    delete[] array3;
    delete[] array4;
    delete[] dynamic_arr;
    
    return 0;
}
