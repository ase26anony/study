/* Compile with: g++ -O2 -fopenmp -fdump-tree-all -std=c++11 tree_coverage.cc */

#include <cstdio>
#include <cstdlib>
#include <cstring>

/* External function to prevent optimization - creates IDENTIFIER_NODEs */
extern "C" void opaque_external_function(int*);
extern "C" void another_external_func(float*);
extern "C" void yet_another_extern(double*);

/* Global variables for IDENTIFIER_NODE creation */
static int static_identifier_1 = 0;
extern int extern_identifier_2;
volatile int volatile_identifier_3;

/* Complex struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    double b;
    char c;
    float d[3];
};

/* Nested struct for more complex CONSTRUCTOR */
struct NestedStruct {
    ComplexStruct inner;
    int* ptr;
    short s;
};

/* C++ classes for TREE_BINFO nodes */
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

class AnotherDerived : public BaseClass {
public:
    virtual int virtual_method(int x) override { return x * 4; }
    int another_data;
};

/* Recursive function returning struct (CONSTRUCTOR) */
ComplexStruct recursive_struct_builder(int depth, int value) {
    ComplexStruct result;
    result.a = value;
    result.b = value * 1.5;
    result.c = 'A' + (value % 26);
    
    if (depth > 0) {
        ComplexStruct inner = recursive_struct_builder(depth - 1, value + 1);
        result.d[0] = inner.a + inner.b;
        result.d[1] = inner.d[0];
        result.d[2] = depth * 0.5f;
    } else {
        result.d[0] = 1.0f;
        result.d[1] = 2.0f;
        result.d[2] = 3.0f;
    }
    
    return result;  /* CONSTRUCTOR node for return value */
}

/* Function with SSA_NAME generation */
int ssa_generator(int iterations, int seed) {
    int x = seed;
    int y, z;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex conditional creating phi nodes */
        if (i % 3 == 0) {
            y = x * 2;
        } else if (i % 3 == 1) {
            y = x + i;
        } else {
            y = x - i;
        }
        
        /* Another SSA opportunity */
        if (y > 100) {
            z = y / 2;
        } else {
            z = y * 3;
        }
        
        x = z + (i % 5);  /* Creates phi node for x */
        
        /* Prevent loop optimization */
        volatile_identifier_3 = x;
    }
    
    return x;
}

/* Function with BLOCK nodes and goto */
int block_and_goto_example(int param) {
    int result = 0;
    
    /* First block with local variable */
    {
        int block_local_1 = param * 2;
        result += block_local_1;
        
        if (param > 10) {
            goto middle_block;  /* Jump to middle of next block */
        }
    }
    
    /* Second block - partially skipped by goto */
    {
        int block_local_2 = param * 3;
        int hidden_variable = 42;  /* This might be skipped */
        
        middle_block:
        result += block_local_2;
        
        /* Nested block inside block */
        {
            int deeply_nested = hidden_variable + 1;
            result += deeply_nested;
        }
    }
    
    return result;
}

int main(int argc, char** argv) {
    /* Use argc to prevent compile-time optimization */
    int iterations = argc > 1 ? atoi(argv[1]) : 100;
    int seed = argc > 2 ? atoi(argv[2]) : 42;
    
    /* 1. CONSTRUCTOR nodes - aggregate initialization */
    ComplexStruct cs = {1, 2.5, 'X', {1.1f, 2.2f, 3.3f}};
    NestedStruct ns = {{2, 3.14, 'Y', {4.4f, 5.5f, 6.6f}}, nullptr, 7};
    
    /* Array with designated initializer (TREE_VEC in C++) */
    int arr[10] = {[0] = 1, [5] = 2, [9] = 3};
    
    /* 2. Recursive struct building */
    ComplexStruct recursive_result = recursive_struct_builder(3, seed);
    
    /* 3. SSA_NAME generation */
    int ssa_result = ssa_generator(iterations, seed);
    
    /* 4. BLOCK and goto */
    int block_result = block_and_goto_example(iterations % 20);
    
    /* 5. C++ classes for TREE_BINFO */
    BaseClass* base_ptr;
    if (iterations % 2 == 0) {
        base_ptr = new DerivedClass();
    } else {
        base_ptr = new AnotherDerived();
    }
    
    /* Virtual call through base pointer */
    int virtual_result = base_ptr->virtual_method(seed);
    
    /* dynamic_cast for BINFO usage */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        virtual_result += 1000;
    }
    
    /* 6. OpenMP with multiple clauses (OMP_CLAUSE nodes) */
    int sum = 0;
    int matrix[100][100];
    
    /* Initialize matrix */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            matrix[i][j] = i * j + seed;
        }
    }
    
    #pragma omp parallel for collapse(2) \
        private(iterations) \
        firstprivate(seed) \
        shared(matrix) \
        reduction(+:sum) \
        schedule(dynamic, 10)
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            sum += matrix[i][j];
            /* Nested OpenMP region */
            #pragma omp atomic
            volatile_identifier_3++;
        }
    }
    
    /* 7. Template usage for TREE_VEC (C++ specific) */
    struct TemplateLike {
        template<typename T>
        T process(T input) {
            return input * 2;
        }
    };
    
    TemplateLike processor;
    int template_result = processor.process(seed);
    
    /* 8. Call external functions with various identifiers */
    opaque_external_function(&ssa_result);
    another_external_func(&cs.d[0]);
    yet_another_extern(&recursive_result.b);
    
    /* 9. Complex expression with multiple identifiers */
    int final_result = 
        static_identifier_1 +
        virtual_result +
        ssa_result +
        block_result +
        sum % 10000 +
        template_result +
        cs.a +
        (derived_ptr ? 1 : 0);
    
    printf("Final result: %d\n", final_result);
    
    delete base_ptr;
    return final_result > 0 ? 0 : 1;
}

/* External function definitions to satisfy linker */
extern "C" void opaque_external_function(int* x) {
    *x += 1;
}

extern "C" void another_external_func(float* f) {
    *f = *f * 2.0f;
}

extern "C" void yet_another_extern(double* d) {
    *d = *d / 2.0;
}

/* Global variable definition */
int extern_identifier_2 = 100;
