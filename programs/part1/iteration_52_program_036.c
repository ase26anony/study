```cpp
// Test program to cover OpenMP depend clause modifiers in GCC C++ pretty-printer
// Compile with: g++ -O1 -fopenmp -fdump-tree-omp -c -xc++ test_openmp_depend.cpp
// Or: g++ -O2 -fopenmp -Werror=openmp-format -c -xc++ test_openmp_depend.cpp
// Or: g++ -O0 -g -fopenmp -fdump-tree-all -save-temps -xc++ test_openmp_depend.cpp

#include <iostream>
#include <omp.h>

// Global variables with different linkages
int global_var = 0;
static int static_global_var = 0;
int global_array[10];
int *global_ptr = global_array;

// Class to test member function scoping
class TestClass {
public:
    int member_var;
    static int static_member;
    
    void member_function() {
        int local_var = 0;
        static int static_local = 0;
        int *heap_ptr = new int[5];
        
        #pragma omp parallel
        {
            #pragma omp single
            {
                // Task with depend(in) - reads initialization
                #pragma omp task depend(in: global_var) depend(in: static_local) \
                                 priority(1) final(false)
                {
                    local_var = global_var + static_local;
                }
                
                // Task with depend(inout) - updates accumulator
                #pragma omp task depend(inout: member_var) depend(in: local_var) \
                                 mergeable
                {
                    member_var += local_var;
                }
                
                // Task with depend(out) - produces result
                #pragma omp task depend(out: heap_ptr[0]) depend(in: member_var)
                {
                    heap_ptr[0] = member_var * 2;
                }
                
                // Task with depend(mutexinoutset) - set-based dependency
                #pragma omp task depend(mutexinoutset: global_array[1]) \
                                 depend(in: heap_ptr[0])
                {
                    global_array[1] = heap_ptr[0] + 1;
                }
                
                // Task with depend(inoutset) - another set-based dependency
                #pragma omp task depend(inoutset: global_array[2]) \
                                 depend(in: global_array[1])
                {
                    global_array[2] = global_array[1] * 2;
                }
                
                // Task with depend(destroy) - cleanup
                #pragma omp task depend(destroy: heap_ptr) \
                                 depend(inout: global_array[2])
                {
                    delete[] heap_ptr;
                    global_array[2] = 0;
                }
                
                #pragma omp taskwait
            }
        }
    }
};

int TestClass::static_member = 0;

// Function with complex data environment
void complex_dependencies() {
    int local_arr[5] = {1, 2, 3, 4, 5};
    static int static_arr[3];
    int &ref = local_arr[0];
    int *ptr = &local_arr[1];
    
    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            // Mix of depend types with references and pointers
            #pragma omp task depend(in: ref) depend(out: static_arr[0])
            {
                static_arr[0] = ref * 10;
            }
            
            #pragma omp task depend(inout: *ptr) depend(in: static_arr[0])
            {
                *ptr += static_arr[0];
            }
            
            #pragma omp task depend(mutexinoutset: local_arr[2]) \
                             depend(in: *ptr)
            {
                local_arr[2] = *ptr - 5;
            }
            
            #pragma omp task depend(inoutset: local_arr[3]) \
                             depend(in: local_arr[2])
            {
                local_arr[3] = local_arr[2] * 3;
            }
            
            #pragma omp task depend(destroy: ptr) \
                             depend(inout: local_arr[4])
            {
                // Simulate cleanup
                local_arr[4] = 0;
            }
            
            #pragma omp taskgroup
            {
                #pragma omp task depend(in: local_arr[3])
                {
                    global_var = local_arr[3];
                }
            }
        }
    }
}

// Function to force pretty-printer invocation via syntax error in declaration
// This will cause the compiler to emit diagnostics and potentially dump the OpenMP constructs
void trigger_pretty_print() {
    // Valid OpenMP code that should be pretty-printed
    int x = 0;
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(in: x) priority(2)
            { x = 1; }
            
            #pragma omp task depend(inout: x) depend(out: global_var)
            { global_var = x + 1; }
            
            #pragma omp task depend(mutexinoutset: static_global_var)
            { static_global_var = 2; }
            
            #pragma omp task depend(inoutset: TestClass::static_member)
            { TestClass::static_member = 3; }
            
            #pragma omp task depend(destroy: global_ptr)
            { /* cleanup simulation */ }
        }
    }
    
    // Syntax error to force diagnostic output
    // Uncomment to trigger compiler error and pretty-printing
    // UndeclaredType error_var;  // This will cause compilation error
}

int main() {
    TestClass obj;
    obj.member_var = 10;
    
    // Initialize global array
    for (int i = 0; i < 10; i++) {
        global_array[i] = i;
    }
    
    // Execute test patterns
    obj.member_function();
    complex_dependencies();
    trigger_pretty_print();
    
    // Perform actual computation to verify OpenMP works
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 10; i++) {
        sum += global_array[i];
    }
    
    std::cout << "Sum: " << sum << std::endl;
    std::cout << "Global var: " << global_var << std::endl;
    std::cout << "Static global: " << static_global_var << std::endl;
    
    return 0;
}

// Additional function with nested constructs
void nested_constructs() {
    #pragma omp parallel
    {
        #pragma omp master
        {
            #pragma omp taskgroup
            {
                int nested_var = 0;
                
                #pragma omp task depend(in: nested_var) final(nested_var > 0)
                { nested_var = 1; }
                
                #pragma omp task depend(inout: nested_var) \
                                 depend(out: global_array[5])
                {
                    global_array[5] = nested_var * 5;
                }
                
                #pragma omp task depend(mutexinoutset: global_array[6]) \
                                 depend(in: global_array[5])
                {
                    global_array[6] = global_array[5] + 1;
                }
                
                #pragma omp task depend(inoutset: global_array[7]) \
                                 depend(in: global_array[6])
                {
                    global_array[7] = global_array[6] * 2;
                }
                
                #pragma omp task depend(destroy: &nested_var)
                {
                    // Simulate cleanup
                }
            }
        }
    }
}
```
