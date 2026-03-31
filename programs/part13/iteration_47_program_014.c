/* Test program to trigger tree_kind dispatch for uncovered TREE_CODE cases */
#ifdef __cplusplus
#include <vector>
#include <iostream>
extern "C" {
#endif

/* External function to prevent optimization */
extern void opaque_external_function(void*);

/* Global identifiers (IDENTIFIER_NODE) */
static int static_global_counter = 0;
extern volatile int external_volatile_var;
int global_array[100];

/* Struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    double b;
    char c;
    void* d;
};

/* Another struct for nested constructors */
struct NestedStruct {
    struct ComplexStruct inner;
    float arr[3];
};

/* Recursive function returning struct (CONSTRUCTOR) */
#ifdef __cplusplus
struct ComplexStruct recursive_builder(int depth, int* counter) {
#else
struct ComplexStruct recursive_builder(int depth, int* counter) {
#endif
    struct ComplexStruct result;
    result.a = depth;
    result.b = depth * 1.5;
    result.c = 'A' + (depth % 26);
    result.d = (void*)(long)depth;
    
    if (depth > 0) {
        struct ComplexStruct inner = recursive_builder(depth - 1, counter);
        result.a += inner.a;
        (*counter)++;
    }
    
    /* Create SSA_NAME opportunities */
    int temp = result.a;
    if (temp > 10) {
        temp = temp * 2;
    } else {
        temp = temp + 5;
    }
    result.a = temp;
    
    return result;
}

#ifdef __cplusplus
/* C++ classes for TREE_BINFO generation */
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
    /* Use argc to prevent optimization */
    int iterations = (argc > 1) ? 100 : 200;
    volatile int prevent_opt = argc;
    
    /* CONSTRUCTOR nodes - complex aggregate initializers */
    struct ComplexStruct cs = { 
        .a = 10, 
        .b = 3.14159, 
        .c = 'X', 
        .d = (void*)main 
    };
    
    /* Array with designated initializer (may create TREE_VEC) */
    int designated_array[10] = { [0] = 1, [5] = 2, [9] = 3 };
    
    /* Nested struct initializer */
    struct NestedStruct ns = {
        .inner = { .a = 1, .b = 2.0, .c = 'Z', .d = &cs },
        .arr = { [1] = 3.14f, [0] = 2.71f }
    };
    
    /* BLOCK nodes with goto */
    int block_var = 0;
    
    if (prevent_opt) {
        goto middle_of_block;
        
        {
            int hidden_in_block = 42;  /* BLOCK with local var */
            middle_of_block:
            block_var = hidden_in_block + argc;
            
            /* Another nested block */
            {
                volatile int another_hidden = 99;
                block_var += another_hidden;
            }
        }
    }
    
    /* SSA_NAME generation - complex control flow */
    int ssa_test = 0;
    for (int i = 0; i < iterations; i++) {
        int temp;
        if (i % 3 == 0) {
            temp = i * 2;
        } else if (i % 3 == 1) {
            temp = i + 5;
        } else {
            temp = i / 2;
        }
        
        /* This creates phi nodes */
        if (temp > 10) {
            ssa_test += temp;
        } else {
            ssa_test -= temp;
        }
        
        /* Loop-carried dependency */
        ssa_test = ssa_test % 1000;
    }
    
    /* OpenMP region with multiple clauses (OMP_CLAUSE) */
    int sum = 0;
    int private_var = 100;
    
    #pragma omp parallel for reduction(+:sum) \
        private(private_var) firstprivate(iterations) \
        shared(global_array) collapse(2) if(iterations > 50)
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            private_var = i + j;
            sum += private_var * (iterations % 10);
            global_array[i * 10 + j] = private_var;
        }
    }
    
    /* More OpenMP clauses */
    #pragma omp parallel
    {
        #pragma omp sections nowait
        {
            #pragma omp section
            { sum += 1; }
            
            #pragma omp section
            { sum += 2; }
        }
        
        #pragma omp barrier
        
        #pragma omp single copyprivate(private_var)
        {
            private_var = sum;
        }
    }
    
    /* Recursive call for CONSTRUCTOR returns */
    int counter = 0;
    struct ComplexStruct recursive_result = recursive_builder(5, &counter);
    
    /* Call external function with various identifiers */
    opaque_external_function(&recursive_result);
    opaque_external_function(&cs);
    opaque_external_function(&ns);
    opaque_external_function(designated_array);
    
#ifdef __cplusplus
    /* C++ specific: TREE_BINFO generation */
    std::vector<BaseClass*> objects;  /* TREE_VEC from template */
    
    DerivedClass derived;
    derived.base_data = 10;
    derived.derived_data = 20;
    
    SecondDerived second_derived;
    second_derived.base_data = 30;
    second_derived.derived_data = 40;
    
    objects.push_back(&derived);
    objects.push_back(&second_derived);
    
    /* Virtual calls through base pointer */
    int virtual_sum = 0;
    for (auto obj : objects) {
        virtual_sum += obj->virtual_method(argc);
    }
    
    /* dynamic_cast for BINFO usage */
    BaseClass* base_ptr = &derived;
    if (DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr)) {
        virtual_sum += derived_ptr->derived_data;
    }
    
    /* Template instantiation (TREE_VEC) */
    std::vector<std::vector<int>> nested_vec(10, std::vector<int>(5));
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 5; j++) {
            nested_vec[i][j] = i * j + virtual_sum;
        }
    }
    
    std::cout << "Result: " << (sum + ssa_test + virtual_sum + block_var) 
              << std::endl;
#else
    /* C version output */
    printf("Result: %d\n", sum + ssa_test + block_var + recursive_result.a);
#endif
    
    /* Complex expression to use all variables */
    int final_result = sum + ssa_test + block_var + recursive_result.a;
    
#ifdef __cplusplus
    final_result += virtual_sum;
    for (int i = 0; i < 10; i++) {
        final_result += designated_array[i];
    }
#endif
    
    return final_result % 256;
}

#ifdef __cplusplus
}  /* extern "C" */
#endif
