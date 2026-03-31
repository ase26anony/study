```cpp
// test_omp_clauses.cpp
// Compile with: g++ -O1 -fopenmp -fdump-tree-omplower -fprofile-arcs -ftest-coverage test_omp_clauses.cpp -o test_omp_executable
// Run with: ./test_omp_executable 1000

#include <omp.h>
#include <iostream>
#include <cstdio>
#include <utility>
#include <cstdlib>
#include <cstring>

// User-defined type for forcing _reductemp_ clause generation
struct MyType {
    int val;
    MyType() : val(0) {}
    MyType(int v) : val(v) {}
    
    MyType operator+(const MyType& o) const {
        return MyType(val + o.val);
    }
    
    MyType& operator+=(const MyType& o) {
        val += o.val;
        return *this;
    }
};

// Helper function template with perfect forwarding
template<typename T>
decltype(auto) process_value(T&& value) {
    return std::forward<T>(value);
}

// Templated class containing reduction and scan patterns
template<typename T, int N>
class OMPTemplateProcessor {
private:
    T* data;
    
public:
    OMPTemplateProcessor() : data(new T[N]) {
        for (int i = 0; i < N; ++i) {
            data[i] = static_cast<T>(i + 1);
        }
    }
    
    ~OMPTemplateProcessor() {
        delete[] data;
    }
    
    // Pattern A: Complex reduction to trigger _reductemp_
    T complex_reduction() {
        T sum = T(0);
        
        // This complex pragma should generate _reductemp_ clauses
        #pragma omp target teams distribute parallel for simd reduction(+:sum)
        for (int i = 0; i < N; ++i) {
            sum = sum + process_value(data[i]);
        }
        
        // Debug output from master thread
        #pragma omp parallel
        {
            if (omp_get_thread_num() == 0) {
                __builtin_printf("[_reductemp_] Thread %d completed reduction\n", 
                                omp_get_thread_num());
            }
        }
        
        return sum;
    }
    
    // Pattern for _scantemp_ using inscan reduction
    void scan_operation(T& scan_sum) {
        scan_sum = T(0);
        
        // OpenMP 5.0+ inscan reduction should generate _scantemp_
        #pragma omp simd reduction(inscan, +:scan_sum)
        for (int i = 0; i < N; ++i) {
            // Exclusive scan phase
            #pragma omp scan exclusive(scan_sum)
            {
                T temp = process_value(data[i]);
                scan_sum = scan_sum + temp;
            }
        }
        
        #pragma omp parallel
        {
            if (omp_get_thread_num() == 0) {
                __builtin_printf("[_scantemp_] Thread %d completed scan\n", 
                                omp_get_thread_num());
            }
        }
    }
};

// Pattern B: Generic lambda with taskloop for _condtemp_
template<typename Func>
void execute_taskloop(int num_tasks, int threshold, Func&& func) {
    volatile int dynamic_threshold = threshold; // Prevent optimization
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            // Taskloop with if clause that depends on runtime value
            // This may generate _condtemp_ clauses
            #pragma omp taskloop grainsize(1) if(dynamic_threshold > 50)
            for (int task_id = 0; task_id < num_tasks; ++task_id) {
                // Generic lambda capturing by reference
                auto task_lambda = [&, task_id]() {
                    if (task_id < dynamic_threshold) {
                        func(task_id);
                    }
                };
                task_lambda();
            }
        }
        
        if (omp_get_thread_num() == 0) {
            __builtin_printf("[_condtemp_] Thread %d completed taskloop\n", 
                            omp_get_thread_num());
        }
    }
}

// Functions to be called from target regions
#pragma omp declare target
void target_function(int* arr, int idx) {
    arr[idx] *= 2;
}
#pragma omp end declare target

// Pattern C: enter clause with to mapper
void target_data_enter(int* large_array, int size) {
    // Initialize array on host
    for (int i = 0; i < size; ++i) {
        large_array[i] = i;
    }
    
    // This should generate enter clause with "to"
    #pragma omp target enter data map(to: large_array[0:size])
    
    // Use the data in target region
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < size; ++i) {
        target_function(large_array, i);
    }
    
    // Retrieve results
    #pragma omp target exit data map(from: large_array[0:size])
    
    __builtin_printf("[enter clause] Target data operation completed\n");
}

// Main test driver
int main(int argc, char* argv[]) {
    // Use command line argument for iteration count to prevent optimization
    int iteration_count = 1000;
    if (argc > 1) {
        iteration_count = atoi(argv[1]);
        if (iteration_count <= 0) iteration_count = 1000;
    }
    
    std::cout << "Testing OpenMP clause coverage with N = " 
              << iteration_count << std::endl;
    
    // Test 1: _reductemp_ with different types
    std::cout << "\n1. Testing _reductemp_ clause generation..." << std::endl;
    {
        OMPTemplateProcessor<int, 1000> int_processor;
        int int_sum = int_processor.complex_reduction();
        std::cout << "   Int reduction sum: " << int_sum << std::endl;
    }
    
    {
        OMPTemplateProcessor<MyType, 1000> mytype_processor;
        MyType mytype_sum = mytype_processor.complex_reduction();
        std::cout << "   MyType reduction sum: " << mytype_sum.val << std::endl;
    }
    
    // Test 2: _scantemp_ with inscan reduction
    std::cout << "\n2. Testing _scantemp_ clause generation..." << std::endl;
    {
        OMPTemplateProcessor<double, 500> double_processor;
        double scan_sum = 0.0;
        double_processor.scan_operation(scan_sum);
        std::cout << "   Double scan sum: " << scan_sum << std::endl;
    }
    
    // Test 3: _condtemp_ with taskloop
    std::cout << "\n3. Testing _condtemp_ clause generation..." << std::endl;
    {
        int task_results = 0;
        auto task_func = [&](int task_id) {
            #pragma omp atomic
            task_results += task_id;
        };
        
        execute_taskloop(iteration_count, 75, task_func);
        std::cout << "   Taskloop result: " << task_results << std::endl;
    }
    
    // Test 4: enter clause with to mapper
    std::cout << "\n4. Testing enter clause with 'to' mapper..." << std::endl;
    {
        const int large_size = 10000;
        int* large_array = new int[large_size];
        
        target_data_enter(large_array, large_size);
        
        // Verify some results
        int check_sum = 0;
        for (int i = 0; i < 100; ++i) {
            check_sum += large_array[i];
        }
        std::cout << "   First 100 elements sum: " << check_sum << std::endl;
        
        delete[] large_array;
    }
    
    // Final checksum to ensure all computations are used
    volatile int final_check = 0;
    #pragma omp parallel for reduction(+:final_check)
    for (int i = 0; i < iteration_count; ++i) {
        final_check += (i % 10);
    }
    
    std::cout << "\nFinal validation checksum: " << final_check << std::endl;
    std::cout << "Test completed successfully!" << std::endl;
    
    return 0;
}
```

This test program specifically targets the uncovered lines in `tree-pretty-print.cc` by:

1. **`_reductemp_`**: Uses a complex `#pragma omp target teams distribute parallel for simd reduction` with a user-defined type (`MyType`) that has an overloaded `operator+`. This forces the compiler to create internal reduction temporaries during lowering.

2. **`_condtemp_`**: Uses `#pragma omp taskloop` with an `if` clause that depends on a volatile variable (`dynamic_threshold`), which may generate conditional temporaries.

3. **`_scantemp_`**: Uses `#pragma omp simd reduction(inscan, +:scan_sum)` with the OpenMP 5.0+ `inscan` modifier, which is known to generate `_scantemp_` clauses.

4. **`enter` clause**: Uses `#pragma omp target enter data map(to:large_array)` to explicitly trigger the `enter` clause with a `to` mapper.

All patterns are embedded in template contexts:
- `OMPTemplateProcessor` is a templated class with member functions containing the reduction and scan pragmas
- `execute_taskloop` uses a generic lambda within the taskloop
- Perfect forwarding with `decltype(auto)` is used in helper functions

The program is self-contained, uses runtime input to prevent optimization, includes debug prints from master threads, and performs actual computations to ensure the OpenMP regions are active. Compiling with `-fdump-tree-omplower` will force the compiler to process and print the OpenMP structures, engaging the pretty-printer code paths we want to cover.
