```cpp
// Test program to cover OpenMP depend clause modifiers in GCC's C++ pretty-printer
// Compile with: g++ -O1 -fopenmp -fdump-tree-omp -c -xc++ this_file.cc
// Or: g++ -O2 -fopenmp -Werror=openmp-format -c -xc++ this_file.cc
// Or: g++ -O0 -g -fopenmp -fdump-tree-all -save-temps -xc++ this_file.cc

#include <iostream>
#include <omp.h>
#include <cstdlib>

// Global variables with different linkages
int global_var = 0;
static int static_global_var = 0;
extern int extern_var; // Forward declaration

// Heap-allocated data
int* heap_array = nullptr;
const int ARRAY_SIZE = 10;

// Class with member function containing OpenMP tasks
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
    
    void execute_tasks() {
        int local_var = 0;
        static int static_local = 0;
        int* local_ptr = new int(5);
        
        // Array for set-based dependencies
        int set_array[5] = {0};
        
        // Enter parallel region
        #pragma omp parallel
        {
            #pragma omp single
            {
                // Task with depend(in) - reads from initialization
                #pragma omp task depend(in: heap_array[0]) \
                                 depend(in: global_var) \
                                 depend(in: *local_ptr) \
                                 priority(1)
                {
                    local_var = heap_array[0] + global_var + *local_ptr;
                }
                
                // Task with depend(inout) - updates accumulator
                #pragma omp task depend(inout: static_global_var) \
                                 depend(in: local_var) \
                                 mergeable
                {
                    static_global_var += local_var;
                }
                
                // Task with depend(out) - produces result
                #pragma omp task depend(out: member_var) \
                                 depend(in: static_global_var) \
                                 final(member_var > 100)
                {
                    member_var = static_global_var * 2;
                }
                
                // Tasks with depend(mutexinoutset) - mutual exclusion on array elements
                #pragma omp task depend(mutexinoutset: set_array[0]) \
                                 depend(in: member_var)
                {
                    set_array[0] = member_var % 10;
                }
                
                #pragma omp task depend(mutexinoutset: set_array[0]) \
                                 depend(in: member_var)
                {
                    set_array[1] = (member_var + 1) % 10;
                }
                
                // Task with depend(inoutset) - set-based dependency
                #pragma omp task depend(inoutset: set_array[2]) \
                                 depend(inoutset: set_array[3]) \
                                 priority(2)
                {
                    set_array[2] = set_array[0] + set_array[1];
                    set_array[3] = set_array[2] * 2;
                }
                
                // Task with depend(destroy) - cleanup
                #pragma omp task depend(destroy: heap_array) \
                                 depend(in: set_array[2], set_array[3])
                {
                    // Simulate cleanup
                    heap_array[0] = set_array[2] + set_array[3];
                }
                
                // Additional task with combined dependencies
                #pragma omp task depend(in: set_array[2]) \
                                 depend(out: set_array[4]) \
                                 depend(inout: static_local)
                {
                    set_array[4] = set_array[2] * 3;
                    static_local = set_array[4];
                }
                
                // Taskwait to ensure dependencies are respected
                #pragma omp taskwait
                
                // Nested task inside taskgroup
                #pragma omp taskgroup
                {
                    #pragma omp task depend(inout: global_var)
                    {
                        global_var = member_var + static_local;
                    }
                }
            }
        }
        
        delete local_ptr;
    }
};

int TaskContainer::static_member = 0;
int extern_var = 42; // Definition

// Function with syntax error to trigger diagnostic output
// This will cause the compiler to engage its diagnostic machinery
// and potentially pretty-print the OpenMP constructs
void problematic_function() {
    // Deliberate benign syntax error: undeclared type in unused parameter
    // This should trigger diagnostics without affecting compilation of the rest
    #ifdef TRIGGER_ERROR
    void some_function(UndefinedType x); // This type is not defined
    #endif
    
    // Valid OpenMP code that might get printed in diagnostics
    int x = 0;
    #pragma omp parallel
    {
        #pragma omp task depend(in: x)
        {
            x = 1;
        }
    }
}

int main() {
    TaskContainer container;
    
    // Execute tasks with various depend modifiers
    container.execute_tasks();
    
    // Perform a verifiable calculation
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += heap_array[i];
    }
    
    std::cout << "Result: " << sum << std::endl;
    std::cout << "Global var: " << global_var << std::endl;
    std::cout << "Static global: " << static_global_var << std::endl;
    std::cout << "Member var: " << container.member_var << std::endl;
    
    // Call function with potential syntax error
    problematic_function();
    
    return 0;
}

// Additional complex dependency patterns
void complex_dependency_patterns() {
    struct Data {
        int a;
        int b;
        int* ptr;
    };
    
    Data data1 = {0, 0, new int[5]};
    Data data2 = {0, 0, new int[5]};
    
    // Complex pointer-based dependencies
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(in: data1.ptr[0]) \
                             depend(out: data2.ptr[0]) \
                             depend(mutexinoutset: data1.a)
            {
                data2.ptr[0] = data1.ptr[0] * 2;
            }
            
            #pragma omp task depend(inoutset: data1.b) \
                             depend(inoutset: data2.b) \
                             depend(destroy: data1.ptr)
            {
                data1.b = data2.ptr[0];
                data2.b = data1.b + 1;
            }
            
            #pragma omp task depend(in: data1.b, data2.b) \
                             depend(out: data1.a)
            {
                data1.a = data1.b + data2.b;
            }
            
            #pragma omp taskwait
        }
    }
    
    delete[] data1.ptr;
    delete[] data2.ptr;
}
```
