/* Compile with: g++ -O2 -fopenmp -std=c++11 -fdump-tree-all tree_test.cc -o tree_test */

#include <cstdio>
#include <cstdlib>

/* External function to prevent optimization */
extern "C" void opaque_external_function(int*);

/* Global identifiers for IDENTIFIER_NODE coverage */
int global_identifier_1;
static int static_identifier_2;
extern int extern_identifier_3;

/* Struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    double b;
    char c;
    int* d;
};

/* Array for TREE_VEC representation */
int multi_dim_array[2][3][4] = {
    { {1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12} },
    { {13, 14, 15, 16}, {17, 18, 19, 20}, {21, 22, 23, 24} }
};

/* C++ classes for TREE_BINFO coverage */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int virtual_method(int x) {
        return x * 2;
    }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int virtual_method(int x) override {
        return x * 3 + base_data;
    }
    int derived_data;
};

/* Recursive function returning struct (CONSTRUCTOR) */
ComplexStruct recursive_struct_builder(int depth, int seed) {
    ComplexStruct result;
    result.a = seed;
    result.b = seed * 1.5;
    result.c = 'A' + (seed % 26);
    result.d = &result.a;
    
    if (depth > 0) {
        ComplexStruct nested = recursive_struct_builder(depth - 1, seed + 1);
        result.a += nested.a;
        result.b += nested.b;
    }
    
    return result;  /* CONSTRUCTOR node for return value */
}

/* Function with complex control flow for SSA_NAME and BLOCK coverage */
int complex_control_flow(int n, int* results) {
    int ssa_var = 0;  /* Will become SSA_NAME */
    
    /* Outer block with local variables */
    {
        int block_local_1 = n * 2;
        volatile int volatile_var = block_local_1;  /* Prevent optimization */
        
        /* Nested block with goto */
        {
            int block_local_2 = 0;
            
            if (n % 3 == 0) {
                goto skip_init;  /* Jump to label */
            }
            
            block_local_2 = n * 3;
            
        skip_init:
            /* Label inside block */
            ssa_var = block_local_2 + volatile_var;
        }
    }
    
    /* Loop with conditional assignment for SSA phi nodes */
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            ssa_var += i * 2;      /* Different assignment in one path */
        } else {
            ssa_var += i * 3 + 1;  /* Different assignment in other path */
        }
        
        /* Use ssa_var to prevent elimination */
        results[i] = ssa_var;
        
        /* Another block with local variable */
        {
            int temp = ssa_var % 100;
            if (temp < 50) {
                ssa_var -= temp;
            }
        }
    }
    
    return ssa_var;
}

/* OpenMP function with multiple clauses */
int openmp_reduction_example(int size) {
    int sum = 0;
    int product = 1;
    
    /* Complex OpenMP region with multiple clauses */
    #pragma omp parallel \
        default(none) \
        shared(size, multi_dim_array) \
        private(global_identifier_1) \
        firstprivate(static_identifier_2) \
        reduction(+:sum) \
        reduction(*:product) \
        collapse(2) \
        schedule(dynamic, 4)
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            int local_sum = 0;
            for (int k = 0; k < 4; k++) {
                local_sum += multi_dim_array[i][j][k];
            }
            sum += local_sum;
            product *= (local_sum > 50) ? local_sum : 1;
        }
    }
    
    return sum + product;
}

/* Template for additional TREE_VEC coverage */
template<typename T, int N>
class FixedVector {
    T data[N];
public:
    T& operator[](int index) { return data[index]; }
    const T& operator[](int index) const { return data[index]; }
};

int main(int argc, char** argv) {
    /* Use argc to prevent compile-time optimization */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    int seed = (argc > 2) ? atoi(argv[2]) : 42;
    
    /* Initialize global identifiers */
    global_identifier_1 = seed;
    static_identifier_2 = seed * 2;
    
    /* CONSTRUCTOR: Aggregate initialization */
    ComplexStruct initialized_struct = {
        .a = 10,
        .b = 20.5,
        .c = 'X',
        .d = &global_identifier_1
    };
    
    /* Another CONSTRUCTOR with designators */
    int designated_array[10] = {[0] = 1, [5] = 2, [9] = 3};
    
    /* TREE_BINFO: C++ class hierarchy usage */
    DerivedClass derived_obj;
    derived_obj.base_data = 100;
    derived_obj.derived_data = 200;
    
    BaseClass* base_ptr = &derived_obj;
    int virtual_result = base_ptr->virtual_method(iterations);
    
    /* Dynamic cast for additional BINFO usage */
    DerivedClass* casted_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (casted_ptr) {
        casted_ptr->derived_data = virtual_result;
    }
    
    /* TREE_VEC: Template instantiation */
    FixedVector<int, 10> vec_template;
    for (int i = 0; i < 10; i++) {
        vec_template[i] = i * seed;
    }
    
    /* Complex control flow for SSA_NAME and BLOCK */
    int* dynamic_results = new int[iterations];
    int control_flow_result = complex_control_flow(iterations, dynamic_results);
    
    /* Recursive struct building */
    ComplexStruct recursive_result = recursive_struct_builder(5, seed);
    
    /* OpenMP with multiple clauses */
    int openmp_result = openmp_reduction_example(iterations);
    
    /* Call external function with various identifiers */
    opaque_external_function(&global_identifier_1);
    opaque_external_function(&static_identifier_2);
    opaque_external_function(&extern_identifier_3);
    opaque_external_function(dynamic_results);
    
    /* Compute final checksum to ensure all code is live */
    int final_checksum = virtual_result + 
                        control_flow_result + 
                        openmp_result + 
                        recursive_result.a + 
                        vec_template[0] +
                        designated_array[0] +
                        initialized_struct.a;
    
    printf("Final checksum: %d\n", final_checksum);
    
    /* Cleanup */
    delete[] dynamic_results;
    
    return (final_checksum > 0) ? 0 : 1;
}

/* Dummy implementation to satisfy external reference */
extern "C" void opaque_external_function(int* ptr) {
    if (ptr) {
        *ptr += 1;
    }
}

/* External variable definition */
int extern_identifier_3 = 300;
