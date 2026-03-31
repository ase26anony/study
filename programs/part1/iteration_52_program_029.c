```cpp
// Test program to cover OpenMP depend clause modifiers in GCC C++ pretty-printer
// Compile with: g++ -O1 -fopenmp -fdump-tree-omp -c -xc++ this_file.cc
// Also try: g++ -O2 -fopenmp -Werror=openmp-format -c -xc++ this_file.cc
// Or: g++ -O0 -g -fopenmp -fdump-tree-all -save-temps -c -xc++ this_file.cc

#include <cstdlib>
#include <iostream>

// Global variables with different linkages
int global_var = 0;
static int static_global_var = 0;
int* global_ptr = nullptr;

// Class to test member function contexts
class TestClass {
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
                // Test all depend clause modifiers
                
                // 1. depend(in) - read dependency
                #pragma omp task depend(in: global_var) depend(in: static_local) \
                                 priority(1) final(false)
                {
                    int temp = global_var + static_local;
                }
                
                // 2. depend(inout) - read-write dependency
                #pragma omp task depend(inout: member_var) depend(inout: *heap_ptr) \
                                 mergeable
                {
                    member_var += 1;
                    heap_ptr[0] = member_var;
                }
                
                // 3. depend(out) - write dependency
                #pragma omp task depend(out: local_var) depend(out: heap_ptr[1])
                {
                    local_var = 42;
                    heap_ptr[1] = 100;
                }
                
                // 4. depend(mutexinoutset) - set-based with mutual exclusion
                #pragma omp task depend(mutexinoutset: heap_ptr[2]) \
                                 depend(mutexinoutset: static_global_var)
                {
                    heap_ptr[2] = heap_ptr[2] + 1;
                    static_global_var = heap_ptr[2];
                }
                
                // 5. depend(inoutset) - set-based read-write
                #pragma omp task depend(inoutset: heap_ptr[3]) \
                                 depend(inoutset: TestClass::static_member)
                {
                    heap_ptr[3] *= 2;
                    TestClass::static_member = heap_ptr[3];
                }
                
                // 6. depend(destroy) - destroy dependency
                #pragma omp task depend(destroy: heap_ptr[4]) \
                                 depend(destroy: global_ptr)
                {
                    // Cleanup operations
                    heap_ptr[4] = 0;
                    if (global_ptr) *global_ptr = 0;
                }
                
                #pragma omp taskwait
            }
        }
        
        delete[] heap_ptr;
    }
    
    // Deliberate syntax error to trigger diagnostics
    // This will cause the compiler to potentially pretty-print surrounding OpenMP constructs
    void error_function() {
        // Syntax error: undeclared type 'UndeclaredType'
        UndeclaredType* ptr = nullptr;  // This line will cause compilation error
    }
};

int TestClass::static_member = 0;

// Function with complex data environment
void complex_dependencies() {
    int array[10] = {0};
    int& ref = array[0];
    int* ptr_array = new int[10];
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            // Create a dependency graph
            #pragma omp task depend(out: array[0])  // Producer
            {
                array[0] = 1;
            }
            
            #pragma omp task depend(in: array[0]) depend(out: array[1])  // Consumer-Producer
            {
                array[1] = array[0] + 1;
            }
            
            #pragma omp task depend(inout: array[1]) depend(inout: ref)  // Updater
            {
                array[1] *= 2;
                ref = array[1];
            }
            
            #pragma omp task depend(mutexinoutset: ptr_array[0]) \
                             depend(mutexinoutset: ptr_array[1])
            {
                ptr_array[0] = 10;
                ptr_array[1] = 20;
            }
            
            #pragma omp task depend(inoutset: ptr_array[2]) \
                             depend(inoutset: ptr_array[3])
            {
                ptr_array[2] = ptr_array[0] + ptr_array[1];
                ptr_array[3] = ptr_array[2] / 2;
            }
            
            #pragma omp task depend(destroy: ptr_array[4]) \
                             depend(destroy: ptr_array[5])
            {
                ptr_array[4] = 0;
                ptr_array[5] = 0;
            }
            
            #pragma omp taskgroup
            {
                #pragma omp task depend(in: array[1])  // Final consumer
                {
                    int result = array[1] + ref;
                    // Use result to prevent optimization
                    volatile int prevent_opt = result;
                }
            }
        }
    }
    
    delete[] ptr_array;
}

// Main function with valid computation
int main() {
    TestClass obj;
    obj.member_var = 10;
    
    // Initialize global pointer
    global_ptr = new int(5);
    
    // Call functions with OpenMP tasks
    obj.member_function();
    complex_dependencies();
    
    // Perform a verifiable calculation
    int sum = 0;
    int data[100];
    
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 100; i++) {
        data[i] = i;
        sum += i;
    }
    
    std::cout << "Sum: " << sum << std::endl;
    
    // Cleanup
    delete global_ptr;
    
    return 0;
}

// Additional function with nested constructs
void nested_constructs() {
    int a = 0, b = 0, c = 0;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(inout: a) shared(a)
            { a = 1; }
            
            #pragma omp task depend(in: a) depend(out: b) shared(a, b)
            { b = a + 1; }
            
            #pragma omp task depend(in: b) depend(out: c) shared(b, c)
            { c = b * 2; }
            
            #pragma omp task depend(in: c) shared(c)
            {
                volatile int final_result = c;
            }
        }
    }
}
```
