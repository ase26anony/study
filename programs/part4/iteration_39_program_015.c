/* test_tree_coverage.c - Comprehensive test for GCC tree node coverage */

/* Prevent inlining to ensure tree nodes are preserved */
#define NOINLINE __attribute__((noinline))

/* For TREE_BINFO coverage via LTO */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

/* ========== IDENTIFIER_NODE coverage ========== */
/* Generate many unique identifiers using macros */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(var_, n)
#define MAKE_FUNC(n) CONCAT(func_, n)

/* Declare many identifiers */
#define DECLARE_VARS(n) int MAKE_ID(n) = n;
#define DECLARE_FUNC(n) \
    NOINLINE int MAKE_FUNC(n)(int x) { \
        volatile int result = x + n; \
        asm volatile("" : : : "memory"); \
        return result; \
    }

/* Generate identifiers */
DECLARE_FUNC(1) DECLARE_FUNC(2) DECLARE_FUNC(3) DECLARE_FUNC(4)
DECLARE_FUNC(5) DECLARE_FUNC(6) DECLARE_FUNC(7) DECLARE_FUNC(8)

/* ========== TREE_VEC coverage ========== */
/* Use GCC vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE v4si test_vector_operations(v4si a, v4si b) {
    /* Multiple vector operations to generate TREE_VEC nodes */
    v4si c = a + b;
    v4si d = a * b;
    v4si e = c - d;
    v4si f = e << 2;
    
    /* Use volatile to prevent optimization */
    volatile v4si result = f;
    asm volatile("" : : : "memory");
    
    return result;
}

/* ========== SSA_NAME coverage ========== */
NOINLINE int test_ssa_name(int n) {
    int x = 0, y = 0, z = 0;
    volatile int trigger = 1;
    
    /* Complex loop with multiple assignments to create SSA form */
    for (int i = 0; i < n; i++) {
        if (trigger & 1) {
            x = i * 2;
            y = x + i;
        } else {
            x = i / 2;
            y = x - i;
        }
        
        /* Another conditional to create phi nodes */
        if (i % 3 == 0) {
            z = x + y;
        } else if (i % 3 == 1) {
            z = x - y;
        } else {
            z = x * y;
        }
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
        trigger ^= z;
    }
    
    return x + y + z;
}

/* ========== BLOCK coverage ========== */
NOINLINE int test_blocks(int n) {
    int result = 0;
    
    /* Outer block */
    {
        int outer_var = n * 2;
        
        /* Nested block 1 */
        {
            int inner_var1 = outer_var + 1;
            
            /* Deeply nested block */
            {
                int deep_var = inner_var1 * 3;
                result += deep_var;
                asm volatile("" : : : "memory");
            }
        }
        
        /* Nested block 2 with different scope */
        if (outer_var > 0) {
            int inner_var2 = outer_var - 1;
            {
                int another_deep = inner_var2 / 2;
                result += another_deep;
            }
        }
        
        /* Block in loop */
        for (int i = 0; i < 3; i++) {
            int loop_var = i * outer_var;
            {
                int loop_inner = loop_var + i;
                result += loop_inner;
            }
        }
    }
    
    return result;
}

/* ========== CONSTRUCTOR coverage ========== */
struct ComplexStruct {
    int a, b, c;
    float x, y, z;
};

NOINLINE int test_constructor(void) {
    /* Use function calls in initializers */
    extern int external_func1(void);
    extern int external_func2(void);
    extern float external_func3(void);
    
    /* Non-constant struct initializer */
    struct ComplexStruct s1 = {
        .a = external_func1(),
        .b = external_func2() + 1,
        .c = 0,
        .x = external_func3(),
        .y = 2.5f,
        .z = s1.x * 2.0f  /* Forward reference creates complex constructor */
    };
    
    /* Array with non-constant initializers */
    int arr[4] = {
        external_func1(),
        external_func2(),
        external_func1() + external_func2(),
        42
    };
    
    /* Nested struct initializer */
    struct Nested {
        struct ComplexStruct inner;
        int extra;
    };
    
    struct Nested n = {
        .inner = {
            .a = arr[0],
            .b = arr[1],
            .c = arr[2],
            .x = 1.0f,
            .y = 2.0f,
            .z = 3.0f
        },
        .extra = arr[3]
    };
    
    volatile struct ComplexStruct vs = s1;
    asm volatile("" : : : "memory");
    
    return vs.a + vs.b + (int)vs.x + arr[0] + n.extra;
}

/* Dummy external functions */
int external_func1(void) { return 1; }
int external_func2(void) { return 2; }
float external_func3(void) { return 3.0f; }

/* ========== OMP_CLAUSE coverage ========== */
#ifdef _OPENMP
#include <omp.h>

NOINLINE int test_omp_clauses(int size) {
    int sum = 0;
    int* arr = (int*)malloc(size * sizeof(int));
    
    if (!arr) return -1;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        arr[i] = i;
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel shared(arr) private(size) reduction(+:sum) \
        num_threads(4) if(size > 1000)
    {
        #pragma omp for schedule(dynamic, 16) nowait
        for (int i = 0; i < size; i++) {
            sum += arr[i];
        }
        
        /* Additional OpenMP construct with different clauses */
        #pragma omp sections private(sum)
        {
            #pragma omp section
            {
                int local_sum = 0;
                #pragma omp parallel for reduction(+:local_sum) \
                    collapse(2) ordered
                for (int i = 0; i < 10; i++) {
                    for (int j = 0; j < 10; j++) {
                        #pragma omp ordered
                        local_sum += i * j;
                    }
                }
                #pragma omp atomic
                sum += local_sum;
            }
            
            #pragma omp section
            {
                #pragma omp task firstprivate(arr) depend(out:arr[0])
                {
                    volatile int task_var = arr[0];
                    asm volatile("" : : : "memory");
                }
            }
        }
    }
    
    free(arr);
    return sum;
}
#else
NOINLINE int test_omp_clauses(int size) {
    /* Dummy implementation when OpenMP not available */
    volatile int dummy = size;
    asm volatile("" : : : "memory");
    return dummy;
}
#endif

/* ========== Main test orchestrator ========== */
int main(int argc, char** argv) {
    int result = 0;
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Test IDENTIFIER_NODE by calling functions with many identifiers */
    result += MAKE_FUNC(1)(seed);
    result += MAKE_FUNC(2)(result);
    result += MAKE_FUNC(3)(result);
    result += MAKE_FUNC(4)(result);
    
    /* Declare and use many identifier variables */
    DECLARE_VARS(100) DECLARE_VARS(101) DECLARE_VARS(102)
    int var_100 = 100, var_101 = 101, var_102 = 102;
    result += var_100 + var_101 + var_102;
    
    /* Test TREE_VEC */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_result = test_vector_operations(vec_a, vec_b);
    result += vec_result[0] + vec_result[1];
    
    /* Test SSA_NAME */
    result += test_ssa_name(seed % 100 + 10);
    
    /* Test BLOCK */
    result += test_blocks(seed % 50 + 5);
    
    /* Test CONSTRUCTOR */
    result += test_constructor();
    
    /* Test OMP_CLAUSE */
    result += test_omp_clauses(seed % 1000 + 100);
    
    /* Memory barrier to ensure all operations complete */
    asm volatile("" : : : "memory");
    
    printf("Result: %d\n", result);
    return result == 0 ? 0 : 1;
}

#ifdef __cplusplus
}
#endif
