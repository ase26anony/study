```cpp
// Test program to cover OpenMP task depend clause modifiers in GCC pretty-printer
// Compile with: g++ -O1 -fopenmp -fdump-tree-omp -c -xc++ this_file.cpp
// Or: g++ -O2 -fopenmp -Werror=openmp-format -c -xc++ this_file.cpp
// Or: g++ -O0 -g -fopenmp -fdump-tree-all -save-temps -xc++ this_file.cpp

#include <iostream>
#include <omp.h>
#include <cstdlib>

// Global variables with different linkages
int global_var = 0;
static int static_global_var = 0;
extern int extern_var; // Will remain undefined to potentially trigger diagnostics

// Class to test member function contexts
class TaskContainer {
public:
    int member_var;
    static int static_member;
    
    void member_function() {
        int local_var = 0;
        static int static_local = 0;
        int* heap_ptr = new int[5];
        
        #pragma omp parallel
        {
            #pragma omp single
            {
                // Test all depend clause modifiers in member function context
                
                // 1. depend(in) - read dependency
                #pragma omp task depend(in: local_var) priority(1)
                {
                    int read = local_var;
                }
                
                // 2. depend(inout) - update dependency
                #pragma omp task depend(inout: member_var) mergeable
                {
                    member_var += 1;
                }
                
                // 3. depend(out) - write dependency
                #pragma omp task depend(out: heap_ptr[0]) final(0)
                {
                    heap_ptr[0] = 42;
                }
                
                // 4. depend(mutexinoutset) - set-based with mutual exclusion
                #pragma omp task depend(mutexinoutset: heap_ptr[1])
                {
                    heap_ptr[1] = omp_get_thread_num();
                }
                
                // 5. depend(inoutset) - set-based without mutual exclusion
                #pragma omp task depend(inoutset: heap_ptr[2])
                {
                    heap_ptr[2] += 1;
                }
                
                // 6. depend(destroy) - destroy dependency
                #pragma omp task depend(destroy: static_local)
                {
                    // Cleanup operation
                }
            }
        }
        
        delete[] heap_ptr;
    }
};

int TaskContainer::static_member = 0;

// Global function with complex data environment
void process_tasks() {
    int array[10] = {0};
    int* dyn_array = new int[10];
    static int static_array[5] = {0};
    int& ref = global_var;
    int* ptr = &global_var;
    
    #pragma omp parallel num_threads(4)
    {
        #pragma omp single nowait
        {
            // Create a task dependency graph
            
            // Initialization task with depend(out)
            #pragma omp task depend(out: array[0]) depend(out: dyn_array[0])
            {
                array[0] = 1;
                dyn_array[0] = 100;
            }
            
            // Chain of tasks with different depend types
            
            // Task with depend(in) reading from initialization
            #pragma omp task depend(in: array[0]) depend(out: array[1])
            {
                array[1] = array[0] + 1;
            }
            
            // Task with depend(inout) updating shared state
            #pragma omp task depend(inout: global_var) priority(2)
            {
                global_var += array[1];
            }
            
            // Task with depend(mutexinoutset) on array element
            #pragma omp task depend(mutexinoutset: array[2])
            {
                array[2] = omp_get_thread_num() * 10;
            }
            
            // Another task with depend(mutexinoutset) on same element
            #pragma omp task depend(mutexinoutset: array[2])
            {
                array[2] += 5;
            }
            
            // Task with depend(inoutset) on static array
            #pragma omp task depend(inoutset: static_array[0])
            {
                static_array[0] += 2;
            }
            
            // Another task with depend(inoutset) on same element
            #pragma omp task depend(inoutset: static_array[0])
            {
                static_array[0] *= 3;
            }
            
            // Task with pointer dereference in depend clause
            #pragma omp task depend(in: *ptr) depend(out: array[3])
            {
                array[3] = *ptr;
            }
            
            // Task with depend(destroy) for cleanup
            #pragma omp task depend(destroy: dyn_array[0])
            {
                // Mark dyn_array[0] as no longer needed
            }
            
            // Final task depending on multiple dependencies
            #pragma omp task depend(in: array[1], array[3]) \
                             depend(inout: static_global_var) \
                             depend(inoutset: static_array[1])
            {
                static_global_var = array[1] + array[3];
                static_array[1] = static_global_var;
            }
            
            #pragma omp taskwait
        }
    }
    
    // Use taskgroup for structured task cleanup
    #pragma omp taskgroup
    {
        #pragma omp task depend(inoutset: array[4])
        {
            array[4] = 99;
        }
        
        #pragma omp task depend(destroy: array[5])
        {
            array[5] = 0;
        }
    }
    
    delete[] dyn_array;
}

// Function with syntax error to trigger diagnostic output
// This will cause the compiler to potentially pretty-print OpenMP constructs
// during error reporting
void problematic_function() {
    // Deliberate syntax error: undeclared type
    UndeclaredType x;  // This line will cause a compilation error
    
    // OpenMP construct that might get pretty-printed in diagnostics
    #pragma omp parallel
    {
        #pragma omp task depend(in: global_var) depend(out: static_global_var)
        {
            static_global_var = global_var;
        }
        
        #pragma omp task depend(mutexinoutset: global_var) 
        depend(inoutset: static_global_var)
        {
            global_var++;
            static_global_var--;
        }
    }
}

int main() {
    // Initialize
    global_var = 10;
    static_global_var = 20;
    
    // Test in class member context
    TaskContainer container;
    container.member_var = 5;
    container.member_function();
    
    // Test in global function context
    process_tasks();
    
    // Verify results
    std::cout << "Global var: " << global_var << std::endl;
    std::cout << "Static global var: " << static_global_var << std::endl;
    
    // The following call is commented out because it contains a syntax error
    // Uncomment to force diagnostic output during compilation
    // problematic_function();
    
    return 0;
}

// Additional test cases in different scopes
namespace TestNamespace {
    int namespace_var = 0;
    
    void namespace_function() {
        #pragma omp task depend(inout: namespace_var) depend(destroy: global_var)
        {
            namespace_var += global_var;
        }
        
        #pragma omp task depend(mutexinoutset: namespace_var)
        {
            namespace_var *= 2;
        }
    }
}

// Template function to test with generic contexts
template<typename T>
void template_function(T& param) {
    static T static_template_var = T();
    
    #pragma omp task depend(in: param) depend(out: static_template_var)
    {
        static_template_var = param;
    }
    
    #pragma omp task depend(inoutset: static_template_var)
    {
        static_template_var += param;
    }
}

// Explicit instantiation
template void template_function<int>(int&);
```
