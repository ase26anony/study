```cpp
// Test program to cover OpenMP depend clause modifiers in GCC C++ pretty-printer
// Compile with: g++ -O1 -fopenmp -fdump-tree-omp -c -xc++ this_file.cpp
// This will generate dump files containing pretty-printed OpenMP clauses

// Global variables with different linkages
int global_var = 0;
static int static_global_var = 0;
extern int extern_var;

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
                // Task with depend(in) - reads from initialization
                #pragma omp task depend(in: local_var) priority(1)
                {
                    member_var = local_var + 1;
                }
                
                // Task with depend(inout) - updates accumulator
                #pragma omp task depend(inout: member_var) mergeable
                {
                    member_var += 10;
                }
                
                // Task with depend(out) - produces result
                #pragma omp task depend(out: static_local) final(member_var > 100)
                {
                    static_local = member_var * 2;
                }
                
                // Task with depend(mutexinoutset) - set-based dependency
                #pragma omp task depend(mutexinoutset: heap_ptr[2])
                {
                    heap_ptr[2] = static_local;
                }
                
                // Task with depend(inoutset) - another set-based dependency
                #pragma omp task depend(inoutset: heap_ptr[3])
                {
                    heap_ptr[3] = heap_ptr[2] + 5;
                }
                
                // Task with depend(destroy) - cleanup
                #pragma omp task depend(destroy: heap_ptr[4])
                {
                    heap_ptr[4] = -1;
                }
            }
        }
        
        delete[] heap_ptr;
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
            // Multiple tasks forming a dependency chain
            #pragma omp task depend(in: array[0]) depend(in: ref)
            {
                array[1] = array[0] + ref;
            }
            
            #pragma omp task depend(inout: array[1]) depend(out: *ptr)
            {
                *ptr = array[1] * 2;
            }
            
            #pragma omp task depend(inoutset: array[2]) depend(inoutset: array[3])
            {
                array[2] = *ptr + 1;
                array[3] = *ptr + 2;
            }
            
            #pragma omp task depend(mutexinoutset: array[4])
            {
                array[4] = array[2] + array[3];
            }
            
            #pragma omp task depend(destroy: array[9])
            {
                array[9] = 0;
            }
            
            #pragma omp taskwait
        }
    }
}

// Nested task constructs
void nested_tasks() {
    int a = 0, b = 0, c = 0;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup
            {
                #pragma omp task depend(in: a) shared(b)
                {
                    b = a + 1;
                }
                
                #pragma omp task depend(inout: b) shared(c)
                {
                    c = b * 2;
                }
                
                #pragma omp task depend(out: c)
                {
                    // Empty task just for depend(out) coverage
                }
            }
        }
    }
}

// Main function with computational pattern
int main() {
    TestClass obj;
    obj.member_var = 42;
    
    // Use all depend clause modifiers in a logical computation
    int data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int result = 0;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            // Initialize with depend(out)
            #pragma omp task depend(out: data[0])
            {
                data[0] = 100;
            }
            
            // Read with depend(in)
            #pragma omp task depend(in: data[0])
            {
                data[1] = data[0] + 10;
            }
            
            // Update with depend(inout)
            #pragma omp task depend(inout: data[1])
            {
                data[1] *= 2;
            }
            
            // Set-based operations
            #pragma omp task depend(inoutset: data[2]) depend(inoutset: data[3])
            {
                data[2] = data[1] + 1;
                data[3] = data[1] + 2;
            }
            
            #pragma omp task depend(mutexinoutset: data[4])
            {
                data[4] = data[2] + data[3];
            }
            
            // Final computation with depend(in) on multiple variables
            #pragma omp task depend(in: data[1]) depend(in: data[4]) \
                             depend(out: result)
            {
                result = data[1] + data[4];
            }
            
            // Cleanup with depend(destroy)
            #pragma omp task depend(destroy: data[7])
            {
                data[7] = 0;
            }
            
            #pragma omp taskwait
        }
    }
    
    // Call functions to trigger more pretty-printing contexts
    obj.member_function();
    complex_dependencies();
    nested_tasks();
    
    // Print result to prevent optimization
    printf("Result: %d\n", result);
    
    // DELIBERATE SYNTAX ERROR to force compiler diagnostics
    // This will cause the compiler to potentially pretty-print the OpenMP constructs
    // while reporting the error
    UndeclaredType x;  // This line will cause a compilation error
    
    return 0;
}

// Additional function with mixed OpenMP constructs
void mixed_clauses() {
    int x = 0, y = 0, z = 0;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(in: x) priority(5) if(false)
            {
                y = x + 1;
            }
            
            #pragma omp task depend(inout: y) mergeable
            {
                y *= 3;
            }
            
            #pragma omp task depend(out: z) final(y > 10)
            {
                z = y + 5;
            }
        }
    }
}
```
