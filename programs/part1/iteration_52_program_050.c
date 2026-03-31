```cpp
// Test program to cover OpenMP depend clause modifiers in GCC C++ pretty-printer
// Compile with: g++ -O1 -fopenmp -fdump-tree-omp -c -xc++ this_file.cc
// Or: g++ -O2 -fopenmp -Werror=openmp-format -c -xc++ this_file.cc
// Or: g++ -O0 -g -fopenmp -fdump-tree-all -save-temps -xc++ this_file.cc

// Global variables with different linkages
int global_var = 0;
static int static_global_var = 0;
extern int extern_global_var;

// Class to test member function scoping
class TestClass {
public:
    int member_var;
    static int static_member;
    
    void member_function();
};

int TestClass::static_member = 0;

// Heap-allocated data
int* heap_array = new int[10];
struct Data {
    int x;
    int y;
    int* ptr;
};
Data* data_obj = new Data{0, 0, nullptr};

// Function with deliberate syntax error to trigger diagnostics
// This will force the compiler to engage its diagnostic machinery
void function_with_error() {
    // Deliberate syntax error: undeclared type in unused parameter
    // This won't affect runtime but will trigger compiler diagnostics
    #ifdef TRIGGER_ERROR
    void foo(UndefinedType x);  // This type is not defined
    #endif
}

void TestClass::member_function() {
    int local_var = 0;
    static int static_local = 0;
    int* local_ptr = new int(5);
    
    // Array for set-based dependencies
    int local_array[5] = {0};
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            // Task with depend(in) - reading from initialized variable
            #pragma omp task depend(in: global_var) priority(1)
            {
                int read_val = global_var;
                static_local = read_val + 1;
            }
            
            // Task with depend(inout) - updating shared accumulator
            #pragma omp task depend(inout: static_global_var) mergeable
            {
                static_global_var += 10;
            }
            
            // Task with depend(out) - producing new value
            #pragma omp task depend(out: *local_ptr) final(0)
            {
                *local_ptr = 100;
            }
            
            // Task with depend(mutexinoutset) - set-based dependency
            #pragma omp task depend(mutexinoutset: local_array[2])
            {
                local_array[2] = 200;
            }
            
            // Task with depend(inoutset) - another set-based dependency
            #pragma omp task depend(inoutset: local_array[3])
            {
                local_array[3] = 300;
            }
            
            // Task with depend(destroy) - cleanup operation
            #pragma omp task depend(destroy: heap_array[0])
            {
                // Simulate cleanup
                heap_array[0] = -1;
            }
            
            // More complex examples with references and pointers
            int& ref = member_var;
            #pragma omp task depend(in: ref) depend(inout: data_obj->x)
            {
                data_obj->x = ref + 1;
            }
            
            // Nested task with dependencies
            #pragma omp taskgroup
            {
                #pragma omp task depend(inout: TestClass::static_member) \
                                 depend(out: data_obj->y)
                {
                    TestClass::static_member++;
                    data_obj->y = TestClass::static_member;
                }
                
                // Dependent task chain
                #pragma omp task depend(in: data_obj->y) \
                                 depend(inout: heap_array[1])
                {
                    heap_array[1] = data_obj->y * 2;
                }
            }
            
            // Taskwait to ensure dependencies are respected
            #pragma omp taskwait
            
            // Array element dependencies in loop
            for (int i = 0; i < 3; i++) {
                #pragma omp task depend(inout: local_array[i]) \
                                 depend(in: heap_array[i])
                {
                    local_array[i] += heap_array[i];
                }
            }
            
            // Multiple dependencies in one clause
            #pragma omp task depend(in: global_var, static_global_var) \
                             depend(out: heap_array[9])
            {
                heap_array[9] = global_var + static_global_var;
            }
        }
    }
    
    delete local_ptr;
}

int main() {
    // Initialize global data
    extern_global_var = 42;
    
    TestClass obj;
    obj.member_var = 10;
    obj.member_function();
    
    // Perform computation using all depend types
    int sum = 0;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            // Initialize array with depend(out)
            #pragma omp task depend(out: heap_array[0:5])
            {
                for (int i = 0; i < 5; i++) {
                    heap_array[i] = i + 1;
                }
            }
            
            // Process with depend(in)
            #pragma omp task depend(in: heap_array[0:5]) \
                             depend(out: heap_array[5])
            {
                int partial_sum = 0;
                for (int i = 0; i < 5; i++) {
                    partial_sum += heap_array[i];
                }
                heap_array[5] = partial_sum;
            }
            
            // Update with depend(inout)
            #pragma omp task depend(inout: heap_array[5])
            {
                heap_array[5] *= 2;
            }
            
            // Set-based operations
            #pragma omp task depend(mutexinoutset: heap_array[6], heap_array[7])
            {
                heap_array[6] = 100;
                heap_array[7] = 200;
            }
            
            #pragma omp task depend(inoutset: heap_array[6], heap_array[7])
            {
                heap_array[6] += 50;
                heap_array[7] += 50;
            }
            
            // Finalize with depend(destroy) on a specific element
            #pragma omp task depend(destroy: heap_array[8])
            {
                heap_array[8] = 999;
            }
            
            #pragma omp taskwait
            
            // Verify results
            sum = heap_array[5] + heap_array[6] + heap_array[7] + heap_array[8];
        }
    }
    
    // Print result to prevent optimization removal
    // Note: This won't execute if compilation fails due to our syntax error,
    // but that's fine since we're testing the pretty-printer during compilation
    #ifndef TRIGGER_ERROR
    printf("Result: %d\n", sum);
    #endif
    
    // Cleanup
    delete[] heap_array;
    delete data_obj;
    
    return 0;
}

// Force instantiation of error-triggering function
// This ensures the compiler sees the syntax error
void trigger_diagnostics() {
    function_with_error();
}
```
