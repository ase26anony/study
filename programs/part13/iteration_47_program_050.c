/* Compile with: g++ -O2 -fopenmp -std=c++11 -fdump-tree-all tree_test.cc -o tree_test */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* External function to prevent optimization */
extern void opaque_external_function(int*);

/* Global identifiers (IDENTIFIER_NODE) */
int global_counter = 0;
static int static_hidden = 42;
volatile int volatile_guard = 0;

/* Struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int values[4];
    double ratio;
    char tag;
};

/* Another struct for nested constructors */
struct NestedStruct {
    ComplexStruct inner;
    int id;
    short flags[2];
};

/* C++ classes for TREE_BINFO nodes */
class BaseClass {
public:
    virtual int virtual_method(int x) {
        return x * 2;
    }
    virtual ~BaseClass() {}
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int virtual_method(int x) override {
        return x * 3 + base_data;
    }
    int derived_data;
};

class SecondDerived : public DerivedClass {
public:
    virtual int virtual_method(int x) override {
        return x * 4 + base_data + derived_data;
    }
};

/* Recursive function returning struct (CONSTRUCTOR) */
ComplexStruct recursive_struct_builder(int depth, int seed) {
    ComplexStruct result;
    
    /* Initialize with constructor */
    result = {{seed, seed + 1, seed + 2, seed + 3}, 
              depth > 0 ? 1.0 / (depth + 1) : 1.0, 
              'A' + (depth % 26)};
    
    if (depth > 0) {
        ComplexStruct inner = recursive_struct_builder(depth - 1, seed * 2);
        /* Combine results */
        for (int i = 0; i < 4; i++) {
            result.values[i] += inner.values[i];
        }
        result.ratio *= inner.ratio;
    }
    
    return result;
}

/* Function with complex control flow for SSA_NAME and BLOCK nodes */
int complex_control_flow(int n, int* results) {
    int sum = 0;
    
    /* Outer block with local variables */
    {
        int block_local = n * 2;
        volatile_guard = block_local;
        
        /* Goto to create interesting control flow */
        if (n % 3 == 0) {
            goto special_case;
        }
        
        /* Normal path */
        for (int i = 0; i < n; i++) {
            /* Multiple assignments for SSA */
            int temp;
            if (i % 2 == 0) {
                temp = i * 3;
            } else {
                temp = i * 5;
            }
            
            /* Another SSA opportunity */
            int final_val;
            if (temp > n) {
                final_val = temp - n;
            } else {
                final_val = temp + n;
            }
            
            results[i] = final_val;
            sum += final_val;
        }
        
        goto end_block;
        
    special_case:
        /* Different block reached by goto */
        {
            int hidden_var = 77;
            for (int i = 0; i < n; i++) {
                results[i] = hidden_var + i;
                sum += results[i];
            }
        }
    }
    
end_block:
    return sum;
}

/* Template for TREE_VEC generation */
template<typename T, int N>
class FixedVector {
    T data[N];
public:
    FixedVector() {
        for (int i = 0; i < N; i++) {
            data[i] = T();
        }
    }
    
    T& operator[](int index) { return data[index]; }
    
    /* Method that should generate TREE_VEC nodes */
    FixedVector<T, N> operator+(const FixedVector<T, N>& other) {
        FixedVector<T, N> result;
        for (int i = 0; i < N; i++) {
            result[i] = data[i] + other.data[i];
        }
        return result;
    }
};

int main(int argc, char** argv) {
    /* Use argc to prevent optimization */
    int iterations = argc > 1 ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    if (iterations > 1000) iterations = 1000;
    
    /* CONSTRUCTOR nodes - various initializations */
    ComplexStruct cs1 = {{1, 2, 3, 4}, 2.5, 'X'};
    ComplexStruct cs2 = {{[0] = 10, [2] = 30, [3] = 40}, 3.14, 'Y'};
    
    /* Array with designated initializer (more CONSTRUCTOR) */
    int designated_array[10] = {[1] = 100, [5] = 500, [9] = 900};
    
    /* Nested struct initialization */
    NestedStruct ns = {{{5, 6, 7, 8}, 1.618, 'Z'}, 1234, {1, 2}};
    
    /* Call recursive function for CONSTRUCTOR nodes */
    ComplexStruct recursive_result = recursive_struct_builder(3, 1);
    
    /* BLOCK nodes - nested blocks with local variables */
    {
        int block_var1 = 42;
        {
            int block_var2 = block_var1 * 2;
            volatile_guard = block_var2;
            
            /* Another nested block */
            {
                static int static_in_block = 99;
                opaque_external_function(&static_in_block);
            }
        }
        
        /* goto creating cross-block flow */
        if (volatile_guard > 50) {
            goto skip_part;
        }
        
        int skipped_var = 777;
        volatile_guard = skipped_var;
        
    skip_part:
        /* Label in middle of block */
        int after_goto = 888;
        volatile_guard = after_goto;
    }
    
    /* SSA_NAME generation - complex conditional assignments */
    int* dynamic_results = new int[iterations];
    int ssa_test = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* This creates phi nodes in SSA form */
        int conditional;
        if (i % 3 == 0) {
            conditional = i * 2;
        } else if (i % 3 == 1) {
            conditional = i * 3;
        } else {
            conditional = i * 5;
        }
        
        /* Another SSA variable with multiple assignments */
        int multi_assign;
        if (conditional > iterations / 2) {
            multi_assign = conditional - iterations;
        } else {
            multi_assign = conditional + iterations;
        }
        
        /* Use in expression to keep alive */
        ssa_test += multi_assign;
        dynamic_results[i] = multi_assign;
    }
    
    /* Complex control flow function */
    int flow_result = complex_control_flow(iterations / 2, dynamic_results);
    
    /* OpenMP region with multiple clauses (OMP_CLAUSE nodes) */
    int openmp_sum = 0;
    int openmp_product = 1;
    
    #pragma omp parallel if(iterations > 50) \
                num_threads(4) \
                default(none) \
                shared(dynamic_results, iterations, openmp_sum) \
                private(openmp_product) \
                reduction(+:openmp_sum)
    {
        int thread_id = omp_get_thread_num();
        
        #pragma omp for schedule(dynamic, 4) \
                    collapse(2) \
                    nowait
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                int idx = i * 10 + j;
                if (idx < iterations) {
                    openmp_sum += dynamic_results[idx] + thread_id;
                }
            }
        }
        
        /* Nested OpenMP directive */
        #pragma omp master
        {
            #pragma omp task if(0) firstprivate(thread_id)
            {
                volatile_guard = thread_id;
            }
        }
    }
    
    /* More OpenMP with different clauses */
    #pragma omp parallel sections private(ssa_test) \
                                 lastprivate(flow_result)
    {
        #pragma omp section
        {
            ssa_test = 1;
            flow_result = ssa_test * 100;
        }
        
        #pragma omp section
        {
            ssa_test = 2;
            flow_result = ssa_test * 200;
        }
    }
    
    /* C++ specific: TREE_BINFO and TREE_VEC nodes */
    BaseClass* poly_obj;
    if (iterations % 2 == 0) {
        poly_obj = new DerivedClass();
    } else {
        poly_obj = new SecondDerived();
    }
    
    poly_obj->base_data = iterations;
    if (DerivedClass* derived = dynamic_cast<DerivedClass*>(poly_obj)) {
        derived->derived_data = iterations * 2;
    }
    
    int virtual_result = poly_obj->virtual_method(iterations);
    
    /* Template instantiation for TREE_VEC */
    FixedVector<int, 8> vec1, vec2;
    for (int i = 0; i < 8; i++) {
        vec1[i] = i * 2;
        vec2[i] = i * 3;
    }
    
    FixedVector<int, 8> vec3 = vec1 + vec2;
    
    /* Use all results to prevent optimization */
    int final_result = 
        cs1.values[0] + cs2.values[1] + 
        designated_array[1] + 
        ns.inner.values[2] + 
        recursive_result.values[3] + 
        ssa_test + 
        flow_result + 
        openmp_sum + 
        virtual_result + 
        vec3[0];
    
    printf("Final checksum: %d\n", final_result);
    
    delete[] dynamic_results;
    delete poly_obj;
    
    return final_result % 256;
}
