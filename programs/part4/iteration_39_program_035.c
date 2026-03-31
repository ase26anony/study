/* test_tree_coverage.c - Comprehensive test to trigger all tree node classifications */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimizations */
static volatile int volatile_var = 0;

/* Memory barrier to prevent reordering */
#define MEMORY_BARRIER() asm volatile("" : : : "memory")

/* Generate many unique identifiers for IDENTIFIER_NODE */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(identifier_, n)

/* Function declarations to ensure tree nodes are created */
void test_identifiers_and_blocks(void);
void test_vectors_and_ssa(void);
void test_aggregate_init(void);
void test_omp_clauses(void);

/* For TREE_BINFO - using LTO and external linkage structures */
struct base_info {
    int base_data;
    void (*base_func)(void);
};

struct derived_info {
    struct base_info base;
    int derived_data;
};

/* ========== Test 1: IDENTIFIER_NODE and BLOCK ========== */
void test_identifiers_and_blocks(void) {
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
    
    MEMORY_BARRIER();
    
    /* Nested blocks for BLOCK nodes */
    {
        int block_local_1 = MAKE_ID(0) + MAKE_ID(1);
        {
            int block_local_2 = block_local_1 * 2;
            {
                int block_local_3 = block_local_2 / 3;
                volatile_var = block_local_3;
            }
        }
    }
    
    /* More blocks in control flow */
    for (int i = 0; i < 3; i++) {
        int loop_block_var = i * 10;
        if (loop_block_var > 15) {
            int if_block_var = loop_block_var + 5;
            volatile_var = if_block_var;
        } else {
            int else_block_var = loop_block_var - 5;
            volatile_var = else_block_var;
        }
    }
}

/* ========== Test 2: TREE_VEC and SSA_NAME ========== */
void test_vectors_and_ssa(void) {
    /* Vector types for TREE_VEC */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c;
    
    /* Vector operations */
    vec_c = vec_a + vec_b;
    vec_c = vec_c * vec_a;
    
    MEMORY_BARRIER();
    
    /* Complex loop for SSA_NAME generation */
    int ssa_var = 0;
    int result = 0;
    
    for (int i = 0; i < 100; i++) {
        /* This creates phi nodes in SSA form */
        if (i % 3 == 0) {
            ssa_var = i * 2;
        } else if (i % 3 == 1) {
            ssa_var = i + 10;
        } else {
            ssa_var = i - 5;
        }
        
        /* Another SSA variable with multiple assignments */
        int temp;
        if (ssa_var > 50) {
            temp = ssa_var / 2;
        } else {
            temp = ssa_var * 2;
        }
        
        result += temp;
    }
    
    volatile_var = result;
    
    /* More vector operations */
    v4sf vec_f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_f2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf vec_f3 = vec_f1 * vec_f2;
    
    /* Use vector result to prevent optimization */
    float sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += vec_f3[i];
    }
    volatile_var = (int)sum;
}

/* ========== Test 3: CONSTRUCTOR ========== */
void test_aggregate_init(void) {
    /* Helper function to get non-constant values */
    int get_value(int x) {
        return x * 2 + volatile_var;
    }
    
    /* Struct with non-constant initializer - creates CONSTRUCTOR */
    struct Point {
        int x;
        int y;
        int z;
    };
    
    struct Line {
        struct Point start;
        struct Point end;
    };
    
    /* Non-constant struct initializer */
    struct Point p1 = {get_value(1), get_value(2), get_value(3)};
    struct Point p2 = {.x = get_value(4), .y = get_value(5), .z = get_value(6)};
    
    /* Array with non-constant initializer */
    int dynamic_array[5] = {
        get_value(10),
        get_value(11),
        get_value(12),
        get_value(13),
        get_value(14)
    };
    
    /* Nested struct initializer */
    struct Line line1 = {
        {get_value(20), get_value(21), get_value(22)},
        {get_value(23), get_value(24), get_value(25)}
    };
    
    MEMORY_BARRIER();
    
    /* Use the initialized values */
    int sum = p1.x + p2.y + dynamic_array[2] + line1.start.z;
    volatile_var = sum;
}

/* ========== Test 4: OMP_CLAUSE ========== */
void test_omp_clauses(void) {
    int i;
    int n = 1000;
    int arr[1000];
    int sum = 0;
    int partial_sum = 0;
    
    /* Initialize array */
    for (i = 0; i < n; i++) {
        arr[i] = i + 1;
    }
    
    /* OpenMP with multiple clauses - generates OMP_CLAUSE nodes */
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) schedule(dynamic, 10) if(n > 100)
    for (i = 0; i < n; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel private(i) firstprivate(n) copyin(volatile_var)
    {
        #pragma omp for nowait
        for (i = 0; i < n; i++) {
            partial_sum += i;
        }
        
        #pragma omp single copyprivate(partial_sum)
        {
            volatile_var = partial_sum;
        }
    }
    
    /* OpenMP sections with clauses */
    #pragma omp parallel sections private(i) num_threads(2)
    {
        #pragma omp section
        {
            for (i = 0; i < n/2; i++) {
                arr[i] *= 2;
            }
        }
        
        #pragma omp section
        {
            for (i = n/2; i < n; i++) {
                arr[i] /= 2;
            }
        }
    }
    
    volatile_var = sum + partial_sum;
}

/* ========== Test 5: TREE_BINFO via LTO ========== */
/* External functions that will be linked and optimized with LTO */
extern void external_base_func(void);
extern int external_processing(struct base_info *b);

void process_with_binfo(void) {
    /* These structures with function pointers and external linkage
       can create BINFO nodes during LTO optimization */
    static struct derived_info derived = {
        {42, external_base_func},
        100
    };
    
    int result = external_processing(&derived.base);
    volatile_var = result;
}

/* ========== Main function ========== */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Run all tests */
    test_identifiers_and_blocks();
    total += volatile_var;
    
    test_vectors_and_ssa();
    total += volatile_var;
    
    test_aggregate_init();
    total += volatile_var;
    
    /* Only run OpenMP test if supported */
    #ifdef _OPENMP
    test_omp_clauses();
    total += volatile_var;
    #endif
    
    /* Try to trigger BINFO through LTO */
    process_with_binfo();
    total += volatile_var;
    
    /* Print result to ensure code isn't optimized away */
    printf("Total checksum: %d\n", total);
    
    return total > 0 ? 0 : 1;
}

/* External function definitions for LTO */
void external_base_func(void) {
    volatile_var++;
}

int external_processing(struct base_info *b) {
    if (b && b->base_func) {
        b->base_func();
        return b->base_data * 2;
    }
    return 0;
}
