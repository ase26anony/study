/* Compile with: g++ -O2 -fopenmp -std=c++11 -fdump-tree-all tree_test.cc -o tree_test */

#include <cstdio>
#include <cstdlib>

/* External function to prevent optimization */
extern "C" void opaque_external_function(int*);

/* Global identifiers for IDENTIFIER_NODE coverage */
int global_identifier_1;
static int static_identifier_2;
extern int extern_identifier_3;

/* Complex struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    double b;
    char c;
    int* d;
};

/* Recursive function returning struct (CONSTRUCTOR) */
ComplexStruct recursive_constructor(int depth, int* counter) {
    ComplexStruct result;
    result.a = depth;
    result.b = depth * 1.5;
    result.c = 'A' + (depth % 26);
    result.d = counter;
    
    if (depth > 0) {
        ComplexStruct inner = recursive_constructor(depth - 1, counter);
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

class SecondDerived : public DerivedClass {
public:
    virtual int virtual_method(int x) override { return x * 4; }
};

/* Function with complex control flow for SSA_NAME and BLOCK nodes */
int complex_control_flow(int n, int* results) {
    int ssa_var = 0;
    
    BLOCK_1: {
        int block_local_1 = n * 2;
        volatile int prevent_opt = block_local_1;
        
        if (prevent_opt > 100) {
            goto BLOCK_3;
        }
        
        BLOCK_2: {
            int block_local_2 = n / 2;
            for (int i = 0; i < n; i++) {
                /* Create phi node for SSA_NAME */
                if (i % 2 == 0) {
                    ssa_var = block_local_1 + i;
                } else {
                    ssa_var = block_local_2 - i;
                }
                results[i] = ssa_var;  /* Force SSA use */
            }
            goto BLOCK_4;
        }
        
        BLOCK_3: {
            int block_local_3 = n * 3;
            for (int i = 0; i < n; i++) {
                ssa_var = block_local_3 ^ i;
                results[i] = ssa_var;
            }
        }
    }
    
    BLOCK_4: {
        int final_value = ssa_var;
        /* Use goto to create interesting control flow */
        if (final_value < 0) {
            goto BLOCK_1;
        }
        return final_value;
    }
}

/* Function with TREE_VEC (template/array initializer) */
void tree_vec_generator(int size) {
    /* Complex array initializer that may create TREE_VEC */
    int complex_array[5][3] = {
        {1, 2, 3},
        {[0] = 4, [2] = 5},
        {6, 7, 8},
        {9},
        {10, 11, 12}
    };
    
    /* Multi-dimensional designated initializer */
    int md_array[2][3][2] = {
        [0][1][0] = 13,
        [1][2][1] = 14,
        [0][0] = {15, 16}
    };
    
    /* Use volatile to prevent removal */
    volatile int* ptr = &complex_array[0][0];
    opaque_external_function(ptr);
}

int main(int argc, char** argv) {
    /* Use argc to prevent compile-time optimization */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    
    /* CONSTRUCTOR nodes - aggregate initialization */
    ComplexStruct cs1 = {1, 2.5, 'X', &iterations};
    ComplexStruct cs2 = {.a = 2, .b = 3.14, .c = 'Y', .d = &iterations};
    ComplexStruct cs_array[3] = {
        {10, 20.5, 'A', nullptr},
        {.b = 30.5, .a = 20, .c = 'B'},
        {}
    };
    
    /* Recursive constructor generation */
    int counter = 0;
    ComplexStruct recursive_result = recursive_constructor(5, &counter);
    
    /* SSA_NAME generation with complex control flow */
    int* results = new int[iterations];
    int ssa_result = complex_control_flow(iterations, results);
    
    /* TREE_VEC generation */
    tree_vec_generator(iterations);
    
    /* OpenMP region with multiple clauses for OMP_CLAUSE nodes */
    int sum = 0;
    #pragma omp parallel for reduction(+:sum) private(counter) \
            firstprivate(iterations) shared(results) collapse(2) \
            schedule(dynamic, 4)
    for (int i = 0; i < iterations; i++) {
        for (int j = 0; j < 10; j++) {
            /* Nested OpenMP directive */
            #pragma omp simd reduction(+:sum)
            for (int k = 0; k < 5; k++) {
                sum += results[i] + j + k;
            }
        }
    }
    
    /* Additional OpenMP constructs */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task private(counter)
            {
                counter = iterations;
            }
        }
        
        #pragma omp sections
        {
            #pragma omp section
            { sum += 1; }
            
            #pragma omp section
            { sum += 2; }
        }
    }
    
    /* C++ class hierarchy for TREE_BINFO */
    BaseClass* base_ptr;
    DerivedClass derived_obj;
    SecondDerived second_derived_obj;
    
    /* Dynamic dispatch for virtual table access */
    if (sum % 2 == 0) {
        base_ptr = &derived_obj;
    } else {
        base_ptr = &second_derived_obj;
    }
    
    int virtual_result = base_ptr->virtual_method(iterations);
    
    /* Use all identifiers */
    global_identifier_1 = sum;
    static_identifier_2 = virtual_result;
    opaque_external_function(&extern_identifier_3);
    
    /* Complex expression with multiple identifiers */
    int final_result = global_identifier_1 + static_identifier_2 + 
                      extern_identifier_3 + ssa_result + sum + virtual_result +
                      recursive_result.a + (int)recursive_result.b;
    
    printf("Final result: %d\n", final_result);
    
    delete[] results;
    return final_result % 256;
}

/* External function definition to satisfy linker */
extern "C" void opaque_external_function(int* x) {
    static int state = 0;
    *x ^= state;
    state = (state * 1103515245 + 12345) & 0x7fffffff;
}
