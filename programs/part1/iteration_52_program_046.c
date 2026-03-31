```cpp
// Test program to cover OpenMP depend clause modifiers in GCC C++ pretty-printer
// Compile with: g++ -O1 -fopenmp -fdump-tree-omp -c -xc++ test_openmp_depend.cpp
// Or: g++ -O0 -g -fopenmp -fdump-tree-all -save-temps -xc++ test_openmp_depend.cpp

#include <cstdlib>
#include <iostream>

// Global variables with different linkages
int global_var = 0;
static int static_global = 0;
extern int extern_var; // Forward declaration

// Heap-allocated data
int* heap_array = nullptr;
const int ARRAY_SIZE = 10;

// Class to test member function scoping
class TaskContainer {
public:
    int member_var;
    static int static_member;
    
    TaskContainer() : member_var(0) {}
    
    void execute_tasks() {
        int local_var = 0;
        static int static_local = 0;
        
        // Pointer to heap data
        int* local_ptr = new int[5];
        
        #pragma omp parallel
        {
            #pragma omp single
            {
                // Task with depend(in) - reading from initialization
                #pragma omp task depend(in: global_var) priority(1)
                {
                    local_var = global_var;
                }
                
                // Task with depend(inout) - updating accumulator
                #pragma omp task depend(inout: static_global) mergeable
                {
                    static_global += 1;
                }
                
                // Task with depend(out) - producing result
                #pragma omp task depend(out: member_var) final(local_var > 100)
                {
                    member_var = 42;
                }
                
                // Task with depend(mutexinoutset) - set-based dependency
                #pragma omp task depend(mutexinoutset: heap_array[0])
                {
                    heap_array[0] = 1;
                }
                
                // Task with depend(inoutset) - another set-based dependency
                #pragma omp task depend(inoutset: heap_array[1])
                {
                    heap_array[1] = 2;
                }
                
                // Task with depend(destroy) - cleanup
                #pragma omp task depend(destroy: local_ptr[0])
                {
                    // Cleanup operation
                }
                
                // More complex examples with references and pointers
                int ref_var = 0;
                int* dyn_ptr = new int;
                
                #pragma omp task depend(in: *dyn_ptr)
                {
                    ref_var = *dyn_ptr;
                }
                
                #pragma omp task depend(inout: ref_var)
                {
                    ref_var *= 2;
                }
                
                #pragma omp task depend(out: *dyn_ptr)
                {
                    *dyn_ptr = ref_var;
                }
                
                // Nested task with depend clause
                #pragma omp taskgroup
                {
                    #pragma omp task depend(inout: static_local)
                    {
                        static_local++;
                        
                        #pragma omp task depend(in: static_local)
                        {
                            local_var = static_local;
                        }
                    }
                }
                
                // Array element dependencies
                for (int i = 0; i < 5; ++i) {
                    #pragma omp task depend(inout: heap_array[i]) depend(in: heap_array[(i+1)%5])
                    {
                        heap_array[i] += heap_array[(i+1)%5];
                    }
                }
                
                // Taskwait to ensure dependencies are respected
                #pragma omp taskwait
                
                delete[] local_ptr;
                delete dyn_ptr;
            }
        }
    }
};

int TaskContainer::static_member = 0;

// Forward declaration with deliberate syntax error to trigger diagnostics
// This will cause the compiler to engage its diagnostic machinery
class UndeclaredType;  // This type is never defined

void function_with_error(UndeclaredType* ptr) {
    // This function uses an undeclared type, causing a compilation error
    // The error will be reported, potentially causing pretty-printing of
    // surrounding OpenMP constructs during diagnostic output
}

int main() {
    // Initialize heap data
    heap_array = new int[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        heap_array[i] = i;
    }
    
    // Initialize global
    global_var = 100;
    
    // Create container and execute tasks
    TaskContainer container;
    container.execute_tasks();
    
    // Perform a verifiable calculation using tasks with depend clauses
    int sum = 0;
    int data[5] = {1, 2, 3, 4, 5};
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            // Producer task
            #pragma omp task depend(out: data[0])
            {
                data[0] = 10;
            }
            
            // Consumer tasks with different depend types
            #pragma omp task depend(in: data[0]) depend(out: sum)
            {
                sum = data[0];
            }
            
            #pragma omp task depend(inout: sum) depend(mutexinoutset: data[1])
            {
                sum += 20;
                data[1] = sum;
            }
            
            #pragma omp task depend(inoutset: data[2]) depend(destroy: data[3])
            {
                data[2] = sum * 2;
            }
            
            #pragma omp taskwait
            
            // Final verification
            std::cout << "Final sum: " << sum << std::endl;
            std::cout << "Data[2]: " << data[2] << std::endl;
        }
    }
    
    // Cleanup
    delete[] heap_array;
    
    // The following line would trigger the syntax error if uncommented
    // function_with_error(nullptr);
    
    return 0;
}

// Missing definition for extern_var - another potential error source
// int extern_var = 0;  // Uncomment to fix link error

// Additional test: template class with OpenMP tasks
template<typename T>
class TemplateTaskRunner {
public:
    T value;
    
    void run() {
        #pragma omp task depend(inout: value)
        {
            value = value + 1;
        }
        
        #pragma omp task depend(out: static_global)
        {
            static_global = static_global + 1;
        }
    }
};

// Instantiate template to ensure it's compiled
template class TemplateTaskRunner<int>;
```
