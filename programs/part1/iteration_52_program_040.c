```cpp
// Test program to cover OpenMP task depend clause modifiers in GCC C++ pretty-printer
// Compile with: g++ -O1 -fopenmp -fdump-tree-omp -c -xc++ test_openmp_depend.cpp
// Or: g++ -O2 -fopenmp -Werror=openmp-format -c -xc++ test_openmp_depend.cpp
// Or: g++ -O0 -g -fopenmp -fdump-tree-all -save-temps -xc++ test_openmp_depend.cpp

#include <cstdlib>
#include <iostream>

// Global variables with different linkages
int global_var = 0;
static int static_global_var = 0;
extern int external_var; // Forward declaration

// Heap-allocated data
int* heap_array = nullptr;

// Class to test member function scoping
class TaskContainer {
public:
    int member_var;
    static int static_member;
    
    void member_function() {
        int local_var = 0;
        static int static_local = 0;
        
        // Use various depend modifiers in member function context
        #pragma omp task depend(in: member_var) depend(in: static_member) \
                         depend(in: local_var) depend(in: static_local)
        {
            member_var = static_member + local_var + static_local;
        }
        
        #pragma omp task depend(inout: member_var) priority(1)
        {
            member_var *= 2;
        }
        
        #pragma omp task depend(out: local_var) mergeable
        {
            local_var = 42;
        }
    }
};

int TaskContainer::static_member = 100;

// Forward declaration with deliberate syntax error to trigger diagnostics
// This will cause the compiler to potentially pretty-print surrounding OpenMP constructs
class UndeclaredType;  // This type is never defined

void function_with_error(UndeclaredType* dummy) {  // Syntax error here
    // This function won't be called, but its declaration will trigger diagnostics
}

int main() {
    const int N = 100;
    heap_array = new int[N];
    
    // Initialize array
    for (int i = 0; i < N; i++) {
        heap_array[i] = i;
    }
    
    // Variables with different storage durations
    int local_array[10];
    static int static_array[10];
    int& ref_var = global_var;
    int* ptr_var = &global_var;
    
    // Enter parallel region to create tasking environment
    #pragma omp parallel
    {
        #pragma omp single
        {
            // Task with depend(in) - reading initial values
            #pragma omp task depend(in: global_var) depend(in: heap_array[0]) \
                             depend(in: ref_var) depend(in: *ptr_var)
            {
                int sum = global_var + heap_array[0] + ref_var + *ptr_var;
                local_array[0] = sum;
            }
            
            // Task with depend(inout) - updating shared accumulator
            #pragma omp task depend(inout: global_var) priority(2)
            {
                global_var += 10;
            }
            
            // Task with depend(out) - producing new value
            #pragma omp task depend(out: static_array[0]) final(global_var > 100)
            {
                static_array[0] = 42;
            }
            
            // Task with depend(mutexinoutset) - set-based dependency
            #pragma omp task depend(mutexinoutset: heap_array[1:5])  // array section
            {
                for (int i = 1; i <= 5; i++) {
                    heap_array[i] *= 2;
                }
            }
            
            // Another task with depend(mutexinoutset) on same set
            #pragma omp task depend(mutexinoutset: heap_array[3:3])
            {
                heap_array[3] += 5;
            }
            
            // Task with depend(inoutset) - another set-based dependency
            #pragma omp task depend(inoutset: local_array[2:4])
            {
                for (int i = 2; i < 6; i++) {
                    local_array[i] = i * 10;
                }
            }
            
            // Task with depend(destroy) - cleanup operation
            #pragma omp task depend(destroy: heap_array[0])
            {
                heap_array[0] = -1;
            }
            
            // Complex nested task with multiple depend clauses
            #pragma omp taskgroup
            {
                #pragma omp task depend(in: static_global_var) \
                                 depend(inout: global_var) \
                                 depend(out: local_array[9])
                {
                    local_array[9] = static_global_var + global_var;
                }
                
                #pragma omp taskwait
                
                // Task depending on previous task's output
                #pragma omp task depend(in: local_array[9])
                {
                    std::cout << "Final value: " << local_array[9] << std::endl;
                }
            }
            
            // Test member function with tasks
            TaskContainer obj;
            obj.member_var = 50;
            
            #pragma omp task
            {
                obj.member_function();
            }
            
            // Ensure all tasks complete
            #pragma omp taskwait
            
            // Additional task with pointer dereference in depend clause
            int** ptr_to_ptr = &ptr_var;
            #pragma omp task depend(in: **ptr_to_ptr)
            {
                **ptr_to_ptr += 1;
            }
        }
    }
    
    // Perform a verifiable calculation using the tasks
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < N; i++) {
        sum += heap_array[i];
    }
    
    std::cout << "Sum of heap array: " << sum << std::endl;
    
    // Cleanup
    delete[] heap_array;
    
    // The following line would trigger the syntax error if uncommented
    // function_with_error(nullptr);
    
    return 0;
}

// Missing external variable definition - another potential diagnostic trigger
// int external_var;  // Commented out to create undefined reference

// Additional OpenMP constructs in different scopes
namespace TestNamespace {
    void namespace_function() {
        thread_local int tl_var = 0;
        
        #pragma omp task depend(inout: tl_var)
        {
            tl_var++;
        }
    }
}

// Template function with OpenMP tasks
template<typename T>
void template_function(T* data, int size) {
    #pragma omp task depend(in: data[0]) depend(out: data[size-1])
    {
        data[size-1] = data[0];
    }
}

// Instantiate template to ensure it's compiled
template void template_function<int>(int*, int);
```
