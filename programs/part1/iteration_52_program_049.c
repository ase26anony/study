```cpp
// Test program to cover OpenMP task depend clause modifiers in GCC's C++ pretty-printer
// Compile with: g++ -O1 -fopenmp -fdump-tree-omp -c -xc++ this_file.cc
// Or: g++ -O2 -fopenmp -Werror=openmp-format -c -xc++ this_file.cc
// Or: g++ -O0 -g -fopenmp -fdump-tree-all -save-temps -xc++ this_file.cc

#include <iostream>
#include <omp.h>
#include <cstdlib>

// Global variables with different linkages
int global_var = 0;
static int static_global_var = 0;
extern int extern_var; // Will remain undefined to potentially trigger diagnostics

// Heap-allocated data
int* heap_array = nullptr;
const int ARRAY_SIZE = 10;

// Class with member function containing OpenMP tasks
class TaskContainer {
public:
    int member_var;
    static int static_member;
    
    void execute_tasks() {
        int local_var = 0;
        static int static_local = 0;
        int local_array[5] = {0};
        
        // Pointer for depend clause
        int* ptr = &local_var;
        
        // Enter parallel region
        #pragma omp parallel
        {
            #pragma omp single
            {
                // Task with depend(in) - reading initial value
                #pragma omp task depend(in: global_var) priority(1)
                {
                    local_var = global_var;
                }
                
                // Task with depend(inout) - updating shared accumulator
                #pragma omp task depend(inout: static_global_var) mergeable
                {
                    static_global_var += 1;
                }
                
                // Task with depend(out) - producing new value
                #pragma omp task depend(out: member_var) final(0)
                {
                    member_var = 42;
                }
                
                // Task with depend(mutexinoutset) - set-based dependency
                #pragma omp task depend(mutexinoutset: heap_array[0])
                {
                    if (heap_array) heap_array[0] = 1;
                }
                
                // Task with depend(inoutset) - another set-based dependency
                #pragma omp task depend(inoutset: heap_array[1])
                {
                    if (heap_array) heap_array[1] = 2;
                }
                
                // Task with depend(destroy) - cleanup
                #pragma omp task depend(destroy: heap_array)
                {
                    // Simulate cleanup
                }
                
                // Additional tasks with pointer/reference dependencies
                #pragma omp task depend(in: *ptr)
                {
                    // Read through pointer
                }
                
                #pragma omp task depend(inout: local_array[2])
                {
                    local_array[2] = 3;
                }
                
                // Nested taskgroup with dependencies
                #pragma omp taskgroup
                {
                    #pragma omp task depend(in: static_local)
                    {
                        static_local = 5;
                    }
                    
                    #pragma omp task depend(inout: local_array[3]) depend(in: static_local)
                    {
                        local_array[3] = static_local + 1;
                    }
                }
                
                // Taskwait to ensure dependencies are respected
                #pragma omp taskwait
            }
        }
    }
};

// Static member definition
int TaskContainer::static_member = 0;

// Undefined function with syntax error to trigger diagnostics
// This will cause the compiler to potentially pretty-print surrounding code
void undefined_function_with_error(UndefinedType x); // ERROR: UndefinedType not declared

int main() {
    // Allocate heap memory
    heap_array = new int[ARRAY_SIZE];
    
    // Initialize array
    for (int i = 0; i < ARRAY_SIZE; i++) {
        heap_array[i] = i;
    }
    
    // Create task container and execute tasks
    TaskContainer container;
    container.member_var = 0;
    
    // Execute tasks with various depend modifiers
    container.execute_tasks();
    
    // Perform computation to ensure tasks execute
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += heap_array[i];
    }
    
    std::cout << "Sum: " << sum << std::endl;
    std::cout << "Global: " << global_var << std::endl;
    std::cout << "Static global: " << static_global_var << std::endl;
    std::cout << "Member: " << container.member_var << std::endl;
    
    // Cleanup
    delete[] heap_array;
    
    // Try to call undefined function - may trigger diagnostics
    // undefined_function_with_error(0); // Commented to allow compilation if needed
    
    return 0;
}

// Additional parallel region with more depend clauses
void extra_tasks() {
    int a = 0, b = 0, c = 0;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            // All depend modifiers in one function
            #pragma omp task depend(in: a)
            { a = 1; }
            
            #pragma omp task depend(inout: b)
            { b += a; }
            
            #pragma omp task depend(out: c)
            { c = a + b; }
            
            #pragma omp task depend(mutexinoutset: a)
            { a = 2; }
            
            #pragma omp task depend(inoutset: b)
            { b = 3; }
            
            #pragma omp task depend(destroy: c)
            { /* destroy */ }
            
            #pragma omp taskwait
        }
    }
}
```
