```cpp
// Test program to cover OpenMP depend clause modifiers in GCC C++ pretty-printer
// Compile with: g++ -O1 -fopenmp -fdump-tree-omp -c -xc++ test_openmp_depend.cpp
// Or: g++ -O2 -fopenmp -Werror=openmp-format -c -xc++ test_openmp_depend.cpp
// Or: g++ -O0 -g -fopenmp -fdump-tree-all -save-temps -xc++ test_openmp_depend.cpp

#include <iostream>
#include <cstdlib>
#include <omp.h>

// Global variables with different linkages
int global_var = 0;
static int static_global_var = 0;
int* global_ptr = nullptr;

// Class to test member function scoping
class TaskContainer {
public:
    int member_var;
    static int static_member;
    int* member_ptr;
    
    TaskContainer() : member_var(0), member_ptr(new int[5]) {
        for (int i = 0; i < 5; i++) member_ptr[i] = i;
    }
    
    ~TaskContainer() { delete[] member_ptr; }
    
    void member_function_with_tasks() {
        int local_var = 0;
        static int static_local = 0;
        int* heap_ptr = new int[3];
        
        // Task with depend(in) - reading from initialization
        #pragma omp task depend(in: global_var, static_global_var) priority(1)
        {
            local_var = global_var + static_global_var;
        }
        
        // Task with depend(inout) - updating shared accumulator
        #pragma omp task depend(inout: member_var) mergeable
        {
            member_var += local_var;
        }
        
        // Task with depend(out) - producing result
        #pragma omp task depend(out: *heap_ptr) final(member_var > 100)
        {
            heap_ptr[0] = member_var * 2;
        }
        
        // Task with depend(mutexinoutset) - set-based dependency
        #pragma omp task depend(mutexinoutset: member_ptr[2])
        {
            member_ptr[2] *= 2;
        }
        
        // Task with depend(inoutset) - another set-based dependency
        #pragma omp task depend(inoutset: member_ptr[3])
        {
            member_ptr[3] += 10;
        }
        
        // Task with depend(destroy) - cleanup
        #pragma omp task depend(destroy: heap_ptr)
        {
            delete[] heap_ptr;
        }
        
        #pragma omp taskwait
    }
};

int TaskContainer::static_member = 0;

// Function with complex data environment
void process_with_dependencies(int* array, int size) {
    int local_acc = 0;
    static int persistent_counter = 0;
    
    // Using references and pointers in depend clauses
    int& ref = local_acc;
    int* dyn_array = new int[size];
    
    // Parallel region to create task execution context
    #pragma omp parallel num_threads(4)
    {
        #pragma omp single
        {
            // Graph of tasks with various depend modifiers
            
            // Initializer task with depend(out)
            #pragma omp task depend(out: dyn_array[0])
            {
                dyn_array[0] = 1;
            }
            
            // Reader task with depend(in)
            #pragma omp task depend(in: dyn_array[0])
            {
                ref = dyn_array[0];
            }
            
            // Updater task with depend(inout)
            #pragma omp task depend(inout: persistent_counter)
            {
                persistent_counter += ref;
            }
            
            // Multiple tasks with set-based dependencies
            for (int i = 1; i < size; i++) {
                #pragma omp task depend(mutexinoutset: array[i]) depend(in: persistent_counter)
                {
                    array[i] = persistent_counter + i;
                }
                
                #pragma omp task depend(inoutset: dyn_array[i]) depend(in: array[i])
                {
                    dyn_array[i] = array[i] * 2;
                }
            }
            
            // Final task with depend(destroy)
            #pragma omp task depend(destroy: dyn_array)
            {
                // Verify computation
                int sum = 0;
                for (int i = 0; i < size; i++) {
                    sum += dyn_array[i];
                }
                std::cout << "Sum: " << sum << std::endl;
                delete[] dyn_array;
            }
        }
    }
}

// Deliberate syntax error to trigger diagnostic pretty-printing
// This will cause the compiler to engage its diagnostic machinery
// and potentially pretty-print the OpenMP constructs in error messages
void trigger_diagnostics() {
    // Syntax error: undeclared type
    UndeclaredType* error_ptr = nullptr;  // This line will cause compilation error
    
    // The compiler may try to pretty-print surrounding context
    // including any OpenMP constructs during error reporting
}

int main() {
    const int N = 10;
    int data_array[N];
    
    // Initialize global pointer
    global_ptr = new int[N];
    
    // Create container object
    TaskContainer container;
    
    // Execute member function with tasks
    #pragma omp parallel
    {
        #pragma omp single
        container.member_function_with_tasks();
    }
    
    // Process with dependencies
    process_with_dependencies(data_array, N);
    
    // Perform a verifiable calculation
    int final_sum = 0;
    #pragma omp parallel for reduction(+:final_sum)
    for (int i = 0; i < N; i++) {
        final_sum += data_array[i];
    }
    
    std::cout << "Final sum: " << final_sum << std::endl;
    
    // Cleanup
    delete[] global_ptr;
    
    // Uncomment to trigger diagnostics
    // trigger_diagnostics();
    
    return 0;
}

// Additional test cases in different scopes
namespace TestNamespace {
    void namespace_function() {
        int ns_var = 0;
        
        // Task with depend(in) in namespace
        #pragma omp task depend(in: ns_var)
        {
            ns_var = 42;
        }
        
        // Task with depend(out) in namespace  
        #pragma omp task depend(out: ns_var)
        {
            ns_var = 100;
        }
        
        #pragma omp taskwait
    }
}

// Template function to test generic context
template<typename T>
void template_function(T* items, int count) {
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (int i = 0; i < count; i++) {
                #pragma omp task depend(inout: items[i]) priority(i)
                {
                    items[i] = items[i] * 2;
                }
            }
            
            #pragma omp task depend(destroy: items)
            {
                // Cleanup logic here
            }
        }
    }
}
```
