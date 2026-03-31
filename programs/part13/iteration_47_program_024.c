/* Compile with: g++ -O2 -fopenmp -std=c++11 -fdump-tree-all tree_test.cc -o tree_test */
/* For C-only: gcc -O2 -fopenmp -fdump-tree-all tree_test.c -o tree_test */

#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void opaque_external_function(int*);

/* Global identifiers (IDENTIFIER_NODE) */
int global_counter = 0;
static int static_hidden = 42;
volatile int volatile_guard = 0;

/* Struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    int b;
    float c;
    double d[2];
};

/* Array for TREE_VEC representation */
int* create_vector_like() {
    /* Complex initializer that may generate TREE_VEC */
    int arr[5][3] = {{1, 2, 3}, {4, 5, 6}, {[2] = 7, 8, 9}, {10}, {11, 12}};
    static int* ptr = (int*)arr;
    return ptr;
}

/* Recursive function returning struct (CONSTRUCTOR) */
struct ComplexStruct recursive_struct_builder(int depth, int base) {
    struct ComplexStruct cs;
    cs.a = base + depth;
    cs.b = base - depth;
    cs.c = (float)depth / 10.0f;
    cs.d[0] = depth * 1.5;
    cs.d[1] = depth * 2.5;
    
    if (depth > 0) {
        struct ComplexStruct inner = recursive_struct_builder(depth - 1, base);
        cs.a += inner.a;
        cs.b += inner.b;
    }
    return cs;  /* CONSTRUCTOR node for return value */
}

/* Function with complex control flow for SSA_NAME and BLOCK */
int ssa_generator(int x, int y) {
    int result;
    
    /* BLOCK node with local variable */
    {
        int local_block_var = x * 2;
        volatile_guard = local_block_var;
        
        /* Conditional with phi node potential */
        if (x > y) {
            result = local_block_var + y;
        } else {
            result = local_block_var - y;
        }
        
        /* Another BLOCK with goto */
        {
            int hidden = 100;
            if (x % 2 == 0) {
                goto skip_point;
            }
            hidden = 200;
            skip_point:
            result += hidden;
        }
    }
    
    /* Loop with SSA variable */
    int ssa_var = 0;
    for (int i = 0; i < x; i++) {
        if (i % 3 == 0) {
            ssa_var += i * 2;  /* Creates phi node at loop header */
        } else {
            ssa_var += i;
        }
    }
    
    return result + ssa_var;
}

#ifdef __cplusplus
/* C++ specific code for TREE_BINFO */
class BaseClass {
public:
    virtual int virtual_method(int x) {
        return x * 2;
    }
    virtual ~BaseClass() {}
};

class DerivedClass : public BaseClass {
public:
    virtual int virtual_method(int x) override {
        return x * 3 + static_hidden;
    }
    
    template<typename T>
    T template_method(T value) {
        return value * 2;  /* May involve TREE_VEC for template */
    }
};

void test_cpp_features() {
    DerivedClass* obj = new DerivedClass();
    BaseClass* base_ptr = obj;
    
    /* Virtual call generates BINFO nodes */
    int vresult = base_ptr->virtual_method(10);
    
    /* Template instantiation */
    int tresult = obj->template_method<int>(5);
    
    printf("C++ Results: virtual=%d, template=%d\n", vresult, tresult);
    
    delete obj;
}
#endif

/* OpenMP function with multiple clauses (OMP_CLAUSE) */
void openmp_test(int size, int* data) {
    int sum = 0;
    int i, j;
    
    /* Complex OpenMP pragma with multiple clauses */
    #pragma omp parallel for private(i) firstprivate(size) shared(data) reduction(+:sum) collapse(2) schedule(dynamic)
    for (i = 0; i < size; i++) {
        for (j = 0; j < 2; j++) {
            int idx = i * 2 + j;
            if (idx < size * 2) {
                sum += data[idx];
            }
        }
    }
    
    /* Nested OpenMP region */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task private(j)
            {
                for (j = 0; j < 10; j++) {
                    #pragma omp atomic
                    global_counter++;
                }
            }
        }
    }
    
    printf("OpenMP sum: %d, counter: %d\n", sum, global_counter);
}

int main(int argc, char** argv) {
    /* Use argv to prevent optimization */
    int iterations = argc > 1 ? atoi(argv[1]) : 100;
    int seed = argc > 2 ? atoi(argv[2]) : 42;
    
    /* CONSTRUCTOR nodes with complex initialization */
    struct ComplexStruct cs1 = {1, 2, 3.0f, {4.0, 5.0}};
    struct ComplexStruct cs2 = {.b = 20, .a = 10, .d = {30.0, 40.0}, .c = 50.0f};
    
    /* Array with designated initializer (may create TREE_VEC) */
    int designated_array[10] = {[0] = seed, [5] = iterations, [9] = seed * iterations};
    
    /* BLOCK nodes with gotos */
    int block_result = 0;
    {
        int x = 10;
        goto middle;
        
        {
            int y = 20;  /* This declaration might be skipped */
            middle:
            block_result = x + 30;
        }
    }
    
    /* Generate SSA_NAME nodes */
    int ssa_result = ssa_generator(iterations, seed);
    
    /* Call recursive function (CONSTRUCTOR returns) */
    struct ComplexStruct recursive_result = recursive_struct_builder(5, seed);
    
    /* OpenMP test with OMP_CLAUSE nodes */
    int* data = (int*)malloc(iterations * 2 * sizeof(int));
    for (int i = 0; i < iterations * 2; i++) {
        data[i] = (i * seed) % 100;
    }
    
    openmp_test(iterations, data);
    
    /* Call external function with various identifiers */
    opaque_external_function(&ssa_result);
    
#ifdef __cplusplus
    /* C++ features for TREE_BINFO */
    test_cpp_features();
    
    /* Template usage for TREE_VEC */
    std::vector<int> template_vec;
    for (int i = 0; i < iterations; i++) {
        template_vec.push_back(i * seed);
    }
#endif
    
    /* Use all results to prevent dead code elimination */
    int final_result = cs1.a + cs2.b + block_result + ssa_result + 
                      (int)recursive_result.c + designated_array[0] + global_counter;
    
    printf("Final checksum: %d\n", final_result);
    
    free(data);
    return final_result % 100;
}

/* Dummy external function definition to satisfy linker */
void opaque_external_function(int* x) {
    *x += volatile_guard;
}
