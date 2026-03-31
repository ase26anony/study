/* Test program to trigger tree_kind dispatch for various TREE_CODE cases */
#ifdef __cplusplus
#include <vector>
#include <iostream>
extern "C" {
#endif

/* External function to prevent optimization */
extern void opaque_external_function(int*);

/* Global identifiers (IDENTIFIER_NODE) */
int global_identifier_1;
static int static_identifier_2;
extern int extern_identifier_3;

/* Struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    int b;
    int c;
};

/* Array for TREE_VEC-like representations */
int multi_dim_array[2][3][4];

#ifdef __cplusplus
/* C++ classes for TREE_BINFO */
class BaseClass {
public:
    virtual int virtual_method(int x) { return x * 2; }
    virtual ~BaseClass() {}
};

class DerivedClass : public BaseClass {
public:
    virtual int virtual_method(int x) override { return x * 3; }
    int additional_method() { return 42; }
};

template<typename T>
class TemplateClass {
    T data;
public:
    TemplateClass(T d) : data(d) {}
    T get_data() { return data; }
};
#endif

/* Recursive function returning struct (CONSTRUCTOR) */
struct ComplexStruct recursive_struct_builder(int depth, int value) {
    struct ComplexStruct result;
    if (depth <= 0) {
        result.a = value;
        result.b = value * 2;
        result.c = value * 3;
    } else {
        struct ComplexStruct inner = recursive_struct_builder(depth - 1, value + 1);
        result.a = inner.c;
        result.b = inner.a;
        result.c = inner.b;
    }
    return result;
}

/* Function with complex control flow for SSA_NAME and BLOCK nodes */
int complex_control_flow(int argc, char** argv) {
    volatile int seed = argc; /* Prevent optimization */
    int result = 0;
    
    /* BLOCK node with local variable */
    {
        int block_local_1 = seed * 2;
        
        /* Another nested BLOCK */
        {
            int block_local_2 = block_local_1 + 10;
            result += block_local_2;
            
            /* goto to create interesting control flow */
            if (seed > 5) {
                goto special_label;
            }
            
            block_local_2 += 20;
            special_label:
            result += block_local_2;
        }
    }
    
    /* SSA_NAME generation through phi nodes */
    int ssa_var;
    for (int i = 0; i < seed; i++) {
        if (i % 3 == 0) {
            ssa_var = i * 2;      /* Assignment 1 */
        } else if (i % 3 == 1) {
            ssa_var = i + 5;      /* Assignment 2 */
        } else {
            ssa_var = i * i;      /* Assignment 3 */
        }
        
        /* Use ssa_var to prevent elimination */
        result += ssa_var;
        
        /* Additional conditional for more SSA complexity */
        int temp;
        if (ssa_var > 50) {
            temp = ssa_var / 2;
        } else {
            temp = ssa_var * 2;
        }
        result += temp;
    }
    
    return result;
}

int main(int argc, char** argv) {
    int final_result = 0;
    
    /* Use command line args to prevent constant folding */
    volatile int use_openmp = (argc > 1) ? 1 : 0;
    volatile int loop_bound = (argc > 2) ? atoi(argv[2]) : 10;
    
    /* CONSTRUCTOR nodes - aggregate initializers */
    struct ComplexStruct cs1 = {1, 2, 3};
    struct ComplexStruct cs2 = {.a = 4, .c = 6, .b = 5}; /* Designated initializer */
    int array_init[5] = {[0] = 10, [2] = 20, [4] = 30}; /* Sparse array init */
    
    /* TREE_VEC generation through complex initializers */
    int nested_init[2][3] = {{1, 2, 3}, {4, 5, 6}};
    
    /* BLOCK nodes with gotos */
    {
        int hidden_in_block = 42;
        
        if (argc > 3) {
            goto skip_declaration;
        }
        
        int skipped_var = 100;
        hidden_in_block += skipped_var;
        
        skip_declaration:
        final_result += hidden_in_block;
        
        /* Another goto creating cross-block flow */
        goto cross_block;
        
        {
            int unreachable = 999;
            cross_block:
            final_result += 1;
        }
    }
    
    /* Call recursive function for CONSTRUCTOR nodes */
    struct ComplexStruct recursive_result = recursive_struct_builder(3, argc);
    final_result += recursive_result.a + recursive_result.b + recursive_result.c;
    
    /* Complex control flow for SSA_NAME */
    final_result += complex_control_flow(argc, argv);
    
    /* OpenMP region for OMP_CLAUSE nodes */
    if (use_openmp) {
        int sum = 0;
        int private_var = 100;
        int shared_var = 200;
        int firstprivate_var = 300;
        
        #pragma omp parallel for private(private_var) \
                 firstprivate(firstprivate_var) shared(shared_var) \
                 reduction(+:sum) collapse(2) schedule(dynamic, 4) \
                 if(loop_bound > 5)
        for (int i = 0; i < loop_bound; i++) {
            for (int j = 0; j < loop_bound; j++) {
                private_var = i + j;
                int local_sum = 0;
                
                /* Nested OpenMP for more clauses */
                #pragma omp simd reduction(+:local_sum) linear(j:1)
                for (int k = 0; k < 10; k++) {
                    local_sum += private_var + k;
                }
                
                sum += local_sum;
                
                /* OMP atomic for atomic clause */
                #pragma omp atomic
                shared_var += 1;
            }
        }
        
        final_result += sum + shared_var;
        
        /* Additional OpenMP sections with different clauses */
        #pragma omp parallel sections private(private_var) \
                 lastprivate(firstprivate_var)
        {
            #pragma omp section
            {
                private_var = 1;
                firstprivate_var = private_var * 2;
            }
            #pragma omp section
            {
                private_var = 2;
                firstprivate_var = private_var * 3;
            }
        }
        final_result += firstprivate_var;
    }
    
    /* Call external function with various identifiers */
    opaque_external_function(&final_result);
    
    /* Use global identifiers */
    global_identifier_1 = final_result % 100;
    static_identifier_2 = global_identifier_1 * 2;
    final_result += static_identifier_2;
    
    /* Multi-dimensional array access (potential TREE_VEC) */
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 4; k++) {
                multi_dim_array[i][j][k] = i * 100 + j * 10 + k;
                final_result += multi_dim_array[i][j][k];
            }
        }
    }
    
#ifdef __cplusplus
    /* C++ specific code for TREE_BINFO */
    /* Template instantiation (TREE_VEC in C++) */
    TemplateClass<int> template_instance(42);
    final_result += template_instance.get_data();
    
    std::vector<int> vec_identifier;
    for (int i = 0; i < 10; i++) {
        vec_identifier.push_back(i * final_result);
    }
    
    /* Class hierarchy for BINFO nodes */
    BaseClass* base_ptr;
    if (argc % 2 == 0) {
        base_ptr = new DerivedClass();
    } else {
        base_ptr = new BaseClass();
    }
    
    /* Virtual call through BINFO */
    final_result += base_ptr->virtual_method(argc);
    
    /* dynamic_cast for BINFO usage */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        final_result += derived_ptr->additional_method();
    }
    
    delete base_ptr;
#endif
    
    /* Print result to ensure code isn't eliminated */
    printf("Final result: %d\n", final_result);
    
    return final_result % 256;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
