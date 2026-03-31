```cpp
// Test program to cover OpenMP task depend clause modifiers in GCC C++ pretty-printer
// Compile with: g++ -O1 -fopenmp -fdump-tree-omp -c -xc++ this_file.cc
// Also try: g++ -O2 -fopenmp -Werror=openmp-format -c -xc++ this_file.cc
// And: g++ -O0 -g -fopenmp -fdump-tree-all -save-temps -c -xc++ this_file.cc

#include <cstdlib>
#include <iostream>

// Global variables for different storage durations
int global_var = 0;
static int static_global_var = 0;
int* heap_ptr = nullptr;

// Class to test member function scoping
class TestClass {
public:
    int member_var;
    static int static_member;
    
    void member_function() {
        int local_var = 0;
        static int static_local = 0;
        int* local_heap = new int[5];
        
        #pragma omp parallel
        {
            #pragma omp single
            {
                // Task with depend(in) - reading from initialization
                #pragma omp task depend(in: local_var) depend(in: static_local) \
                                 priority(1) mergeable
                {
                    // Read operation
                    int temp = local_var + static_local;
                }
                
                // Task with depend(inout) - updating shared accumulator
                #pragma omp task depend(inout: member_var) depend(inout: TestClass::static_member) \
                                 priority(2)
                {
                    member_var += 5;
                    TestClass::static_member += 3;
                }
                
                // Task with depend(out) - producing final result
                #pragma omp task depend(out: local_heap[2]) depend(out: *local_heap) \
                                 final(member_var > 10)
                {
                    local_heap[2] = 42;
                    *local_heap = 100;
                }
                
                // Task with depend(mutexinoutset) - set-based dependency
                #pragma omp task depend(mutexinoutset: local_heap[1]) \
                                 depend(mutexinoutset: local_heap[3])
                {
                    local_heap[1] = local_heap[3] + 1;
                }
                
                // Task with depend(inoutset) - another set-based dependency
                #pragma omp task depend(inoutset: local_heap[0]) \
                                 depend(inoutset: local_heap[4])
                {
                    local_heap[0] = local_heap[4] * 2;
                }
                
                // Task with depend(destroy) - cleanup
                #pragma omp task depend(destroy: local_heap) \
                                 depend(destroy: local_var)
                {
                    // Cleanup operations
                    local_var = 0;
                }
                
                #pragma omp taskwait
            }
        }
        
        delete[] local_heap;
    }
};

int TestClass::static_member = 0;

// Function with complex data environment
void complex_dependencies() {
    int array[10] = {0};
    int& ref = array[0];
    int* ptr = &array[5];
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            // Mix of different depend types in nested contexts
            #pragma omp taskgroup
            {
                // Multiple depend clauses with different modifiers
                #pragma omp task depend(in: array[0]) depend(in: ref) \
                             depend(in: global_var) priority(3)
                {
                    int read_val = array[0] + ref + global_var;
                }
                
                #pragma omp task depend(inout: array[1]) depend(inout: *ptr) \
                             depend(inout: static_global_var)
                {
                    array[1] += 2;
                    *ptr += 3;
                    static_global_var += 4;
                }
                
                #pragma omp task depend(out: array[2]) depend(out: heap_ptr) \
                             mergeable
                {
                    array[2] = 99;
                    heap_ptr = new int(50);
                }
                
                #pragma omp task depend(mutexinoutset: array[3]) \
                             depend(mutexinoutset: array[7])
                {
                    array[3] = array[7] + 10;
                }
                
                #pragma omp task depend(inoutset: array[4]) \
                             depend(inoutset: array[8])
                {
                    array[4] = array[8] * 3;
                }
                
                #pragma omp task depend(destroy: array) \
                             depend(destroy: ptr)
                {
                    // Reset values
                    for (int i = 0; i < 10; i++) {
                        array[i] = 0;
                    }
                }
            }
        }
    }
}

// Function with deliberate syntax error to trigger diagnostics
// This will cause the compiler to potentially pretty-print the OpenMP constructs
// when generating error messages
void trigger_diagnostics() {
    // Valid OpenMP code that should be pretty-printed
    int x = 0, y = 0, z = 0;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            // All depend modifiers in one place for concentrated coverage
            #pragma omp task depend(in: x)
            { x = 1; }
            
            #pragma omp task depend(inout: y)
            { y += x; }
            
            #pragma omp task depend(out: z)
            { z = x + y; }
            
            #pragma omp task depend(mutexinoutset: x)
            { x = y * 2; }
            
            #pragma omp task depend(inoutset: y)
            { y = z / 2; }
            
            #pragma omp task depend(destroy: z)
            { z = 0; }
            
            #pragma omp taskwait
        }
    }
    
    // Syntax error: undeclared type in unused function signature
    // This will trigger compiler diagnostics without affecting execution
    // if the function isn't called
    // UndeclaredType error_var;  // Uncomment to force error
}

// Main function with valid computation to verify OpenMP works
int main() {
    // Initialize heap pointer
    heap_ptr = new int(10);
    
    // Test class member function
    TestClass obj;
    obj.member_var = 5;
    obj.member_function();
    
    // Test complex dependencies
    complex_dependencies();
    
    // Trigger pretty-printer via diagnostics (function exists but isn't called)
    // The mere presence of trigger_diagnostics() with OpenMP constructs
    // should cause them to be processed during compilation
    
    // Valid computation to ensure OpenMP directives are active
    int sum = 0;
    int data[100];
    
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 100; i++) {
        data[i] = i;
        sum += i;
    }
    
    std::cout << "Sum: " << sum << std::endl;
    
    // Cleanup
    delete heap_ptr;
    
    return 0;
}

// Additional function with syntax error to force compiler diagnostics
// This ensures the pretty-printer is invoked during error reporting
void force_error() {
    // This line contains a syntax error that will trigger diagnostics
    // The compiler may pretty-print surrounding OpenMP constructs in error messages
    // NonExistentType error_variable;  // Uncomment to force error
}
```
