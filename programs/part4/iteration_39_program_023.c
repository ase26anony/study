/* test_tree_coverage.c - Comprehensive test for GCC tree node coverage */

/* Prevent inlining to ensure tree nodes are preserved */
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

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int prevent_opt = 0;

/* Function declarations */
NOINLINE void test_identifiers_and_blocks(void);
NOINLINE void test_vectors_and_ssa(void);
NOINLINE void test_aggregate_init(void);
NOINLINE void test_omp_clauses(void);
NOINLINE int use_result(int val);

/* Generate many identifiers */
void test_identifiers_and_blocks(void) {
    /* Multiple identifiers at different scopes */
    {
        int MAKE_ID(0) = 1;
        int MAKE_ID(1) = MAKE_ID(0) + 1;
        
        /* Nested block with its own identifiers */
        {
            int MAKE_ID(2) = MAKE_ID(1) * 2;
            int MAKE_ID(3) = MAKE_ID(2) / 2;
            
            /* Another nested level */
            if (prevent_opt) {
                int MAKE_ID(4) = MAKE_ID(3) << 1;
                MAKE_ID(4) |= 0xFF;
            }
        }
    }
    
    /* More blocks with different identifiers */
    for (int MAKE_ID(5) = 0; MAKE_ID(5) < 10; MAKE_ID(5)++) {
        int MAKE_ID(6) = MAKE_ID(5) * MAKE_ID(5);
        
        /* Block inside loop */
        {
            int MAKE_ID(7) = MAKE_ID(6) % 7;
            global_counter += MAKE_ID(7);
        }
    }
    
    /* Switch with blocks */
    switch (global_counter & 3) {
        case 0: {
            int MAKE_ID(8) = 100;
            global_counter += MAKE_ID(8);
            break;
        }
        case 1: {
            int MAKE_ID(9) = 200;
            global_counter += MAKE_ID(9);
            break;
        }
        default: {
            int MAKE_ID(10) = 300;
            global_counter += MAKE_ID(10);
            break;
        }
    }
}

/* Test vector operations and SSA form */
void test_vectors_and_ssa(void) {
    /* Vector declarations and operations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3;
    
    /* Multiple vector operations */
    vec3 = vec1 + vec2;
    vec3 = vec3 * vec1;
    vec3 = vec3 - vec2;
    
    /* Complex loop with SSA variables */
    int ssa_var1 = 0;
    int ssa_var2 = 1;
    
    for (int i = 0; i < 100; i++) {
        /* This creates phi nodes in SSA form */
        if (i & 1) {
            ssa_var1 = ssa_var2 + i;
        } else {
            ssa_var1 = ssa_var2 - i;
        }
        
        /* Another SSA variable with multiple assignments */
        int ssa_var3;
        if (i % 3 == 0) {
            ssa_var3 = ssa_var1 * 2;
        } else if (i % 3 == 1) {
            ssa_var3 = ssa_var1 / 2;
        } else {
            ssa_var3 = ssa_var1 + ssa_var1;
        }
        
        ssa_var2 = ssa_var3 + (i % 5);
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
    }
    
    /* Use vector result to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += vec3[i];
    }
    global_counter += sum;
}

/* Test aggregate initializations with non-constant expressions */
void test_aggregate_init(void) {
    /* Array with non-constant initializers */
    int arr[4] = {
        global_counter + 1,
        global_counter * 2,
        global_counter / 2,
        global_counter % 10
    };
    
    /* Struct with designated initializers */
    struct ComplexStruct s1 = {
        .a = global_counter,
        .b = global_counter * 1.5f,
        .c = global_counter * 2.5,
        .d = (global_counter & 0xFF) + 'A'
    };
    
    /* Nested struct initialization */
    struct Nested {
        struct ComplexStruct inner;
        int extra;
    };
    
    struct Nested n1 = {
        .inner = {
            .a = arr[0],
            .b = arr[1] * 1.0f,
            .c = arr[2] * 1.0,
            .d = arr[3] & 0xFF
        },
        .extra = global_counter << 2
    };
    
    /* Array of structs with mixed initializers */
    struct ComplexStruct struct_array[3] = {
        {global_counter, 1.0f, 2.0, 'X'},
        {.a = global_counter + 1, .b = 2.0f, .c = 3.0, .d = 'Y'},
        {global_counter + 2, 3.0f}
    };
    
    /* Use the aggregates to prevent optimization */
    for (int i = 0; i < 4; i++) {
        global_counter += arr[i];
    }
    global_counter += s1.a + (int)s1.b;
    global_counter += n1.extra;
    global_counter += struct_array[0].a;
}

/* Test OpenMP with various clauses */
#ifdef _OPENMP
void test_omp_clauses(void) {
    int i;
    int local_sum = 0;
    int shared_arr[100];
    int reduction_sum = 0;
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        shared_arr[i] = i + global_counter;
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel for private(i) shared(shared_arr) \
        reduction(+:reduction_sum) schedule(dynamic) \
        num_threads(4) if(global_counter > 0)
    for (i = 0; i < 100; i++) {
        /* Each thread has private copy of i */
        int temp = shared_arr[i];
        
        /* Nested block inside parallel region */
        {
            int block_local = temp * 2;
            temp = block_local / 3;
        }
        
        reduction_sum += temp;
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel sections private(local_sum) \
        firstprivate(reduction_sum)
    {
        #pragma omp section
        {
            local_sum = reduction_sum + 1;
            #pragma omp critical
            {
                global_counter += local_sum;
            }
        }
        
        #pragma omp section
        {
            local_sum = reduction_sum - 1;
            #pragma omp critical
            {
                global_counter -= local_sum;
            }
        }
    }
    
    /* OpenMP task with clause */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task default(none) shared(global_counter) \
                depend(out: global_counter)
            {
                global_counter = (global_counter * 1103515245 + 12345) & 0x7fffffff;
            }
        }
    }
}
#else
void test_omp_clauses(void) {
    /* Fallback without OpenMP */
    for (int i = 0; i < 100; i++) {
        global_counter += i;
    }
}
#endif

/* Force usage of results */
int use_result(int val) {
    volatile int sink = val;
    return sink;
}

/* Main function that orchestrates everything */
int main(void) {
    /* Call all test functions multiple times */
    for (int iteration = 0; iteration < 3; iteration++) {
        test_identifiers_and_blocks();
        test_vectors_and_ssa();
        test_aggregate_init();
        test_omp_clauses();
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    /* Final result computation */
    int result = global_counter;
    result = use_result(result);
    
    /* Print result to ensure execution */
    printf("Result: %d\n", result);
    
    return result != 0;
}
