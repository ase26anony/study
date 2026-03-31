```cpp
// Test program to cover OpenMP task depend clause modifiers in GCC's C++ pretty-printer
// Compile with: g++ -O1 -fopenmp -fdump-tree-omp -c -xc++ test_openmp_depend.cpp
// Or: g++ -O2 -fopenmp -Werror=openmp-format -c -xc++ test_openmp_depend.cpp
// Or: g++ -O0 -g -fopenmp -fdump-tree-all -save-temps -xc++ test_openmp_depend.cpp

#include <iostream>
#include <omp.h>
#include <cstdlib>

// Global variables with different linkages
int global_var = 0;
static int static_global_var = 0;
extern int extern_var; // Will be defined later

// Heap-allocated data
int* heap_array = nullptr;
const int ARRAY_SIZE = 10;

// Class to test member function scoping
class TaskContainer {
public:
    int member_var;
    static int static_member;
    
    TaskContainer() : member_var(0) {
        heap_array = new int[ARRAY_SIZE];
        for (int i = 0; i < ARRAY_SIZE; i++) {
            heap_array[i] = i;
        }
    }
    
    ~TaskContainer() {
        delete[] heap_array;
    }
    
    void member_function_with_tasks() {
        int local_var = 0;
        static int static_local = 0;
        
        // Task with depend(in) - reading from initialized data
        #pragma omp task depend(in: heap_array[0]) priority(1)
        {
            local_var = heap_array[0];
        }
        
        // Task with depend(inout) - updating shared accumulator
        #pragma omp task depend(inout: global_var) mergeable
        {
            global_var += 1;
        }
        
        // Task with depend(out) - producing new value
        #pragma omp task depend(out: static_local) final(global_var > 5)
        {
            static_local = 42;
        }
        
        // Task with depend(mutexinoutset) - set-based dependency
        #pragma omp task depend(mutexinoutset: heap_array[1])
        {
            heap_array[1] *= 2;
        }
        
        // Task with depend(inoutset) - another set-based dependency
        #pragma omp task depend(inoutset: heap_array[2])
        {
            heap_array[2] += 10;
        }
        
        // Task with depend(destroy) - cleanup operation
        #pragma omp task depend(destroy: heap_array[3])
        {
            heap_array[3] = -1;
        }
        
        #pragma omp taskwait
    }
    
    void complex_dependency_graph() {
        // Use references and pointers in depend clauses
        int& ref_var = member_var;
        int* ptr_var = &member_var;
        
        // Chain of dependencies using different modifiers
        #pragma omp task depend(in: ref_var)
        {
            // Read operation
        }
        
        #pragma omp task depend(inout: *ptr_var) depend(in: ref_var)
        {
            *ptr_var += ref_var;
        }
        
        #pragma omp task depend(out: heap_array[4]) depend(in: *ptr_var)
        {
            heap_array[4] = *ptr_var * 2;
        }
        
        // Array section with mutexinoutset
        #pragma omp task depend(mutexinoutset: heap_array[5:2])
        {
            heap_array[5] = 99;
            heap_array[6] = 100;
        }
        
        // Multiple dependencies in one clause
        #pragma omp task depend(inoutset: heap_array[7], heap_array[8])
        {
            heap_array[7] += heap_array[8];
        }
        
        #pragma omp task depend(destroy: heap_array[9])
        {
            heap_array[9] = 0;
        }
        
        #pragma omp taskwait
    }
};

int TaskContainer::static_member = 0;
int extern_var = 100; // Definition of extern variable

// Function with deliberate syntax error to trigger diagnostics
// This will force the compiler to engage its diagnostic machinery
void function_with_error() {
    // Deliberate benign syntax error: undeclared type in unused context
    // This should trigger diagnostics without affecting compilation of the rest
    #ifdef FORCE_ERROR
    UndeclaredType* ptr = nullptr; // This line will cause an error when FORCE_ERROR is defined
    #endif
    
    // Valid OpenMP code that should still be pretty-printed
    int local_error_var = 0;
    
    #pragma omp task depend(in: local_error_var) priority(5)
    {
        local_error_var = 1;
    }
    
    #pragma omp task depend(inout: local_error_var)
    {
        local_error_var *= 2;
    }
    
    #pragma omp task depend(out: local_error_var)
    {
        // Final operation
    }
    
    #pragma omp taskwait
}

int main() {
    TaskContainer container;
    
    // Initialize data
    global_var = 5;
    container.member_var = 10;
    container.static_member = 20;
    
    // Enter parallel region
    #pragma omp parallel num_threads(4)
    {
        #pragma omp single
        {
            // Taskgroup for structured task creation
            #pragma omp taskgroup
            {
                // Sequence of tasks with different depend modifiers
                
                // 1. Initialization task with depend(out)
                #pragma omp task depend(out: heap_array[0])
                {
                    heap_array[0] = 1;
                }
                
                // 2. Computation task with depend(in) and depend(inout)
                #pragma omp task depend(in: heap_array[0]) depend(inout: global_var)
                {
                    global_var += heap_array[0];
                }
                
                // 3. Another task with depend(inout) on different variable
                #pragma omp task depend(inout: static_global_var)
                {
                    static_global_var += global_var;
                }
                
                // 4. Task with depend(mutexinoutset) on array element
                #pragma omp task depend(mutexinoutset: heap_array[1])
                {
                    heap_array[1] = static_global_var;
                }
                
                // 5. Task with depend(inoutset) on multiple array elements
                #pragma omp task depend(inoutset: heap_array[2], heap_array[3])
                {
                    heap_array[2] = heap_array[1] * 2;
                    heap_array[3] = heap_array[1] * 3;
                }
                
                // 6. Final task with depend(destroy)
                #pragma omp task depend(destroy: heap_array[4])
                {
                    heap_array[4] = heap_array[2] + heap_array[3];
                }
                
                // Call member function which contains more tasks
                container.member_function_with_tasks();
                
                // Create a more complex dependency graph
                container.complex_dependency_graph();
            } // end taskgroup
            
            // Additional tasks outside taskgroup
            #pragma omp task depend(in: extern_var) priority(10)
            {
                // High priority task
            }
            
            #pragma omp task depend(inout: container.static_member)
            {
                container.static_member++;
            }
        } // end single
    } // end parallel
    
    // Verify computation was performed
    int sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += heap_array[i];
    }
    
    std::cout << "Global var: " << global_var << std::endl;
    std::cout << "Static global var: " << static_global_var << std::endl;
    std::cout << "Array sum: " << sum << std::endl;
    std::cout << "Member var: " << container.member_var << std::endl;
    std::cout << "Static member: " << container.static_member << std::endl;
    
    // Call function with potential error (but don't define FORCE_ERROR)
    function_with_error();
    
    return 0;
}

// Additional namespace to test scope resolution
namespace TestNamespace {
    int namespace_var = 0;
    
    void namespace_function() {
        #pragma omp task depend(in: namespace_var)
        {
            namespace_var = 50;
        }
        
        #pragma omp task depend(inout: namespace_var)
        {
            namespace_var *= 2;
        }
        
        #pragma omp task depend(out: namespace_var)
        {
            // Final operation
        }
        
        #pragma omp taskwait
    }
}
```
