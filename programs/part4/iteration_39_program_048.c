/* test_tree_coverage.c - Comprehensive test to trigger specific tree node classifications */

/* Prevent inlining to ensure tree nodes are fully built */
#define NOINLINE __attribute__((noinline))

/* Generate many unique identifiers */
#define CONCAT(a, b) a##b
#define UNIQUE_ID(base) CONCAT(base, __LINE__)

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

/* Another struct for more complex initialization */
struct NestedStruct {
    struct ComplexStruct inner;
    int arr[3];
    long extra;
};

/* Global variables to prevent optimization */
volatile int global_trigger = 0;
volatile int prevent_opt = 0;

/* Function declarations */
NOINLINE void test_identifiers_and_blocks(void);
NOINLINE void test_vectors_and_ssa(void);
NOINLINE void test_aggregate_init(void);
NOINLINE void test_omp_clauses(void);
NOINLINE int use_result(int val);

/* Helper to create side effects */
NOINLINE void memory_barrier(void) {
    asm volatile("" : : : "memory");
}

/* ========== Test 1: IDENTIFIER_NODE and BLOCK ========== */
NOINLINE void test_identifiers_and_blocks(void) {
    /* Generate many unique identifiers */
    int UNIQUE_ID(var_) = 1;
    int UNIQUE_ID(var_) = 2;
    int UNIQUE_ID(var_) = 3;
    int UNIQUE_ID(var_) = 4;
    int UNIQUE_ID(var_) = 5;
    int UNIQUE_ID(var_) = 6;
    int UNIQUE_ID(var_) = 7;
    int UNIQUE_ID(var_) = 8;
    int UNIQUE_ID(var_) = 9;
    int UNIQUE_ID(var_) = 10;
    
    /* Nested blocks creating BLOCK nodes */
    {
        int block_local_1 = 100;
        memory_barrier();
        
        {
            int block_local_2 = 200;
            memory_barrier();
            
            {
                int block_local_3 = 300;
                /* Use all identifiers to prevent optimization */
                int sum = block_local_1 + block_local_2 + block_local_3;
                for (int i = 0; i < 10; i++) {
                    sum += UNIQUE_ID(var_);
                }
                prevent_opt = sum;
            }
        }
    }
    
    /* More blocks in control flow */
    for (int i = 0; i < 5; i++) {
        int loop_block_var = i * 2;
        if (loop_block_var > 3) {
            int if_block_var = loop_block_var * 3;
            prevent_opt += if_block_var;
        } else {
            int else_block_var = loop_block_var + 10;
            prevent_opt += else_block_var;
        }
    }
}

/* ========== Test 2: TREE_VEC and SSA_NAME ========== */
NOINLINE void test_vectors_and_ssa(void) {
    /* Vector operations creating TREE_VEC nodes */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {9, 10, 11, 12};
    
    /* Multiple vector operations */
    v4si vec_result = vec_a + vec_b;
    vec_result = vec_result * vec_c;
    vec_result = vec_result - vec_a;
    
    /* Another vector type */
    v4sf float_vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf float_vec_b = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf float_result = float_vec_a * float_vec_b;
    
    /* Complex loop with SSA variables */
    int ssa_var = 0;
    int ssa_temp = 0;
    
    for (int i = 0; i < 100; i++) {
        /* This creates phi nodes in SSA form */
        if (i % 3 == 0) {
            ssa_var = i * 2;
        } else if (i % 3 == 1) {
            ssa_var = i + 10;
        } else {
            ssa_var = i - 5;
        }
        
        /* Another SSA variable with multiple definitions */
        if (ssa_var > 50) {
            ssa_temp = ssa_var / 2;
        } else {
            ssa_temp = ssa_var * 3;
        }
        
        /* Use results to prevent optimization */
        prevent_opt += ssa_temp;
        
        /* Memory barrier to prevent reordering */
        if (i % 20 == 0) {
            memory_barrier();
        }
    }
    
    /* Use vector results */
    int* vec_ptr = (int*)&vec_result;
    for (int i = 0; i < 4; i++) {
        prevent_opt += vec_ptr[i];
    }
}

/* ========== Test 3: CONSTRUCTOR ========== */
NOINLINE void test_aggregate_init(void) {
    /* Function to create side effects */
    NOINLINE int get_value(int seed) {
        memory_barrier();
        return seed * 3 + 1;
    }
    
    /* Aggregate initialization with non-constant expressions */
    struct ComplexStruct cs = {
        .a = get_value(1),
        .b = get_value(2) * 1.5f,
        .c = get_value(3) * 2.5,
        .d = get_value(4) % 128
    };
    
    /* Array initialization with function calls */
    int dynamic_array[4] = {
        get_value(10),
        get_value(11),
        get_value(12),
        get_value(13)
    };
    
    /* Nested struct with array initialization */
    struct NestedStruct ns = {
        .inner = {
            .a = get_value(20),
            .b = get_value(21) * 0.5f,
            .c = get_value(22) * 1.5,
            .d = get_value(23) % 64
        },
        .arr = {
            get_value(30),
            get_value(31),
            get_value(32)
        },
        .extra = get_value(40)
    };
    
    /* Use all initialized values */
    prevent_opt += cs.a + (int)cs.b + (int)cs.c + cs.d;
    for (int i = 0; i < 4; i++) {
        prevent_opt += dynamic_array[i];
    }
    prevent_opt += ns.inner.a + ns.arr[0] + (int)ns.extra;
}

/* ========== Test 4: OMP_CLAUSE ========== */
#ifdef _OPENMP
NOINLINE void test_omp_clauses(void) {
    int i;
    int n = 1000;
    int sum = 0;
    int private_var = 0;
    int shared_arr[1000];
    
    /* Initialize array */
    for (i = 0; i < n; i++) {
        shared_arr[i] = i;
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel for private(i) private(private_var) \
        shared(shared_arr) reduction(+:sum) schedule(dynamic, 10) \
        num_threads(4) if(n > 100)
    for (i = 0; i < n; i++) {
        private_var = i * 2;
        sum += shared_arr[i] + private_var;
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel sections firstprivate(sum) \
        copyin(private_var) nowait
    {
        #pragma omp section
        {
            int section_local = sum / 2;
            prevent_opt += section_local;
        }
        
        #pragma omp section
        {
            int section_local = sum * 2;
            prevent_opt += section_local;
        }
    }
    
    /* Task construct with depend clauses */
    int task_result1 = 0, task_result2 = 0;
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(out: task_result1)
        {
            for (int j = 0; j < 100; j++) {
                task_result1 += j;
            }
        }
        
        #pragma omp task depend(in: task_result1) depend(out: task_result2)
        {
            task_result2 = task_result1 * 2;
        }
        
        #pragma omp task depend(in: task_result2)
        {
            prevent_opt += task_result2;
        }
    }
    
    prevent_opt += sum;
}
#else
NOINLINE void test_omp_clauses(void) {
    /* Dummy implementation when OpenMP is not available */
    prevent_opt += 999;
}
#endif

/* ========== Test 5: TREE_BINFO (requires C++ or LTO) ========== */
/* This would normally be in a separate C++ file, but we'll create
   a C-compatible version that might generate BINFO with LTO */
NOINLINE void test_binfo_like(void) {
    /* Complex struct with function pointers to create type hierarchy-like structures */
    struct VirtualTable {
        void (*func1)(void);
        int (*func2)(int);
    };
    
    struct BaseType {
        struct VirtualTable* vtable;
        int base_data;
    };
    
    struct DerivedType {
        struct BaseType base;
        int derived_data;
    };
    
    /* Force creation of complex type information */
    struct DerivedType dt;
    dt.base.vtable = 0;
    dt.base.base_data = 42;
    dt.derived_data = 84;
    
    prevent_opt += dt.base.base_data + dt.derived_data;
}

/* ========== Main function ========== */
NOINLINE int use_result(int val) {
    /* Complex computation to ensure all values are used */
    int result = val;
    for (int i = 0; i < 10; i++) {
        result = (result * 1103515245 + 12345) & 0x7fffffff;
        if (result % 3 == 0) {
            result /= 3;
        } else {
            result = result * 2 + 1;
        }
    }
    return result;
}

int main(void) {
    int final_result = 0;
    
    /* Run all tests */
    test_identifiers_and_blocks();
    final_result += use_result(prevent_opt);
    
    test_vectors_and_ssa();
    final_result += use_result(prevent_opt);
    
    test_aggregate_init();
    final_result += use_result(prevent_opt);
    
    test_omp_clauses();
    final_result += use_result(prevent_opt);
    
    test_binfo_like();
    final_result += use_result(prevent_opt);
    
    /* Print result to ensure execution */
    printf("Final checksum: %d\n", final_result);
    
    return final_result % 256;
}
