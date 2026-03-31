/* Compile with: g++ -O2 -fopenmp -std=c++11 -fdump-tree-all tree_test.cc -o tree_test */

#include <cstdio>
#include <cstdlib>
#include <algorithm>

/* External function to prevent optimization */
extern "C" void opaque_external_function(int*);

/* Global identifiers for IDENTIFIER_NODE coverage */
int global_var_1 = 10;
static int static_var_2 = 20;
extern int extern_var_3;

/* Complex struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    double b;
    char c;
    int* d;
};

/* Nested struct for more complex constructors */
struct NestedStruct {
    ComplexStruct inner;
    float extra[3];
};

/* Class hierarchy for TREE_BINFO nodes */
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

class SecondDerived : public DerivedClass {
public:
    virtual int virtual_method(int x) override { return x * 4; }
    int second_data;
};

/* Recursive function returning struct (CONSTRUCTOR nodes) */
ComplexStruct recursive_struct_builder(int depth, int seed) {
    ComplexStruct result;
    result.a = seed;
    result.b = seed * 1.5;
    result.c = (char)(seed % 256);
    
    static int local_static = 100;
    result.d = &local_static;
    
    if (depth > 0) {
        ComplexStruct inner = recursive_struct_builder(depth - 1, seed + 1);
        result.a += inner.a;
        result.b += inner.b;
    }
    
    return result;  /* CONSTRUCTOR node for return value */
}

/* Function with complex control flow for SSA_NAME and BLOCK nodes */
int complex_control_flow(int n, int* results) {
    int sum = 0;
    
    /* Outer block with local variables */
    {
        int block_local_1 = n * 2;
        volatile int volatile_var = block_local_1;  /* Prevent optimization */
        
    mid_block_label:
        /* Nested block */
        {
            int nested_local = volatile_var + 10;
            
            /* Conditional with phi node potential */
            int phi_candidate;
            if (nested_local > 50) {
                phi_candidate = nested_local * 2;
                goto skip_part;  /* Jump to create interesting CFG */
            } else {
                phi_candidate = nested_local / 2;
            }
            
            /* Unreachable code that creates additional blocks */
            {
                int unreachable_var = 999;
                results[0] = unreachable_var;
            }
            
        skip_part:
            /* Use phi_candidate to ensure SSA_NAME creation */
            sum = phi_candidate + volatile_var;
        }
        
        /* Another block with goto */
        if (sum < 100) {
            goto mid_block_label;
        }
    }
    
    return sum;
}

/* Function using TREE_VEC nodes (template instantiation) */
template<typename T, size_t N>
struct FixedVector {
    T data[N];
    
    void initialize_with_designators() {
        /* This may generate TREE_VEC nodes during compilation */
        for (size_t i = 0; i < N; ++i) {
            data[i] = T(i * 2);
        }
    }
};

int main(int argc, char** argv) {
    /* Use argc to prevent compile-time optimization */
    int iterations = argc > 1 ? atoi(argv[1]) : 100;
    int seed = argc > 2 ? atoi(argv[2]) : 42;
    
    /* CONSTRUCTOR nodes - aggregate initialization */
    ComplexStruct cs = {1, 2.5, 'A', &global_var_1};
    NestedStruct ns = {{2, 3.14, 'B', &static_var_2}, {1.1f, 2.2f, 3.3f}};
    
    /* Array with designator (may create TREE_VEC) */
    int designated_array[10] = {[0] = 1, [5] = 2, [9] = 3};
    
    /* TREE_VEC through template instantiation */
    FixedVector<int, 8> vec1;
    FixedVector<double, 4> vec2;
    vec1.initialize_with_designators();
    vec2.initialize_with_designators();
    
    /* TREE_BINFO through class hierarchy */
    BaseClass* base_ptr;
    DerivedClass derived_obj;
    SecondDerived second_obj;
    
    /* Dynamic dispatch to engage BINFO logic */
    if (seed % 2 == 0) {
        base_ptr = &derived_obj;
    } else {
        base_ptr = &second_obj;
    }
    
    int virtual_result = base_ptr->virtual_method(iterations);
    
    /* BLOCK and SSA_NAME through complex control flow */
    int* dynamic_results = new int[iterations];
    int control_flow_result = complex_control_flow(iterations, dynamic_results);
    
    /* CONSTRUCTOR from recursive function */
    ComplexStruct recursive_result = recursive_struct_builder(5, seed);
    
    /* OpenMP region with multiple clauses for OMP_CLAUSE nodes */
    int openmp_sum = 0;
    int openmp_product = 1;
    
    #pragma omp parallel for reduction(+:openmp_sum) \
        reduction(*:openmp_product) private(seed) \
        firstprivate(iterations) shared(dynamic_results) \
        schedule(dynamic, 4) collapse(2) if(iterations > 50)
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            int local_seed = seed + i * j;
            
            /* Nested OpenMP directive with more clauses */
            #pragma omp atomic update
            openmp_sum += local_seed;
            
            #pragma omp critical
            {
                openmp_product *= (local_seed % 10 + 1);
            }
        }
    }
    
    /* Additional OpenMP constructs */
    #pragma omp parallel
    {
        #pragma omp sections private(seed)
        {
            #pragma omp section
            {
                seed = 1;
            }
            #pragma omp section
            {
                seed = 2;
            }
        }
        
        #pragma omp single copyprivate(seed)
        {
            seed = openmp_sum % 100;
        }
    }
    
    /* Call external function with various identifiers */
    opaque_external_function(&global_var_1);
    opaque_external_function(&static_var_2);
    opaque_external_function(dynamic_results);
    
    /* Use all computed values to prevent dead code elimination */
    int final_result = virtual_result + control_flow_result + 
                      recursive_result.a + openmp_sum + 
                      (openmp_product % 1000) + designated_array[0];
    
    printf("Final checksum: %d\n", final_result);
    
    delete[] dynamic_results;
    return final_result != 0 ? 0 : 1;
}

/* Dummy definition to satisfy linker (not called in practice) */
extern "C" void opaque_external_function(int* ptr) {
    *ptr += 1;
}
