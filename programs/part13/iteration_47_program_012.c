/* Test program to trigger tree_kind dispatch for various TREE_CODE cases */
#ifdef __cplusplus
#include <vector>
#include <iostream>
extern "C" {
#endif

/* External function to prevent optimization */
extern void opaque_external_function(int*);

/* Global identifiers (IDENTIFIER_NODE) */
static int static_global_counter = 0;
extern int external_global_data;
int global_sum = 0;

/* Struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    int b;
    int c[3];
};

/* Another struct for nested constructors */
struct NestedStruct {
    struct ComplexStruct inner;
    float f;
    double d;
};

#ifdef __cplusplus
/* C++ classes for TREE_BINFO generation */
class BaseClass {
public:
    virtual int virtual_method(int x) {
        return x * 2;
    }
    virtual ~BaseClass() {}
};

class DerivedClass : public BaseClass {
public:
    int virtual_method(int x) override {
        return x * 3;
    }
    
    template<typename T>
    T template_method(T value) {
        return value * 4;
    }
};

class SecondDerived : public DerivedClass {
public:
    int virtual_method(int x) override {
        return x * 5;
    }
};
#endif

/* Recursive function returning struct (CONSTRUCTOR) */
struct ComplexStruct recursive_struct_builder(int depth, int value) {
    struct ComplexStruct result;
    
    /* BLOCK node with local variable */
    {
        int local_block_var = value * 2;
        result.a = local_block_var;
        
        /* Jump to skip part of block */
        if (depth > 1) {
            goto skip_part;
        }
        
        int hidden_in_block = 100;
        result.b = hidden_in_block;
        
        skip_part:
        /* Still in the block */
        result.b = depth;
    }
    
    /* CONSTRUCTOR for array */
    result.c[0] = value;
    result.c[1] = value + depth;
    result.c[2] = value * depth;
    
    if (depth > 0) {
        struct ComplexStruct inner = recursive_struct_builder(depth - 1, value + 1);
        result.a += inner.a;
        result.b += inner.b;
    }
    
    return result;
}

/* Function with SSA_NAME generation */
int ssa_generator(int iterations, int condition) {
    int x = 0;
    int y = 0;
    int z = 0;
    
    /* Complex control flow for SSA */
    for (int i = 0; i < iterations; i++) {
        volatile int vol = condition; /* Prevent optimization */
        
        if (vol > 0) {
            x = i * 2;
            y = x + 1;
        } else {
            x = i * 3;
            y = x - 1;
        }
        
        /* This creates phi nodes */
        z = y + x;
        
        /* Nested condition for more SSA complexity */
        if (i % 2 == 0) {
            x = z * 2;
        } else {
            x = z * 3;
        }
        
        /* Use all variables to keep them live */
        global_sum += x + y + z;
    }
    
    return z;
}

int main(int argc, char* argv[]) {
    /* Use argc to prevent compile-time optimization */
    int iterations = argc > 1 ? 10 : 5;
    int use_openmp = argc > 2;
    int use_cpp_features = argc > 3;
    
    /* CONSTRUCTOR nodes with various initializers */
    struct ComplexStruct cs1 = {1, 2, {3, 4, 5}};
    struct ComplexStruct cs2 = {.a = 10, .c = {[1] = 20, [0] = 15, [2] = 25}};
    struct NestedStruct ns = {{6, 7, {8, 9, 10}}, 3.14f, 2.71828};
    
    /* Array with designated initializer (TREE_VEC-like representation) */
    int complex_array[5][3] = {
        [0] = {1, 2, 3},
        [2] = {[1] = 10, [0] = 9, [2] = 11},
        [4] = {7, 8, 9}
    };
    
    /* Call recursive function for CONSTRUCTOR nodes */
    struct ComplexStruct recursive_result = recursive_struct_builder(3, 1);
    
    /* Generate SSA_NAME nodes */
    int ssa_result = ssa_generator(iterations, argc);
    
    /* BLOCK nodes with goto */
    {
        int block_var1 = 100;
        goto middle_of_block;
        
        int skipped_var = 200; /* This may be skipped */
        
        middle_of_block:
        {
            int inner_block_var = 300;
            block_var1 += inner_block_var;
        }
        
        int after_label = 400;
        block_var1 += after_label;
        
        /* Use the variable */
        opaque_external_function(&block_var1);
    }
    
    /* OpenMP region for OMP_CLAUSE nodes */
    int openmp_sum = 0;
    int openmp_array[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        openmp_array[i] = i;
    }
    
    if (use_openmp) {
        #pragma omp parallel for private(iterations) firstprivate(openmp_array) \
                shared(openmp_sum) reduction(+:openmp_sum) collapse(2) \
                schedule(dynamic)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                int idx = i * 10 + j;
                /* Nested OpenMP directive */
                #pragma omp atomic
                openmp_sum += openmp_array[idx] * (argc + 1);
            }
        }
        
        /* Another OpenMP region with different clauses */
        #pragma omp parallel num_threads(4) default(none) \
                shared(openmp_sum, openmp_array)
        {
            #pragma omp for nowait
            for (int i = 0; i < 50; i++) {
                #pragma omp atomic
                openmp_sum -= openmp_array[i];
            }
        }
    }
    
#ifdef __cplusplus
    /* C++ specific features for TREE_BINFO */
    if (use_cpp_features) {
        /* Template instantiation (TREE_VEC) */
        std::vector<int> template_vec;
        template_vec.push_back(1);
        template_vec.push_back(2);
        template_vec.push_back(3);
        
        /* Class hierarchy for BINFO */
        BaseClass* poly_obj;
        if (argc % 2 == 0) {
            poly_obj = new DerivedClass();
        } else {
            poly_obj = new SecondDerived();
        }
        
        /* Virtual call through BINFO */
        int virtual_result = poly_obj->virtual_method(argc);
        
        /* Dynamic cast for BINFO usage */
        DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(poly_obj);
        if (derived_ptr) {
            int template_result = derived_ptr->template_method(argc);
            virtual_result += template_result;
        }
        
        delete poly_obj;
        
        /* Use the result */
        opaque_external_function(&virtual_result);
    }
#endif
    
    /* Call external function with various identifiers */
    opaque_external_function(&global_sum);
    opaque_external_function(&ssa_result);
    opaque_external_function(&openmp_sum);
    opaque_external_function(&recursive_result.a);
    
    /* Complex expression using all results */
    int final_result = global_sum + ssa_result + openmp_sum + 
                      recursive_result.a + recursive_result.b +
                      cs1.a + cs2.a + ns.inner.a;
    
    /* Add array elements */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 3; j++) {
            final_result += complex_array[i][j];
        }
    }
    
    /* Print to ensure code isn't eliminated */
#ifdef __cplusplus
    std::cout << "Final result: " << final_result << std::endl;
#else
    printf("Final result: %d\n", final_result);
#endif
    
    return final_result > 100 ? 0 : 1;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
