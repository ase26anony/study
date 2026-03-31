/* test_tree_codes.c - Comprehensive test to trigger specific tree node classifications */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimizations from removing our test constructs */
#define NOOPT __attribute__((noinline))

/* Generate many unique identifiers for IDENTIFIER_NODE */
#define GEN_ID(n) identifier_##n
#define USE_ID(n) volatile int GEN_ID(n) = n;

/* Vector types for TREE_VEC */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Struct for CONSTRUCTOR */
struct ComplexStruct {
    int a;
    float b;
    double c;
    char d;
};

/* Function prototypes */
NOOPT void test_identifiers_and_blocks(void);
NOOPT void test_vectors_and_ssa(void);
NOOPT struct ComplexStruct test_aggregate_init(int x);
NOOPT int test_omp_clauses(int *arr, int n);
#ifdef __cplusplus
NOOPT void test_cpp_inheritance(void);
#endif

/* Test 1: IDENTIFIER_NODE and BLOCK nodes */
NOOPT void test_identifiers_and_blocks(void) {
    /* Generate many identifiers */
    USE_ID(0); USE_ID(1); USE_ID(2); USE_ID(3); USE_ID(4);
    USE_ID(5); USE_ID(6); USE_ID(7); USE_ID(8); USE_ID(9);
    
    /* Nested blocks for BLOCK nodes */
    {
        volatile int block_var_1 = 1;
        {
            volatile int block_var_2 = 2;
            {
                volatile int block_var_3 = 3;
                /* Use all identifiers to prevent optimization */
                asm volatile("" : : "r"(identifier_0), "r"(identifier_1), 
                             "r"(identifier_2), "r"(identifier_3) : "memory");
            }
        }
    }
    
    /* More blocks in control flow */
    for (int i = 0; i < 3; i++) {
        volatile int loop_block_var = i * 2;
        if (loop_block_var > 2) {
            volatile int if_block_var = loop_block_var + 1;
            asm volatile("" : : "r"(if_block_var) : "memory");
        }
    }
}

/* Test 2: TREE_VEC and SSA_NAME nodes */
NOOPT void test_vectors_and_ssa(void) {
    /* Vector operations for TREE_VEC */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;
    v4si vec4 = vec1 * vec2;
    
    /* Complex loop with SSA variables */
    volatile int ssa_base = 10;
    int ssa_result = 0;
    
    for (int i = 0; i < 100; i++) {
        int ssa_temp;
        if (i % 2 == 0) {
            ssa_temp = ssa_base + i;
        } else {
            ssa_temp = ssa_base - i;
        }
        
        /* Multiple assignments to create phi nodes */
        if (ssa_temp > 50) {
            ssa_result += ssa_temp * 2;
        } else {
            ssa_result += ssa_temp / 2;
        }
        
        /* Prevent loop unrolling */
        asm volatile("" : : "r"(i) : "memory");
    }
    
    /* Use vectors to prevent elimination */
    volatile v4si *vec_ptr = &vec3;
    asm volatile("" : : "r"(vec_ptr), "r"(ssa_result) : "memory");
}

/* Test 3: CONSTRUCTOR nodes with non-constant initializers */
NOOPT struct ComplexStruct test_aggregate_init(int x) {
    /* Function to make initialization non-constant */
    NOOPT int get_value(void) { return 42; }
    
    /* Array constructor with non-constant elements */
    int dynamic_array[4] = { x, get_value(), x * 2, x + get_value() };
    
    /* Struct constructor with designated initializers */
    struct ComplexStruct cs = {
        .a = x,
        .b = (float)x / 2.0f,
        .c = (double)x * 3.14,
        .d = (char)(x % 256)
    };
    
    /* Nested struct initialization */
    struct { 
        struct ComplexStruct inner; 
        int extra; 
    } nested = { 
        .inner = { x + 1, (float)x, (double)x, 'X' },
        .extra = dynamic_array[1]
    };
    
    asm volatile("" : : "r"(dynamic_array[0]), "r"(nested.extra) : "memory");
    return cs;
}

/* Test 4: OMP_CLAUSE nodes with various OpenMP constructs */
#ifdef _OPENMP
#include <omp.h>
#endif

NOOPT int test_omp_clauses(int *arr, int n) {
    int sum = 0;
    int i;
    
    /* Complex OpenMP pragma with multiple clauses */
    #pragma omp parallel for private(i) shared(arr, n) reduction(+:sum) \
            schedule(dynamic, 4) if(n > 1000) num_threads(4)
    for (i = 0; i < n; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel
    {
        #pragma omp sections private(i) nowait
        {
            #pragma omp section
            {
                volatile int section_var = 1;
                asm volatile("" : : "r"(section_var) : "memory");
            }
            #pragma omp section
            {
                volatile int section_var = 2;
                asm volatile("" : : "r"(section_var) : "memory");
            }
        }
        
        #pragma omp single copyprivate(i)
        {
            i = omp_get_thread_num();
        }
    }
    
    /* Task construct with dependencies */
    #pragma omp parallel
    #pragma omp single
    {
        int task_var = 0;
        #pragma omp task depend(out: task_var) priority(high)
        {
            task_var = 1;
        }
        #pragma omp task depend(in: task_var)
        {
            sum += task_var;
        }
    }
    
    return sum;
}

/* C++ specific test for TREE_BINFO */
#ifdef __cplusplus
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual void method() { volatile int x = 1; asm volatile("" : : "r"(x) : "memory"); }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual void method() override { volatile int y = 2; asm volatile("" : : "r"(y) : "memory"); }
    int derived_data;
};

NOOPT void test_cpp_inheritance(void) {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* Virtual call through base pointer */
    base_ptr->method();
    
    /* Access through reference */
    BaseClass& base_ref = derived;
    base_ref.base_data = 42;
    
    /* Dynamic cast */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        derived_ptr->derived_data = 24;
    }
    
    asm volatile("" : : "r"(base_ptr), "r"(derived_ptr) : "memory");
}
#endif

/* Main function that orchestrates all tests */
int main(int argc, char **argv) {
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Test 1: IDENTIFIER_NODE and BLOCK */
    test_identifiers_and_blocks();
    
    /* Test 2: TREE_VEC and SSA_NAME */
    test_vectors_and_ssa();
    
    /* Test 3: CONSTRUCTOR */
    struct ComplexStruct cs = test_aggregate_init(seed);
    
    /* Test 4: OMP_CLAUSE */
    int arr[100];
    for (int i = 0; i < 100; i++) arr[i] = i + seed;
    int omp_result = test_omp_clauses(arr, 100);
    
    #ifdef __cplusplus
    /* Test 5: TREE_BINFO (C++ only) */
    test_cpp_inheritance();
    #endif
    
    /* Final result to prevent dead code elimination */
    volatile int final_result = cs.a + omp_result;
    printf("Result: %d\n", final_result);
    
    return final_result % 256;
}
