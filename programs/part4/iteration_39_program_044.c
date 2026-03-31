/* test_tree_coverage.c - Comprehensive test for GCC tree node classification */

/* Prevent inlining to ensure tree nodes are preserved */
#define NOINLINE __attribute__((noinline))

/* Generate many unique identifiers */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(identifier_, n)

/* Vector type for TREE_VEC */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Struct for CONSTRUCTOR */
struct ComplexStruct {
    int a;
    float b;
    double c;
    char d;
};

struct NestedStruct {
    struct ComplexStruct inner;
    int extra;
};

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile float global_float = 1.0f;

/* Function declarations */
NOINLINE void test_identifiers_and_blocks(void);
NOINLINE void test_vectors_and_ssa(void);
NOINLINE void test_aggregate_init(void);
NOINLINE void test_omp_clauses(void);
NOINLINE int external_func(int x);

/* Many unique identifiers for IDENTIFIER_NODE */
void test_identifiers_and_blocks(void) {
    /* Level 1 block */
    {
        int MAKE_ID(1) = 1;
        float MAKE_ID(2) = 2.0f;
        double MAKE_ID(3) = 3.0;
        
        /* Level 2 nested block */
        {
            char MAKE_ID(4) = 'A';
            short MAKE_ID(5) = 5;
            
            /* Level 3 deeply nested block */
            {
                long MAKE_ID(6) = 6L;
                long long MAKE_ID(7) = 7LL;
                
                /* Use all identifiers to prevent removal */
                asm volatile("" : : "r"(MAKE_ID(1)), "r"(MAKE_ID(2)), 
                             "r"(MAKE_ID(3)), "r"(MAKE_ID(4)), 
                             "r"(MAKE_ID(5)), "r"(MAKE_ID(6)), 
                             "r"(MAKE_ID(7)) : "memory");
            }
        }
    }
    
    /* Another block with different identifiers */
    {
        unsigned int MAKE_ID(8) = 8;
        unsigned char MAKE_ID(9) = 9;
        
        /* Block inside if statement */
        if (global_counter) {
            int MAKE_ID(10) = 10;
            global_counter += MAKE_ID(10);
        }
    }
    
    /* Block in loop */
    for (int i = 0; i < 3; i++) {
        int MAKE_ID(11) = i * 2;
        {
            int MAKE_ID(12) = MAKE_ID(11) + 1;
            global_counter += MAKE_ID(12);
        }
    }
}

/* Test TREE_VEC and SSA_NAME */
NOINLINE void test_vectors_and_ssa(void) {
    /* Vector operations for TREE_VEC */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = {9, 10, 11, 12};
    
    /* Multiple vector operations */
    v4si result1 = vec1 + vec2;
    v4si result2 = vec1 * vec3;
    v4si result3 = result1 - result2;
    
    /* Float vectors */
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fresult = fvec1 * fvec2 + fvec1;
    
    /* Complex SSA pattern with loop */
    int ssa_var1 = 0;
    int ssa_var2 = 0;
    int ssa_var3 = 0;
    
    /* Loop creates phi nodes (SSA_NAME) */
    for (int i = 0; i < 100; i++) {
        /* Conditional creates merge point */
        if (i & 1) {
            ssa_var1 = i * 2;
            ssa_var2 = ssa_var1 + 1;
        } else {
            ssa_var1 = i * 3;
            ssa_var2 = ssa_var1 - 1;
        }
        
        /* Another SSA variable with different flow */
        ssa_var3 = ssa_var2 + (i % 3);
        
        /* Use volatile to prevent optimization */
        asm volatile("" : : "r"(ssa_var1), "r"(ssa_var2), "r"(ssa_var3) : "memory");
    }
    
    /* Use vectors to prevent removal */
    asm volatile("" : : "m"(result3), "m"(fresult) : "memory");
}

/* Test CONSTRUCTOR nodes */
NOINLINE void test_aggregate_init(void) {
    /* Array constructor with non-constant initializers */
    int dynamic_array[4] = {
        external_func(1),
        global_counter + 1,
        external_func(2) * 2,
        4
    };
    
    /* Struct constructor with designated initializers */
    struct ComplexStruct cs = {
        .a = external_func(3),
        .b = global_float * 2.0f,
        .c = 3.14159,
        .d = 'X'
    };
    
    /* Nested struct constructor */
    struct NestedStruct ns = {
        .inner = {
            .a = external_func(4),
            .b = 2.5f,
            .c = cs.c + 1.0,
            .d = 'Y'
        },
        .extra = 100
    };
    
    /* Array of structs constructor */
    struct ComplexStruct cs_array[2] = {
        { external_func(5), 1.1f, 2.2, 'A' },
        { external_func(6), 3.3f, 4.4, 'B' }
    };
    
    /* Use all constructors */
    asm volatile("" : : "m"(dynamic_array), "m"(cs), "m"(ns), "m"(cs_array) : "memory");
}

/* Test OMP_CLAUSE nodes */
#ifdef _OPENMP
NOINLINE void test_omp_clauses(void) {
    int i;
    int sum = 0;
    int arr[100];
    int private_var = 0;
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) \
        schedule(dynamic) num_threads(4) if(global_counter > 0)
    for (i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel private(private_var) firstprivate(sum) \
        copyin(global_counter)
    {
        private_var = omp_get_thread_num();
        #pragma omp critical
        {
            global_counter += private_var;
        }
    }
    
    /* OpenMP sections with clause */
    #pragma omp parallel sections private(i) lastprivate(private_var)
    {
        #pragma omp section
        {
            for (i = 0; i < 50; i++) {
                private_var = i;
            }
        }
        #pragma omp section
        {
            for (i = 50; i < 100; i++) {
                private_var = i * 2;
            }
        }
    }
    
    /* Use results */
    asm volatile("" : : "r"(sum), "r"(private_var) : "memory");
}
#else
NOINLINE void test_omp_clauses(void) {
    /* Dummy implementation when OpenMP not available */
    global_counter += 1;
}
#endif

/* External function to prevent constant folding */
NOINLINE int external_func(int x) {
    return x * 2 + global_counter;
}

/* Main function that orchestrates everything */
int main(void) {
    int result = 0;
    
    /* Call all test functions */
    test_identifiers_and_blocks();
    test_vectors_and_ssa();
    test_aggregate_init();
    test_omp_clauses();
    
    /* Create more SSA complexity in main */
    int main_ssa = 0;
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            main_ssa = external_func(i);
        } else {
            main_ssa = external_func(i * 3);
        }
        result += main_ssa;
    }
    
    /* Final volatile store to ensure all code executes */
    asm volatile("" : : "r"(result) : "memory");
    
    return result > 0 ? 0 : 1;
}
