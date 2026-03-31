/* Test program to exercise tree node dispatch in GCC */
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
#include <vector>
#endif

/* External function to prevent optimization */
extern void opaque_external_function(int*);

/* Global identifiers for IDENTIFIER_NODE coverage */
int global_var_1 = 0;
static int static_var_2 = 0;
extern int extern_var_3;

/* Struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    int b;
    double c;
    char d;
};

/* Array for TREE_VEC-like representations */
int multi_array[2][3][4];

/* Recursive function returning struct (CONSTRUCTOR) */
struct ComplexStruct recursive_struct_builder(int depth, int base) {
    struct ComplexStruct result;
    
    if (depth <= 0) {
        /* CONSTRUCTOR node: aggregate initializer */
        result = (struct ComplexStruct){base, base * 2, base * 3.14, 'A' + base};
        return result;
    }
    
    /* Recursive call */
    struct ComplexStruct inner = recursive_struct_builder(depth - 1, base + 1);
    
    /* Build new struct from inner (another CONSTRUCTOR) */
    result.a = inner.b;
    result.b = inner.a + base;
    result.c = inner.c * 1.1;
    result.d = inner.d + 1;
    
    return result;
}

/* Function with complex control flow for SSA_NAME and BLOCK nodes */
int ssa_generator(int x, int y) {
    int result;
    
    /* Outer block */
    {
        int temp = x * y;
        
        /* Inner block with goto */
        {
            int hidden = 100;
            
            if (x > y) {
                goto skip_part;
            }
            
            /* This part might be skipped */
            hidden = x + y;
            
        skip_part:
            /* SSA_NAME: phi node for 'result' */
            if (hidden > 50) {
                result = temp * 2;
            } else {
                result = temp / 2;
            }
            
            /* Use hidden to prevent optimization */
            result += hidden;
        }
    }
    
    /* Loop with SSA */
    for (int i = 0; i < 10; i++) {
        int phi_var;
        
        if (i % 2 == 0) {
            phi_var = result + i;
        } else {
            phi_var = result - i;
        }
        
        /* Use phi_var to create SSA_NAME */
        result = phi_var * (i + 1);
    }
    
    return result;
}

#ifdef __cplusplus
/* C++ classes for TREE_BINFO nodes */
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
    int iterations = (argc > 1) ? atoi(argv[1]) : 5;
    int seed = (argc > 2) ? atoi(argv[2]) : 42;
    
    /* CONSTRUCTOR: Struct initialization */
    struct ComplexStruct cs1 = {1, 2, 3.14, 'X'};
    struct ComplexStruct cs2 = {.b = 20, .a = 10, .d = 'Y', .c = 6.28};
    
    /* CONSTRUCTOR: Array initialization with designators */
    int complex_array[10] = {[0] = 1, [5] = seed, [9] = iterations};
    
    /* BLOCK: Nested blocks with labels */
    int block_result = 0;
    
    {
        int inner_var = 100;
        
        if (seed % 2 == 0) {
            goto middle_of_block;
        }
        
        inner_var = 200;
        
    middle_of_block:
        {
            int another_inner = inner_var + 50;
            block_result = another_inner * 2;
        }
        
        /* Another goto target */
        if (iterations > 10) {
            goto end_of_block;
        }
        
        block_result += 1000;
        
    end_of_block:
        /* Empty target */;
    }
    
    /* Call recursive function for CONSTRUCTOR nodes */
    struct ComplexStruct recursive_result = recursive_struct_builder(3, seed);
    
    /* Generate SSA_NAME nodes */
    int ssa_result = ssa_generator(seed, iterations);
    
    /* OpenMP region for OMP_CLAUSE nodes */
    int openmp_sum = 0;
    int openmp_array[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        openmp_array[i] = i * seed;
    }
    
    /* Complex OpenMP with multiple clauses */
    #pragma omp parallel for reduction(+:openmp_sum) \
        private(seed) firstprivate(iterations) \
        shared(openmp_array) schedule(dynamic, 4) \
        collapse(2) if(iterations > 3)
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int idx = i * 10 + j;
            int local_seed = seed + i;  /* Uses private seed */
            
            /* Conditional for SSA */
            int temp;
            if (idx % 3 == 0) {
                temp = openmp_array[idx] * 2;
            } else if (idx % 3 == 1) {
                temp = openmp_array[idx] / 2;
            } else {
                temp = openmp_array[idx] + local_seed;
            }
            
            openmp_sum += temp;
        }
    }
    
    /* Nested OpenMP for more clause coverage */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task private(seed) firstprivate(iterations)
            {
                int task_local = seed * iterations;
                opaque_external_function(&task_local);
            }
        }
    }
    
    #ifdef __cplusplus
    /* C++ specific: TREE_BINFO through inheritance */
    BaseClass* base_ptr;
    DerivedClass derived_obj;
    SecondDerived second_derived_obj;
    
    /* Virtual calls for BINFO dispatch */
    if (seed % 3 == 0) {
        base_ptr = &derived_obj;
    } else {
        base_ptr = &second_derived_obj;
    }
    
    derived_obj.base_data = seed;
    derived_obj.derived_data = iterations;
    
    int virtual_result = base_ptr->virtual_method(ssa_result);
    
    /* TREE_VEC through template instantiation */
    std::vector<std::vector<int>> vec_of_vecs;
    for (int i = 0; i < iterations % 5 + 1; i++) {
        std::vector<int> inner_vec;
        for (int j = 0; j < 3; j++) {
            inner_vec.push_back(i * j + seed);
        }
        vec_of_vecs.push_back(inner_vec);
    }
    #endif
    
    /* Use various identifiers (IDENTIFIER_NODE) */
    global_var_1 = ssa_result;
    static_var_2 = openmp_sum % 1000;
    
    /* Call external function with identifiers */
    opaque_external_function(&global_var_1);
    opaque_external_function(&static_var_2);
    opaque_external_function(&block_result);
    
    /* Complex expression using all results */
    int final_result = 
        recursive_result.a +
        recursive_result.b +
        (int)recursive_result.c +
        recursive_result.d +
        block_result +
        ssa_result +
        openmp_sum +
        #ifdef __cplusplus
        virtual_result +
        #endif
        complex_array[0] +
        complex_array[5] +
        complex_array[9];
    
    /* Print to ensure code isn't optimized away */
    printf("Final result: %d\n", final_result);
    
    return final_result % 256;
}
