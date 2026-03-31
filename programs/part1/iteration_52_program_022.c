```cpp
// Test program to cover OpenMP depend clause modifiers in GCC C++ pretty-printer
// Compile with: g++ -O1 -fopenmp -fdump-tree-omp -c -xc++ this_file.cpp
// Or: g++ -O2 -fopenmp -Werror=openmp-format -c -xc++ this_file.cpp
// Or: g++ -O0 -g -fopenmp -fdump-tree-all -save-temps -xc++ this_file.cpp

// Global variables with different linkages
int global_var = 0;
static int static_global_var = 0;
int* global_ptr = nullptr;

// Class to test member function scoping
class TestClass {
public:
    int member_var;
    static int static_member;
    
    void member_function();
    
    // Deliberate syntax error to trigger diagnostics
    // This will force the compiler to engage its diagnostic machinery
    void error_function(UndefinedType x); // ERROR: UndefinedType not declared
};

int TestClass::static_member = 0;

// Heap-allocated data
struct Data {
    int values[10];
    int* ptr;
};

// Function with static local
void process_with_static() {
    static int static_local = 0;
    // Use in depend clause
    #pragma omp task depend(in: static_local)
    {
        static_local += 1;
    }
}

void TestClass::member_function() {
    // Array for set-based dependencies
    int local_array[5] = {0};
    Data* heap_data = new Data{};
    
    // Enter parallel region
    #pragma omp parallel
    {
        #pragma omp single
        {
            // Task with depend(in) - reads initialization
            #pragma omp task depend(in: global_var) priority(1)
            {
                // Read operation
                int read_val = global_var;
            }
            
            // Task with depend(inout) - updates accumulator
            #pragma omp task depend(inout: static_global_var) mergeable
            {
                static_global_var += 5;
            }
            
            // Task with depend(out) - produces result
            #pragma omp task depend(out: member_var) final(0)
            {
                member_var = 100;
            }
            
            // Tasks with mutexinoutset on array elements
            #pragma omp task depend(mutexinoutset: local_array[0])
            {
                local_array[0] = 1;
            }
            
            #pragma omp task depend(mutexinoutset: local_array[1])
            {
                local_array[1] = 2;
            }
            
            // Task with inoutset on heap data
            #pragma omp task depend(inoutset: heap_data->values[2])
            {
                heap_data->values[2] = 3;
            }
            
            #pragma omp task depend(inoutset: heap_data->values[3])
            {
                heap_data->values[3] = 4;
            }
            
            // Task with pointer dereference in depend clause
            int local_int = 0;
            int* local_ptr = &local_int;
            #pragma omp task depend(in: *local_ptr)
            {
                // Read through pointer
                int val = *local_ptr;
            }
            
            // Task with depend(destroy) for cleanup
            #pragma omp task depend(destroy: heap_data->ptr)
            {
                delete heap_data->ptr;
                heap_data->ptr = nullptr;
            }
            
            // Taskwait to ensure dependencies are respected
            #pragma omp taskwait
            
            // Nested task with combined clauses
            #pragma omp task depend(inout: TestClass::static_member) priority(2)
            {
                TestClass::static_member++;
                
                // Inner task with depend(in)
                #pragma omp task depend(in: TestClass::static_member)
                {
                    int inner_val = TestClass::static_member;
                }
            }
            
            // Taskgroup for structured cleanup
            #pragma omp taskgroup
            {
                #pragma omp task depend(destroy: global_ptr)
                {
                    delete global_ptr;
                    global_ptr = nullptr;
                }
            }
        }
    }
    
    delete heap_data;
}

// Function with reference parameter
void process_reference(int& ref_var) {
    #pragma omp task depend(inout: ref_var)
    {
        ref_var *= 2;
    }
}

int main() {
    TestClass obj;
    obj.member_var = 0;
    
    // Initialize global pointer
    global_ptr = new int(42);
    
    // Call member function with various depend clauses
    obj.member_function();
    
    // Process with reference
    int ref_var = 10;
    #pragma omp parallel
    {
        #pragma omp single
        {
            process_reference(ref_var);
            #pragma omp taskwait
        }
    }
    
    // Process with static local
    process_with_static();
    
    // Wait for all tasks
    #pragma omp taskwait
    
    // Perform verifiable calculation
    int array[4] = {1, 2, 3, 4};
    int sum = 0;
    
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 4; i++) {
        sum += array[i];
    }
    
    // Print result to ensure code executes
    // (though compilation may fail due to syntax error)
    // printf("Sum: %d\n", sum); // Commented to avoid requiring stdio
    
    // Cleanup
    delete global_ptr;
    
    return 0;
}

// Additional function with complex depend graph
void complex_dependency_graph() {
    int a = 0, b = 0, c = 0, d = 0;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            // Create a diamond dependency pattern
            #pragma omp task depend(out: a)
            { a = 1; }
            
            #pragma omp task depend(in: a) depend(out: b)
            { b = a + 1; }
            
            #pragma omp task depend(in: a) depend(out: c)
            { c = a + 2; }
            
            #pragma omp task depend(in: b, c) depend(out: d)
            { d = b + c; }
            
            #pragma omp task depend(in: d) depend(destroy: a)
            { /* cleanup a */ }
        }
    }
}
```
