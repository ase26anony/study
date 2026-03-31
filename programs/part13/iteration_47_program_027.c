/* Test program to trigger tree_kind dispatch for uncovered TREE_CODE cases */
#ifdef __cplusplus
#include <vector>
#include <iostream>
extern "C" {
#endif

/* External function to prevent optimization */
extern void opaque_external_function(int*);

/* Global identifiers (IDENTIFIER_NODE) */
int global_counter = 0;
static int static_hidden = 42;
extern int external_reference;

/* Struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    double b;
    char c;
    int* d;
};

/* Array with complex initializer (TREE_VEC in C mode) */
int complex_array[5] = {[0] = 1, [2] = 3, [4] = 5};

/* Recursive function returning struct (CONSTRUCTOR) */
struct ComplexStruct recursive_constructor(int depth, int* counter) {
    struct ComplexStruct result;
    result.a = depth;
    result.b = depth * 1.5;
    result.c = 'A' + (depth % 26);
    result.d = counter;
    
    if (depth > 0) {
        struct ComplexStruct inner = recursive_constructor(depth - 1, counter);
        result.a += inner.a;
        (*counter)++;
    }
    
    return result;
}

#ifdef __cplusplus
/* C++ specific code for TREE_BINFO */
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

template<typename T>
class TemplateClass {
public:
    std::vector<T> data;
    void add(const T& item) {
        data.push_back(item);
    }
};
#endif

int main(int argc, char** argv) {
    volatile int use_argc = argc; /* Prevent optimization */
    
    /* BLOCK nodes with goto */
    {
        int block_local = 10;
        goto skip_init;
        int unused = 20; /* This won't be initialized due to goto */
    skip_init:
        block_local += 5;
        
        /* Another nested block */
        {
            int inner_block = 30;
            block_local += inner_block;
        }
    }
    
    /* CONSTRUCTOR nodes */
    struct ComplexStruct cs = {.a = 1, .b = 2.5, .c = 'X', .d = &global_counter};
    struct ComplexStruct cs2 = recursive_constructor(3, &global_counter);
    
    /* SSA_NAME generation with complex control flow */
    int ssa_var = 0;
    for (int i = 0; i < (use_argc > 1 ? 10 : 20); i++) {
        if (i % 3 == 0) {
            ssa_var += i * 2;
        } else if (i % 3 == 1) {
            ssa_var += i * 3;
        } else {
            ssa_var += i;
        }
        
        /* Additional SSA complexity */
        int temp = ssa_var;
        for (int j = 0; j < (i % 5); j++) {
            temp += j;
        }
        ssa_var = temp;
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
    
    /* Additional OpenMP clauses */
    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            int single_var = 1;
        }
        
        #pragma omp for ordered
        for (int i = 0; i < 5; i++) {
            #pragma omp ordered
            {
                sum += i;
            }
        }
    }
    
#ifdef __cplusplus
    /* C++ specific: TREE_BINFO and TREE_VEC through templates */
    BaseClass* base_ptr = new DerivedClass();
    base_ptr->base_data = 10;
    int virt_result = base_ptr->virtual_method(5);
    
    SecondDerived sd;
    sd.base_data = 20;
    sd.derived_data = 30;
    
    TemplateClass<int> tc;
    tc.add(1);
    tc.add(2);
    tc.add(3);
    
    std::vector<double> vec_double = {1.1, 2.2, 3.3, 4.4};
    
    delete base_ptr;
#endif
    
    /* Call external function with various identifiers */
    opaque_external_function(&global_counter);
    opaque_external_function(&ssa_var);
    opaque_external_function(&sum);
    
    /* Final computation to ensure all code is live */
    int final_result = global_counter + ssa_var + sum + cs.a;
#ifdef __cplusplus
    final_result += virt_result;
#endif
    
    /* Use result to prevent dead code elimination */
    if (final_result > 1000) {
        return 1;
    }
    
    return 0;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
