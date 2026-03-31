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
ComplexStruct recursive_struct_builder(int depth, int* counter) {
    ComplexStruct result;
    result.a = depth;
    result.b = depth * 1.5;
    result.c = 'A' + (depth % 26);
    result.d = counter;
    
    if (depth > 0) {
        ComplexStruct inner = recursive_struct_builder(depth - 1, counter);
        result.a += inner.a;
        result.b += inner.b;
        (*counter)++;
    }
    
    return result; /* CONSTRUCTOR node for return value */
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

/* Template for TREE_VEC coverage */
template<typename T, int N>
class TemplateVecWrapper {
    T data[N];
public:
    TemplateVecWrapper() {
        for (int i = 0; i < N; i++) {
            data[i] = T();
        }
    }
    
    T& operator[](int index) { return data[index]; }
    
    /* Complex initializer list */
    void initialize_with_designators() {
        /* This may generate TREE_VEC nodes for template/initializer handling */
        T local_arr[5] = {T(), T(), T(), T(), T()};
        data[0] = local_arr[0];
    }
};

/* Function with complex control flow for SSA_NAME and BLOCK coverage */
int complex_control_flow(int argc, char** argv) {
    volatile int seed = argc; /* Prevent optimization */
    int result = 0;
    
    /* BLOCK node with local variable */
    {
        int block_local = seed * 2;
        
        /* Jump to label skipping part of block */
        if (seed % 3 == 0) {
            goto skip_middle;
        }
        
        int hidden_in_block = block_local + 10;
        result += hidden_in_block;
        
    skip_middle:
        /* Still in same block */
        result += block_local;
        
        /* Nested block */
        {
            int nested_block_var = 42;
            result += nested_block_var;
        }
    }
    
    /* SSA_NAME generation with phi nodes */
    int ssa_var;
    for (int i = 0; i < seed % 100 + 10; i++) {
        if (i % 2 == 0) {
            ssa_var = i * 2; /* Assignment in one path */
        } else {
            ssa_var = i * 3 + 1; /* Assignment in another path */
        }
        
        /* Use ssa_var to create phi node */
        result += ssa_var;
        
        /* Additional complexity to prevent optimization */
        if (i % 7 == 0) {
            ssa_var += result;
        }
    }
    
    /* Another SSA pattern with loop */
    int x = 0;
    for (int i = 0; i < 50; i++) {
        int y;
        if (i < 25) {
            y = x + i;
        } else {
            y = x - i;
        }
        x = y + (i % 5);
        result += x;
    }
    
    return result;
}

/* OpenMP function for OMP_CLAUSE coverage */
int openmp_reduction_example(int size, int** matrix) {
    int total = 0;
    
    /* Multiple OpenMP clauses */
    #pragma omp parallel for collapse(2) private(size) \
        firstprivate(matrix) shared(total) reduction(+:total) \
        schedule(dynamic, 4)
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            /* Nested OpenMP directive */
            #pragma omp atomic
            total += matrix[i][j];
            
            /* Additional clause usage */
            #pragma omp critical
            {
                matrix[i][j] += (i + j) % 7;
            }
        }
    }
    
    /* Another OpenMP region with different clauses */
    int private_var = 0;
    #pragma omp parallel num_threads(4) \
        default(none) copyin(private_var) if(size > 100)
    {
        #pragma omp for nowait
        for (int i = 0; i < size; i++) {
            #pragma omp atomic
            private_var += i;
        }
    }
    
    return total + private_var;
}

int main(int argc, char** argv) {
    /* Use argc to prevent compile-time optimization */
    volatile int dynamic_size = argc > 1 ? atoi(argv[1]) : 50;
    if (dynamic_size < 10) dynamic_size = 10;
    
    /* CONSTRUCTOR nodes with aggregate initialization */
    ComplexStruct structs[3] = {
        {1, 2.5, 'X', &dynamic_size},
        {.a = 2, .b = 3.14, .c = 'Y', .d = nullptr},
        {3, 4.2, 'Z', (int*)0x1000}
    };
    
    /* Array with designator (potential TREE_VEC) */
    int array_with_designators[10] = {[0] = 1, [5] = 2, [9] = 3};
    
    /* Call recursive function for CONSTRUCTOR nodes */
    int counter = 0;
    ComplexStruct recursive_result = recursive_struct_builder(5, &counter);
    
    /* Complex control flow for SSA_NAME and BLOCK */
    int control_flow_result = complex_control_flow(argc, argv);
    
    /* C++ class hierarchy for TREE_BINFO */
    BaseClass* poly_obj;
    if (dynamic_size % 2 == 0) {
        poly_obj = new DerivedClass();
    } else {
        poly_obj = new SecondDerived();
    }
    
    poly_obj->base_data = dynamic_size;
    if (DerivedClass* derived = dynamic_cast<DerivedClass*>(poly_obj)) {
        derived->derived_data = dynamic_size * 2;
    }
    
    int virtual_result = poly_obj->virtual_method(dynamic_size);
    
    /* Template instantiation for TREE_VEC */
    TemplateVecWrapper<double, 10> template_vec;
    template_vec.initialize_with_designators();
    
    /* OpenMP computation */
    int** matrix = new int*[dynamic_size];
    for (int i = 0; i < dynamic_size; i++) {
        matrix[i] = new int[dynamic_size];
        for (int j = 0; j < dynamic_size; j++) {
            matrix[i][j] = (i * j + argc) % 100;
        }
    }
    
    int omp_result = openmp_reduction_example(dynamic_size, matrix);
    
    /* Call external function with various identifiers */
    opaque_external_function(&global_identifier_1);
    opaque_external_function(&static_identifier_2);
    opaque_external_function(&extern_identifier_3);
    
    /* Use goto for BLOCK coverage */
    int goto_var = 0;
    
    if (dynamic_size > 30) {
        goto special_case;
    }
    
    {
        int inside_block = 100;
        goto_var = inside_block;
        goto after_block;
    }
    
special_case:
    {
        int another_block_var = 200;
        goto_var = another_block_var;
    }
    
after_block:
    /* Final computation using all results */
    int final_result = 
        recursive_result.a +
        control_flow_result % 1000 +
        virtual_result +
        omp_result % 10000 +
        goto_var +
        array_with_designators[0] +
        template_vec[0];
    
    printf("Final result: %d\n", final_result);
    
    /* Cleanup */
    for (int i = 0; i < dynamic_size; i++) {
        delete[] matrix[i];
    }
    delete[] matrix;
    delete poly_obj;
    
    return final_result % 256;
}

/* Dummy definition to satisfy linker (in real test would be in separate file) */
extern "C" void opaque_external_function(int* ptr) {
    *ptr += 1;
}

int extern_identifier_3 = 0;
