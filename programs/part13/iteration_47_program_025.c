/* Compile with: g++ -O2 -fopenmp -fdump-tree-all -std=c++11 tree_test.cc */

#include <cstdio>
#include <cstdlib>

/* External function to prevent optimization */
extern "C" void opaque_external_function(int*);

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

/* Recursive function returning struct (CONSTRUCTOR) */
ComplexStruct recursive_struct_builder(int depth, int* counter) {
    ComplexStruct result;
    result.a = depth;
    result.b = depth * 3.14;
    result.c = 'A' + (depth % 26);
    result.d = counter;
    
    if (depth > 0) {
        ComplexStruct inner = recursive_struct_builder(depth - 1, counter);
        result.a += inner.a;
        result.b += inner.b;
        (*counter)++;
    }
    return result;  /* CONSTRUCTOR node for return value */
}

/* C++ classes for TREE_BINFO coverage */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int virtual_method(int x) { return x * 2; }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int virtual_method(int x) override { return x * 3; }
    int derived_data;
};

template<typename T>
class TemplateClass {
public:
    T data;
    void method() { data = T(); }
};

/* Function with complex control flow for SSA_NAME and BLOCK nodes */
int complex_control_flow(int argc, char** argv) {
    volatile int seed = argc;  /* Prevent constant folding */
    int result = 0;
    
    /* BLOCK node with local variable */
    {
        int local_in_block = seed * 2;
        result += local_in_block;
        
        /* Jump to label skipping initialization */
        if (seed > 100) goto skip_init;
        
        int another_local = 42;
        result += another_local;
        
    skip_init:
        /* Use potentially uninitialized variable to force SSA phi nodes */
        int phi_candidate;
        if (seed % 2 == 0) {
            phi_candidate = 10;
        } else {
            phi_candidate = 20;
        }
        result += phi_candidate;  /* SSA_NAME node here */
    }
    
    /* Another BLOCK with goto */
    {
        int x = 5;
        goto middle;
        
        int unused = 99;  /* Will be skipped */
        
    middle:
        x += 10;
        result += x;
    }
    
    return result;
}

/* Function with TREE_VEC (template instantiation) */
void template_instantiation_test() {
    /* TREE_VEC nodes from template instantiation */
    TemplateClass<int> int_template;
    TemplateClass<double> double_template;
    TemplateClass<ComplexStruct> struct_template;
    
    int_template.method();
    double_template.method();
    struct_template.method();
}

int main(int argc, char** argv) {
    int final_result = 0;
    
    /* Use command line args to prevent optimization */
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    int use_openmp = (argc > 2) ? atoi(argv[2]) : 1;
    
    /* CONSTRUCTOR nodes - aggregate initialization */
    ComplexStruct structs[] = {
        {1, 3.14, 'A', &global_var_1},
        {2, 6.28, 'B', &static_var_2},
        {3, 9.42, 'C', nullptr}
    };
    
    /* Array with designated initializer (more CONSTRUCTOR nodes) */
    int designated_array[10] = {[0] = 1, [5] = 2, [9] = 3};
    
    /* Complex array initializer */
    int matrix[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    
    /* Process structs */
    for (int i = 0; i < 3; i++) {
        final_result += structs[i].a;
    }
    
    /* Recursive function call */
    int counter = 0;
    ComplexStruct recursive_result = recursive_struct_builder(3, &counter);
    final_result += recursive_result.a;
    
    /* Complex control flow */
    final_result += complex_control_flow(argc, argv);
    
    /* Template instantiation */
    template_instantiation_test();
    
    /* C++ class hierarchy for TREE_BINFO */
    BaseClass* base_ptr;
    if (iterations % 2 == 0) {
        base_ptr = new DerivedClass();
    } else {
        base_ptr = new BaseClass();
    }
    
    /* Virtual call - uses BINFO for dispatch */
    final_result += base_ptr->virtual_method(iterations);
    
    /* Dynamic cast for BINFO usage */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        final_result += 1000;
    }
    
    delete base_ptr;
    
    /* OpenMP region with multiple clauses for OMP_CLAUSE nodes */
    if (use_openmp) {
        int sum = 0;
        int private_var = 42;
        int shared_var = 0;
        
        #pragma omp parallel for reduction(+:sum) \
                private(private_var) firstprivate(final_result) \
                shared(shared_var) collapse(2) schedule(dynamic)
        for (int i = 0; i < iterations; i++) {
            for (int j = 0; j < iterations; j++) {
                /* SSA_NAME generation inside parallel region */
                int temp;
                if ((i + j) % 2 == 0) {
                    temp = i * j;
                } else {
                    temp = i + j;
                }
                sum += temp;
                private_var = i;
            }
        }
        
        /* Nested OpenMP directive */
        #pragma omp parallel
        {
            #pragma omp single
            {
                #pragma omp task shared(shared_var)
                {
                    shared_var = sum;
                }
            }
        }
        
        final_result += sum + shared_var;
    }
    
    /* Call external function with various identifiers */
    opaque_external_function(&final_result);
    
    /* Use all global identifiers */
    global_var_1 = final_result;
    static_var_2 = final_result * 2;
    
    /* Print result to ensure code isn't eliminated */
    printf("Final result: %d\n", final_result);
    
    return (final_result > 1000) ? 0 : 1;
}

/* Dummy definition to satisfy linker (in real test would be in separate file) */
extern "C" void opaque_external_function(int* x) {
    *x += *x % 7;
}
