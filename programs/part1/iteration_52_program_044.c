```cpp
// Test program to cover OpenMP depend clause modifiers in GCC C++ pretty-printer
// Compile with: g++ -O1 -fopenmp -fdump-tree-omp -c -xc++ this_file.cc
// Or: g++ -O2 -fopenmp -Werror=openmp-format -c -xc++ this_file.cc
// Or: g++ -O0 -g -fopenmp -fdump-tree-all -save-temps -xc++ this_file.cc

#include <cstdlib>
#include <iostream>

// Global variables with different linkages
int global_var = 0;
static int static_global_var = 0;
extern int extern_var; // Will remain undefined to potentially trigger diagnostics

// Heap-allocated data
int* heap_array = nullptr;

// Class to test member function scoping
class TestClass {
public:
    int member_var;
    static int static_member;
    
    void member_function() {
        int local_var = 0;
        static int static_local = 0;
        int* local_ptr = new int[5];
        
        #pragma omp parallel
        {
            #pragma omp single
            {
                // Task with depend(in) - reads from initialization
                #pragma omp task depend(in: local_var) priority(1)
                {
                    member_var = local_var + 1;
                }
                
                // Task with depend(inout) - updates shared accumulator
                #pragma omp task depend(inout: member_var) mergeable
                {
                    member_var += 10;
                }
                
                // Task with depend(out) - produces final result
                #pragma omp task depend(out: static_local) final(member_var > 0)
                {
                    static_local = member_var * 2;
                }
                
                // Task with depend(mutexinoutset) - set-based dependency
                #pragma omp task depend(mutexinoutset: local_ptr[0])
                {
                    local_ptr[0] = 100;
                }
                
                // Task with depend(inoutset) - another set-based dependency
                #pragma omp task depend(inoutset: local_ptr[1])
                {
                    local_ptr[1] = local_ptr[0] + 50;
                }
                
                // Task with depend(destroy) - cleanup
                #pragma omp task depend(destroy: local_ptr[2])
                {
                    local_ptr[2] = -1;
                }
            }
        }
        
        delete[] local_ptr;
    }
};

int TestClass::static_member = 0;

// Function with complex data environment
void complex_dependencies() {
    int local_array[10] = {0};
    static int static_array[5] = {0};
    int& ref_var = local_array[0];
    int* ptr_var = &local_array[1];
    
    heap_array = new int[20];
    
    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            // Graph of tasks with various depend modifiers
            
            // Initial producer
            #pragma omp task depend(out: local_array[0])
            {
                local_array[0] = 42;
            }
            
            // Consumer with depend(in)
            #pragma omp task depend(in: local_array[0]) depend(out: local_array[1])
            {
                local_array[1] = local_array[0] * 2;
            }
            
            // Task with depend(inout) on reference
            #pragma omp task depend(inout: ref_var)
            {
                ref_var += 10;
            }
            
            // Task with depend(inout) on pointer dereference
            #pragma omp task depend(inout: *ptr_var)
            {
                *ptr_var += 5;
            }
            
            // Multiple tasks with set-based dependencies
            #pragma omp task depend(mutexinoutset: heap_array[0])
            {
                heap_array[0] = 1000;
            }
            
            #pragma omp task depend(inoutset: heap_array[1])
            {
                heap_array[1] = heap_array[0] / 2;
            }
            
            #pragma omp task depend(inoutset: heap_array[2])
            {
                heap_array[2] = heap_array[1] + 200;
            }
            
            // Task with depend(destroy) on heap memory
            #pragma omp task depend(destroy: heap_array[3])
            {
                heap_array[3] = 0;
            }
            
            // Task with depend on static variable
            #pragma omp task depend(inout: static_array[0])
            {
                static_array[0] = 999;
            }
            
            // Nested taskgroup with dependencies
            #pragma omp taskgroup
            {
                #pragma omp task depend(in: local_array[1]) depend(out: local_array[2])
                {
                    local_array[2] = local_array[1] + static_array[0];
                }
                
                #pragma omp task depend(inout: local_array[2])
                {
                    local_array[2] *= 2;
                }
            }
        }
    }
    
    // Verify calculations
    int sum = 0;
    for (int i = 0; i < 10; ++i) {
        sum += local_array[i];
    }
    std::cout << "Sum of local_array: " << sum << std::endl;
    
    delete[] heap_array;
}

// Function with deliberate syntax error to trigger diagnostics
// This will force the compiler to engage its diagnostic machinery
void trigger_diagnostics() {
    // Benign syntax error: undeclared type in unused function signature
    // This should trigger diagnostics without affecting execution
    #ifdef FORCE_ERROR
    UndeclaredType* ptr = nullptr; // This line will cause an error if FORCE_ERROR is defined
    #endif
    
    // Valid OpenMP code that should still be pretty-printed
    int x = 0, y = 0, z = 0;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            // Use all depend modifiers in one parallel region
            #pragma omp task depend(in: x)
            { x = 1; }
            
            #pragma omp task depend(inout: y)
            { y = x + 1; }
            
            #pragma omp task depend(out: z)
            { z = y * 2; }
            
            #pragma omp task depend(mutexinoutset: global_var)
            { global_var = z + 10; }
            
            #pragma omp task depend(inoutset: static_global_var)
            { static_global_var = global_var - 5; }
            
            #pragma omp task depend(destroy: TestClass::static_member)
            { TestClass::static_member = 0; }
        }
    }
}

int main() {
    // Initialize
    TestClass obj;
    obj.member_var = 0;
    
    // Test member function with dependencies
    obj.member_function();
    
    // Test complex dependencies
    complex_dependencies();
    
    // Trigger diagnostics (without actually causing compilation failure)
    trigger_diagnostics();
    
    // Final calculation and output to ensure code runs
    int result = global_var + static_global_var + TestClass::static_member;
    std::cout << "Final result: " << result << std::endl;
    
    return 0;
}

// Additional code to potentially trigger pretty-printer in different contexts
namespace TestNamespace {
    struct Container {
        int data[5];
        
        void process() {
            #pragma omp parallel
            {
                #pragma omp single
                {
                    #pragma omp task depend(in: data[0]) depend(out: data[1])
                    { data[1] = data[0] + 1; }
                    
                    #pragma omp task depend(inout: data[1])
                    { data[1] *= 2; }
                    
                    #pragma omp task depend(mutexinoutset: data[2])
                    { data[2] = 100; }
                    
                    #pragma omp task depend(inoutset: data[3])
                    { data[3] = data[2] + data[1]; }
                    
                    #pragma omp task depend(destroy: data[4])
                    { data[4] = -1; }
                }
            }
        }
    };
}

// Template function to test pretty-printer with templates
template<typename T>
void template_function(T& value) {
    #pragma omp task depend(inout: value)
    {
        value = value + T(1);
    }
}

// Explicit instantiation
template void template_function<int>(int&);
```
