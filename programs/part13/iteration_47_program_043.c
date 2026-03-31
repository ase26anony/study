/* Compile with: g++ -O2 -fopenmp -std=c++11 -fdump-tree-all tree_test.cc -o tree_test */

#include <cstdio>
#include <cstdlib>

/* External function to prevent optimization */
extern "C" void opaque_external_function(int*);

/* Global identifiers (IDENTIFIER_NODE) */
int global_identifier_1 = 0;
static int static_identifier_2 = 0;
extern int extern_identifier_3;

/* Complex struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    double b;
    int* c;
    struct Inner {
        float x;
        float y;
    } inner;
};

/* Array with designator (TREE_VEC potential) */
int array_with_designator[10] = {[3] = 7, [7] = 13};

/* C++ classes for TREE_BINFO */
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
    result.c = &global_identifier_1;
    result.inner.x = seed * 0.5f;
    result.inner.y = seed * 0.75f;
    
    if (depth > 0) {
        ComplexStruct nested = recursive_struct_builder(depth - 1, seed + 1);
        result.a += nested.a;
    }
    
    return result;  /* CONSTRUCTOR node for return value */
}

/* Function with complex control flow for SSA_NAME and BLOCK */
int complex_control_flow(int argc, char** argv) {
    volatile int volatile_arg = argc;  /* Prevent optimization */
    int result = 0;
    
    /* BLOCK with local variable */
    {
        int block_local_1 = volatile_arg * 2;
        
        /* Another nested BLOCK */
        {
            int block_local_2 = block_local_1 + 10;
            goto skip_initialization;  /* Jump to create interesting CFG */
            
            int unused = 100;  /* This won't be executed due to goto */
            
            skip_initialization:
            result = block_local_2;
        }
    }
    
    /* Create SSA_NAME nodes through phi nodes */
    int ssa_variable;
    if (volatile_arg > 2) {
        ssa_variable = 100;
        /* Nested block with goto */
        {
            int temp = 50;
            if (volatile_arg > 5) {
                goto label_inside_block;
            }
            temp = 75;
            label_inside_block:
            ssa_variable += temp;
        }
    } else {
        ssa_variable = 200;
        /* Another block */
        {
            int another_temp = 300;
            ssa_variable += another_temp;
        }
    }
    
    /* Use the variable to prevent removal */
    result += ssa_variable;
    
    /* More blocks with labels */
    {
        int x = 10;
        goto middle;
        
        start:
        x += 20;
        goto end;
        
        middle:
        x += 30;
        goto start;
        
        end:
        result += x;
    }
    
    return result;
}

/* OpenMP function with multiple clauses (OMP_CLAUSE) */
int openmp_reduction_test(int size, int** matrix) {
    int total = 0;
    
    #pragma omp parallel for collapse(2) reduction(+:total) \
                private(size) firstprivate(matrix) shared(global_identifier_1) \
                schedule(dynamic, 16)
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            total += matrix[i][j];
        }
    }
    
    /* Nested OpenMP with more clauses */
    #pragma omp parallel
    {
        #pragma omp for nowait private(static_identifier_2) \
                    lastprivate(total) ordered
        for (int i = 0; i < 100; i++) {
            #pragma omp ordered
            {
                static_identifier_2 = i;
            }
        }
    }
    
    return total;
}

/* Template for TREE_VEC */
template<typename T, int N>
class FixedVector {
    T data[N];
public:
    T& operator[](int index) { return data[index]; }
};

int main(int argc, char** argv) {
    /* Use argc to prevent compile-time optimization */
    volatile int volatile_argc = argc;
    
    /* Initialize array with constructor */
    int complex_array[5] = {volatile_argc, 2, [3] = volatile_argc * 2, 4, 5};
    
    /* Struct with aggregate initializer (CONSTRUCTOR) */
    ComplexStruct my_struct = {
        .a = volatile_argc,
        .b = 3.14159,
        .c = &global_identifier_1,
        .inner = {.x = 1.0f, .y = 2.0f}
    };
    
    /* Another constructor style */
    ComplexStruct another_struct = {10, 2.71828, array_with_designator, {3.0f, 4.0f}};
    
    /* Call recursive function */
    ComplexStruct recursive_result = recursive_struct_builder(3, volatile_argc);
    
    /* Complex control flow */
    int control_flow_result = complex_control_flow(argc, argv);
    
    /* C++ class hierarchy (TREE_BINFO) */
    BaseClass* base_ptr;
    if (volatile_argc > 1) {
        DerivedClass derived_obj;
        derived_obj.base_data = 100;
        derived_obj.derived_data = 200;
        base_ptr = &derived_obj;
        
        /* Virtual call */
        int virtual_result = base_ptr->virtual_method(volatile_argc);
        control_flow_result += virtual_result;
        
        /* dynamic_cast for RTTI */
        DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
        if (derived_ptr) {
            control_flow_result += derived_ptr->derived_data;
        }
    }
    
    /* Template instantiation (TREE_VEC) */
    FixedVector<int, 10> vec_template;
    for (int i = 0; i < 10; i++) {
        vec_template[i] = i * volatile_argc;
    }
    
    /* Create matrix for OpenMP */
    int matrix_size = (volatile_argc > 10) ? 10 : 5;
    int** matrix = new int*[matrix_size];
    for (int i = 0; i < matrix_size; i++) {
        matrix[i] = new int[matrix_size];
        for (int j = 0; j < matrix_size; j++) {
            matrix[i][j] = i * matrix_size + j;
        }
    }
    
    /* OpenMP with multiple clauses */
    int omp_result = openmp_reduction_test(matrix_size, matrix);
    
    /* Call external function with various identifiers */
    opaque_external_function(&global_identifier_1);
    opaque_external_function(&static_identifier_2);
    opaque_external_function(&control_flow_result);
    
    /* Use all results to prevent dead code elimination */
    int final_result = 
        my_struct.a + 
        another_struct.a + 
        recursive_result.a + 
        control_flow_result + 
        omp_result +
        complex_array[2] +
        vec_template[3];
    
    printf("Final result: %d\n", final_result);
    
    /* Cleanup */
    for (int i = 0; i < matrix_size; i++) {
        delete[] matrix[i];
    }
    delete[] matrix;
    
    return (final_result > 1000) ? 0 : 1;
}

/* Dummy external function definition */
extern "C" void opaque_external_function(int* ptr) {
    *ptr += 1;
}
