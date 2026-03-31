/* test_tree_coverage.c - Comprehensive test to trigger specific tree node classifications */

/* Prevent inlining to ensure tree nodes are fully built */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Generate many unique identifiers */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(identifier_, n)

/* Vector type for TREE_VEC */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Struct for CONSTRUCTOR */
struct ComplexStruct {
    int a, b, c;
    float f;
    double d;
};

/* Function prototypes */
NOINLINE void test_identifiers_and_blocks(void);
NOINLINE void test_vectors_and_ssa(void);
NOINLINE void test_aggregate_init(void);
NOINLINE void test_omp_clauses(void);
NOINLINE int compute_checksum(void);

/* Global variables to prevent optimization */
volatile int global_trigger = 0;
int checksum = 0;

/* ========== Test IDENTIFIER_NODE and BLOCK ========== */
NOINLINE void test_identifiers_and_blocks(void) {
    /* Generate many unique identifiers */
    int MAKE_ID(0) = 1;
    int MAKE_ID(1) = 2;
    int MAKE_ID(2) = 3;
    int MAKE_ID(3) = 4;
    int MAKE_ID(4) = 5;
    int MAKE_ID(5) = 6;
    int MAKE_ID(6) = 7;
    int MAKE_ID(7) = 8;
    int MAKE_ID(8) = 9;
    int MAKE_ID(9) = 10;
    
    /* Nested blocks creating BLOCK nodes */
    {
        int block_local_1 = MAKE_ID(0) + MAKE_ID(1);
        checksum += block_local_1;
        
        {
            int block_local_2 = block_local_1 * 2;
            checksum += block_local_2;
            
            {
                int block_local_3 = block_local_2 / 2;
                checksum += block_local_3;
                
                /* Memory barrier to prevent optimization */
                asm volatile("" : : : "memory");
            }
        }
    }
    
    /* More blocks in loops */
    for (int i = 0; i < 3; i++) {
        int loop_block_var = i * 10;
        checksum += loop_block_var;
        
        if (loop_block_var > 10) {
            int if_block_var = loop_block_var - 5;
            checksum += if_block_var;
        } else {
            int else_block_var = loop_block_var + 5;
            checksum += else_block_var;
        }
    }
}

/* ========== Test TREE_VEC and SSA_NAME ========== */
NOINLINE void test_vectors_and_ssa(void) {
    /* Vector operations creating TREE_VEC nodes */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {0};
    
    /* Multiple vector operations */
    vec_c = vec_a + vec_b;
    vec_c = vec_c * vec_a;
    vec_c = vec_c - vec_b;
    
    /* Force use of vector results */
    int* p = (int*)&vec_c;
    checksum += p[0] + p[1] + p[2] + p[3];
    
    /* Complex loop creating SSA_NAME nodes */
    int ssa_var = 0;
    int ssa_temp = 0;
    
    for (int i = 0; i < 100; i++) {
        /* This creates phi nodes in SSA form */
        if (i % 3 == 0) {
            ssa_var = i * 2;
        } else if (i % 3 == 1) {
            ssa_var = i * 3;
        } else {
            ssa_var = i * 4;
        }
        
        /* Another SSA variable with multiple definitions */
        if (i % 2 == 0) {
            ssa_temp = ssa_var + 1;
        } else {
            ssa_temp = ssa_var - 1;
        }
        
        checksum += ssa_temp;
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    /* More SSA complexity */
    int x = 0, y = 0, z = 0;
    for (int i = 0; i < 50; i++) {
        x = y + z;
        y = x + i;
        z = y - x;
        checksum += z;
    }
}

/* ========== Test CONSTRUCTOR ========== */
NOINLINE void test_aggregate_init(void) {
    /* Non-constant initializers creating CONSTRUCTOR nodes */
    int r1 = checksum % 100;
    int r2 = (checksum + 1) % 100;
    int r3 = (checksum + 2) % 100;
    
    /* Struct with non-constant initializer */
    struct ComplexStruct s1 = {
        .a = r1,
        .b = r2,
        .c = r3,
        .f = (float)r1 / 10.0f,
        .d = (double)r2 / 20.0
    };
    
    checksum += s1.a + s1.b + s1.c;
    
    /* Array with non-constant initializers */
    int arr[4] = { r1, r2, r3, r1 + r2 };
    for (int i = 0; i < 4; i++) {
        checksum += arr[i];
    }
    
    /* Nested struct initialization */
    struct Nested {
        struct ComplexStruct inner;
        int extra;
    };
    
    struct Nested n1 = {
        .inner = { r2, r3, r1, (float)r3/5.0f, (double)r1/3.0 },
        .extra = r1 + r2 + r3
    };
    
    checksum += n1.extra;
    
    /* Complex array initialization */
    int matrix[2][3] = {
        { r1, r2, r3 },
        { r3, r1, r2 }
    };
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            checksum += matrix[i][j];
        }
    }
}

/* ========== Test OMP_CLAUSE ========== */
#ifdef _OPENMP
NOINLINE void test_omp_clauses(void) {
    int i;
    int local_sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        arr[i] = i + 1;
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel for private(i) shared(arr) reduction(+:local_sum) schedule(dynamic) num_threads(4)
    for (i = 0; i < 100; i++) {
        local_sum += arr[i];
    }
    
    checksum += local_sum;
    
    /* Another OpenMP construct with different clauses */
    int a = 0, b = 0;
    #pragma omp parallel sections private(i) firstprivate(a) lastprivate(b) nowait
    {
        #pragma omp section
        {
            a = 1;
            for (i = 0; i < 50; i++) {
                checksum += i;
            }
            b = a;
        }
        
        #pragma omp section
        {
            a = 2;
            for (i = 50; i < 100; i++) {
                checksum -= i;
            }
            b = a;
        }
    }
    
    /* OpenMP task with clauses */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(out: arr[0]) if(global_trigger > 0)
            {
                arr[0] = checksum % 1000;
            }
        }
    }
}
#else
NOINLINE void test_omp_clauses(void) {
    /* Fallback if OpenMP not available */
    checksum += 12345;
}
#endif

/* ========== Main function ========== */
int main(void) {
    /* Run all tests */
    test_identifiers_and_blocks();
    test_vectors_and_ssa();
    test_aggregate_init();
    test_omp_clauses();
    
    /* Compute final checksum */
    int result = compute_checksum();
    
    /* Print result to prevent dead code elimination */
    printf("Checksum: %d\n", result);
    
    return result % 256;
}

NOINLINE int compute_checksum(void) {
    /* Complex computation to ensure all code is used */
    int final = checksum;
    
    /* More operations to create additional tree nodes */
    for (int i = 0; i < 10; i++) {
        final = (final * 1103515245 + 12345) & 0x7fffffff;
    }
    
    return final;
}
