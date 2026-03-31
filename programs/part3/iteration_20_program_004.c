/* Test program to cover SIMT transformation in omp-low.cc
 * Specifically targets lines 2941-2975 in omp-low.cc.gcov
 * Compile with: g++ -O2 -fopenmp -foffload=nvptx-none -fno-inline -fdump-tree-omplower -o simt_test simt_test.cpp
 */

#include <cstdlib>
#include <iostream>
#include <cmath>

#define N 1024
#define THRESHOLD 500.0f

__attribute__((noinline))
void target_simt_vector_add(float* a, float* b, float* c, int size) {
    #pragma omp target teams distribute parallel for simd \
        map(to: a[0:size], b[0:size]) map(from: c[0:size]) \
        num_teams(4) thread_limit(128)
    for (int i = 0; i < size; ++i) {
        c[i] = a[i] + b[i];
    }
}

__attribute__((noinline))
void target_simt_conditional(float* data, int size, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) \
        num_teams(8) thread_limit(256)
    for (int i = 0; i < size; ++i) {
        // Complex enough control flow to create interesting GIMPLE
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]) * 2.0f;
        } else {
            data[i] = data[i] * data[i] / 3.14f;
        }
    }
}

__attribute__((noinline))
void target_simt_nested_control(float* in, float* out, int size) {
    #pragma omp target teams distribute parallel for simd \
        map(to: in[0:size]) map(from: out[0:size]) \
        num_teams(2) thread_limit(64)
    for (int i = 0; i < size; ++i) {
        // Multiple nested conditions to create complex GIMPLE
        float val = in[i];
        if (val < 0.0f) {
            out[i] = -val;
        } else if (val < 100.0f) {
            out[i] = val * 0.5f;
        } else if (val < 200.0f) {
            out[i] = val + 10.0f;
        } else {
            out[i] = val / 2.0f;
        }
    }
}

__attribute__((noinline))
void target_mixed_clauses(float* a, float* b, float* c, int size) {
    // Using collapse to create more complex loop structure
    #pragma omp target teams distribute parallel for collapse(2) \
        map(to: a[0:size*2], b[0:size*2]) map(from: c[0:size*2]) \
        num_teams(4) thread_limit(128)
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < 2; ++j) {
            int idx = i * 2 + j;
            c[idx] = a[idx] * b[idx] - (a[idx] + b[idx]);
        }
    }
}

int main(int argc, char** argv) {
    // Allocate and initialize test data
    float* a = new float[N];
    float* b = new float[N];
    float* c = new float[N];
    float* d = new float[N];
    float* e = new float[N * 2];
    float* f = new float[N * 2];
    float* g = new float[N * 2];
    
    for (int i = 0; i < N; ++i) {
        a[i] = static_cast<float>(i);
        b[i] = static_cast<float>(i * 2);
        c[i] = 0.0f;
        d[i] = static_cast<float>(i * 3);
    }
    
    for (int i = 0; i < N * 2; ++i) {
        e[i] = static_cast<float>(i % 100);
        f[i] = static_cast<float>((i + 1) % 100);
        g[i] = 0.0f;
    }
    
    // Use command-line arguments to select different test paths
    int test_mode = 1;
    if (argc > 1) {
        test_mode = atoi(argv[1]);
    }
    
    // Execute target regions multiple times with different configurations
    for (int iteration = 0; iteration < 3; ++iteration) {
        std::cout << "Iteration " << iteration << ", mode " << test_mode << std::endl;
        
        switch (test_mode) {
            case 1:
                // Basic SIMD clause - most likely to trigger SIMT transformation
                target_simt_vector_add(a, b, c, N);
                break;
                
            case 2:
                // Conditional with SIMT transformation
                target_simt_conditional(d, N, THRESHOLD);
                break;
                
            case 3:
                // Nested control flow with explicit simd clause
                target_simt_nested_control(a, c, N);
                break;
                
            case 4:
                // Collapsed loops for complex GIMPLE
                target_mixed_clauses(e, f, g, N);
                break;
                
            default:
                // Run all tests
                target_simt_vector_add(a, b, c, N);
                target_simt_conditional(d, N, THRESHOLD);
                target_simt_nested_control(a, c, N);
                target_mixed_clauses(e, f, g, N);
                break;
        }
        
        // Verify results to prevent dead code elimination
        float sum = 0.0f;
        #pragma omp parallel for reduction(+:sum)
        for (int i = 0; i < N; ++i) {
            sum += c[i] + d[i];
        }
        
        std::cout << "Verification sum: " << sum << std::endl;
        
        // Change test mode for next iteration
        test_mode = (test_mode % 4) + 1;
    }
    
    // Cleanup
    delete[] a;
    delete[] b;
    delete[] c;
    delete[] d;
    delete[] e;
    delete[] f;
    delete[] g;
    
    return 0;
}
