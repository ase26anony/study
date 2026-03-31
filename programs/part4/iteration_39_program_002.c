/* test_tree_coverage.c - Comprehensive test to trigger all tree node classifications */

/* For OMP_CLAUSE nodes */
#ifdef _OPENMP
#include <omp.h>
#endif

#include <stdio.h>
#include <stdlib.h>

/* ========== TREE_VEC nodes ========== */
/* Use GCC vector extensions to generate TREE_VEC nodes */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* ========== CONSTRUCTOR nodes ========== */
/* Struct for aggregate initialization */
struct ComplexStruct {
    int a, b, c;
    float f;
    double d;
};

struct NestedStruct {
    struct ComplexStruct inner;
    int extra;
};

/* ========== Helper functions to prevent optimization ========== */
static volatile int volatile_counter = 0;
static void memory_barrier(void) {
    asm volatile("" : : : "memory");
}

/* ========== Test IDENTIFIER_NODE ========== */
/* Generate many unique identifiers using macros */
#define MAKE_IDENTIFIER(n) identifier_##n
#define USE_IDENTIFIER(n) int MAKE_IDENTIFIER(n) = n;

void test_identifiers(void) {
    /* Generate many distinct identifiers */
    USE_IDENTIFIER(1)
    USE_IDENTIFIER(2)
    USE_IDENTIFIER(3)
    USE_IDENTIFIER(4)
    USE_IDENTIFIER(5)
    USE_IDENTIFIER(6)
    USE_IDENTIFIER(7)
    USE_IDENTIFIER(8)
    USE_IDENTIFIER(9)
    USE_IDENTIFIER(10)
    
    /* Use them in expressions */
    int sum = identifier_1 + identifier_2 + identifier_3 + identifier_4 + identifier_5 +
              identifier_6 + identifier_7 + identifier_8 + identifier_9 + identifier_10;
    
    volatile_counter += sum;
    memory_barrier();
}

/* ========== Test TREE_VEC ========== */
void test_vector_operations(void) {
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    
    /* Multiple vector operations to generate TREE_VEC nodes */
    v4si result1 = vec_a + vec_b;
    v4si result2 = vec_a * vec_c;
    v4si result3 = result1 - result2;
    
    /* Use volatile to prevent optimization */
    volatile v4si volatile_vec = result3;
    memory_barrier();
    
    /* Also test float vectors */
    v4sf fvec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec_b = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fresult = fvec_a * fvec_b;
    
    volatile v4sf volatile_fvec = fresult;
    memory_barrier();
}

/* ========== Test SSA_NAME ========== */
int test_ssa_formation(int n) {
    int x = 0, y = 0, z = 0;
    
    /* Complex loop with multiple branches to force SSA */
    for (int i = 0; i < n; i++) {
        if (i % 3 == 0) {
            x = i * 2;
            y = x + 1;
        } else if (i % 3 == 1) {
            x = i * 3;
            y = x - 1;
        } else {
            x = i * 4;
            y = x / 2;
        }
        
        /* Phi node will be created for z */
        if (i % 2 == 0) {
            z = z + x;
        } else {
            z = z + y;
        }
        
        /* Another SSA variable with multiple definitions */
        int temp;
        if (z > 100) {
            temp = z % 100;
        } else {
            temp = z;
        }
        
        volatile_counter += temp;
    }
    
    memory_barrier();
    return z;
}

/* ========== Test BLOCK nodes ========== */
void test_block_scopes(int iterations) {
    /* Outer block with local variables */
    int outer_var = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Nested block inside loop */
        {
            int inner_var = i * 2;
            outer_var += inner_var;
            
            /* Deeper nested block */
            {
                int deeper_var = inner_var + 1;
                outer_var -= deeper_var;
                
                /* Even deeper with its own variables */
                {
                    int deepest_var = deeper_var * 3;
                    volatile_counter += deepest_var;
                }
            }
        }
        
        /* Another block in the same loop */
        if (i % 2 == 0) {
            int conditional_var = i * i;
            outer_var += conditional_var;
        } else {
            int else_var = i + 1;
            outer_var -= else_var;
        }
    }
    
    memory_barrier();
}

/* ========== Test CONSTRUCTOR nodes ========== */
struct ComplexStruct test_aggregate_init(int a_val, int b_val) {
    /* Non-constant initializer with function calls */
    int computed = a_val * b_val + volatile_counter;
    
    /* Struct with designated initializer and non-constant expressions */
    struct ComplexStruct s = {
        .a = computed,
        .b = a_val + b_val,
        .c = computed % 100,
        .f = (float)computed / 100.0f,
        .d = (double)(a_val - b_val) * 0.5
    };
    
    /* Array with non-constant initializer */
    int arr[4] = {
        computed,
        a_val,
        b_val,
        computed % 10
    };
    
    /* Nested struct initialization */
    struct NestedStruct nested = {
        .inner = s,
        .extra = arr[0] + arr[1]
    };
    
    memory_barrier();
    return nested.inner;
}

/* ========== Test OMP_CLAUSE nodes ========== */
#ifdef _OPENMP
void test_omp_clauses(int size) {
    int i;
    int sum = 0;
    int* array = (int*)malloc(size * sizeof(int));
    
    if (!array) return;
    
    /* Initialize array */
    for (i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel private(i) shared(array, size) reduction(+:sum) \
        default(none) if(size > 1000)
    {
        #pragma omp for schedule(dynamic, 16) nowait
        for (i = 0; i < size; i++) {
            sum += array[i];
        }
        
        /* Nested OpenMP construct with different clauses */
        #pragma omp single copyprivate(i)
        {
            i = omp_get_thread_num();
        }
    }
    
    /* Another OpenMP construct with task clauses */
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (i = 0; i < 10; i++) {
                #pragma omp task firstprivate(i) depend(out: array[i])
                {
                    array[i] *= 2;
                }
            }
        }
    }
    
    volatile_counter += sum;
    free(array);
    memory_barrier();
}
#endif

/* ========== C++ specific for TREE_BINFO ========== */
#ifdef __cplusplus
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int method() { return 1; }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int method() override { return 2; }
    int derived_data;
};

class SecondDerived : public DerivedClass {
public:
    virtual int method() override { return 3; }
    int second_data;
};

void test_binfo_nodes(void) {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    SecondDerived second;
    
    /* Use virtual calls and casts to work with binfo nodes */
    int result = base_ptr->method();
    
    BaseClass* another_ptr = dynamic_cast<BaseClass*>(&second);
    if (another_ptr) {
        result += another_ptr->method();
    }
    
    volatile_counter += result;
    memory_barrier();
}
#endif

/* ========== Main function ========== */
int main(int argc, char** argv) {
    int iterations = 100;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    printf("Starting tree coverage test with %d iterations...\n", iterations);
    
    /* Test all tree node types */
    test_identifiers();
    printf("  - IDENTIFIER_NODE test complete\n");
    
    test_vector_operations();
    printf("  - TREE_VEC test complete\n");
    
    int ssa_result = test_ssa_formation(iterations);
    printf("  - SSA_NAME test complete (result: %d)\n", ssa_result);
    
    test_block_scopes(iterations);
    printf("  - BLOCK test complete\n");
    
    struct ComplexStruct cs = test_aggregate_init(iterations, iterations * 2);
    printf("  - CONSTRUCTOR test complete (struct.a = %d)\n", cs.a);
    
#ifdef _OPENMP
    test_omp_clauses(iterations);
    printf("  - OMP_CLAUSE test complete\n");
#else
    printf("  - OMP_CLAUSE test skipped (compile with -fopenmp)\n");
#endif

#ifdef __cplusplus
    test_binfo_nodes();
    printf("  - TREE_BINFO test complete\n");
#else
    printf("  - TREE_BINFO test skipped (compile as C++)\n");
#endif
    
    printf("All tests completed. Volatile counter: %d\n", volatile_counter);
    
    return 0;
}
