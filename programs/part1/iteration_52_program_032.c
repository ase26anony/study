```cpp
// Test program to cover OpenMP depend clause pretty-printer lines
// Compile with: g++ -O1 -fopenmp -fdump-tree-omp -c -xc++ this_file.cc
// Or: g++ -O2 -fopenmp -Werror=openmp-format -c -xc++ this_file.cc
// Or: g++ -O0 -g -fopenmp -fdump-tree-all -save-temps -c -xc++ this_file.cc

// Global variables with different linkages
int global_var = 0;
static int static_global_var = 0;
int* global_ptr = nullptr;

// Class to test member function scoping
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
                // Task with depend(in) - reads initialization
                #pragma omp task depend(in: global_var) depend(in: static_local) \
                                 priority(1) mergeable
                {
                    local_var = global_var + static_local;
                }
                
                // Task with depend(inout) - updates accumulator
                #pragma omp task depend(inout: member_var) depend(in: local_var) \
                                 final(local_var > 100)
                {
                    member_var += local_var;
                }
                
                // Task with depend(out) - produces result
                #pragma omp task depend(out: heap_ptr[0]) \
                                 depend(in: member_var)
                {
                    heap_ptr[0] = member_var * 2;
                }
                
                // Task with depend(mutexinoutset) - set-based dependency
                #pragma omp task depend(mutexinoutset: heap_ptr[1]) \
                                 depend(in: heap_ptr[0])
                {
                    heap_ptr[1] = heap_ptr[0] + 1;
                }
                
                // Task with depend(inoutset) - another set-based
                #pragma omp task depend(inoutset: heap_ptr[2]) \
                                 depend(in: heap_ptr[1])
                {
                    heap_ptr[2] = heap_ptr[1] * 2;
                }
                
                // Task with depend(destroy) - cleanup
                #pragma omp task depend(destroy: heap_ptr[3]) \
                                 depend(in: heap_ptr[2])
                {
                    heap_ptr[3] = -1; // Mark as destroyed
                }
                
                #pragma omp taskwait
            }
        }
        
        delete[] heap_ptr;
    }
    
    // Nested class for additional complexity
    struct Nested {
        int nested_var;
        void nested_method() {
            #pragma omp task depend(inout: nested_var)
            {
                nested_var++;
            }
        }
    };
};

int TestClass::static_member = 0;

// Function with complex depend usage
void complex_depend_graph() {
    int array[10] = {0};
    int& ref = array[0];
    int* ptr = &array[1];
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            // Chain of dependencies using different modifiers
            #pragma omp task depend(out: array[0]) priority(2)
            { array[0] = 1; }
            
            #pragma omp task depend(in: array[0]) depend(out: array[1])
            { array[1] = array[0] + 1; }
            
            #pragma omp task depend(inout: array[1]) depend(out: array[2])
            { array[2] = array[1]++; }
            
            #pragma omp task depend(mutexinoutset: array[3]) \
                             depend(in: array[2])
            { array[3] = array[2] * 2; }
            
            #pragma omp task depend(inoutset: array[4]) \
                             depend(in: array[3])
            { array[4] = array[3] + 1; }
            
            #pragma omp task depend(destroy: array[5]) \
                             depend(in: array[4])
            { array[5] = 0; }
            
            // Using references and pointers in depend
            #pragma omp task depend(inout: ref) depend(in: *ptr)
            { ref += *ptr; }
            
            #pragma omp taskgroup
            {
                #pragma omp task depend(in: array[6])
                { /* Some computation */ }
            }
            
            #pragma omp taskwait
        }
    }
}

// Template to increase C++ complexity
template<typename T>
T template_function(T val) {
    T local = val;
    #pragma omp task depend(inout: local) if(local > 0)
    {
        local *= 2;
    }
    #pragma omp taskwait
    return local;
}

// Deliberate syntax error to trigger diagnostics
// This will cause the compiler to potentially pretty-print surrounding OpenMP constructs
void function_with_error() {
    // Syntax error: undeclared type 'UndeclaredType'
    // Uncomment to force diagnostics:
    // UndeclaredType* p = nullptr;
    
    // But we need valid code for compilation, so instead we'll use
    // a construct that might trigger OpenMP-specific warnings/errors
    int x = 0;
    // Using invalid modifier to potentially trigger diagnostic
    // (though the real uncovered lines are for valid modifiers)
    #pragma omp task depend(unknown: x)  // This might trigger a warning
    {
        x = 1;
    }
}

int main() {
    TestClass obj;
    obj.member_var = 10;
    
    // Allocate for global pointer
    global_ptr = new int[3];
    
    // Call functions to create depend clauses
    obj.member_function();
    complex_depend_graph();
    
    // Use template
    int result = template_function(5);
    
    // Perform actual computation to verify correctness
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 100; i++) {
        sum += i;
    }
    
    // Cleanup
    delete[] global_ptr;
    
    return 0;
}
```
