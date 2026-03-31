/* test_tree_coverage.c - Comprehensive test to trigger specific tree node classifications */

/* Prevent optimizations from removing our test constructs */
#define NO_INLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Generate many unique identifiers for IDENTIFIER_NODE coverage */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(identifier_, n)

/* Vector types for TREE_VEC coverage */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

/* Struct for CONSTRUCTOR coverage */
struct ComplexStruct {
    int a, b, c;
    float f;
    double d;
};

struct NestedStruct {
    struct ComplexStruct inner;
    int extra;
};

/* Function declarations to prevent inlining */
NO_INLINE void test_identifiers_and_blocks(void);
NO_INLINE void test_vectors_and_ssa(int n);
NO_INLINE void test_aggregate_init(int x);
NO_INLINE int test_omp_clauses(int *arr, int n);
NO_INLINE void memory_barrier(void);

/* Many distinct identifiers for IDENTIFIER_NODE coverage */
volatile int MAKE_ID(0), MAKE_ID(1), MAKE_ID(2), MAKE_ID(3), MAKE_ID(4);
volatile int MAKE_ID(5), MAKE_ID(6), MAKE_ID(7), MAKE_ID(8), MAKE_ID(9);
volatile int MAKE_ID(10), MAKE_ID(11), MAKE_ID(12), MAKE_ID(13), MAKE_ID(14);

/* Memory barrier to prevent optimization */
void memory_barrier(void) {
    asm volatile("" : : : "memory");
}

/* Test function for IDENTIFIER_NODE and BLOCK coverage */
NO_INLINE void test_identifiers_and_blocks(void) {
    /* Multiple nested blocks creating BLOCK nodes */
    {
        int block_local_1 = 1;
        volatile int block_volatile_1 = block_local_1;
        
        {
            int block_local_2 = 2;
            volatile int block_volatile_2 = block_local_2;
            
            {
                int block_local_3 = 3;
                volatile int block_volatile_3 = block_local_3;
                
                /* Use all identifiers */
                MAKE_ID(0) = block_local_1;
                MAKE_ID(1) = block_local_2;
                MAKE_ID(2) = block_local_3;
            }
        }
    }
    
    /* More blocks in loops */
    for (int loop_id_1 = 0; loop_id_1 < 3; loop_id_1++) {
        int loop_block_var = loop_id_1 * 2;
        
        if (loop_block_var > 0) {
            int if_block_var = loop_block_var + 1;
            MAKE_ID(3) = if_block_var;
        } else {
            int else_block_var = loop_block_var - 1;
            MAKE_ID(4) = else_block_var;
        }
    }
    
    memory_barrier();
}

/* Test function for TREE_VEC and SSA_NAME coverage */
NO_INLINE void test_vectors_and_ssa(int n) {
    /* Vector operations for TREE_VEC */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {0};
    
    /* Multiple vector operations */
    vec_c = vec_a + vec_b;
    vec_c = vec_c * vec_a;
    vec_c = vec_c - vec_b;
    
    volatile v4si vec_result = vec_c;
    
    /* Complex loop with SSA variables for SSA_NAME coverage */
    int ssa_var_1 = 0;
    int ssa_var_2 = 1;
    int ssa_var_3 = 0;
    
    for (int i = 0; i < n; i++) {
        /* This creates phi nodes in SSA form */
        if (i % 3 == 0) {
            ssa_var_1 = ssa_var_2 + i;
        } else if (i % 3 == 1) {
            ssa_var_1 = ssa_var_3 * i;
        } else {
            ssa_var_1 = ssa_var_1 - i;
        }
        
        /* More SSA complexity */
        ssa_var_2 = ssa_var_1 + ssa_var_2;
        ssa_var_3 = ssa_var_2 - ssa_var_3;
        
        /* Use vector in loop to prevent removal */
        if (i % 10 == 0) {
            vec_c = vec_c + vec_a;
        }
    }
    
    MAKE_ID(5) = ssa_var_1;
    MAKE_ID(6) = ssa_var_2;
    MAKE_ID(7) = ssa_var_3;
    
    memory_barrier();
}

/* Test function for CONSTRUCTOR coverage */
NO_INLINE void test_aggregate_init(int x) {
    /* Non-constant initializers for CONSTRUCTOR nodes */
    struct ComplexStruct cs = {
        .a = x + 1,
        .b = x * 2,
        .c = x - 3,
        .f = (float)x / 2.0f,
        .d = (double)x * 1.5
    };
    
    /* Array with non-constant initializers */
    int dynamic_array[4] = {
        x,
        x + 1,
        x * 2,
        x - 5
    };
    
    /* Nested struct initialization */
    struct NestedStruct ns = {
        .inner = {
            .a = x,
            .b = x + 2,
            .c = x * 3,
            .f = (float)x + 1.5f,
            .d = (double)x / 3.0
        },
        .extra = x * 4
    };
    
    /* Designated initializers with expressions */
    int sparse_array[10] = {
        [0] = x,
        [3] = x + 3,
        [7] = x * 7,
        [9] = x - 9
    };
    
    volatile struct ComplexStruct v_cs = cs;
    volatile struct NestedStruct v_ns = ns;
    
    MAKE_ID(8) = dynamic_array[0];
    MAKE_ID(9) = sparse_array[3];
    
    memory_barrier();
}

/* Test function for OMP_CLAUSE coverage */
NO_INLINE int test_omp_clauses(int *arr, int n) {
    int sum = 0;
    int private_var = 0;
    int firstprivate_var = 42;
    int lastprivate_var = 0;
    int reduction_sum = 0;
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel \
        private(private_var) \
        firstprivate(firstprivate_var) \
        shared(arr) \
        reduction(+:reduction_sum) \
        default(none)
    {
        private_var = omp_get_thread_num();
        
        #pragma omp for \
            schedule(dynamic, 4) \
            nowait \
            lastprivate(lastprivate_var) \
            ordered
        for (int i = 0; i < n; i++) {
            private_var = i;
            arr[i] = arr[i] * 2 + private_var;
            reduction_sum += arr[i];
            
            #pragma omp ordered
            {
                if (i == n - 1) {
                    lastprivate_var = arr[i];
                }
            }
        }
        
        #pragma omp barrier
        
        #pragma omp single copyprivate(private_var)
        {
            private_var = 999;
        }
        
        #pragma omp atomic
        sum += reduction_sum;
    }
    
    /* More OpenMP constructs */
    int task_var = 0;
    
    #pragma omp parallel
    {
        #pragma omp task \
            depend(inout: task_var) \
            priority(1) \
            untied \
            mergeable
        {
            task_var = 1;
        }
        
        #pragma omp taskwait
    }
    
    MAKE_ID(10) = sum;
    MAKE_ID(11) = lastprivate_var;
    MAKE_ID(12) = task_var;
    
    memory_barrier();
    return sum;
}

/* Main function orchestrating all tests */
int main(int argc, char **argv) {
    int test_size = 100;
    if (argc > 1) {
        test_size = atoi(argv[1]);
        if (test_size <= 0) test_size = 100;
    }
    
    /* Test array for OpenMP */
    int *test_array = (int*)malloc(test_size * sizeof(int));
    for (int i = 0; i < test_size; i++) {
        test_array[i] = i;
    }
    
    /* Run all test functions */
    test_identifiers_and_blocks();
    test_vectors_and_ssa(test_size);
    test_aggregate_init(test_size);
    int omp_result = test_omp_clauses(test_array, test_size);
    
    /* Use results to prevent dead code elimination */
    volatile int final_result = 0;
    final_result += MAKE_ID(0);
    final_result += MAKE_ID(5);
    final_result += MAKE_ID(8);
    final_result += MAKE_ID(10);
    final_result += omp_result;
    
    printf("Test completed with result: %d\n", final_result);
    
    free(test_array);
    return final_result != 0 ? 0 : 1;
}
