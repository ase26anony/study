/* test_tree_coverage.c - Comprehensive test to trigger specific tree node classifications */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization from removing our test constructs */
#define NOOPT __attribute__((optimize("O0")))
#define VOLATILE_VAR volatile int

/* Helper to prevent dead code elimination */
static void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* ========== IDENTIFIER_NODE and BLOCK coverage ========== */
NOOPT void test_identifiers_and_blocks(int iter) {
    /* Generate many unique identifiers */
    int identifier_node_test_001 = 1;
    int identifier_node_test_002 = 2;
    int identifier_node_test_003 = 3;
    int identifier_node_test_004 = 4;
    int identifier_node_test_005 = 5;
    int identifier_node_test_006 = 6;
    int identifier_node_test_007 = 7;
    int identifier_node_test_008 = 8;
    int identifier_node_test_009 = 9;
    int identifier_node_test_010 = 10;
    
    /* Nested blocks create BLOCK nodes */
    {
        VOLATILE_VAR block_inner_1 = identifier_node_test_001;
        {
            VOLATILE_VAR block_inner_2 = identifier_node_test_002;
            {
                VOLATILE_VAR block_inner_3 = identifier_node_test_003;
                use(&block_inner_3);
            }
            use(&block_inner_2);
        }
        use(&block_inner_1);
    }
    
    /* More blocks in loops */
    for (int i = 0; i < iter; i++) {
        VOLATILE_VAR loop_block_var = i;
        {
            VOLATILE_VAR inner_loop_block = loop_block_var + 1;
            use(&inner_loop_block);
        }
    }
    
    /* Conditional blocks */
    if (iter > 0) {
        VOLATILE_VAR if_block_var = 100;
        use(&if_block_var);
    } else {
        VOLATILE_VAR else_block_var = 200;
        use(&else_block_var);
    }
}

/* ========== TREE_VEC coverage ========== */
NOOPT void test_vector_operations(void) {
    /* Vector types create TREE_VEC nodes */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = {0};
    
    /* Various vector operations */
    vec_c = vec_a + vec_b;
    vec_c = vec_a * vec_b;
    vec_c = vec_a & vec_b;
    vec_c = vec_a | vec_b;
    
    v4sf vec_f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_f2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf vec_f3 = vec_f1 * vec_f2;
    
    /* Use results to prevent optimization */
    VOLATILE_VAR vec_store[4];
    for (int i = 0; i < 4; i++) {
        vec_store[i] = vec_c[i];
    }
    use(vec_store);
    use(&vec_f3);
}

/* ========== SSA_NAME coverage ========== */
NOOPT int test_ssa_formation(int n) {
    /* Complex control flow to generate SSA_NAME nodes */
    int x = 0;
    int y = 1;
    int z = 2;
    
    /* Loop with multiple assignments to same variable */
    for (int i = 0; i < n; i++) {
        VOLATILE_VAR condition = i & 1;
        if (condition) {
            x = y + z;
        } else {
            x = y - z;
        }
        
        /* Another SSA variable with phi node potential */
        int temp;
        if (i % 3 == 0) {
            temp = x * 2;
        } else if (i % 3 == 1) {
            temp = x / 2;
        } else {
            temp = x + 5;
        }
        
        y = temp + i;
        z = y - x;
    }
    
    /* More complex SSA with nested conditionals */
    int result = 0;
    for (int i = 0; i < n; i++) {
        int val;
        if (i < n/2) {
            if (i % 2 == 0) {
                val = x;
            } else {
                val = y;
            }
        } else {
            val = z;
        }
        result += val;
    }
    
    return result;
}

/* ========== CONSTRUCTOR coverage ========== */
NOOPT void test_aggregate_constructors(int a, int b, int c) {
    /* Non-constant struct initializer creates CONSTRUCTOR nodes */
    struct Point {
        int x;
        int y;
        int z;
    };
    
    struct Line {
        struct Point start;
        struct Point end;
    };
    
    /* Designated initializers with function calls/non-constants */
    struct Point p1 = { .x = a, .y = b, .z = c };
    struct Point p2 = { a + b, b + c, c + a };
    
    /* Array with non-constant initializer */
    int arr[5] = { a, b, c, a + b, b + c };
    
    /* Nested struct initializer */
    struct Line line1 = { 
        .start = { a, b, c },
        .end = { c, b, a }
    };
    
    /* Complex constructor with mixed expressions */
    struct {
        int first;
        int second[3];
        struct Point third;
    } complex = {
        .first = a * b,
        .second = { a, b, a + b },
        .third = { .x = c, .y = a + c, .z = b + c }
    };
    
    use(&p1);
    use(&p2);
    use(arr);
    use(&line1);
    use(&complex);
}

/* ========== OMP_CLAUSE coverage ========== */
#ifdef _OPENMP
#include <omp.h>

NOOPT void test_omp_clauses(int size) {
    int i;
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP with multiple clauses to generate OMP_CLAUSE nodes */
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) schedule(dynamic, 4) if(size > 100)
    for (i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP construct with different clauses */
    int max_val = 0;
    #pragma omp parallel private(i) shared(arr, max_val)
    {
        #pragma omp for reduction(max:max_val) nowait
        for (i = 0; i < 100; i++) {
            if (arr[i] > max_val) {
                max_val = arr[i];
            }
        }
        
        #pragma omp barrier
        
        #pragma omp single copyprivate(max_val)
        {
            max_val *= 2;
        }
    }
    
    /* OpenMP sections with various clauses */
    #pragma omp parallel sections private(i) num_threads(2)
    {
        #pragma omp section
        {
            for (i = 0; i < 50; i++) {
                arr[i] += sum;
            }
        }
        
        #pragma omp section
        {
            for (i = 50; i < 100; i++) {
                arr[i] += max_val;
            }
        }
    }
    
    use(&sum);
    use(&max_val);
    use(arr);
}
#endif

/* ========== TREE_BINFO coverage attempt ========== */
/* Even in C, we can try to trigger BINFO nodes with certain optimizations */
NOOPT void test_binfo_like_structures(void) {
    /* Complex structure that might generate BINFO-like nodes during LTO */
    struct Base {
        int type_id;
        void (*print)(struct Base*);
    };
    
    struct Derived {
        struct Base base;
        int value;
    };
    
    /* Use function pointers to create vtable-like structures */
    void print_base(struct Base* b) {
        printf("Base: %d\n", b->type_id);
    }
    
    void print_derived(struct Base* b) {
        struct Derived* d = (struct Derived*)b;
        printf("Derived: %d, %d\n", d->base.type_id, d->value);
    }
    
    struct Base b = {1, print_base};
    struct Derived d = {{2, print_derived}, 42};
    
    /* Array of base pointers (potential for BINFO in LTO) */
    struct Base* objects[2] = {&b, (struct Base*)&d};
    
    for (int i = 0; i < 2; i++) {
        objects[i]->print(objects[i]);
    }
}

/* ========== Main function ========== */
int main(int argc, char** argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
    }
    
    /* Test all tree node types */
    test_identifiers_and_blocks(iterations);
    
    test_vector_operations();
    
    int ssa_result = test_ssa_formation(iterations);
    
    test_aggregate_constructors(ssa_result, iterations, argc);
    
    test_binfo_like_structures();
    
#ifdef _OPENMP
    test_omp_clauses(iterations);
#endif
    
    /* Final result to prevent optimization of entire program */
    printf("Test completed with iterations=%d, ssa_result=%d\n", 
           iterations, ssa_result);
    
    return 0;
}
