/* Compile with: g++ -O2 -fopenmp -std=c++11 -fdump-tree-all tree_test.cc -o tree_test */

#include <cstdio>
#include <cstdlib>

/* External function to prevent optimization */
extern "C" void opaque_external_function(int*);

/* Global identifiers for IDENTIFIER_NODE coverage */
int global_identifier_1;
static int static_identifier_2;
extern int extern_identifier_3;

/* Struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    double b;
    char c;
    int* d;
};

/* Class hierarchy for TREE_BINFO nodes */
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

/* Recursive function returning struct (CONSTRUCTOR) */
ComplexStruct recursive_struct_builder(int depth, int seed) {
    ComplexStruct result;
    result.a = seed;
    result.b = seed * 1.5;
    result.c = 'A' + (seed % 26);
    result.d = &result.a;
    
    if (depth > 0) {
        ComplexStruct nested = recursive_struct_builder(depth - 1, seed + 1);
        result.a += nested.a;
        result.b += nested.b;
    }
    
    return result;  /* CONSTRUCTOR node for return value */
}

/* Function with complex control flow for SSA_NAME and BLOCK nodes */
int complex_control_flow(int argc, char** argv) {
    volatile int x = argc;  /* Prevent optimization */
    int y, z;
    
    /* BLOCK node with local variable */
    {
        int block_local = x * 2;
        if (block_local > 10) {
            goto skip_initialization;
        }
        y = block_local + 5;
    }
    
    y = 0;  /* This will be skipped by goto */
    
skip_initialization:
    
    /* SSA_NAME generation through phi nodes */
    for (int i = 0; i < argc * 2; i++) {
        if (i % 3 == 0) {
            y = i * 2;      /* SSA phi node for y */
        } else if (i % 3 == 1) {
            y = i * 3;      /* Another SSA phi node */
        } else {
            y = i + 1;      /* Third SSA phi node */
        }
        z = y + i;          /* Uses phi result */
    }
    
    /* Another BLOCK with goto */
    {
        int hidden_value = 42;
        if (z > 100) {
            goto use_hidden;
        }
        hidden_value = 24;
use_hidden:
        y += hidden_value;
    }
    
    return y + z;
}

/* Template for TREE_VEC nodes */
template<typename T, int N>
class FixedVector {
    T data[N];
public:
    T& operator[](int idx) { return data[idx]; }
    const T& operator[](int idx) const { return data[idx]; }
};

int main(int argc, char** argv) {
    /* Use argc to prevent compile-time optimization */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    
    /* CONSTRUCTOR nodes - aggregate initialization */
    ComplexStruct structs[3] = {
        {1, 2.5, 'X', &global_identifier_1},
        {.a = 2, .b = 3.7, .c = 'Y', .d = &static_identifier_2},
        {3, 4.9, 'Z', nullptr}
    };
    
    /* Array constructor with designators */
    int array_constructor[10] = {[0] = 1, [5] = 2, [9] = 3};
    
    /* TREE_VEC through template instantiation */
    FixedVector<double, 7> vec1;
    FixedVector<int, 4> vec2;
    for (int i = 0; i < 7; i++) vec1[i] = i * 1.1;
    for (int i = 0; i < 4; i++) vec2[i] = i * 2;
    
    /* TREE_BINFO through class hierarchy */
    BaseClass* poly_obj = new DerivedClass();
    int virtual_result = poly_obj->virtual_method(iterations);
    delete poly_obj;
    
    /* Complex control flow for SSA_NAME and BLOCK */
    int control_result = complex_control_flow(argc, argv);
    
    /* OpenMP region with multiple clauses for OMP_CLAUSE nodes */
    int sum = 0;
    int private_var = 0;
    
    #pragma omp parallel for reduction(+:sum) \
            private(private_var) firstprivate(iterations) \
            shared(structs, array_constructor) \
            schedule(dynamic, 4) collapse(2) \
            if(iterations > 50)
    for (int i = 0; i < iterations; i++) {
        for (int j = 0; j < 10; j++) {
            private_var = i * j;
            sum += private_var + array_constructor[j % 10];
            
            /* Nested OpenMP directive with more clauses */
            #pragma omp simd aligned(structs:16) linear(j:1)
            for (int k = 0; k < 3; k++) {
                structs[k].a += (i + j + k) % 7;
            }
        }
    }
    
    /* Another OpenMP construct with different clauses */
    int max_val = 0;
    #pragma omp parallel sections lastprivate(max_val)
    {
        #pragma omp section
        {
            max_val = control_result;
        }
        #pragma omp section
        {
            if (virtual_result > max_val) max_val = virtual_result;
        }
    }
    
    /* Recursive function call for CONSTRUCTOR returns */
    ComplexStruct recursive_result = recursive_struct_builder(5, iterations);
    
    /* Call external function with various identifiers */
    opaque_external_function(&global_identifier_1);
    opaque_external_function(&static_identifier_2);
    opaque_external_function(&recursive_result.a);
    
    /* Use all results to prevent dead code elimination */
    int final_result = sum + control_result + virtual_result + 
                      max_val + recursive_result.a + vec2[0];
    
    printf("Final result: %d\n", final_result);
    
    /* Additional TREE_BINFO usage with dynamic_cast */
    BaseClass* base_ptr = new DerivedClass();
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        derived_ptr->derived_data = final_result % 100;
    }
    delete base_ptr;
    
    return final_result % 256;
}

/* Dummy definition to satisfy external reference */
extern "C" void opaque_external_function(int* ptr) {
    *ptr += 1;
}
