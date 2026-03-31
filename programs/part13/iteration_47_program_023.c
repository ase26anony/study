Here's a comprehensive test program that targets all the uncovered tree node types:

```cpp
// Compile with: g++ -O2 -fopenmp -std=c++11 tree_coverage.cc -o tree_coverage
// For C++ BINFO coverage, or use gcc for C without BINFO

#include <cstdio>
#include <cstdlib>

// External function to prevent optimization
extern "C" void opaque_external_function(int*);

// Global identifiers (IDENTIFIER_NODE)
extern int global_extern_var;
static int global_static_var;
int global_regular_var;

// Complex struct for CONSTRUCTOR nodes
struct ComplexStruct {
    int a;
    double b;
    char c;
    int* d;
};

// C++ classes for TREE_BINFO (inheritance hierarchy)
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int virtual_method(int x) { return x * 2; }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int virtual_method(int x) override { return x * 3; }
    int derived_data;
};

class AnotherDerived : public BaseClass {
public:
    virtual int virtual_method(int x) override { return x * 4; }
};

// Recursive function returning struct (CONSTRUCTOR)
ComplexStruct recursive_struct_builder(int depth, int seed) {
    ComplexStruct result;
    result.a = seed;
    result.b = seed * 1.5;
    result.c = 'A' + (seed % 26);
    
    static int static_data = 42;
    result.d = &static_data;
    
    if (depth > 0) {
        ComplexStruct inner = recursive_struct_builder(depth - 1, seed + 1);
        result.a += inner.a;
        result.b += inner.b;
    }
    
    return result; // CONSTRUCTOR node for return value
}

// Function with complex control flow for SSA_NAME and BLOCK
int complex_control_flow(int n, int* results) {
    int sum = 0;
    
    // Outer block with local variables
    {
        int block_local_1 = n * 2;
        volatile int volatile_var = block_local_1; // Prevent optimization
        
        // Goto to create interesting control flow
        if (n > 100) goto special_case;
        
        // Loop with conditional for SSA_NAME
        for (int i = 0; i < n; i++) {
            int temp;
            if (i % 2 == 0) {
                temp = i * 3; // SSA_NAME phi node candidate
            } else {
                temp = i * 5; // SSA_NAME phi node candidate
            }
            
            // Use temp in a way that requires phi node
            results[i] = temp + volatile_var;
            sum += results[i];
        }
        
        goto normal_exit;
        
    special_case:
        // Different block reached by goto
        {
            int hidden_var = 777;
            for (int i = 0; i < 10; i++) {
                results[i] = hidden_var + i;
                sum += results[i];
            }
        }
        
    normal_exit:
        // Empty label for goto target
        ;
    }
    
    // Another nested block
    {
        int block_local_2 = sum % 100;
        sum += block_local_2 * 2;
    }
    
    return sum;
}

// Template for TREE_VEC generation
template<typename T, int N>
struct TemplateVec {
    T data[N];
    
    TemplateVec() {
        for (int i = 0; i < N; i++) {
            data[i] = T();
        }
    }
};

int main(int argc, char** argv) {
    // Use argc to prevent compile-time optimization
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    
    // CONSTRUCTOR nodes: Array and struct initializers
    int array_init[5] = {[0] = 1, [2] = 3, [4] = 5}; // Designated initializer
    ComplexStruct struct_init = {.a = 10, .b = 20.5, .c = 'X', .d = &iterations};
    
    // TREE_VEC through template instantiation
    TemplateVec<int, 10> int_vec;
    TemplateVec<double, 5> double_vec;
    
    // Complex array initializer (may create TREE_VEC)
    int matrix[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    
    // Results array
    int* results = new int[iterations];
    
    // Call function with complex control flow
    int control_flow_result = complex_control_flow(iterations, results);
    
    // Recursive struct building
    ComplexStruct recursive_result = recursive_struct_builder(5, 1);
    
    // OpenMP region with multiple clauses for OMP_CLAUSE nodes
    int openmp_sum = 0;
    #pragma omp parallel for reduction(+:openmp_sum) \
            private(iterations) firstprivate(control_flow_result) \
            shared(results) schedule(dynamic, 4) collapse(1) \
            num_threads(2) if(iterations > 50)
    for (int i = 0; i < iterations; i++) {
        int local_iter = iterations; // Private copy
        int local_sum = 0;
        
        // Nested OpenMP for more clauses
        #pragma omp simd reduction(+:local_sum) linear(i:1)
        for (int j = 0; j < 10; j++) {
            local_sum += results[i] + j + local_iter;
        }
        
        openmp_sum += local_sum;
    }
    
    // More OpenMP directives
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(inout: openmp_sum)
            {
                openmp_sum += 1000;
            }
        }
    }
    
    // C++ polymorphism for TREE_BINFO
    BaseClass* poly_obj;
    if (openmp_sum % 2 == 0) {
        poly_obj = new DerivedClass();
    } else {
        poly_obj = new AnotherDerived();
    }
    
    // Virtual call through BINFO
    int poly_result = poly_obj->virtual_method(control_flow_result % 10);
    
    // Call external function with various identifiers
    opaque_external_function(&poly_result);
    
    // Use all computed values to prevent dead code elimination
    int final_result = control_flow_result 
                     + openmp_sum 
                     + poly_result 
                     + recursive_result.a
                     + array_init[0]
                     + matrix[1][1]
                     + int_vec.data[0];
    
    // Use global identifiers
    global_regular_var = final_result;
    global_static_var = final_result % 100;
    
    printf("Final result: %d\n", final_result);
    
    delete[] results;
    delete poly_obj;
    
    return final_result % 256;
}

// Dummy implementation of external function
extern "C" void opaque_external_function(int* x) {
    *x = (*x) * 2 + 1;
}

// Initialize globals
int global_extern_var = 1;
int global_static_var = 2;
int global_regular_var = 3;
```

**Key features targeting specific tree nodes:**

1. **IDENTIFIER_NODE**: Multiple global variables with different storage classes (`extern`, `static`, regular), function names, and parameter names.

2. **TREE_VEC**: Template instantiations (`TemplateVec<int,10>`, `TemplateVec<double,5>`), multi-dimensional array initializers, and designated array initializers.

3. **TREE_BINFO**: C++ class hierarchy with `BaseClass`, `DerivedClass`, `AnotherDerived`, virtual methods, and polymorphic usage through base class pointer.

4. **SSA_NAME**: Complex control flow in `complex_control_flow()` with conditional assignments inside loops, variables used after conditional blocks requiring phi nodes.

5. **BLOCK**: Multiple nested blocks using `{}`, `goto` statements jumping between blocks, local variables declared inside blocks.

6. **CONSTRUCTOR**: Struct initializers with designated initializers, array initializers, return values from `recursive_struct_builder()`.

7. **OMP_CLAUSE**: Multiple OpenMP pragmas with various clauses (`reduction`, `private`, `firstprivate`, `shared`, `schedule`, `collapse`, `num_threads`, `if`, `depend`, `simd`, `linear`).

**Compilation recommendations:**
- For full C++ coverage with BINFO: `g++ -O2 -fopenmp -std=c++11 -fdump-tree-all tree_coverage.cc`
- For C coverage (no BINFO): Compile as C and remove C++-specific features
- Add `-fdump-tree-*` flags to trigger more tree inspection code paths

The program uses command-line arguments, volatile variables, external function calls, and complex control flow to ensure code survives optimization and reaches the middle-end passes where the tree node inspection occurs.
