/* Test program to trigger tree_kind dispatch for various TREE_CODE cases */
#ifdef __cplusplus
#include <vector>
#include <iostream>
#else
#include <stdio.h>
#include <stdlib.h>
#endif

/* External function declarations to prevent optimization */
extern void opaque_external_function(int*);
extern int unpredictable(int);

/* Global identifiers (IDENTIFIER_NODE) */
int global_counter = 0;
static int static_hidden = 42;
volatile int volatile_tracker = 0;

#ifdef __cplusplus
/* C++ class hierarchy for TREE_BINFO */
class BaseClass {
public:
    virtual int virtual_method(int x) { return x * 2; }
    virtual ~BaseClass() {}
};

class DerivedClass : public BaseClass {
private:
    int extra_data;
public:
    DerivedClass(int val) : extra_data(val) {}
    virtual int virtual_method(int x) override { 
        return x * 3 + extra_data; 
    }
    
    /* Template method that might use TREE_VEC */
    template<typename T>
    T process_template(T value) {
        return value + extra_data;
    }
};
#endif

/* Struct with constructor initialization */
struct ComplexStruct {
    int a;
    int b;
    int c[3];
};

/* Recursive function returning struct (CONSTRUCTOR) */
struct ComplexStruct recursive_struct_builder(int depth, int seed) {
    struct ComplexStruct result;
    result.a = seed;
    result.b = depth;
    
    /* Array constructor with designators */
    result.c[0] = seed * 2;
    result.c[1] = seed * 3;
    result.c[2] = seed * 4;
    
    if (depth > 0) {
        struct ComplexStruct inner = recursive_struct_builder(depth - 1, seed + 1);
        result.a += inner.a;
        result.b += inner.b;
    }
    
    return result;
}

int main(int argc, char** argv) {
    /* Use argc to prevent compile-time optimization */
    int iterations = (argc > 1) ? unpredictable(argc) : 10;
    if (iterations < 0) iterations = 5;
    
    /* BLOCK nodes with goto jumps */
    volatile_tracker = 1;
    
    {
        /* Inner block with local variable */
        int block_local = 100;
        volatile_tracker += block_local;
        
        if (argc > 2) {
            goto skip_initialization;
        }
        
        int normally_initialized = 50;
        volatile_tracker += normally_initialized;
        
    skip_initialization:
        /* Use of potentially uninitialized variable forces SSA */
        int use_after_skip = block_local + 1;
        volatile_tracker += use_after_skip;
    }
    
    /* CONSTRUCTOR nodes - various initializations */
    struct ComplexStruct cs1 = {1, 2, {3, 4, 5}};
    struct ComplexStruct cs2 = {.b = 20, .a = 10, .c = {[1] = 30}};
    int array_with_designator[5] = {[0] = 1, [2] = argc, [4] = 3};
    
    /* Complex array initialization that might create TREE_VEC */
    int multi_dim_init[2][3] = {{1, 2, 3}, {4, 5, 6}};
    
#ifdef __cplusplus
    /* TREE_VEC through template instantiation */
    std::vector<int> template_vec;
    for (int i = 0; i < iterations; ++i) {
        template_vec.push_back(i * 2);
    }
    
    /* TREE_BINFO through class hierarchy */
    DerivedClass derived_obj(7);
    BaseClass* base_ptr = &derived_obj;
    int virtual_result = base_ptr->virtual_method(argc);
    
    /* Call template method */
    int template_result = derived_obj.process_template(argc);
#endif
    
    /* SSA_NAME generation through complex control flow */
    int ssa_var = 0;
    for (int i = 0; i < iterations; ++i) {
        if (i % 3 == 0) {
            ssa_var = i * 2;
        } else if (i % 3 == 1) {
            ssa_var = i * 3 + argc;
        } else {
            ssa_var = i * 4 - argc;
        }
        
        /* Use ssa_var in computation to keep it live */
        volatile_tracker += ssa_var;
        
        /* Additional SSA complexity with nested conditions */
        int inner_ssa;
        if (ssa_var > 10) {
            inner_ssa = ssa_var / 2;
        } else {
            inner_ssa = ssa_var * 2;
        }
        
        /* Phi node creation */
        int phi_user = (inner_ssa > 5) ? inner_ssa : -inner_ssa;
        volatile_tracker += phi_user;
    }
    
    /* OpenMP region with multiple clauses (OMP_CLAUSE) */
    int openmp_sum = 0;
    int openmp_array[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; ++i) {
        openmp_array[i] = i + argc;
    }
    
    #pragma omp parallel for reduction(+:openmp_sum) \
        private(ssa_var) firstprivate(iterations) \
        shared(openmp_array, volatile_tracker) \
        schedule(dynamic, 4) collapse(2) if(iterations > 1)
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            int idx = i * 10 + j;
            ssa_var = (i + j) % 3;  /* Reuse ssa_var as private */
            openmp_sum += openmp_array[idx] * ssa_var;
            
            /* Nested OpenMP directive */
            #pragma omp atomic
            volatile_tracker++;
        }
    }
    
    /* Additional OpenMP construct with different clauses */
    #pragma omp parallel num_threads(2) default(none) \
        copyin(static_hidden)
    {
        #pragma omp single
        {
            int single_var = openmp_sum;
            opaque_external_function(&single_var);
        }
    }
    
    /* Call recursive function */
    struct ComplexStruct recursive_result = recursive_struct_builder(3, argc);
    
    /* Use all computed values to prevent dead code elimination */
    int final_result = 
        volatile_tracker + 
        openmp_sum + 
        recursive_result.a + 
        recursive_result.b +
        array_with_designator[0];
        
#ifdef __cplusplus
    final_result += virtual_result + template_result;
    std::cout << "Result: " << final_result << std::endl;
#else
    printf("Result: %d\n", final_result);
#endif
    
    /* Call external function with various identifiers */
    opaque_external_function(&final_result);
    
    return (final_result > 0) ? 0 : 1;
}

/* Dummy definitions to satisfy linker (in real test would be in separate file) */
#ifdef STANDALONE_TEST
void opaque_external_function(int* x) {
    *x += 1;
}

int unpredictable(int x) {
    return (x * 1103515245 + 12345) & 0x7fffffff;
}
#endif
