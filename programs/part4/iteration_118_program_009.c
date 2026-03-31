```cpp
// cxx-pretty-print-test.cc
// Compile with: g++ -O2 -fopenmp -std=c++11 -fdump-tree-all -fdump-rtl-all -c cxx-pretty-print-test.cc

#include <iostream>
#include <vector>
#include <algorithm>

// Prevent optimization and ensure code generation
#define NOINLINE __attribute__((noinline,noipa))

// Global volatile to prevent dead code elimination
volatile int global_lock = 0;
volatile int global_sync = 0;

// Test basic depend clause types
NOINLINE void test_depend_basic(int* arr, int N) {
    #pragma omp parallel
    #pragma omp single
    {
        // depend(in: var)
        #pragma omp task depend(in: arr[0]) priority(1)
        {
            int temp = arr[0] + 1;
            arr[0] = temp;
        }
        
        // depend(inout: var)
        #pragma omp task depend(inout: arr[1]) final(global_sync > 100)
        {
            arr[1] *= 2;
        }
        
        // depend(out: var)
        #pragma omp task depend(out: arr[2]) mergeable
        {
            arr[2] = 42;
        }
        
        #pragma omp taskwait
    }
}

// Test mutexinoutset and inoutset depend types
NOINLINE void test_depend_sets(int* arr, int N) {
    volatile int lock_var = 0;
    
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp taskgroup
        {
            // depend(mutexinoutset: lock_var)
            #pragma omp task depend(mutexinoutset: lock_var) if(N > 0)
            {
                lock_var = 1;
                arr[10] = 100;
            }
            
            // depend(inoutset: arr[0:50])
            #pragma omp task depend(inoutset: arr[0:50]) priority(2)
            {
                for (int i = 0; i < 50; ++i) {
                    arr[i] += i;
                }
            }
            
            // Another task with mutexinoutset
            #pragma omp task depend(mutexinoutset: lock_var)
            {
                arr[20] = lock_var * 10;
            }
        }
    }
}

// Test destroy depend type
NOINLINE void test_destroy() {
    volatile int resource = 1;
    
    #pragma omp parallel
    #pragma omp single
    {
        // Create resource
        #pragma omp task depend(out: resource)
        {
            resource = 100;
        }
        
        // Use resource
        #pragma omp task depend(in: resource)
        {
            int temp = resource;
            global_sync = temp;
        }
        
        // Destroy resource
        #pragma omp task depend(destroy: resource)
        {
            resource = 0;
        }
        
        #pragma omp taskwait
    }
}

// Test with array sections and complex expressions
NOINLINE void test_array_sections(int* arr, int N, int M) {
    #pragma omp parallel
    #pragma omp single
    {
        // depend(in: arr[0:N]) with array section
        #pragma omp task depend(in: arr[0:N]) priority(3)
        {
            for (int i = 0; i < N; ++i) {
                arr[i] = arr[i] * 2;
            }
        }
        
        // depend(out: arr[N:M]) with array section
        #pragma omp task depend(out: arr[N:M])
        {
            for (int i = N; i < M; ++i) {
                arr[i] = i * 3;
            }
        }
        
        #pragma omp taskwait
    }
}

// Test with iterators
NOINLINE void test_iterator_depend(std::vector<int>& vec) {
    auto begin = vec.begin();
    auto end = vec.end();
    
    #pragma omp parallel
    #pragma omp single
    {
        // depend(inout: *(begin))
        #pragma omp task depend(inout: *begin) if(vec.size() > 0)
        {
            *begin += 10;
        }
        
        // depend(out: *(end-1))
        #pragma omp task depend(out: *(end-1))
        {
            *(end-1) = 999;
        }
        
        #pragma omp taskwait
    }
}

// Template function with depend clause
template<typename T>
NOINLINE void test_template_depend(T& val) {
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(inout: val) mergeable
        {
            val = val + T(1);
        }
        
        #pragma omp taskwait
    }
}

// Lambda with depend clause
NOINLINE void test_lambda_depend(int& result) {
    int local_var = 0;
    
    #pragma omp parallel
    #pragma omp single
    {
        auto lambda = [&local_var]() {
            #pragma omp task depend(out: local_var)
            {
                local_var = 42;
            }
            
            #pragma omp task depend(in: local_var)
            {
                result = local_var * 2;
            }
            
            #pragma omp taskwait
        };
        
        lambda();
    }
}

// Complex nested OpenMP construct with depend
NOINLINE void test_nested_depend(int* input, int* output, int N) {
    // Combined directive with depend clause
    #pragma omp target teams distribute parallel for \
        depend(in: input[0:N]) map(to: input[0:N]) map(from: output[0:N])
    for (int i = 0; i < N; ++i) {
        output[i] = input[i] * input[i];
    }
    
    // Nested tasks inside parallel region
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(in: output[0]) priority(1)
            {
                output[0] += 1;
            }
            
            #pragma omp task depend(inout: output[N/2])
            {
                output[N/2] *= 2;
            }
            
            #pragma omp task depend(out: output[N-1])
            {
                output[N-1] = -1;
            }
            
            #pragma omp taskwait
        }
    }
}

// Main test driver
int main() {
    const int N = 100;
    const int M = 200;
    int arr[M] = {0};
    std::vector<int> vec(N, 0);
    int result = 0;
    
    // Initialize arrays
    for (int i = 0; i < M; ++i) {
        arr[i] = i;
    }
    std::fill(vec.begin(), vec.end(), 1);
    
    // Test all depend clause types
    test_depend_basic(arr, N);
    
    test_depend_sets(arr, N);
    
    test_destroy();
    
    test_array_sections(arr, 10, 50);
    
    test_iterator_depend(vec);
    
    // Test template instantiations
    int int_val = 5;
    test_template_depend(int_val);
    
    float float_val = 3.14f;
    test_template_depend(float_val);
    
    test_lambda_depend(result);
    
    // Test nested complex construct
    int input[N], output[N];
    for (int i = 0; i < N; ++i) {
        input[i] = i;
        output[i] = 0;
    }
    test_nested_depend(input, output, N);
    
    // Compute checksum to ensure all code paths are live
    long long checksum = 0;
    for (int i = 0; i < M; ++i) {
        checksum += arr[i];
    }
    for (int val : vec) {
        checksum += val;
    }
    checksum += int_val + static_cast<int>(float_val) + result;
    for (int i = 0; i < N; ++i) {
        checksum += output[i];
    }
    
    std::cout << "Checksum: " << checksum << std::endl;
    
    return 0;
}
```

This test program specifically targets the uncovered lines in `cxx-pretty-print.cc` by:

1. **Using all five update types and destroy**: 
   - `depend(in: arr[0])` (line 2154)
   - `depend(inout: arr[1])` (line 2157)
   - `depend(out: arr[2])` (line 2160)
   - `depend(mutexinoutset: lock_var)` (line 2163)
   - `depend(inoutset: arr[0:50])` (line 2166)
   - `depend(destroy: resource)` (line 2169)

2. **Combining with array sections and iterators**:
   - Array sections: `arr[0:N]`, `arr[N:M]`, `arr[0:50]`
   - Iterators: `*begin`, `*(end-1)`

3. **Nesting in complex constructs**:
   - Combined `#pragma omp target teams distribute parallel for depend(in: input[0:N])`
   - Nested tasks inside parallel regions
   - Combined with `if`, `final`, `mergeable`, `priority` clauses

4. **Using templates and lambdas**:
   - Template function `test_template_depend<T>` instantiated for `int` and `float`
   - Lambda capturing variables with `depend(out: local_var)`

5. **Ensuring code isn't optimized away**:
   - `volatile` variables
   - Computations and assignments
   - Final checksum computation and output

Compile with the recommended flags to trigger the pretty-printer on all the OpenMP depend clause variations.
