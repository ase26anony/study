/* test_tree_coverage.c - Comprehensive test to trigger specific tree node classifications */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization */
static volatile int volatile_counter = 0;
#define NOOPT asm volatile("" : : : "memory")

/* ========== IDENTIFIER_NODE generation ========== */
/* Generate many unique identifiers using macros */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(identifier_, n)

void test_identifiers_and_blocks(void) {
    /* Multiple unique identifiers */
    int MAKE_ID(0) = 1, MAKE_ID(1) = 2, MAKE_ID(2) = 3;
    int MAKE_ID(3) = 4, MAKE_ID(4) = 5, MAKE_ID(5) = 6;
    int MAKE_ID(6) = 7, MAKE_ID(7) = 8, MAKE_ID(8) = 9;
    
    /* Nested blocks create BLOCK nodes */
    {
        int block_local_1 = MAKE_ID(0) + MAKE_ID(1);
        NOOPT;
        {
            int block_local_2 = block_local_1 * 2;
            volatile_counter += block_local_2;
            {
                int block_local_3 = block_local_2 / 3;
                NOOPT;
                volatile_counter -= block_local_3;
            }
        }
    }
    
    /* More identifiers in different scopes */
    for (int MAKE_ID(loop_i) = 0; MAKE_ID(loop_i) < 5; MAKE_ID(loop_i)++) {
        int MAKE_ID(loop_var) = MAKE_ID(loop_i) * 2;
        NOOPT;
        volatile_counter += MAKE_ID(loop_var);
    }
}

/* ========== TREE_VEC generation ========== */
/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

v4si test_vectors(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    
    /* Vector operations generate TREE_VEC nodes */
    v4si result = a + b * c - a / (b + 1);
    NOOPT;
    
    return result;
}

/* ========== SSA_NAME generation ========== */
int test_ssa_name(int n) {
    int x = 0, y = 0, z = 0;
    
    /* Complex loop with multiple assignments to create SSA form */
    for (int i = 0; i < n; i++) {
        NOOPT;
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
        
        /* Phi-node candidate */
        z += x + y;
        
        /* Another branch to create more SSA complexity */
        if (i % 5 == 0) {
            int temp = z;
            z = temp * 2;
        }
    }
    
    NOOPT;
    return z;
}

/* ========== CONSTRUCTOR generation ========== */
struct ComplexStruct {
    int a, b, c;
    float f;
    double d;
};

struct NestedStruct {
    struct ComplexStruct inner;
    int extra;
};

int test_constructors(void) {
    /* Non-constant initializers create CONSTRUCTOR nodes */
    int base = volatile_counter;
    
    /* Array with non-constant initializer */
    int arr[4] = { base + 1, base + 2, base + 3, base + 4 };
    
    /* Struct with designated initializers */
    struct ComplexStruct s1 = { 
        .a = arr[0] * 2,
        .b = arr[1] + 3,
        .c = arr[2] - 1,
        .f = (float)arr[3] / 2.0f,
        .d = (double)base * 1.5
    };
    
    /* Nested struct initialization */
    struct NestedStruct ns = {
        .inner = { 
            .a = s1.b,
            .b = s1.c,
            .c = s1.a,
            .f = s1.f * 2.0f,
            .d = s1.d / 2.0
        },
        .extra = 42
    };
    
    NOOPT;
    return s1.a + ns.inner.b + arr[0];
}

/* ========== OMP_CLAUSE generation ========== */
#ifdef _OPENMP
#include <omp.h>

int test_omp_clauses(int size) {
    int sum = 0;
    int* array = (int*)malloc(size * sizeof(int));
    
    if (!array) return 0;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    /* OpenMP with multiple clauses to generate OMP_CLAUSE nodes */
    #pragma omp parallel for private(size) shared(array) reduction(+:sum) \
            schedule(dynamic, 4) if(size > 100) num_threads(4)
    for (int i = 0; i < size; i++) {
        sum += array[i];
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel sections private(sum) firstprivate(size) \
            nowait
    {
        #pragma omp section
        {
            int local_sum = 0;
            for (int i = 0; i < size/2; i++) {
                local_sum += array[i];
            }
            NOOPT;
        }
        
        #pragma omp section
        {
            int local_sum = 0;
            for (int i = size/2; i < size; i++) {
                local_sum += array[i];
            }
            NOOPT;
        }
    }
    
    free(array);
    return sum;
}
#else
int test_omp_clauses(int size) {
    /* Fallback without OpenMP */
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += i + 1;
    }
    return sum;
}
#endif

/* ========== TREE_BINFO generation (requires C++ or LTO) ========== */
/* For C compilation, we rely on -flto to potentially generate BINFO nodes */
/* This function creates complex call graph for LTO analysis */
static int recursive_func(int n, int depth) {
    NOOPT;
    if (depth <= 0) return n;
    
    int a = recursive_func(n * 2, depth - 1);
    int b = recursive_func(n + 1, depth - 1);
    
    NOOPT;
    return a + b;
}

/* ========== Main orchestrator ========== */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Trigger IDENTIFIER_NODE and BLOCK cases */
    test_identifiers_and_blocks();
    result += volatile_counter;
    
    /* Trigger TREE_VEC case */
    v4si vec_result = test_vectors();
    for (int i = 0; i < 4; i++) {
        result += vec_result[i];
    }
    
    /* Trigger SSA_NAME case */
    result += test_ssa_name(50);
    
    /* Trigger CONSTRUCTOR case */
    result += test_constructors();
    
    /* Trigger OMP_CLAUSE case */
    result += test_omp_clauses(200);
    
    /* Create complex call graph for potential TREE_BINFO with LTO */
    result += recursive_func(1, 3);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional volatile operations */
    NOOPT;
    volatile_counter = result % 100;
    
    return 0;
}
