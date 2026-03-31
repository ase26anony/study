/* test_tree_coverage.c - Comprehensive test for GCC tree node classification */

/* Prevent inlining to ensure tree nodes are fully built */
#define NOINLINE __attribute__((noinline))

/* Generate many unique identifiers */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(identifier_, n)

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

/* Global volatile to prevent optimization */
volatile int global_volatile = 0;

/* ========== IDENTIFIER_NODE and BLOCK coverage ========== */
NOINLINE void test_identifiers_and_blocks(int iterations) {
    /* Multiple identifiers in outer scope */
    int MAKE_ID(0) = 1;
    int MAKE_ID(1) = 2;
    int MAKE_ID(2) = 3;
    int MAKE_ID(3) = 4;
    int MAKE_ID(4) = 5;
    
    /* Nested block 1 */
    {
        int MAKE_ID(5) = 10;
        int MAKE_ID(6) = 20;
        int MAKE_ID(7) = MAKE_ID(5) + MAKE_ID(6);
        
        /* Deeper nested block */
        {
            int MAKE_ID(8) = 100;
            int MAKE_ID(9) = 200;
            MAKE_ID(7) += MAKE_ID(8) * MAKE_ID(9);
            asm volatile("" : : : "memory"); /* Prevent reordering */
        }
    }
    
    /* Nested block 2 with loop */
    for (int MAKE_ID(10) = 0; MAKE_ID(10) < iterations; MAKE_ID(10)++) {
        int MAKE_ID(11) = MAKE_ID(10) * 2;
        
        /* Block inside loop */
        {
            int MAKE_ID(12) = MAKE_ID(11) + global_volatile;
            int MAKE_ID(13) = MAKE_ID(12) ^ 0xFF;
            MAKE_ID(0) += MAKE_ID(13);
        }
        
        /* Conditional block */
        if (MAKE_ID(10) % 3 == 0) {
            int MAKE_ID(14) = MAKE_ID(10) / 3;
            MAKE_ID(1) += MAKE_ID(14);
        } else {
            int MAKE_ID(15) = MAKE_ID(10) % 3;
            MAKE_ID(2) += MAKE_ID(15);
        }
    }
    
    /* More identifiers */
    int MAKE_ID(16) = MAKE_ID(0) + MAKE_ID(1) + MAKE_ID(2);
    int MAKE_ID(17) = MAKE_ID(16) * MAKE_ID(3);
    int MAKE_ID(18) = MAKE_ID(17) / (MAKE_ID(4) + 1);
    
    global_volatile = MAKE_ID(18);
}

/* ========== TREE_VEC and SSA_NAME coverage ========== */
NOINLINE void test_vectors_and_ssa(int n) {
    /* Vector operations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = {9, 10, 11, 12};
    
    /* Multiple vector operations */
    v4si result1 = vec1 + vec2;
    v4si result2 = vec1 * vec3;
    v4si result3 = result1 - result2;
    v4si result4 = vec2 / (vec1 + 1);
    
    /* Complex SSA pattern with loop */
    int ssa_var1 = 0;
    int ssa_var2 = 1;
    int ssa_var3 = 2;
    
    for (int i = 0; i < n; i++) {
        /* Multiple assignments creating SSA phi nodes */
        if (i % 2 == 0) {
            ssa_var1 = ssa_var2 + ssa_var3;
            ssa_var2 = ssa_var1 * i;
        } else if (i % 3 == 0) {
            ssa_var1 = ssa_var3 - i;
            ssa_var2 = ssa_var1 / (i + 1);
        } else {
            ssa_var1 = ssa_var2 ^ ssa_var3;
            ssa_var2 = ssa_var1 | i;
        }
        
        /* Another SSA variable with complex flow */
        int ssa_var4;
        if (ssa_var1 > 100) {
            ssa_var4 = ssa_var2 * 2;
        } else {
            ssa_var4 = ssa_var3 + 5;
        }
        
        ssa_var3 = ssa_var4 + global_volatile;
        
        /* Vector operation inside loop (creates more TREE_VEC nodes) */
        result1 = result1 + vec1;
        result2 = result2 * (vec2 + i);
    }
    
    /* Use vectors to prevent optimization */
    int* r1 = (int*)&result1;
    int* r2 = (int*)&result2;
    global_volatile = r1[0] + r2[1] + ssa_var1 + ssa_var2 + ssa_var3;
}

/* ========== CONSTRUCTOR coverage ========== */
NOINLINE int helper_func1(void) { return global_volatile + 1; }
NOINLINE int helper_func2(void) { return global_volatile + 2; }
NOINLINE float helper_func3(void) { return global_volatile * 1.5f; }
NOINLINE double helper_func4(void) { return global_volatile * 2.5; }

NOINLINE void test_aggregate_init(void) {
    /* Array with non-constant initializers */
    int dynamic_array[5] = {
        helper_func1(),
        helper_func2(),
        global_volatile,
        helper_func1() * 2,
        helper_func2() + 3
    };
    
    /* Struct with designated initializers */
    struct ComplexStruct cs1 = {
        .a = helper_func1(),
        .b = helper_func3(),
        .c = helper_func4(),
        .d = (char)(global_volatile & 0xFF)
    };
    
    /* Nested struct initialization */
    struct Nested {
        struct ComplexStruct inner;
        int extra[2];
    };
    
    struct Nested ns1 = {
        .inner = {
            .a = dynamic_array[0],
            .b = dynamic_array[1] * 1.0f,
            .c = dynamic_array[2] * 1.0,
            .d = (char)dynamic_array[3]
        },
        .extra = { helper_func1(), helper_func2() }
    };
    
    /* Array of structs */
    struct ComplexStruct cs_array[3] = {
        { helper_func1(), 1.0f, 2.0, 'a' },
        { .a = helper_func2(), .b = 2.0f, .c = 3.0, .d = 'b' },
        { dynamic_array[0], cs1.b, ns1.inner.c, 'c' }
    };
    
    /* Use aggregates to prevent optimization */
    global_volatile = dynamic_array[0] + cs1.a + ns1.extra[0] + cs_array[1].a;
}

/* ========== OMP_CLAUSE coverage ========== */
#ifdef _OPENMP
#include <omp.h>

NOINLINE void test_omp_clauses(int size) {
    int i;
    int sum = 0;
    int* array = (int*)__builtin_alloca(size * sizeof(int));
    
    /* Initialize array */
    for (i = 0; i < size; i++) {
        array[i] = i + global_volatile;
    }
    
    /* OpenMP with multiple clauses */
    #pragma omp parallel for private(i) shared(array, size) reduction(+:sum) \
            schedule(dynamic, 4) num_threads(2) if(size > 100)
    for (i = 0; i < size; i++) {
        sum += array[i] * (i % 10);
    }
    
    /* Another OpenMP construct with different clauses */
    int max_val = 0;
    #pragma omp parallel sections private(i) shared(array, size) \
            reduction(max:max_val) default(none)
    {
        #pragma omp section
        {
            for (i = 0; i < size/2; i++) {
                if (array[i] > max_val) max_val = array[i];
            }
        }
        
        #pragma omp section
        {
            for (i = size/2; i < size; i++) {
                if (array[i] > max_val) max_val = array[i];
            }
        }
    }
    
    /* OpenMP task with clauses */
    int task_result = 0;
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(out: task_result) firstprivate(size) \
                    priority(1) untied
            {
                for (int j = 0; j < size; j++) {
                    task_result += j * global_volatile;
                }
            }
            
            #pragma omp task depend(in: task_result)
            {
                sum += task_result;
            }
        }
    }
    
    global_volatile = sum + max_val;
}
#endif

/* ========== Main function ========== */
int main(int argc, char** argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 1000) iterations = 1000;
    }
    
    /* Test all constructs */
    test_identifiers_and_blocks(iterations);
    test_vectors_and_ssa(iterations);
    test_aggregate_init();
    
#ifdef _OPENMP
    test_omp_clauses(iterations);
#endif
    
    /* Final computation using all results */
    int final_result = global_volatile;
    
    /* Additional complex expression with many identifiers */
    {
        int temp1 = final_result * 2;
        int temp2 = temp1 / 3;
        int temp3 = temp2 ^ 0xABCD;
        int temp4 = temp3 << 2;
        final_result = temp4;
    }
    
    return final_result % 256;
}
