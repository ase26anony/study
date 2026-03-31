```cpp
// Test program to cover OpenMP depend clause modifiers in GCC's C++ pretty-printer
// Compile with: g++ -O1 -fopenmp -fdump-tree-omp -c -xc++ this_file.cc
// Also try: g++ -O2 -fopenmp -Werror=openmp-format -c -xc++ this_file.cc
// And: g++ -O0 -g -fopenmp -fdump-tree-all -save-temps -xc++ this_file.cc

#include <iostream>
#include <omp.h>

// Global variables with different linkages
int global_var = 0;
static int static_global_var = 0;
int global_array[10];
static int static_array[5];

// Heap allocated pointer
int* heap_ptr = new int[20];

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
                // Test all depend clause modifiers inside class member function
                
                // 1. depend(in) - read dependency
                #pragma omp task depend(in: member_var) depend(in: static_local) \
                                 priority(1) mergeable
                {
                    int temp = member_var + static_local;
                }
                
                // 2. depend(inout) - update dependency
                #pragma omp task depend(inout: member_var) depend(in: local_var) \
                                 priority(2)
                {
                    member_var += local_var;
                }
                
                // 3. depend(out) - write dependency  
                #pragma omp task depend(out: local_var) final(member_var > 100)
                {
                    local_var = 42;
                }
                
                // 4. depend(mutexinoutset) - set-based with mutual exclusion
                #pragma omp task depend(mutexinoutset: local_ptr[0]) \
                                 depend(mutexinoutset: local_ptr[1])
                {
                    local_ptr[0] = local_ptr[1] + 1;
                }
                
                // 5. depend(inoutset) - set-based without mutual exclusion
                #pragma omp task depend(inoutset: local_ptr[2]) \
                                 depend(inoutset: local_ptr[3])
                {
                    local_ptr[2] = local_ptr[3] * 2;
                }
                
                // 6. depend(destroy) - destroy dependency
                #pragma omp task depend(destroy: local_ptr[4])
                {
                    // Cleanup task
                    local_ptr[4] = 0;
                }
            }
        }
        
        delete[] local_ptr;
    }
};

int TestClass::static_member = 0;

// Function with complex data environment
void complex_dependency_test() {
    int local_vars[10] = {0};
    static int static_local_vars[5] = {0};
    int* dynamic_array = new int[15];
    
    // References and pointers
    int& ref_var = local_vars[0];
    int* ptr_var = &local_vars[1];
    
    #pragma omp parallel num_threads(4)
    {
        #pragma omp single nowait
        {
            // Create a task dependency graph
            
            // Initialization task with out dependency
            #pragma omp task depend(out: local_vars[0]) depend(out: static_local_vars[0]) \
                         depend(out: *heap_ptr)
            {
                local_vars[0] = 1;
                static_local_vars[0] = 2;
                heap_ptr[0] = 3;
            }
            
            // Chain of tasks with different dependency types
            #pragma omp task depend(in: local_vars[0]) depend(out: local_vars[1]) \
                         depend(in: heap_ptr[0])
            {
                local_vars[1] = local_vars[0] + heap_ptr[0];
            }
            
            #pragma omp task depend(inout: local_vars[1]) depend(in: static_local_vars[0]) \
                         depend(out: local_vars[2])
            {
                local_vars[1] += static_local_vars[0];
                local_vars[2] = local_vars[1] * 2;
            }
            
            // Array element dependencies with set types
            #pragma omp task depend(mutexinoutset: dynamic_array[0]) \
                         depend(mutexinoutset: dynamic_array[1]) \
                         depend(in: local_vars[2])
            {
                dynamic_array[0] = local_vars[2];
                dynamic_array[1] = local_vars[2] + 1;
            }
            
            #pragma omp task depend(inoutset: dynamic_array[2]) \
                         depend(inoutset: dynamic_array[3]) \
                         depend(in: dynamic_array[0])
            {
                dynamic_array[2] = dynamic_array[0] * 2;
                dynamic_array[3] = dynamic_array[0] * 3;
            }
            
            // Final task with destroy dependency
            #pragma omp task depend(in: local_vars[2]) depend(destroy: dynamic_array[4]) \
                         depend(inout: ref_var) depend(in: *ptr_var)
            {
                dynamic_array[4] = local_vars[2] + ref_var + *ptr_var;
                ref_var = dynamic_array[4];
            }
            
            // Taskwait to ensure dependencies are respected
            #pragma omp taskwait
            
            // Nested taskgroup with additional dependencies
            #pragma omp taskgroup
            {
                #pragma omp task depend(inout: global_var) \
                             depend(inout: static_global_var)
                {
                    global_var++;
                    static_global_var++;
                }
                
                #pragma omp task depend(in: global_var) depend(out: global_array[0])
                {
                    global_array[0] = global_var;
                }
            }
        }
    }
    
    // Verify computation
    int sum = 0;
    for (int i = 0; i < 15; i++) {
        sum += dynamic_array[i];
    }
    
    std::cout << "Sum: " << sum << std::endl;
    
    delete[] dynamic_array;
}

// Main function with comprehensive test
int main() {
    // Initialize global data
    for (int i = 0; i < 10; i++) {
        global_array[i] = i;
    }
    
    for (int i = 0; i < 5; i++) {
        static_array[i] = i * 2;
    }
    
    // Test in class context
    TestClass obj;
    obj.member_var = 10;
    obj.member_function();
    
    // Test with complex dependencies
    complex_dependency_test();
    
    // Additional tests with different variable types
    
    // Test with pointer dereference in depend clause
    int* ptr1 = new int;
    int* ptr2 = new int;
    *ptr1 = 5;
    *ptr2 = 10;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(in: *ptr1) depend(out: *ptr2)
            {
                *ptr2 = *ptr1 * 2;
            }
            
            #pragma omp task depend(inout: *ptr2) depend(destroy: ptr1[0])
            {
                *ptr2 += 1;
            }
        }
    }
    
    delete ptr1;
    delete ptr2;
    
    // Test with array sections (OpenMP 4.5+)
    int arr[100];
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(out: arr[0:50])
            {
                for (int i = 0; i < 50; i++) arr[i] = i;
            }
            
            #pragma omp task depend(in: arr[0:25]) depend(out: arr[50:50])
            {
                for (int i = 50; i < 100; i++) arr[i] = arr[i-50] * 2;
            }
            
            #pragma omp task depend(inout: arr[25:75]) depend(destroy: arr[99])
            {
                for (int i = 25; i < 100; i++) arr[i] += 1;
            }
        }
    }
    
    // Final verification
    int total = 0;
    for (int i = 0; i < 100; i++) {
        total += arr[i];
    }
    std::cout << "Total: " << total << std::endl;
    
    // Cleanup
    delete[] heap_ptr;
    
    return 0;
}

// DELIBERATE SYNTAX ERROR to trigger compiler diagnostics
// This will cause the compiler to potentially pretty-print OpenMP constructs
// while reporting the error
class UndeclaredType;  // Forward declaration

void function_with_error(UndeclaredType* param) {
    // This references an incomplete type, causing a diagnostic
    // The compiler may dump surrounding context including OpenMP clauses
}
```
