```cpp
// Generated to trigger OpenMP depend clause pretty-printing coverage
// Compile with: g++ -O2 -fopenmp -std=c++11 -fdump-tree-all -fdump-rtl-all -c this_file.cc

#include <iostream>
#include <vector>
#include <algorithm>

// Prevent optimization
volatile int sink = 0;
#define USE(x) do { sink += (x); } while(0)

// Test functions marked to prevent inlining/optimization
__attribute__((noinline, noipa))
void test_depend_basic() {
    int var_in = 1, var_inout = 2, var_out = 3;
    int arr[100];
    
    #pragma omp parallel
    #pragma omp single
    {
        // Basic depend clauses - each in separate task
        #pragma omp task depend(in: var_in)
        {
            arr[0] = var_in;
            USE(arr[0]);
        }
        
        #pragma omp task depend(inout: var_inout)
        {
            var_inout *= 2;
            USE(var_inout);
        }
        
        #pragma omp task depend(out: var_out)
        {
            var_out = 42;
            USE(var_out);
        }
        
        #pragma omp taskwait
    }
}

__attribute__((noinline, noipa))
void test_depend_sets() {
    volatile int lock_var = 0;
    int arr[100];
    
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp taskgroup
        {
            // mutexinoutset on scalar
            #pragma omp task depend(mutexinoutset: lock_var)
            {
                lock_var = 1;
                USE(lock_var);
            }
            
            // inoutset on array section
            #pragma omp task depend(inoutset: arr[0:50])
            {
                for (int i = 0; i < 50; ++i) arr[i] = i;
                USE(arr[25]);
            }
            
            #pragma omp task depend(inoutset: arr[0:50])
            {
                for (int i = 0; i < 50; ++i) arr[i] *= 2;
                USE(arr[25]);
            }
        }
    }
}

__attribute__((noinline, noipa))
void test_destroy() {
    volatile int resource = 100;
    
    #pragma omp parallel
    #pragma omp single
    {
        // Create resource
        #pragma omp task depend(out: resource)
        {
            resource = 200;
            USE(resource);
        }
        
        // Destroy resource
        #pragma omp task depend(destroy: resource)
        {
            resource = 0;
            USE(resource);
        }
        
        #pragma omp taskwait
    }
}

__attribute__((noinline, noipa))
void test_array_sections() {
    int arr[200];
    
    #pragma omp parallel
    #pragma omp single
    {
        // Array section with in dependency
        #pragma omp task depend(in: arr[0:100])
        {
            for (int i = 0; i < 100; ++i) USE(arr[i]);
        }
        
        // Array section with out dependency
        #pragma omp task depend(out: arr[100:100])
        {
            for (int i = 100; i < 200; ++i) arr[i] = i;
            USE(arr[150]);
        }
        
        #pragma omp taskwait
    }
}

__attribute__((noinline, noipa))
void test_iterators() {
    std::vector<int> vec(100);
    auto begin = vec.begin();
    auto end = vec.end();
    
    #pragma omp parallel
    #pragma omp single
    {
        // Iterator expressions in depend clauses
        #pragma omp task depend(inout: *(begin))
        {
            *begin = 42;
            USE(*begin);
        }
        
        #pragma omp task depend(out: *(end - 1))
        {
            *(end - 1) = 99;
            USE(*(end - 1));
        }
        
        #pragma omp taskwait
    }
}

__attribute__((noinline, noipa))
void test_nested_complex() {
    int input[1000];
    int output[1000];
    
    // Combined directive with depend clause
    #pragma omp target teams distribute parallel for depend(in: input)
    for (int i = 0; i < 1000; ++i) {
        output[i] = input[i] * 2;
        USE(output[i]);
    }
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            // Task with multiple clauses including depend
            #pragma omp task depend(inout: input[0]) if(true) final(false) mergeable priority(1)
            {
                input[0] += 10;
                USE(input[0]);
            }
            
            // Nested tasks with dependencies
            #pragma omp task depend(out: output[0])
            {
                output[0] = 0;
                #pragma omp task depend(inout: output[0])
                {
                    output[0] += 5;
                    USE(output[0]);
                }
            }
            
            #pragma omp taskwait
        }
    }
}

// Template function with depend clause
template<typename T>
__attribute__((noinline, noipa))
void test_template_depend(T& val) {
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(inout: val)
        {
            val = val + T(1);
            USE(val);
        }
        
        #pragma omp taskwait
    }
}

// Lambda with depend clause
__attribute__((noinline, noipa))
void test_lambda_depend() {
    int captured_var = 0;
    
    #pragma omp parallel
    {
        auto lambda = [&captured_var]() {
            #pragma omp task depend(out: captured_var)
            {
                captured_var = 100;
                USE(captured_var);
            }
            
            #pragma omp taskwait
        };
        
        #pragma omp single
        lambda();
    }
}

int main() {
    // Initialize data
    int arr[100];
    volatile int lock_var = 0;
    for (int i = 0; i < 100; ++i) arr[i] = i;
    
    // Run all tests
    test_depend_basic();
    test_depend_sets();
    test_destroy();
    test_array_sections();
    test_iterators();
    test_nested_complex();
    
    // Template instantiations
    int int_val = 5;
    float float_val = 3.14f;
    test_template_depend(int_val);
    test_template_depend(float_val);
    
    test_lambda_depend();
    
    // Compute checksum to ensure all code is live
    int checksum = 0;
    #pragma omp parallel for reduction(+:checksum)
    for (int i = 0; i < 100; ++i) {
        checksum += arr[i];
    }
    
    checksum += lock_var + int_val + static_cast<int>(float_val);
    std::cout << "Checksum: " << checksum << std::endl;
    
    return 0;
}
```

This generated program:

1. **Covers all five update types and destroy**: 
   - `depend(in: var_in)` in `test_depend_basic()`
   - `depend(inout: var_inout)` in `test_depend_basic()`
   - `depend(out: var_out)` in `test_depend_basic()`
   - `depend(mutexinoutset: lock_var)` in `test_depend_sets()`
   - `depend(inoutset: arr[0:50])` in `test_depend_sets()`
   - `depend(destroy: resource)` in `test_destroy()`

2. **Uses array sections and iterators**:
   - Array sections in `test_array_sections()` and `test_depend_sets()`
   - Iterator expressions in `test_iterators()`

3. **Nests depend clauses in complex constructs**:
   - Combined `target teams distribute parallel for` in `test_nested_complex()`
   - Nested tasks in `test_nested_complex()`
   - Combined with `if`, `final`, `mergeable`, `priority` clauses

4. **Uses templates and lambdas**:
   - Template function `test_template_depend<T>` instantiated for `int` and `float`
   - Lambda capturing variable in `test_lambda_depend()`

5. **Prevents optimization**:
   - `volatile` variables
   - `USE()` macro to force usage
   - `__attribute__((noinline, noipa))` on test functions
   - Final checksum computation and output

Compile with the recommended flags to trigger the pretty-printer on all the OpenMP depend clause variations.
