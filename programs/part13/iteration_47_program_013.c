/* Compile with: g++ -O2 -fopenmp -std=c++11 -fdump-tree-all tree_test.cc -o tree_test */

#include <cstdio>
#include <cstdlib>
#include <omp.h>

/* External function to prevent optimization - creates IDENTIFIER_NODE */
extern "C" void opaque_external_function(int*);

/* Struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    double b;
    char c;
    int* d;
};

/* Array for TREE_VEC representation */
typedef int MultiDimArray[2][3][4];

/* Global variables for IDENTIFIER_NODES */
static int static_global_counter = 0;
extern int external_global_reference;

/* Recursive function returning struct (CONSTRUCTOR) */
ComplexStruct recursive_struct_builder(int depth, int seed) {
    ComplexStruct result;
    result.a = seed;
    result.b = seed * 1.5;
    result.c = 'A' + (seed % 26);
    
    if (depth > 0) {
        ComplexStruct inner = recursive_struct_builder(depth - 1, seed + 1);
        result.a += inner.a;
        result.b += inner.b;
        result.d = &inner.a;  /* Create pointer reference */
    } else {
        result.d = &result.a;
    }
    
    /* BLOCK with local variable and goto */
    {
        int block_local = result.a * 2;
        if (block_local > 100) {
            goto skip_calculation;
        }
        result.b += block_local;
        skip_calculation:
        /* Use volatile to prevent optimization */
        volatile int force_use = block_local;
        (void)force_use;
    }
    
    return result;
}

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
        /* SSA_NAME generation inside virtual method */
        int ssa_var;
        if (x > 0) {
            ssa_var = x * 3;
        } else {
            ssa_var = x * 4;
        }
        /* Force SSA phi node */
        int result = ssa_var + base_data;
        return result;
    }
    
    int derived_data;
};

/* Template for TREE_VEC generation */
template<typename T, int N>
class TemplateContainer {
    T data[N];
public:
    TemplateContainer() {
        for (int i = 0; i < N; i++) {
            data[i] = T();
        }
    }
    
    T get(int index) const {
        return data[index];
    }
};

int main(int argc, char* argv[]) {
    /* Use argc to prevent compile-time optimization */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    int seed = (argc > 2) ? atoi(argv[2]) : 42;
    
    /* CONSTRUCTOR: Aggregate initialization */
    ComplexStruct cs = {1, 2.5, 'X', nullptr};
    
    /* CONSTRUCTOR: Designated initializer (C++20) */
    int arr[10] = {[0] = 1, [5] = 2, [9] = 3};
    
    /* TREE_VEC: Template instantiation */
    TemplateContainer<double, 5> container;
    
    /* TREE_BINFO: Class hierarchy usage */
    BaseClass* poly_obj = new DerivedClass();
    poly_obj->base_data = seed;
    
    /* Complex control flow with BLOCK nodes */
    int total = 0;
    
    /* Outer block with goto */
    {
        int outer_block_var = 0;
        if (seed % 3 == 0) {
            goto middle_of_block;
        }
        
        outer_block_var = seed * 2;
        
        middle_of_block:
        /* Nested block */
        {
            int inner_block_var = outer_block_var + 1;
            total += inner_block_var;
        }
    }
    
    /* OpenMP region with multiple clauses - generates OMP_CLAUSE nodes */
    #pragma omp parallel reduction(+:total) private(seed) shared(cs, arr) \
            firstprivate(iterations) if(iterations > 50)
    {
        int local_sum = 0;
        
        /* Nested OpenMP for more clauses */
        #pragma omp for schedule(dynamic, 4) collapse(2) nowait
        for (int i = 0; i < iterations; i++) {
            for (int j = 0; j < 10; j++) {
                /* SSA_NAME: Conditional assignment creating phi nodes */
                int ssa_variable;
                if ((i + j) % 2 == 0) {
                    ssa_variable = i * j;
                } else {
                    ssa_variable = i + j;
                }
                
                /* Use in expression to keep alive */
                local_sum += ssa_variable;
                
                /* More SSA complexity */
                for (int k = 0; k < 3; k++) {
                    int inner_ssa;
                    if (k % 2 == 0) {
                        inner_ssa = ssa_variable * k;
                    } else {
                        inner_ssa = ssa_variable / (k + 1);
                    }
                    local_sum += inner_ssa;
                }
            }
        }
        
        total += local_sum;
        
        /* Call virtual method (uses BINFO) */
        if (omp_get_thread_num() == 0) {
            total += poly_obj->virtual_method(local_sum % 100);
        }
    }
    
    /* Recursive call for CONSTRUCTOR nodes */
    ComplexStruct recursive_result = recursive_struct_builder(3, seed);
    total += recursive_result.a;
    
    /* Multi-dimensional array initializer (TREE_VEC-like) */
    MultiDimArray mda = {
        {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}},
        {{13, 14, 15, 16}, {17, 18, 19, 20}, {21, 22, 23, 24}}
    };
    
    /* Process array */
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 4; k++) {
                total += mda[i][j][k];
            }
        }
    }
    
    /* Call external function with IDENTIFIER_NODES */
    opaque_external_function(&total);
    
    /* Use static and potentially external identifiers */
    static_global_counter++;
    total += static_global_counter;
    
    /* Final output to ensure all code is live */
    printf("Result: %d\n", total);
    
    delete poly_obj;
    return 0;
}

/* Dummy external function definition to satisfy linker */
extern "C" void opaque_external_function(int* ptr) {
    *ptr += 1;
}

int external_global_reference = 0;
