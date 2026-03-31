/* Compile with: g++ -O2 -fopenmp -std=c++11 -fdump-tree-all tree_test.cc -o tree_test */
/* For C-only: gcc -O2 -fopenmp -fdump-tree-all tree_test.c -o tree_test */

#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void opaque_external_function(int*);

/* Global identifiers for IDENTIFIER_NODE generation */
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

/* Array with designator for TREE_VEC representation */
int array_with_designator[10] = {[3] = 7, [7] = 13};

/* Recursive function returning struct (CONSTRUCTOR) */
struct ComplexStruct recursive_struct_builder(int depth, int* counter) {
    volatile int prevent_opt = *counter; /* Prevent tail recursion optimization */
    
    struct ComplexStruct result;
    result.a = depth;
    result.b = depth * 3.14;
    result.c = 'A' + (depth % 26);
    result.d = counter;
    
    if (depth > 0 && *counter < 100) {
        (*counter)++;
        /* Create different paths for SSA */
        if (depth % 2 == 0) {
            result.a += recursive_struct_builder(depth - 1, counter).a;
        } else {
            result.a += recursive_struct_builder(depth - 2, counter).a;
        }
    }
    
    return result; /* CONSTRUCTOR node for return value */
}

/* Function with complex control flow for BLOCK and SSA_NAME */
int complex_control_flow(int n, int* results) {
    int sum = 0;
    
    /* Outer block with local variable */
    {
        int block_local_1 = n * 2;
        
        /* Inner block with goto */
        {
            int block_local_2 = block_local_1 + 5;
            
            if (n % 3 == 0) {
                goto skip_middle; /* Jump to label */
            }
            
            /* Middle block that might be skipped */
            int middle_var = 42;
            sum += middle_var;
            
        skip_middle:
            /* Use variable from outer scope - creates phi node */
            sum += block_local_2;
        }
        
        /* Loop with conditional assignment for SSA_NAME */
        for (int i = 0; i < n; i++) {
            int temp;
            if (i % 2 == 0) {
                temp = i * 2; /* SSA_NAME assignment path 1 */
            } else {
                temp = i * 3; /* SSA_NAME assignment path 2 */
            }
            results[i] = temp + sum; /* Use temp to prevent elimination */
        }
    }
    
    return sum;
}

#ifdef __cplusplus
/* C++ specific code for TREE_BINFO generation */
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
        return x * 3;
    }
    
    template<typename T>
    T template_method(T value) {
        return value * 4; /* Template instantiation for TREE_VEC */
    }
};

void cpp_binfo_test() {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* Virtual call generates BINFO lookups */
    int result = base_ptr->virtual_method(10);
    
    /* Template instantiation */
    int templ_result = derived.template_method<int>(5);
    
    printf("C++ results: %d, %d\n", result, templ_result);
}
#endif

int main(int argc, char** argv) {
    /* Use argc to prevent compile-time optimization */
    int iterations = (argc > 1) ? atoi(argv[1]) : 50;
    int seed = (argc > 2) ? atoi(argv[2]) : 42;
    
    /* Initialize identifiers */
    global_identifier_1 = seed;
    static_identifier_2 = seed * 2;
    
    /* Complex struct initialization (CONSTRUCTOR) */
    struct ComplexStruct cs = { 
        .a = 10, 
        .b = 3.14159, 
        .c = 'X',
        .d = &global_identifier_1
    };
    
    /* Array initialization with nested designators */
    int matrix[3][3] = {
        {[0] = 1, [2] = 3},
        {[1] = 5},
        {[0] = 7, [1] = 8, [2] = 9}
    };
    
    /* Call external function with various identifiers */
    opaque_external_function(&global_identifier_1);
    opaque_external_function(&static_identifier_2);
    
    /* Recursive struct building */
    int counter = 0;
    struct ComplexStruct recursive_result = recursive_struct_builder(5, &counter);
    
    /* Complex control flow with blocks and SSA */
    int* results = (int*)malloc(iterations * sizeof(int));
    int control_flow_result = complex_control_flow(iterations, results);
    
    /* OpenMP region with multiple clauses */
    int openmp_sum = 0;
    #pragma omp parallel for private(seed) firstprivate(iterations) \
            shared(results) reduction(+:openmp_sum) collapse(2) \
            schedule(dynamic)
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int local_var = i * j;
            /* Conditional for SSA in OpenMP region */
            if (local_var % 3 == 0) {
                openmp_sum += local_var * 2;
            } else {
                openmp_sum += local_var * 3;
            }
        }
    }
    
    /* Nested OpenMP with more clauses */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task private(seed) shared(openmp_sum)
            {
                openmp_sum += matrix[0][0];
            }
        }
    }
    
#ifdef __cplusplus
    /* C++ specific tests */
    cpp_binfo_test();
    
    /* Template usage for TREE_VEC */
    std::vector<int> template_vec;
    for (int i = 0; i < iterations; i++) {
        template_vec.push_back(i * seed);
    }
#endif
    
    /* Final computation using all results */
    int final_result = 
        recursive_result.a + 
        control_flow_result + 
        openmp_sum + 
        cs.a +
        results[iterations / 2];
    
    printf("Final result: %d\n", final_result);
    
    free(results);
    return 0;
}
