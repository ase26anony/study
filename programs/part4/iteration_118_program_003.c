```cpp
// Generated to trigger uncovered lines in cxx-pretty-print.cc (lines 2154-2173)
// Compile with: g++ -O2 -fopenmp -std=c++11 -fdump-tree-all -fdump-rtl-all -c this_file.cc

#include <iostream>
#include <vector>
#include <iterator>

// Prevent optimization
volatile int sink = 0;
#define USE(x) do { sink += (x); } while(0)

// Test functions marked to prevent inlining/optimization
__attribute__((noinline, noipa))
void test_depend_basic(int* arr, int N, int M) {
    #pragma omp parallel
    #pragma omp single
    {
        // Basic depend clauses with different update types
        #pragma omp task depend(in: arr[0]) if(N > 0) final(N < 100) priority(1)
        { USE(arr[0]); }
        
        #pragma omp task depend(inout: arr[1]) mergeable
        { arr[1]++; USE(arr[1]); }
        
        #pragma omp task depend(out: arr[2])
        { arr[2] = 42; USE(arr[2]); }
    }
}

__attribute__((noinline, noipa))
void test_depend_sets(int* arr, volatile int& lock_var) {
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp taskgroup
        {
            // mutexinoutset and inoutset in taskgroup context
            #pragma omp task depend(mutexinoutset: lock_var)
            { lock_var = 1; USE(lock_var); }
            
            #pragma omp task depend(inoutset: arr[0:50])
            { for(int i = 0; i < 50; i++) arr[i]++; USE(arr[0]); }
        }
    }
}

__attribute__((noinline, noipa))
void test_destroy(volatile int& lock_var) {
    #pragma omp parallel
    #pragma omp single
    {
        // destroy dependency for cleanup
        #pragma omp task depend(destroy: lock_var)
        { lock_var = 0; USE(lock_var); }
    }
}

__attribute__((noinline, noipa))
void test_array_sections(int* arr, int N, int M) {
    #pragma omp parallel
    #pragma omp single
    {
        // Array sections with depend clauses
        #pragma omp task depend(in: arr[0:N])
        { int sum = 0; for(int i = 0; i < N; i++) sum += arr[i]; USE(sum); }
        
        #pragma omp task depend(out: arr[N:M])
        { for(int i = N; i < M; i++) arr[i] = i; USE(arr[N]); }
    }
}

__attribute__((noinline, noipa))
void test_iterators(std::vector<int>& vec) {
    #pragma omp parallel
    #pragma omp single
    {
        auto begin = vec.begin();
        auto end = vec.end();
        
        // Iterator expressions with depend clauses
        #pragma omp task depend(inout: *(begin))
        { (*begin)++; USE(*begin); }
        
        #pragma omp task depend(out: *(end-1))
        { *(end-1) = 99; USE(*(end-1)); }
    }
}

__attribute__((noinline, noipa))
void test_target_offload(int* input, int* output, int N) {
    // Combined directive with depend clause
    #pragma omp target teams distribute parallel for depend(in: input[0:N]) map(to: input[0:N]) map(from: output[0:N])
    for(int i = 0; i < N; i++) {
        output[i] = input[i] * 2;
    }
    USE(output[0]);
}

// Template function with depend clause
template<typename T>
__attribute__((noinline, noipa))
void test_template_depend(T& val) {
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(inout: val)
        { val += T(1); USE(val); }
    }
}

// Lambda with depend clause
__attribute__((noinline, noipa))
void test_lambda_depend(int& val) {
    #pragma omp parallel
    {
        auto lambda = [&val]() {
            #pragma omp single
            {
                #pragma omp task depend(out: val)
                { val = 100; USE(val); }
            }
        };
        lambda();
    }
}

// Complex nested structure
__attribute__((noinline, noipa))
void test_nested_depend(int* arr, volatile int& lock_var) {
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(in: arr[0])  // Nested task with depend
        {
            #pragma omp task depend(inout: arr[1]) priority(2)
            { arr[1] *= 2; USE(arr[1]); }
            
            #pragma omp task depend(mutexinoutset: lock_var)
            { lock_var++; USE(lock_var); }
        }
    }
}

int main() {
    const int N = 100;
    const int M = 200;
    int arr[M];
    volatile int lock_var = 0;
    std::vector<int> vec(N, 1);
    int input[N], output[N];
    
    // Initialize arrays
    for(int i = 0; i < M; i++) arr[i] = i;
    for(int i = 0; i < N; i++) input[i] = i;
    
    // Call all test functions to trigger pretty-printing
    test_depend_basic(arr, N, M);
    test_depend_sets(arr, lock_var);
    test_destroy(lock_var);
    test_array_sections(arr, 10, 20);
    test_iterators(vec);
    test_target_offload(input, output, N);
    
    // Template instantiations
    int int_val = 5;
    float float_val = 3.14f;
    test_template_depend(int_val);
    test_template_depend(float_val);
    
    test_lambda_depend(int_val);
    test_nested_depend(arr, lock_var);
    
    // Ensure all code is live by computing checksum
    int checksum = 0;
    #pragma omp parallel for reduction(+:checksum)
    for(int i = 0; i < M; i++) {
        checksum += arr[i];
    }
    checksum += vec[0] + vec[N-1];
    checksum += output[0];
    checksum += int_val + (int)float_val;
    checksum += lock_var;
    
    std::cout << "Checksum: " << checksum << std::endl;
    
    return 0;
}
```

This program specifically targets the uncovered lines in `cxx-pretty-print.cc` by:

1. **Using all five update types and destroy**: 
   - `depend(in: arr[0])` (line 2154)
   - `depend(inout: arr[1])` (line 2156)
   - `depend(out: arr[2])` (line 2158)
   - `depend(mutexinoutset: lock_var)` (line 2160)
   - `depend(inoutset: arr[0:50])` (line 2162)
   - `depend(destroy: lock_var)` (line 2164)

2. **Combining with array sections and iterators**:
   - `depend(in: arr[0:N])` and `depend(out: arr[N:M])`
   - `depend(inout: *(begin))` and `depend(out: *(end-1))`

3. **Nesting in complex constructs**:
   - `#pragma omp target teams distribute parallel for depend(in: input[0:N])`
   - Nested tasks with depend clauses
   - Combined with `if`, `final`, `mergeable`, `priority` clauses

4. **Using templates and lambdas**:
   - Template function `test_template_depend<T>` with `depend(inout: val)`
   - Lambda capturing variables with `depend(out: captured_var)`

5. **Ensuring code isn't optimized away**:
   - `volatile` variables
   - `USE()` macro to force usage
   - Final checksum computation
   - `__attribute__((noinline, noipa))` to prevent optimization

Compile with the recommended flags to trigger the pretty-printer during compilation. The `-fdump-tree-all` and `-fdump-rtl-all` options will generate intermediate representations where the uncovered pretty-printing code should be exercised.
