/* Test program to trigger tree_kind dispatch for various TREE_CODE cases */
#ifdef __cplusplus
#include <vector>
#include <iostream>
using namespace std;

/* For TREE_BINFO - C++ class hierarchy */
class Base {
public:
    virtual int method() { return 1; }
    virtual ~Base() {}
};

class Derived : public Base {
public:
    virtual int method() override { return 2; }
    int extra() { return 3; }
};

class Derived2 : public Derived {
public:
    virtual int method() override { return 4; }
};
#endif

/* External function declarations to prevent optimization */
extern void opaque_external_function(int*);
extern int unpredictable(int);

/* Global identifiers (IDENTIFIER_NODE) */
int global_counter = 0;
static int static_hidden = 42;
volatile int volatile_signal = 0;

/* Struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    int b;
    int c[3];
};

struct NestedStruct {
    ComplexStruct inner;
    float f;
};

/* Recursive function returning struct (CONSTRUCTOR) */
#ifdef __cplusplus
ComplexStruct recursive_builder(int depth, int seed) {
    ComplexStruct result;
    result.a = seed;
    result.b = depth;
    
    if (depth > 0) {
        ComplexStruct inner = recursive_builder(depth - 1, seed * 2);
        result.c[0] = inner.a;
        result.c[1] = inner.b;
        result.c[2] = depth * seed;
    } else {
        result.c[0] = result.c[1] = result.c[2] = 0;
    }
    
    return result;  /* CONSTRUCTOR node for return value */
}
#else
struct ComplexStruct recursive_builder(int depth, int seed) {
    struct ComplexStruct result;
    result.a = seed;
    result.b = depth;
    
    if (depth > 0) {
        struct ComplexStruct inner = recursive_builder(depth - 1, seed * 2);
        result.c[0] = inner.a;
        result.c[1] = inner.b;
        result.c[2] = depth * seed;
    } else {
        result.c[0] = result.c[1] = result.c[2] = 0;
    }
    
    return result;
}
#endif

/* Function with complex control flow for SSA_NAME and BLOCK nodes */
int complex_control_flow(int n, int* results) {
    int sum = 0;
    
    /* Outer block with local variables */
    {
        int block_local = n * 2;
        
        /* goto for BLOCK node stress */
        if (n < 0) goto abnormal_exit;
        
        for (int i = 0; i < n; i++) {
            /* Inner block */
            {
                int inner_local = i * 3;
                
                /* Conditional for SSA phi nodes */
                int temp;
                if (i % 3 == 0) {
                    temp = inner_local + block_local;
                } else if (i % 3 == 1) {
                    temp = inner_local - block_local;
                } else {
                    temp = inner_local * block_local;
                }
                
                /* Multiple assignments to same variable for SSA */
                int ssa_var = temp;
                if (i % 2 == 0) {
                    ssa_var += unpredictable(i);
                }
                ssa_var *= 2;
                
                results[i] = ssa_var;
                sum += ssa_var;
            }
        }
        
        abnormal_exit:
        /* Use block_local after label to prevent optimization */
        sum += block_local;
    }
    
    return sum;
}

int main(int argc, char** argv) {
    int result = 0;
    
    /* Use argc to prevent compile-time optimization */
    int iterations = (argc > 1) ? unpredictable(argc) : 10;
    if (iterations < 2) iterations = 2;
    if (iterations > 100) iterations = 100;
    
    /* CONSTRUCTOR nodes - various initializers */
    struct ComplexStruct cs1 = {1, 2, {3, 4, 5}};
    struct ComplexStruct cs2 = {.a = 10, .c = {[1] = 20, [0] = 30}};
    struct NestedStruct ns = {{5, 6, {7, 8, 9}}, 3.14f};
    
    int array_init[5] = {[0] = 100, [3] = 200, [2] = unpredictable(1)};
    
    /* Call recursive function */
    struct ComplexStruct recursive_result = recursive_builder(3, iterations);
    result += recursive_result.a + recursive_result.c[1];
    
    /* Complex control flow for SSA_NAME and BLOCK */
    int* dynamic_array = (int*)__builtin_alloca(iterations * sizeof(int));
    result += complex_control_flow(iterations, dynamic_array);
    
    /* OpenMP region with multiple clauses (OMP_CLAUSE) */
    int openmp_sum = 0;
    int shared_counter = 0;
    
    #pragma omp parallel for reduction(+:openmp_sum) \
            private(iterations) firstprivate(result) \
            shared(shared_counter) collapse(2) \
            schedule(dynamic)
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int private_var = i * 10 + j;
            #pragma omp atomic
            shared_counter++;
            openmp_sum += private_var + result;
        }
    }
    
    result += openmp_sum + shared_counter;
    
    /* Additional OpenMP constructs */
    #pragma omp parallel
    {
        #pragma omp sections private(cs1)
        {
            #pragma omp section
            {
                cs1.a = 100;
                opaque_external_function(&cs1.a);
            }
            #pragma omp section
            {
                cs1.b = 200;
            }
        }
        
        #pragma omp single copyprivate(cs2)
        {
            cs2 = recursive_result;
        }
    }
    
    #ifdef __cplusplus
    /* TREE_VEC via template instantiation */
    vector<int> template_vec;
    for (int i = 0; i < iterations; i++) {
        template_vec.push_back(unpredictable(i));
    }
    
    /* TREE_BINFO via polymorphism */
    Base* poly_obj;
    if (iterations % 2 == 0) {
        poly_obj = new Derived();
    } else {
        poly_obj = new Derived2();
    }
    
    result += poly_obj->method();  /* Virtual call */
    
    Base& ref = *poly_obj;
    result += ref.method();  /* Another virtual call */
    
    /* dynamic_cast for BINFO usage */
    if (Derived* d = dynamic_cast<Derived*>(poly_obj)) {
        result += d->extra();
    }
    
    delete poly_obj;
    
    /* More TREE_VEC with nested templates */
    vector<vector<float>> nested_vec(3);
    for (int i = 0; i < 3; i++) {
        nested_vec[i].push_back(i * 1.5f);
    }
    #endif
    
    /* Call external function with various identifiers */
    opaque_external_function(&result);
    opaque_external_function(&global_counter);
    opaque_external_function(&static_hidden);
    opaque_external_function(&volatile_signal);
    opaque_external_function(array_init);
    opaque_external_function(dynamic_array);
    
    /* Final computation to ensure all code is live */
    volatile int final_check = 0;
    for (int i = 0; i < iterations; i++) {
        final_check += dynamic_array[i] + array_init[i % 5];
    }
    
    result += final_check + volatile_signal;
    
    /* Prevent dead code elimination */
    if (unpredictable(result) != 0) {
        return result % 255;
    }
    
    return 0;
}
