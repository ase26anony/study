/* Test program to trigger tree_kind dispatch for various TREE_CODE cases */
#ifdef __cplusplus
#include <vector>
#include <iostream>
#endif

/* External function to prevent optimization */
extern void opaque_external_function(int*);

/* Global identifiers (IDENTIFIER_NODE) */
int global_identifier_1 = 0;
static int static_identifier_2 = 0;
extern int extern_identifier_3;

/* Struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    int b;
    double c;
    int* d;
};

/* Another struct for nested constructors */
struct NestedStruct {
    struct ComplexStruct inner;
    int extra;
};

/* Recursive function returning struct (CONSTRUCTOR) */
struct ComplexStruct recursive_struct_builder(int depth, int base) {
    /* Local block with variable (BLOCK) */
    {
        volatile int block_local = depth * 2;
        if (block_local > 100) {
            /* goto to stress BLOCK handling */
            goto early_return;
        }
    }
    
    if (depth <= 0) {
        /* Aggregate initializer (CONSTRUCTOR) */
        struct ComplexStruct result = {base, base + 1, base * 1.5, &global_identifier_1};
        return result;
    }
    
    struct ComplexStruct child = recursive_struct_builder(depth - 1, base * 2);
    child.a += depth;
    child.b -= depth;
    
early_return:
    /* Another constructor with designators */
    struct ComplexStruct final_result = {
        .a = base,
        .b = child.b,
        .c = child.c * 1.1,
        .d = &static_identifier_2
    };
    return final_result;
}

#ifdef __cplusplus
/* C++ classes for TREE_BINFO */
class BaseClass {
public:
    virtual int virtual_method(int x) {
        return x * 2;
    }
    virtual ~BaseClass() {}
};

class DerivedClass : public BaseClass {
private:
    int derived_data;
public:
    DerivedClass(int val) : derived_data(val) {}
    
    virtual int virtual_method(int x) override {
        return x * 3 + derived_data;
    }
    
    template<typename T>
    void template_method(T value) {
        /* Template instantiation may involve TREE_VEC */
        derived_data += static_cast<int>(value);
    }
};

void test_cpp_features() {
    /* Template instantiation (TREE_VEC) */
    std::vector<int> template_vec;
    template_vec.push_back(1);
    template_vec.push_back(2);
    
    /* Class hierarchy (TREE_BINFO) */
    DerivedClass derived(42);
    BaseClass* base_ptr = &derived;
    
    /* Virtual call through base pointer */
    int result = base_ptr->virtual_method(10);
    
    /* dynamic_cast for BINFO usage */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        derived_ptr->template_method(5.0);
    }
}
#endif

/* Function with SSA_NAME generation */
int ssa_name_generator(int iterations, int threshold) {
    int ssa_var = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Conditional assignment creating phi node */
        if (i % 2 == 0) {
            ssa_var = ssa_var + i * 2;
        } else {
            ssa_var = ssa_var - i;
        }
        
        /* Another use to keep variable live */
        if (ssa_var > threshold) {
            ssa_var = ssa_var / 2;
        }
    }
    
    /* Complex expression with multiple SSA opportunities */
    int result = ssa_var;
    for (int j = 0; j < 5; j++) {
        result = (result % 2 == 0) ? result + j : result - j;
    }
    
    return result;
}

/* Main function with OpenMP clauses */
int main(int argc, char* argv[]) {
    /* Use argc to prevent optimization */
    int use_argc = (argc > 1) ? atoi(argv[1]) : 100;
    
    /* Array with complex initializer (CONSTRUCTOR) */
    int arr[5] = {[0] = 1, [2] = use_argc, [4] = 999};
    
    /* Struct with nested initializer (CONSTRUCTOR) */
    struct NestedStruct nested = {
        .inner = {10, 20, 30.5, arr},
        .extra = 1000
    };
    
    /* Call recursive function */
    struct ComplexStruct built = recursive_struct_builder(3, use_argc);
    
    /* Generate SSA names */
    int ssa_result = ssa_name_generator(use_argc % 100, 500);
    
    /* OpenMP region with multiple clauses (OMP_CLAUSE) */
    int sum = 0;
    #pragma omp parallel for private(use_argc) firstprivate(nested) \
             shared(arr, built) reduction(+:sum) collapse(2) \
             schedule(dynamic, 4)
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* Complex expression to prevent optimization */
            int local = i * j + built.a + nested.inner.b;
            
            /* Conditional with goto between blocks (BLOCK) */
            if (local > 50) {
                goto special_case;
            }
            
            sum += local;
            continue;
            
        special_case:
            {
                /* Inner block with local var */
                volatile int special_value = local * 2;
                sum += special_value / 3;
            }
        }
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task private(sum) firstprivate(ssa_result)
            {
                /* Task with private/firstprivate clauses */
                int task_local = sum + ssa_result;
                opaque_external_function(&task_local);
            }
        }
    }
    
    #ifdef __cplusplus
    /* C++ specific features */
    test_cpp_features();
    
    /* More template usage (TREE_VEC) */
    std::vector<std::vector<int>> matrix;
    matrix.push_back(std::vector<int>{1, 2, 3});
    matrix.push_back(std::vector<int>{4, 5, 6});
    #endif
    
    /* Call external function with various identifiers */
    opaque_external_function(&global_identifier_1);
    opaque_external_function(&static_identifier_2);
    opaque_external_function(arr);
    opaque_external_function(&built.a);
    
    /* Final computation using all results */
    int final_result = sum + ssa_result + built.a + nested.extra;
    
    /* Print to prevent dead code elimination */
    printf("Final result: %d\n", final_result);
    
    return final_result % 256;
}
