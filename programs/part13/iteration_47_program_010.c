/* Test program to exercise specific tree node types in GCC's tree.cc */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void opaque_external_function(int*);

/* Global identifiers for IDENTIFIER_NODE coverage */
int global_var_1;
static int static_var_2;
extern int extern_var_3;

/* Struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    double b;
    char c;
    int* d;
};

/* Array for TREE_VEC representation */
int multi_array[2][3][4];

/* Recursive function returning struct (CONSTRUCTOR) */
struct ComplexStruct recursive_struct_builder(int depth, int* counter) {
    struct ComplexStruct result;
    result.a = depth;
    result.b = depth * 1.5;
    result.c = 'A' + (depth % 26);
    result.d = counter;
    
    if (depth > 0) {
        struct ComplexStruct inner = recursive_struct_builder(depth - 1, counter);
        result.a += inner.a;
        result.b += inner.b;
        (*counter)++;
    }
    
    return result;
}

/* Function with complex control flow for SSA_NAME and BLOCK nodes */
int complex_control_flow(int x, int y) {
    int result;
    
    /* BLOCK node with local variable */
    {
        int local_block_var = x * 2;
        if (local_block_var > 100) {
            goto early_exit;
        }
        result = local_block_var;
    }
    
    /* Another block with goto */
    {
        int temp = y;
        if (temp < 0) {
            goto negative_path;
        }
        result += temp;
        goto continue_main;
        
    negative_path:
        result -= temp;
        goto continue_main;
    }
    
early_exit:
    return result * 2;

continue_main:
    /* SSA_NAME creation through phi-like behavior */
    int ssa_candidate;
    for (int i = 0; i < x; i++) {
        if (i % 2 == 0) {
            ssa_candidate = i * 3;
        } else {
            ssa_candidate = i * 5;
        }
        result += ssa_candidate; /* Use to prevent elimination */
    }
    
    return result;
}

#ifdef __cplusplus
/* C++ specific code for TREE_BINFO nodes */
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

void test_cpp_classes() {
    DerivedClass derived;
    derived.base_data = 10;
    derived.derived_data = 20;
    
    BaseClass* base_ptr = &derived;
    int result = base_ptr->virtual_method(5);
    
    SecondDerived second;
    BaseClass* base_ptr2 = dynamic_cast<BaseClass*>(&second);
    if (base_ptr2) {
        result += base_ptr2->virtual_method(3);
    }
    
    opaque_external_function(&result);
}
#endif

/* Main function with OpenMP for OMP_CLAUSE nodes */
int main(int argc, char** argv) {
    int final_result = 0;
    
    /* Use argc to prevent compile-time optimization */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    int use_openmp = (argc > 2) ? atoi(argv[2]) : 1;
    
    /* CONSTRUCTOR node - struct initialization */
    struct ComplexStruct cs = {1, 2.5, 'X', &iterations};
    
    /* CONSTRUCTOR node - array with designators (C99) */
    int designated_array[10] = {[0] = 1, [5] = 2, [9] = 3};
    
    /* TREE_VEC - complex array initialization */
    int vec_init[3][2] = {{1, 2}, {3, 4}, {5, 6}};
    
    /* Call recursive function for CONSTRUCTOR nodes */
    int counter = 0;
    struct ComplexStruct recursive_result = recursive_struct_builder(3, &counter);
    final_result += recursive_result.a;
    
    /* Complex control flow for SSA_NAME and BLOCK nodes */
    final_result += complex_control_flow(iterations, iterations / 2);
    
    /* OpenMP region with multiple clauses for OMP_CLAUSE nodes */
    if (use_openmp) {
        int sum = 0;
        int private_var = 100;
        
        #pragma omp parallel for reduction(+:sum) \
                private(private_var) firstprivate(iterations) \
                shared(final_result) collapse(2) schedule(dynamic)
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                private_var = i * j;
                int local_result = 0;
                
                /* Nested OpenMP directive */
                #pragma omp simd reduction(+:local_result)
                for (int k = 0; k < 8; k++) {
                    local_result += k * private_var;
                }
                
                sum += local_result;
                
                /* Another OpenMP construct */
                #pragma omp atomic
                final_result += 1;
            }
        }
        
        final_result += sum;
        
        /* OpenMP sections with different clauses */
        #pragma omp parallel sections private(private_var) \
                lastprivate(final_result)
        {
            #pragma omp section
            {
                private_var = 1;
                #pragma omp critical
                final_result += private_var;
            }
            
            #pragma omp section
            {
                private_var = 2;
                #pragma omp critical
                final_result += private_var;
            }
        }
    }
    
    /* Use global identifiers */
    global_var_1 = final_result;
    static_var_2 = final_result * 2;
    
    /* Call external function with identifiers */
    opaque_external_function(&final_result);
    
    #ifdef __cplusplus
    /* C++ class hierarchy for TREE_BINFO */
    test_cpp_classes();
    #endif
    
    /* Use designated array to prevent elimination */
    for (int i = 0; i < 10; i++) {
        final_result += designated_array[i];
    }
    
    /* Use vec_init */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            final_result += vec_init[i][j];
        }
    }
    
    /* Use multi_array for TREE_VEC */
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 4; k++) {
                multi_array[i][j][k] = i + j + k;
                final_result += multi_array[i][j][k];
            }
        }
    }
    
    printf("Final result: %d\n", final_result);
    return final_result > 0 ? 0 : 1;
}
