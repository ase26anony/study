/* Test program to trigger tree_kind dispatch for various TREE_CODE cases */
#ifdef __cplusplus
#include <vector>
#include <iostream>
#endif

/* External function to prevent optimization */
extern void opaque_external_func(int*);

/* Global identifiers (IDENTIFIER_NODE) */
int global_identifier_1;
static int static_identifier_2;
extern int extern_identifier_3;

/* Struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    int b;
    float c;
    double d;
};

/* Recursive function returning struct (CONSTRUCTOR) */
struct ComplexStruct recursive_constructor_func(int n, int* counter) {
    struct ComplexStruct result;
    if (n <= 0) {
        result.a = 1;
        result.b = 2;
        result.c = 3.0f;
        result.d = 4.0;
    } else {
        struct ComplexStruct inner = recursive_constructor_func(n - 1, counter);
        result.a = inner.b + *counter;
        result.b = inner.a - *counter;
        result.c = inner.c * 2.0f;
        result.d = inner.d / 2.0;
        (*counter)++;
    }
    return result;
}

#ifdef __cplusplus
/* C++ classes for TREE_BINFO */
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

class SecondDerived : public DerivedClass {
public:
    virtual int virtual_method(int x) override {
        return x * 4 + base_data + derived_data;
    }
};
#endif

int main(int argc, char** argv) {
    /* Use argc to prevent compile-time optimization */
    int iterations = (argc > 1) ? 10 : 5;
    volatile int force_volatile = argc;
    
    /* CONSTRUCTOR nodes - aggregate initialization */
    struct ComplexStruct cs = {1, 2, 3.0f, 4.0};
    int array_with_designator[10] = {[0] = 1, [5] = 2, [9] = 3};
    
    /* BLOCK nodes with goto */
    int block_var = 0;
    
    {
        /* Inner block with local variable */
        int inner_block_var = 100;
        block_var = inner_block_var;
        
        if (force_volatile > 1) {
            goto skip_middle;
        }
        
        int unused_in_middle = 999; /* This might be skipped */
        
    skip_middle:
        /* Use inner_block_var to keep it alive */
        opaque_external_func(&inner_block_var);
    }
    
    /* SSA_NAME generation - complex conditional assignments */
    int ssa_var = 0;
    for (int i = 0; i < iterations; i++) {
        if (i % 2 == 0) {
            ssa_var = i * 2;
        } else {
            ssa_var = i * 3 + 1;
        }
        
        /* Use ssa_var in computation to create phi nodes */
        int ssa_use = ssa_var + i;
        opaque_external_func(&ssa_use);
    }
    
    /* Recursive function call for CONSTRUCTOR */
    int counter = 0;
    struct ComplexStruct recursive_result = recursive_constructor_func(5, &counter);
    
    /* OpenMP region with multiple clauses (OMP_CLAUSE) */
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    #pragma omp parallel for private(iterations) firstprivate(counter) \
            shared(arr) reduction(+:sum) collapse(2) schedule(dynamic)
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int idx = i * 10 + j;
            sum += arr[idx] + counter;
        }
    }
    
    /* Nested OpenMP with more clauses */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task private(ssa_var) firstprivate(sum)
            {
                int task_local = ssa_var + sum;
                opaque_external_func(&task_local);
            }
        }
    }
    
    #ifdef __cplusplus
    /* TREE_VEC through template instantiation */
    std::vector<int> template_vec;
    std::vector<std::vector<double>> nested_template_vec;
    
    /* TREE_BINFO through polymorphism */
    BaseClass* poly_obj;
    if (force_volatile % 2 == 0) {
        poly_obj = new DerivedClass();
    } else {
        poly_obj = new SecondDerived();
    }
    
    poly_obj->base_data = 42;
    int poly_result = poly_obj->virtual_method(iterations);
    
    /* Use dynamic_cast for BINFO */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(poly_obj);
    if (derived_ptr) {
        derived_ptr->derived_data = poly_result;
    }
    
    delete poly_obj;
    #endif
    
    /* Call external function with various identifiers */
    opaque_external_func(&global_identifier_1);
    opaque_external_func(&static_identifier_2);
    opaque_external_func(&extern_identifier_3);
    opaque_external_func(&sum);
    opaque_external_func(&block_var);
    opaque_external_func(&ssa_var);
    
    /* Complex array initialization with designators (TREE_VEC in C) */
    struct InitWithGaps {
        int values[10];
    };
    
    struct InitWithGaps init_gaps = {
        .values = {[0] = 1, [3] = 2, [7] = 3, [9] = 4}
    };
    
    /* Final computation to ensure all code is live */
    int final_result = sum + block_var + ssa_var + recursive_result.a + 
                      recursive_result.b + force_volatile;
    
    #ifdef __cplusplus
    final_result += poly_result;
    #endif
    
    /* Use final result to prevent dead code elimination */
    opaque_external_func(&final_result);
    
    return final_result % 256;
}
