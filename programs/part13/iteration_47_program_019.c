/* Test program to trigger tree_kind dispatch for uncovered TREE_CODE cases */
#ifdef __cplusplus
#include <vector>
#include <iostream>
extern "C" {
#endif

/* External function to prevent optimization */
extern void opaque_external_function(void*);

/* Global identifiers (IDENTIFIER_NODE) */
int global_identifier_1 = 0;
static int static_identifier_2 = 0;
extern int extern_identifier_3;

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
    int arr[3];
};

/* Recursive function returning struct (CONSTRUCTOR) */
#ifdef __cplusplus
struct ComplexStruct recursive_struct_builder(int depth, int* counter) {
#else
struct ComplexStruct recursive_struct_builder(int depth, int* counter) {
#endif
    struct ComplexStruct result;
    result.a = depth;
    result.b = depth * 1.5;
    result.c = 'A' + (depth % 26);
    result.d = (void*)counter;
    
    if (depth > 0) {
        (*counter)++;
        struct ComplexStruct inner = recursive_struct_builder(depth - 1, counter);
        result.a += inner.a;
        result.b += inner.b;
    }
    
    return result;  /* CONSTRUCTOR node for return value */
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
    /* Use argc to prevent optimization */
    volatile int use_argc = argc;
    
    /* BLOCK nodes with gotos */
    {
        int block_local_1 = 10;
        goto skip_part;
        
        int unused_in_block = 20;  /* This won't be executed */
        
        skip_part:
        block_local_1 += 5;
        
        {
            /* Nested block */
            int nested_block_var = 30;
            if (use_argc > 1) {
                goto outer_label;
            }
            nested_block_var += block_local_1;
        }
        
        outer_label:
        block_local_1 += 2;
    }
    
    /* CONSTRUCTOR nodes - various initializations */
    struct ComplexStruct cs1 = {1, 2.5, 'X', (void*)&argc};
    struct ComplexStruct cs2 = {.a = 2, .c = 'Y', .b = 3.14, .d = 0};
    struct NestedStruct ns = {
        .inner = {10, 20.5, 'Z', &cs1},
        .arr = {[0] = 1, [2] = 3}  /* Designated initializer */
    };
    
    int array_constructor[5] = {[1] = 100, [3] = 300, [4] = 400};
    
    /* Recursive call for constructor nodes */
    int counter = 0;
    struct ComplexStruct recursive_result = recursive_struct_builder(3, &counter);
    
    /* SSA_NAME generation - complex control flow */
    int ssa_var = 0;
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            ssa_var += i * 2;
        } else {
            ssa_var += i * 3;
        }
        
        /* Another SSA opportunity */
        int temp;
        if (ssa_var > 20) {
            temp = ssa_var / 2;
        } else {
            temp = ssa_var * 2;
        }
        ssa_var = temp + 1;
    }
    
    /* More SSA complexity with loops */
    int x = 0, y = 0, z = 0;
    for (int i = 0; i < use_argc * 2; i++) {
        if (i % 3 == 0) {
            x = i + 1;
            y = x * 2;
        } else if (i % 3 == 1) {
            x = i - 1;
            y = x / 2;
        } else {
            x = i * 2;
            y = x + 3;
        }
        z = x + y;  /* This will create phi nodes */
    }
    
    /* OpenMP region with multiple clauses (OMP_CLAUSE) */
    int sum = 0;
    int n = 100;
    int* arr = (int*)__builtin_alloca(n * sizeof(int));
    
    #pragma omp parallel for private(i) firstprivate(n) shared(arr) reduction(+:sum) collapse(2) schedule(dynamic)
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            arr[i * 10 + j] = i * j;
            sum += arr[i * 10 + j];
        }
    }
    
    /* Nested OpenMP with more clauses */
    #pragma omp parallel num_threads(2)
    {
        #pragma omp for nowait private(x) lastprivate(y)
        for (int i = 0; i < 5; i++) {
            x = i * 2;
            y = x + 1;
        }
        
        #pragma omp single copyprivate(z)
        {
            z = 42;
        }
    }
    
#ifdef __cplusplus
    /* TREE_VEC via template instantiation */
    std::vector<int> template_vec;
    std::vector<double> another_vec;
    std::vector<std::vector<int>> nested_vec;
    
    /* TREE_BINFO via polymorphism */
    BaseClass* base_ptr;
    if (use_argc % 2 == 0) {
        base_ptr = new DerivedClass();
    } else {
        base_ptr = new SecondDerived();
    }
    
    base_ptr->base_data = 10;
    int virtual_result = base_ptr->virtual_method(5);
    
    /* dynamic_cast for BINFO usage */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        derived_ptr->derived_data = 20;
    }
    
    delete base_ptr;
#endif
    
    /* Call external function with various identifiers */
    opaque_external_function(&cs1);
    opaque_external_function(&recursive_result);
    opaque_external_function(&ssa_var);
    
    /* Use all computed values to prevent elimination */
    int final_result = 
        cs1.a + 
        ns.arr[0] + 
        ssa_var + 
        sum + 
        z +
#ifdef __cplusplus
        virtual_result +
#endif
        counter;
    
    /* Print to ensure code isn't eliminated */
    printf("Result: %d\n", final_result);
    
    return final_result > 100 ? 0 : 1;
}

#ifdef __cplusplus
}  /* extern "C" */
#endif
