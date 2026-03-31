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
    result.d = &global_identifier_1;
    
    if (depth > 0) {
        ComplexStruct nested = recursive_struct_builder(depth - 1, seed + 1);
        result.a += nested.a;
        result.b += nested.b;
    }
    
    return result;  /* CONSTRUCTOR node for return value */
}

/* Function with complex control flow for SSA_NAME and BLOCK nodes */
int ssa_generator(int x, int y) {
    int result;
    
    /* BLOCK node with local variable */
    {
        int local_in_block = x * 2;
        
        /* Conditional with phi node potential */
        if (x > 0) {
            result = local_in_block + y;
        } else {
            result = y - local_in_block;
        }
        
        /* Another block with goto */
        goto skip_part;
        
        {
            int hidden_var = 100;  /* This won't be executed due to goto */
            result += hidden_var;
        }
        
        skip_part:
        /* Use result to keep it alive */
        result += 10;
    }
    
    /* Loop with SSA formation */
    int sum = 0;
    for (int i = 0; i < x; i++) {
        int temp;
        if (i % 2 == 0) {
            temp = i * 2;
        } else {
            temp = i * 3;
        }
        sum += temp;  /* SSA_NAME for temp and sum */
    }
    
    return result + sum;
}

int main(int argc, char** argv) {
    /* Use argc to prevent optimization */
    int iterations = argc > 1 ? atoi(argv[1]) : 100;
    int seed = argc > 2 ? atoi(argv[2]) : 42;
    
    /* CONSTRUCTOR nodes for aggregate initialization */
    ComplexStruct cs1 = {1, 2.5, 'X', &global_identifier_1};
    ComplexStruct cs2 = {.a = 2, .b = 3.14, .c = 'Y', .d = &static_identifier_2};
    
    /* Array with designated initializer (TREE_VEC potential) */
    int arr[10] = {[0] = 1, [5] = 2, [9] = 3};
    
    /* TREE_VEC through template-like usage (C++ specific) */
    struct IntVec {
        int data[3];
        int size;
    };
    IntVec vec = {{1, 2, 3}, 3};
    
    /* BLOCK nodes with nested scopes */
    {
        int block_var_1 = 10;
        {
            int block_var_2 = 20;
            block_var_1 += block_var_2;
            
            /* goto between blocks */
            if (block_var_1 > 15) {
                goto outer_block;
            }
            
            int unused = 99;  /* Might be optimized out */
        }
        
        outer_block:
        block_var_1 += 5;
    }
    
    /* Generate SSA_NAME nodes */
    int ssa_result = ssa_generator(iterations, seed);
    
    /* OpenMP region with multiple clauses (OMP_CLAUSE nodes) */
    int openmp_sum = 0;
    #pragma omp parallel for reduction(+:openmp_sum) \
            private(seed) firstprivate(iterations) \
            shared(arr, cs1) collapse(2) schedule(dynamic)
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int private_var = i * j;  /* Private to each thread */
            openmp_sum += private_var + arr[i];
        }
    }
    
    /* Additional OpenMP clauses */
    #pragma omp parallel sections private(ssa_result)
    {
        #pragma omp section
        {
            ssa_result += 1;
        }
        #pragma omp section
        {
            ssa_result += 2;
        }
    }
    
    /* C++ class hierarchy usage for TREE_BINFO */
    BaseClass* base_ptr;
    if (iterations % 2 == 0) {
        base_ptr = new BaseClass();
    } else {
        base_ptr = new DerivedClass();  /* Dynamic type */
    }
    
    /* Virtual call through pointer */
    int virtual_result = base_ptr->virtual_method(seed);
    
    /* dynamic_cast for BINFO usage */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        virtual_result += derived_ptr->derived_data;
    }
    
    /* Call recursive function for CONSTRUCTOR nodes */
    ComplexStruct recursive_result = recursive_struct_builder(3, seed);
    
    /* Use various identifiers (IDENTIFIER_NODE) */
    global_identifier_1 = ssa_result;
    static_identifier_2 = openmp_sum;
    extern_identifier_3 = virtual_result;
    
    /* Call external function to prevent dead code elimination */
    opaque_external_function(&global_identifier_1);
    
    /* Complex expression using all results */
    int final_result = 
        ssa_result + 
        openmp_sum + 
        virtual_result + 
        recursive_result.a +
        cs1.a +
        cs2.a +
        arr[0] +
        vec.data[0];
    
    printf("Final result: %d\n", final_result);
    
    delete base_ptr;
    return final_result > 0 ? 0 : 1;
}

/* Dummy implementation of external function */
extern "C" void opaque_external_function(int* x) {
    *x += 1;
}
