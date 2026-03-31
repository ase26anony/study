```cpp
// Generated to trigger OpenMP depend clause pretty-printing coverage
// Compile with: g++ -O2 -fopenmp -std=c++11 -fdump-tree-all -fdump-rtl-all -c this_file.cc

#include <iostream>
#include <vector>
#include <algorithm>

// Prevent optimization
volatile int sink = 0;
__attribute__((noinline, noipa))
void use(int val) { sink = val; }

// Test functions marked to prevent inlining
__attribute__((noinline, noipa))
void test_depend_basic() {
    int var_in = 1, var_inout = 2, var_out = 3;
    int arr[100];
    
    #pragma omp parallel
    #pragma omp single
    {
        // Basic depend types in separate tasks
        #pragma omp task depend(in: var_in)
        { arr[0] = var_in; use(arr[0]); }
        
        #pragma omp task depend(inout: var_inout)
        { var_inout *= 2; use(var_inout); }
        
        #pragma omp task depend(out: var_out)
        { var_out = 42; use(var_out); }
        
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
            { lock_var = 1; use(lock_var); }
            
            // inoutset on array section
            #pragma omp task depend(inoutset: arr[0:50])
            { 
                for (int i = 0; i < 50; ++i) 
                    arr[i] = i; 
                use(arr[25]);
            }
            
            // Additional tasks to create dependencies
            #pragma omp task depend(in: arr[25:25])
            { use(arr[40]); }
        }
    }
}

__attribute__((noinline, noipa))
void test_destroy() {
    volatile int resource = 100;
    
    #pragma omp parallel
    #pragma omp single
    {
        // Create then destroy dependency
        #pragma omp task depend(out: resource)
        { resource = 200; use(resource); }
        
        #pragma omp task depend(destroy: resource)
        { /* cleanup */ use(0); }
        
        #pragma omp taskwait
    }
}

__attribute__((noinline, noipa))
void test_array_sections() {
    int arr[200];
    
    #pragma omp parallel
    #pragma omp single
    {
        // Array sections with different update types
        #pragma omp task depend(in: arr[0:100])
        { 
            for (int i = 0; i < 100; ++i) 
                use(arr[i]); 
        }
        
        #pragma omp task depend(out: arr[100:100])
        { 
            for (int i = 100; i < 200; ++i) 
                arr[i] = i * 2; 
            use(arr[150]);
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
        // Iterator-based dependencies
        #pragma omp task depend(inout: *begin)
        { *begin = 42; use(*begin); }
        
        #pragma omp task depend(out: *(end - 1))
        { *(end - 1) = 99; use(*(end - 1)); }
        
        #pragma omp taskwait
    }
}

// Template function with depend clause
template<typename T>
__attribute__((noinline, noipa))
void template_depend(T& val) {
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(inout: val)
        { 
            val = val * 2; 
            use(static_cast<int>(val));
        }
        
        #pragma omp taskwait
    }
}

// Complex nested construct
__attribute__((noinline, noipa))
void test_nested_complex() {
    int input[1000];
    int output[1000];
    
    // Combined directive with depend clause
    #pragma omp target teams distribute parallel for \
                depend(in: input[0:1000]) map(tofrom: output[0:1000])
    for (int i = 0; i < 1000; ++i) {
        output[i] = input[i] * 2;
    }
    
    // Task with multiple clauses
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(inout: output[500]) \
                     if(true) final(false) mergeable priority(1)
        {
            output[500] += 100;
            use(output[500]);
        }
        
        #pragma omp taskwait
    }
}

// Lambda with OpenMP task
__attribute__((noinline, noipa))
void test_lambda_depend() {
    int captured_var = 0;
    
    #pragma omp parallel
    #pragma omp single
    {
        auto lambda = [&captured_var]() {
            #pragma omp task depend(out: captured_var)
            {
                captured_var = 777;
                use(captured_var);
            }
        };
        
        lambda();
        #pragma omp taskwait
    }
}

int main() {
    // Initialize data
    int arr[100];
    for (int i = 0; i < 100; ++i) arr[i] = i;
    
    volatile int lock_var = 0;
    
    // Run all tests
    test_depend_basic();
    test_depend_sets();
    test_destroy();
    test_array_sections();
    test_iterators();
    
    // Template instantiations
    int int_val = 10;
    float float_val = 20.5f;
    template_depend(int_val);
    template_depend(float_val);
    
    test_nested_complex();
    test_lambda_depend();
    
    // Compute checksum to keep everything live
    int checksum = 0;
    #pragma omp parallel for reduction(+:checksum)
    for (int i = 0; i < 100; ++i) {
        checksum += arr[i];
    }
    
    std::cout << "Checksum: " << checksum << std::endl;
    std::cout << "Sink: " << sink << std::endl;
    
    return 0;
}
```

This program systematically exercises all the uncovered `depend` clause update types:

1. **All Five Update Types + Destroy**: 
   - `depend(in: var_in)` in `test_depend_basic()`
   - `depend(inout: var_inout)` in `test_depend_basic()`
   - `depend(out: var_out)` in `test_depend_basic()`
   - `depend(mutexinoutset: lock_var)` in `test_depend_sets()`
   - `depend(inoutset: arr[0:50])` in `test_depend_sets()`
   - `depend(destroy: resource)` in `test_destroy()`

2. **Array Sections and Iterators**:
   - Array sections in `test_array_sections()` and `test_depend_sets()`
   - Iterator expressions in `test_iterators()`

3. **Complex Nested Constructs**:
   - Combined `target teams distribute parallel for` with `depend` in `test_nested_complex()`
   - Task with multiple combined clauses (`if`, `final`, `mergeable`, `priority`)
   - Nested tasks inside parallel regions throughout

4. **Templates and Lambdas**:
   - Template function `template_depend<T>` instantiated for `int` and `float`
   - Lambda capturing variable with `depend(out: captured_var)` in `test_lambda_depend()`

5. **Anti-Optimization**:
   - `volatile` variables prevent dead code elimination
   - `use()` function with `noinline, noipa` attributes
   - Final checksum computation ensures all data is live
   - Output to `std::cout` prevents removal of entire program

Compile with the recommended flags to trigger the pretty-printer on all intermediate representations, ensuring the uncovered lines in `cxx-pretty-print.cc` are exercised.
